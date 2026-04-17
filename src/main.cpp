#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <tchar.h>
#include <thread>
#include <atomic>
#include <chrono>
#include <random>
#include <vector>
#include <string>
#include <fstream>
#include <timeapi.h>
#include <shellapi.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "json.hpp"
using json = nlohmann::json;

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "imgui_internal.h"

static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

struct Config {
    int CPS = 20;
    int ClickType = 0;
    int Mode = 0;
    int Hotkey = VK_F6;
    bool Randomize = true;
    float SpeedVariation = 0.15f;
    int DutyCycle = 50;
    bool DoubleClick = false;
    int DoubleClickDelay = 40;
    bool ClickLimitEnabled = false;
    int ClickLimit = 1000;
    bool TimeLimitEnabled = false;
    int TimeLimit = 60;
} g_Cfg;

std::atomic<bool> g_Running{ true };
std::atomic<bool> g_Clicking{ false };
std::atomic<int>  g_ClickCount{ 0 };
std::chrono::steady_clock::time_point g_StartTime;
bool g_ShowWelcome = true;

void SaveConfig() {
    json j;
    j["CPS"] = g_Cfg.CPS;
    j["ClickType"] = g_Cfg.ClickType;
    j["Mode"] = g_Cfg.Mode;
    j["Hotkey"] = g_Cfg.Hotkey;
    j["Randomize"] = g_Cfg.Randomize;
    j["SpeedVariation"] = g_Cfg.SpeedVariation;
    j["DutyCycle"] = g_Cfg.DutyCycle;
    j["DoubleClick"] = g_Cfg.DoubleClick;
    j["DoubleClickDelay"] = g_Cfg.DoubleClickDelay;
    j["ClickLimitEnabled"] = g_Cfg.ClickLimitEnabled;
    j["ClickLimit"] = g_Cfg.ClickLimit;
    j["TimeLimitEnabled"] = g_Cfg.TimeLimitEnabled;
    j["TimeLimit"] = g_Cfg.TimeLimit;
    
    std::ofstream ofs("mog_config.json");
    if (ofs.is_open()) {
        ofs << j.dump(4);
    }
}

void LoadConfig() {
    std::ifstream ifs("mog_config.json");
    if (ifs.is_open()) {
        try {
            json j;
            ifs >> j;
            if (j.contains("CPS")) g_Cfg.CPS = j["CPS"];
            if (j.contains("ClickType")) g_Cfg.ClickType = j["ClickType"];
            if (j.contains("Mode")) g_Cfg.Mode = j["Mode"];
            if (j.contains("Hotkey")) g_Cfg.Hotkey = j["Hotkey"];
            if (j.contains("Randomize")) g_Cfg.Randomize = j["Randomize"];
            if (j.contains("SpeedVariation")) g_Cfg.SpeedVariation = j["SpeedVariation"];
            if (j.contains("DutyCycle")) g_Cfg.DutyCycle = j["DutyCycle"];
            if (j.contains("DoubleClick")) g_Cfg.DoubleClick = j["DoubleClick"];
            if (j.contains("DoubleClickDelay")) g_Cfg.DoubleClickDelay = j["DoubleClickDelay"];
            if (j.contains("ClickLimitEnabled")) g_Cfg.ClickLimitEnabled = j["ClickLimitEnabled"];
            if (j.contains("ClickLimit")) g_Cfg.ClickLimit = j["ClickLimit"];
            if (j.contains("TimeLimitEnabled")) g_Cfg.TimeLimitEnabled = j["TimeLimitEnabled"];
            if (j.contains("TimeLimit")) g_Cfg.TimeLimit = j["TimeLimit"];
        } catch (...) {}
    }
}

void PreciseSleep(double milliseconds) {
    if (milliseconds <= 0) return;
    static HANDLE timer = CreateWaitableTimer(NULL, FALSE, NULL);
    LARGE_INTEGER ft;
    ft.QuadPart = -(LONGLONG)(milliseconds * 10000);
    SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
    WaitForSingleObject(timer, INFINITE);
}

void ClickerThread() {
    timeBeginPeriod(1);
    std::default_random_engine generator;
    while (g_Running) {
        if (g_Clicking) {
            auto next_click_time = std::chrono::high_resolution_clock::now();
            while (g_Clicking && g_Running) {
                if (g_Cfg.ClickLimitEnabled && g_ClickCount >= g_Cfg.ClickLimit) { g_Clicking = false; break; }
                if (g_Cfg.TimeLimitEnabled) {
                    auto now = std::chrono::steady_clock::now();
                    if (std::chrono::duration_cast<std::chrono::seconds>(now - g_StartTime).count() >= g_Cfg.TimeLimit) { g_Clicking = false; break; }
                }

                INPUT input = { 0 }; input.type = INPUT_MOUSE;
                DWORD downFlag, upFlag;
                if (g_Cfg.ClickType == 0) { downFlag = MOUSEEVENTF_LEFTDOWN; upFlag = MOUSEEVENTF_LEFTUP; }
                else if (g_Cfg.ClickType == 1) { downFlag = MOUSEEVENTF_RIGHTDOWN; upFlag = MOUSEEVENTF_RIGHTUP; }
                else { downFlag = MOUSEEVENTF_MIDDLEDOWN; upFlag = MOUSEEVENTF_MIDDLEUP; }

                input.mi.dwFlags = downFlag; SendInput(1, &input, sizeof(INPUT));
                input.mi.dwFlags = upFlag; SendInput(1, &input, sizeof(INPUT));
                g_ClickCount++;

                if (g_Cfg.DoubleClick) {
                    PreciseSleep(g_Cfg.DoubleClickDelay);
                    input.mi.dwFlags = downFlag; SendInput(1, &input, sizeof(INPUT));
                    input.mi.dwFlags = upFlag; SendInput(1, &input, sizeof(INPUT));
                    g_ClickCount++;
                }

                double interval = 1000.0 / g_Cfg.CPS;
                if (g_Cfg.Randomize) {
                    std::uniform_real_distribution<double> dist(-g_Cfg.SpeedVariation * interval, g_Cfg.SpeedVariation * interval);
                    interval += dist(generator);
                }
                if (interval < 0.1) interval = 0.1;

                next_click_time += std::chrono::microseconds((long long)(interval * 1000.0));
                auto now = std::chrono::high_resolution_clock::now();
                if (next_click_time > now) {
                    auto diff = std::chrono::duration_cast<std::chrono::microseconds>(next_click_time - now).count();
                    if (diff > 1500) PreciseSleep(diff / 1000.0);
                    else while (std::chrono::high_resolution_clock::now() < next_click_time) std::this_thread::yield();
                } else next_click_time = now;
            }
        } else PreciseSleep(10);
    }
    timeEndPeriod(1);
}

void SetWindowIcon(HWND hwnd, const char* filename) {
    int w, h, c; unsigned char* data = stbi_load(filename, &w, &h, &c, 4);
    if (!data) return;
    for (int i = 0; i < w * h; i++) { unsigned char r = data[i * 4 + 0]; data[i * 4 + 0] = data[i * 4 + 2]; data[i * 4 + 2] = r; }
    HBITMAP hbmColor = CreateBitmap(w, h, 1, 32, data); HBITMAP hbmMask = CreateCompatibleBitmap(GetDC(NULL), w, h);
    ICONINFO ii = { 0 }; ii.fIcon = TRUE; ii.hbmColor = hbmColor; ii.hbmMask = hbmMask;
    HICON hIcon = CreateIconIndirect(&ii); SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon); SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    DeleteObject(hbmColor); DeleteObject(hbmMask); stbi_image_free(data);
}

ID3D11ShaderResourceView* LoadTextureFromFile(const char* filename, ID3D11Device* pd3dDevice, int* out_width, int* out_height) {
    int image_width = 0, image_height = 0; unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == NULL) return NULL;
    D3D11_TEXTURE2D_DESC desc = { (UINT)image_width, (UINT)image_height, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, {1,0}, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0 };
    ID3D11Texture2D* pTexture = NULL; D3D11_SUBRESOURCE_DATA subResource = { image_data, (UINT)(image_width * 4), 0 };
    pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);
    ID3D11ShaderResourceView* out_srv = NULL; pd3dDevice->CreateShaderResourceView(pTexture, NULL, &out_srv);
    pTexture->Release(); *out_width = image_width; *out_height = image_height; stbi_image_free(image_data); return out_srv;
}

void SetupStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(15, 15); style.FramePadding = ImVec2(8, 6); style.ItemSpacing = ImVec2(10, 10);
    style.WindowRounding = 12.0f; style.ChildRounding = 10.0f; style.FrameRounding = 8.0f;
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f); colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.12f, 0.12f, 0.14f, 1.00f); colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.17f, 1.00f); colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 0.73f, 0.18f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 0.73f, 0.18f, 1.00f); colors[ImGuiCol_Button] = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(1.00f, 0.73f, 0.18f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.12f, 0.12f, 0.14f, 1.00f); colors[ImGuiCol_TabActive] = ImVec4(1.00f, 0.73f, 0.18f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.25f, 0.25f, 0.28f, 1.00f);
}

int main(int, char**) {
    LoadConfig();
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"MOG_V3", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowExW(0, wc.lpszClassName, L"MOG-AUTOCLICKER_V3", WS_OVERLAPPEDWINDOW, 100, 100, 520, 680, nullptr, nullptr, wc.hInstance, nullptr);
    if (!CreateDeviceD3D(hwnd)) { CleanupDeviceD3D(); ::UnregisterClassW(wc.lpszClassName, wc.hInstance); return 1; }
    SetWindowIcon(hwnd, "logo.png"); ::ShowWindow(hwnd, SW_SHOWDEFAULT); ::UpdateWindow(hwnd);
    IMGUI_CHECKVERSION(); ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); io.IniFilename = NULL; SetupStyle();
    ImGui_ImplWin32_Init(hwnd); ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
    int lw, lh; ID3D11ShaderResourceView* logo = LoadTextureFromFile("logo.png", g_pd3dDevice, &lw, &lh);
    std::thread cThread(ClickerThread);
    bool lastHKState = false, waitHK = false;
    while (g_Running) {
        MSG msg; while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) { ::TranslateMessage(&msg); ::DispatchMessage(&msg); if (msg.message == WM_QUIT) g_Running = false; }
        if (!g_Running) break;
        if (!waitHK) {
            bool currHK = (GetAsyncKeyState(g_Cfg.Hotkey) & 0x8000) != 0;
            if (g_Cfg.Mode == 0) { if (currHK && !lastHKState) { g_Clicking = !g_Clicking; if (g_Clicking) { g_ClickCount = 0; g_StartTime = std::chrono::steady_clock::now(); } } }
            else { g_Clicking = currHK; if (currHK && !lastHKState) { g_ClickCount = 0; g_StartTime = std::chrono::steady_clock::now(); } }
            lastHKState = currHK;
        } else {
            for (int i = 0x08; i <= 0xFE; i++) { if (GetAsyncKeyState(i) & 0x8000) { if (i != VK_LBUTTON && i != VK_RBUTTON && i != VK_MBUTTON) { g_Cfg.Hotkey = i; waitHK = false; while(GetAsyncKeyState(i) & 0x8000) Sleep(10); break; } } }
        }
        ImGui_ImplDX11_NewFrame(); ImGui_ImplWin32_NewFrame(); ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(0, 0)); ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("Main", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
        if (g_ShowWelcome) {
            ImGui::SetCursorPos(ImVec2(io.DisplaySize.x/2 - 50, 80));
            if (logo) ImGui::Image((void*)logo, ImVec2(100, 100));
            ImGui::SetCursorPosX(io.DisplaySize.x/2 - 130);
            ImGui::SetCursorPosY(200);
            ImGui::TextColored(ImVec4(1, 0.73f, 0.18f, 1), "Welcome to MOG-AUTOCLICKER V3");
            ImGui::SetCursorPosX(io.DisplaySize.x/2 - 70);
            ImGui::Text("By MOG-Developing");
            ImGui::SetCursorPos(ImVec2(io.DisplaySize.x/2 - 80, 350));
            if (ImGui::Button("GET STARTED", ImVec2(160, 50))) g_ShowWelcome = false;
        } else {
            if (logo) { ImGui::Image((void*)logo, ImVec2(35, 35)); ImGui::SameLine(); }
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8); ImGui::TextColored(ImVec4(1, 0.73f, 0.18f, 1), "MOG-AUTOCLICKER V3");
            ImGui::Separator();
            if (ImGui::BeginTabBar("Tabs")) {
                if (ImGui::BeginTabItem("Dashboard")) {
                    ImGui::BeginChild("DashChild", ImVec2(0, 0), true);
                    ImGui::Text("Current Status:"); ImGui::SameLine();
                    if (g_Clicking) ImGui::TextColored(ImVec4(0,1,0,1), "CLICKING"); else ImGui::TextColored(ImVec4(1,0,0,1), "IDLE");
                    ImGui::Text("Total Clicks this Session: %d", g_ClickCount.load());
                    ImGui::Separator();
                    if (ImGui::Button("Save Current Configuration", ImVec2(-1, 40))) SaveConfig();
                    if (ImGui::Button("Exit Application", ImVec2(-1, 40))) g_Running = false;
                    ImGui::EndChild(); ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Auto Clicker")) {
                    ImGui::BeginChild("ClickChild", ImVec2(0, 0), true);
                    ImGui::SliderInt("Clicks Per Second", &g_Cfg.CPS, 1, 1000);
                    const char* btns[] = { "Left Click", "Right Click", "Middle Click" }; ImGui::Combo("Mouse Button", &g_Cfg.ClickType, btns, 3);
                    const char* mods[] = { "Toggle Mode", "Hold Mode" }; ImGui::Combo("Click Method", &g_Cfg.Mode, mods, 2);
                    char hk[64]; UINT sc = MapVirtualKey(g_Cfg.Hotkey, MAPVK_VK_TO_VSC); LONG lp = (sc << 16); GetKeyNameTextA(lp, hk, 64);
                    ImGui::Text("Hotkey Binding:"); ImGui::SameLine();
                    if (ImGui::Button(waitHK ? "???" : hk, ImVec2(120, 30))) waitHK = true;
                    ImGui::EndChild(); ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Extra Features")) {
                    ImGui::BeginChild("ExtraChild", ImVec2(0, 0), true);
                    if (ImGui::CollapsingHeader("Legit Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
                        ImGui::Checkbox("Randomize CPS", &g_Cfg.Randomize);
                        if (g_Cfg.Randomize) ImGui::SliderFloat("Variation", &g_Cfg.SpeedVariation, 0.01f, 1.0f);
                        ImGui::SliderInt("Duty Cycle (%)", &g_Cfg.DutyCycle, 1, 99);
                        ImGui::Checkbox("Enable Double Click", &g_Cfg.DoubleClick);
                        if (g_Cfg.DoubleClick) ImGui::SliderInt("Double Click Delay (ms)", &g_Cfg.DoubleClickDelay, 1, 200);
                    }
                    if (ImGui::CollapsingHeader("Limits")) {
                        ImGui::Checkbox("Click Limit", &g_Cfg.ClickLimitEnabled);
                        if (g_Cfg.ClickLimitEnabled) ImGui::InputInt("Max Clicks", &g_Cfg.ClickLimit);
                        ImGui::Checkbox("Time Limit", &g_Cfg.TimeLimitEnabled);
                        if (g_Cfg.TimeLimitEnabled) ImGui::InputInt("Max Seconds", &g_Cfg.TimeLimit);
                    }
                    ImGui::EndChild(); ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("About")) {
                    ImGui::BeginChild("AboutChild", ImVec2(0, 0), true);
                    ImGui::Text("MOG-AUTOCLICKER V3");
                    ImGui::Text("Developer: MOG-Developing");
                    ImGui::Separator();
                    ImGui::Text("GitHub Repository:");
                    if (ImGui::Button("Visit Repository", ImVec2(-1, 40))) {
                        ShellExecuteA(NULL, "open", "https://github.com/MOG-Developing/MOG-AUTOCLICKER", NULL, NULL, SW_SHOWNORMAL);
                    }
                    ImGui::TextDisabled("https://github.com/MOG-Developing/MOG-AUTOCLICKER");
                    ImGui::EndChild();
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
        }
        ImGui::End(); ImGui::Render();
        const float clear_color_with_alpha[4] = { 0.08f, 0.08f, 0.09f, 1.00f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); g_pSwapChain->Present(1, 0);
    }
    g_Running = false; if (cThread.joinable()) cThread.join();
    if (logo) logo->Release(); ImGui_ImplDX11_Shutdown(); ImGui_ImplWin32_Shutdown(); ImGui::DestroyContext();
    CleanupDeviceD3D(); ::DestroyWindow(hwnd); ::UnregisterClassW(wc.lpszClassName, wc.hInstance); return 0;
}

bool CreateDeviceD3D(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC sd; ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2; sd.BufferDesc.Width = 0; sd.BufferDesc.Height = 0; sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60; sd.BufferDesc.RefreshRate.Denominator = 1; sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; sd.OutputWindow = hWnd; sd.SampleDesc.Count = 1; sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE; sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    D3D_FEATURE_LEVEL fl; const D3D_FEATURE_LEVEL fla[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    HRESULT res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, fla, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &fl, &g_pd3dDeviceContext);
    if (res != S_OK) return false; CreateRenderTarget(); return true;
}
void CleanupDeviceD3D() { CleanupRenderTarget(); if (g_pSwapChain) g_pSwapChain->Release(); if (g_pd3dDeviceContext) g_pd3dDeviceContext->Release(); if (g_pd3dDevice) g_pd3dDevice->Release(); }
void CreateRenderTarget() { ID3D11Texture2D* bb; g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&bb)); g_pd3dDevice->CreateRenderTargetView(bb, NULL, &g_mainRenderTargetView); bb->Release(); }
void CleanupRenderTarget() { if (g_mainRenderTargetView) g_mainRenderTargetView->Release(); }
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return true;
    switch (msg) {
    case WM_SIZE: if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED) { CleanupRenderTarget(); g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0); CreateRenderTarget(); } return 0;
    case WM_SYSCOMMAND: if ((wParam & 0xFFF0) == SC_KEYMENU) return 0; break;
    case WM_DESTROY: ::PostQuitMessage(0); return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
