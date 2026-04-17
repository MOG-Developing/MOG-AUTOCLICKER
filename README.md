# MOG-AUTOCLICKER

MOG-AUTOCLICKER is a lightweight, fast, open source, and customizable mouse clicking program.

[WEBSITE](https://mog-developing.github.io/MOG-AUTOCLICKER)

# MOG-AUTOCLICKER

MOG-AUTOCLICKER is a lightweight, fast, open source, and customizable mouse clicking program.
[WEBSITE](https://mog-developing.github.io/MOG-AUTOCLICKER)

[![Downloads](https://img.shields.io/github/downloads/MOG-Developing/MOG-AUTOCLICKER/total?style=flat-square&label=Downloads&color=orange)](https://github.com/MOG-Developing/MOG-AUTOCLICKER/releases) [![License](https://img.shields.io/github/license/MOG-Developing/MOG-AUTOCLICKER?style=flat-square&color=orange)](https://github.com/MOG-Developing/MOG-AUTOCLICKER/blob/main/LICENSE) [![Stars](https://img.shields.io/github/stars/MOG-Developing/MOG-AUTOCLICKER?style=flat-square&color=orange)](https://github.com/MOG-Developing/MOG-AUTOCLICKER/stargazers)

## Why you should use this AutoClicker?
- Our AutoClicker is modern, lightweight and easy to use!
- It's made in C++ (there is a single release made in Python).
- It is user-friendly, easy to navigate!
- You can use it on Windows (adding support for Linux soon!).

## Made with:
- **Compiler:** winlibs-x86_64-posix-seh-gcc-15.2.0-mingw-w64ucrt-13.0.0-r1
- **Graphics:** DirectX 11
- **UI Framework:** Dear ImGui (1.92.7)
- **JSON Handler:** nlohmann/json

## Current features:
- Config saving (`mog_config.json`).
- Up to 1000 CPS (uses `PreciseSleep` for accuracy).
- Left, Right, and Middle click support.
- Hotkey binding (fully customizable).
- CPS randomization and Variation sliders.
- Double clicking mode.
- Click limit and Time limit.

---

## FAQ

**Is it safe?**
Yes. It is open source, you can check the code yourself.

**Why am I not getting exactly 1000 CPS?**
Windows has a timer resolution limit. The app uses `timeBeginPeriod(1)` to get as close as possible, but your CPU and OS will always cause a small variance.

**Does this work in games?**
Yes. Use the **Randomize CPS** feature so you don't get flagged by anti-cheats for having a perfect clicking pattern.

**Where is the config saved?**
It creates a `mog_config.json` in the same folder as the exe.

**The app won't open / UI is broken?**
Make sure you have `logo.png` in the same folder as the exe. The app needs it to load the interface.

**How do I stop the clicking?**
Use your hotkey to toggle it, or use "Hold Mode" so it only clicks while the key is pressed.

## Why you should use this AutoClicker?
- Our AutoClicker is modern, lightweight and easy to use!
- It's made in C++ (there is a single release made in Python).
- It is user-friendly, easy to navigate!
- You can use it on Windows (adding support for Linux soon!).

 ## Made with:
 - **Compiler:** winlibs-x86_64-posix-seh-gcc-15.2.0-mingw-w64ucrt-13.0.0-r1
 - **Graphics:** DirectX 11 (DX11)
 - **UI Framework:** Dear ImGui (imgui-1.92.7)
 - **JSON Handler:** nlohmann/json

## Current features:
- Config saving.
- Up to 1000CPS(you will get less).
- Left click, Right click, Middle click.
- Hotkey binding.
- CPS randomization.
- Double clicking.
- Click limit and Time limit.

## FAQ

**Is it safe?**

Yes. It is open source, you can check the code yourself.

*More questions and responses for your FAQ soon!*
