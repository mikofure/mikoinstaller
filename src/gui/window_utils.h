#pragma once

#include <windows.h>
#include <SDL.h>
#include <string>

// Forward declaration
class InstallerWindow;

// Window procedure for handling native Windows messages
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Utility functions for window management
namespace WindowUtils {
    // Apply Windows 11 native effects and styling
    void ApplyNativeEffects(HWND hwnd);
    
    // Setup window style for native animations
    void SetupWindowStyle(HWND hwnd);
    
    // Get expanded install path from environment
    std::string GetDefaultInstallPath();
}
