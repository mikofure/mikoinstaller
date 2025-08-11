#include <windows.h>
#include <dwmapi.h>
#include "window_utils.h"
#include "installer_window.h"
#include "../config/app_config.h"
#include <iostream>
#include <string>

#pragma comment(lib, "dwmapi.lib")

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    InstallerWindow* window = reinterpret_cast<InstallerWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    
    switch (uMsg) {
        case WM_SYSCOMMAND: {
            switch (wParam & 0xFFF0) {
                case SC_CLOSE:
                    if (window) {
                        window->SetRunning(false);
                    }
                    return 0;
                case SC_MINIMIZE:
                case SC_MAXIMIZE:
                case SC_RESTORE:
                    // Let Windows handle these with native animations
                    break;
            }
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    
    // Call original window procedure for unhandled messages
    if (window && window->GetOriginalWndProc()) {
        return CallWindowProc(window->GetOriginalWndProc(), hwnd, uMsg, wParam, lParam);
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

namespace WindowUtils {
    void ApplyNativeEffects(HWND hwnd) {
        // Enable Windows 11 native effects
        MARGINS margins = {-1, -1, -1, -1};
        DwmExtendFrameIntoClientArea(hwnd, &margins);
        
        // Set window attributes for Windows 11 appearance
        BOOL darkMode = TRUE;
        DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkMode, sizeof(darkMode));
        
        // Enable rounded corners (Windows 11)
        DWM_WINDOW_CORNER_PREFERENCE cornerPreference = DWMWCP_ROUND;
        DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &cornerPreference, sizeof(cornerPreference));
        
        // Enable mica backdrop effect (Windows 11)
        DWM_SYSTEMBACKDROP_TYPE backdropType = DWMSBT_MAINWINDOW;
        DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdropType, sizeof(backdropType));
        
        // Enable caption color customization
        COLORREF captionColor = RGB(32, 32, 32);
        DwmSetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, &captionColor, sizeof(captionColor));
        
        // Enable border color customization
        COLORREF borderColor = RGB(64, 64, 64);
        DwmSetWindowAttribute(hwnd, DWMWA_BORDER_COLOR, &borderColor, sizeof(borderColor));
        
        // Enable native window animations by NOT disabling transitions
        BOOL disableTransitions = FALSE;
        DwmSetWindowAttribute(hwnd, DWMWA_TRANSITIONS_FORCEDISABLED, &disableTransitions, sizeof(disableTransitions));
        
        // Enable window composition for smooth animations
        BOOL enableComposition = TRUE;
        DwmEnableComposition(enableComposition);
    }
    
    void SetupWindowStyle(HWND hwnd) {
        // Set window style to support native animations while keeping borderless appearance
        LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
        style |= WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU;
        SetWindowLongPtr(hwnd, GWL_STYLE, style);
    }
    
    std::string GetDefaultInstallPath() {
        char* localAppData = nullptr;
        size_t len = 0;
        
        // Get %LOCALAPPDATA% environment variable
        errno_t err = _dupenv_s(&localAppData, &len, "LOCALAPPDATA");
        
        if (err == 0 && localAppData != nullptr) {
            std::string path = std::string(localAppData) + "\\" + DEFAULT_INSTALL_SUBDIR;
            free(localAppData);
            return path;
        } else {
            // Fallback if environment variable is not available
            return "C:\\Users\\Default\\AppData\\Local\\" + std::string(DEFAULT_INSTALL_SUBDIR);
        }
    }
}
