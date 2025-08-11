#pragma once

// Application constants
#define APP_NAME "MikoIDE Installer"
#define APP_VERSION "0.1.2"
#define APP_TITLE "MikoIDE Installer 0.1.2"

// Window constants
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 570
#define IMAGE_HEIGHT 365
#define PANEL_HEIGHT 168
#define TITLEBAR_HEIGHT 32

// Windows 11 DWM constants (for compatibility with older SDKs)
#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
#define DWMWA_WINDOW_CORNER_PREFERENCE 33
#endif

#ifndef DWMWA_SYSTEMBACKDROP_TYPE
#define DWMWA_SYSTEMBACKDROP_TYPE 38
#endif

#ifndef DWMWA_CAPTION_COLOR
#define DWMWA_CAPTION_COLOR 35
#endif

#ifndef DWMWA_BORDER_COLOR
#define DWMWA_BORDER_COLOR 34
#endif

#ifndef DWMWA_TRANSITIONS_FORCEDISABLED
#define DWMWA_TRANSITIONS_FORCEDISABLED 3
#endif

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

// Installation constants
#define DEFAULT_INSTALL_SUBDIR "MikoIDE"
#define CONTROL_BUTTON_COUNT 3
#define CONTROL_BUTTON_WIDTH 46
