#pragma once

#include <SDL.h>
#include <SDL_syswm.h>
#include <windows.h>
#include <string>
// Ensure Nuklear API declarations used in this TU are available
#ifndef NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_FONT_BAKING
#endif
#ifndef NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_DEFAULT_FONT
#endif
#ifndef NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_IO
#endif
#ifndef NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_STANDARD_VARARGS
#endif
#include "framework/nuklear.h"

class InstallerWindow {
public:
    InstallerWindow();
    ~InstallerWindow();
    
    // Main application lifecycle
    bool Initialize();
    void Run();
    void Cleanup();
    
    // Getters for window procedure
    bool IsRunning() const { return running; }
    void SetRunning(bool value) { running = value; }
    WNDPROC GetOriginalWndProc() const { return originalWndProc; }
    
private:
    // SDL and rendering
    SDL_Window* window;
    SDL_Renderer* renderer;
    struct nk_context* ctx;
    struct nk_font* font;
    struct nk_font* iconFont;
    SDL_Texture* backgroundTexture;
    
    // Windows integration
    HWND hwnd;
    WNDPROC originalWndProc;
    
    // Application state
    bool running;
    std::string installPath;
    nk_bool addToPath;
    nk_bool assignFileExtension;
    
    // Window control state
    bool isMaximized;
    RECT windowRect;
    RECT normalRect;
    
    // Initialization methods
    bool InitializeSDL();
    bool InitializeWindow();
    bool InitializeRenderer();
    bool InitializeNuklear();
    bool LoadBackgroundImage();
    void SetupWindowsIntegration();
    void SetupCustomStyle();
    
    // Event handling
    void HandleEvents();
    void HandleWindowControls(int mouseX, int mouseY, bool clicked);
    
    // UI rendering
    void RenderFrame();
    void RenderTitleBar();
    void RenderInstallerPanel();
    
    // UI interactions
    void OpenFolderDialog();
    void PerformInstallation();
};
