#pragma once
#include <SDL.h>
#include <SDL_syswm.h>
#include <string>
#include <windows.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include "framework/nuklear.h"

#include <atomic>
#include <thread>

class UninstallerWindow {
public:
    UninstallerWindow();
    ~UninstallerWindow();

    bool initialize();
    void run();
    void cleanup();

    bool running;
    WNDPROC originalWndProc;

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    struct nk_context* ctx;
    struct nk_font* font;
    HWND hwnd;

    // UI state
    std::string installPath;

    // Uninstall state
    std::atomic<bool> uninstalling{false};
    std::atomic<size_t> progress{0};
    std::atomic<bool> uninstallOk{false};
    std::atomic<bool> uninstallFailed{false};
    std::thread worker;
    std::string lastAction;

private:
    std::string detectInstallPath();
    void setupCustomStyle();
    void startUninstallAsync();
    void doUninstall();
};

LRESULT CALLBACK UninstallWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
