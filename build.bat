@echo off
echo Compiling MOG-AUTOCLICKER_V3...

windres src/resource.rc -O coff -o src/resource.res

g++ -Os -s -DNDEBUG ^
src/main.cpp ^
imgui-1.92.7/imgui.cpp ^
imgui-1.92.7/imgui_draw.cpp ^
imgui-1.92.7/imgui_widgets.cpp ^
imgui-1.92.7/imgui_tables.cpp ^
imgui-1.92.7/backends/imgui_impl_win32.cpp ^
imgui-1.92.7/backends/imgui_impl_dx11.cpp ^
src/resource.res ^
-Iimgui-1.92.7 ^
-Iimgui-1.92.7/backends ^
-Isrc ^
-o MOG-AUTOCLICKER_V3.exe ^
-mwindows ^
-ld3d11 -ld3dcompiler -ldxgi -ldwmapi ^
-lgdi32 -limm32 -lwinmm -lshell32 ^
-static-libgcc -static-libstdc++ ^
-ffunction-sections -fdata-sections ^
-Wl,--gc-sections

echo Done!
pause
