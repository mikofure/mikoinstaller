#include "UninstallerWindow.h"

#include <iostream>
#include <algorithm>
#include <dwmapi.h>
#include <filesystem>
#include "fonts/InterVariable.h"
#include "framework/nuklear_sdl_renderer.h"

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

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 220
#define PANEL_HEIGHT 220

using namespace std::chrono_literals;

UninstallerWindow::UninstallerWindow() : window(nullptr), renderer(nullptr), ctx(nullptr), running(false),
                                         font(nullptr), hwnd(nullptr),
                                         installPath(detectInstallPath()) {}

UninstallerWindow::~UninstallerWindow() { cleanup(); }

std::string UninstallerWindow::detectInstallPath() {
    char* localAppData = nullptr; size_t len = 0; 
    if (_dupenv_s(&localAppData, &len, "LOCALAPPDATA") == 0 && localAppData) {
        std::string path = std::string(localAppData) + "\\MikoIDE";
        free(localAppData);
        return path;
    }
    return "C:\\Users\\Default\\AppData\\Local\\MikoIDE";
}

bool UninstallerWindow::initialize() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL init failed: " << SDL_GetError() << std::endl; return false;
    }
    window = SDL_CreateWindow("MikoIDE Uninstaller",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              WINDOW_WIDTH, WINDOW_HEIGHT,
                              SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS);
    if (!window) { std::cerr << "Window create failed: " << SDL_GetError() << std::endl; return false; }

    SDL_SysWMinfo wmInfo; SDL_VERSION(&wmInfo.version);
    if (SDL_GetWindowWMInfo(window, &wmInfo)) {
        hwnd = wmInfo.info.win.window;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        originalWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(UninstallWindowProc)));
        MARGINS margins = {-1, -1, -1, -1}; DwmExtendFrameIntoClientArea(hwnd, &margins);
        BOOL darkMode = TRUE; DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkMode, sizeof(darkMode));
        DWM_WINDOW_CORNER_PREFERENCE cornerPreference = DWMWCP_ROUND; DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &cornerPreference, sizeof(cornerPreference));
        DWM_SYSTEMBACKDROP_TYPE backdropType = DWMSBT_MAINWINDOW; DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdropType, sizeof(backdropType));
        COLORREF captionColor = RGB(32, 32, 32); DwmSetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, &captionColor, sizeof(captionColor));
        COLORREF borderColor = RGB(64, 64, 64); DwmSetWindowAttribute(hwnd, DWMWA_BORDER_COLOR, &borderColor, sizeof(borderColor));
        BOOL disableTransitions = FALSE; DwmSetWindowAttribute(hwnd, DWMWA_TRANSITIONS_FORCEDISABLED, &disableTransitions, sizeof(disableTransitions));
        LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE); style |= WS_MINIMIZEBOX | WS_SYSMENU; SetWindowLongPtr(hwnd, GWL_STYLE, style);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) { std::cerr << "Renderer create failed: " << SDL_GetError() << std::endl; return false; }

    ctx = nk_sdl_init(window, renderer);
    {
        struct nk_font_atlas *atlas; struct nk_font_config config = nk_font_config(0);
        nk_sdl_font_stash_begin(&atlas);
        config.oversample_h = 2; config.oversample_v = 2; config.pixel_snap = true;
        font = nk_font_atlas_add_from_memory(atlas, (void*)InterVariable_ttf, sizeof(InterVariable_ttf), 16, &config);
        nk_sdl_font_stash_end();
        if (font) nk_style_set_font(ctx, &font->handle);
    }
    setupCustomStyle();
    running = true; return true;
}

void UninstallerWindow::setupCustomStyle() {
    struct nk_style *s = &ctx->style;
    s->window.background = nk_rgba(0, 0, 0, 255);
    s->window.fixed_background = nk_style_item_color(nk_rgba(0, 0, 0, 255));
    s->window.padding = nk_vec2(17, 17);
    s->window.group_padding = nk_vec2(17, 17);
    s->button.normal = nk_style_item_color(nk_rgba(255, 255, 255, 255));
    s->button.hover = nk_style_item_color(nk_rgba(240, 240, 240, 255));
    s->button.active = nk_style_item_color(nk_rgba(220, 220, 220, 255));
    s->button.text_normal = nk_rgba(0, 0, 0, 255);
    s->button.text_hover = nk_rgba(0, 0, 0, 255);
    s->button.text_active = nk_rgba(0, 0, 0, 255);
    s->button.border = 0; s->button.rounding = 4; s->button.padding = nk_vec2(8, 8);
    s->text.color = nk_rgba(255, 255, 255, 255);
    s->edit.normal = nk_style_item_color(nk_rgba(40, 40, 40, 255));
    s->edit.hover = nk_style_item_color(nk_rgba(50, 50, 50, 255));
    s->edit.active = nk_style_item_color(nk_rgba(60, 60, 60, 255));
    s->edit.text_normal = nk_rgba(255, 255, 255, 255);
    s->edit.text_hover = nk_rgba(255, 255, 255, 255);
    s->edit.text_active = nk_rgba(255, 255, 255, 255);
    s->edit.border = 1; s->edit.border_color = nk_rgba(100, 100, 100, 255); s->edit.rounding = 4;
}

void UninstallerWindow::startUninstallAsync() {
    if (uninstalling) return;
    uninstallOk = false; uninstallFailed = false; progress = 0; uninstalling = true; lastAction.clear();
    worker = std::thread([this]{ doUninstall(); });
}

static void scheduleSelfDelete() {
    // Use cmd.exe to delete the exe after the process exits
    char path[MAX_PATH]; GetModuleFileNameA(NULL, path, MAX_PATH);
    std::string cmd = "cmd /c timeout 1 >nul & del \"" + std::string(path) + "\"";
    STARTUPINFOA si{}; PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);
    if (CreateProcessA(nullptr, cmd.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
        CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
    }
}

void UninstallerWindow::doUninstall() {
    try {
        // Example steps, update lastAction for UI
        lastAction = "removing: shortcuts"; std::this_thread::sleep_for(300ms); progress = 10;
        lastAction = "removing: file associations"; std::this_thread::sleep_for(300ms); progress = 25;
        lastAction = "removing: PATH entries"; std::this_thread::sleep_for(300ms); progress = 40;
        lastAction = std::string("removing: ") + installPath; std::this_thread::sleep_for(300ms); progress = 70;
        // Pretend to remove files
        // TODO: add real filesystem removal
        std::this_thread::sleep_for(500ms); progress = 100;
        uninstallOk = true;
    } catch (...) {
        uninstallFailed = true;
    }
    uninstalling = false;
}

void UninstallerWindow::run() {
    SDL_Event e;
    while (running) {
        nk_input_begin(ctx);
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false; else nk_sdl_handle_event(&e);
        }
        nk_sdl_handle_grab();
        nk_input_end(ctx);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (nk_begin(ctx, "Uninstall Panel", nk_rect(0, 0, WINDOW_WIDTH, PANEL_HEIGHT), NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND)) {
            nk_layout_row_dynamic(ctx, 25, 1);
            nk_label(ctx, "Uninstall MikoIDE", NK_TEXT_LEFT);

            nk_layout_row_dynamic(ctx, 25, 1);
            nk_label(ctx, installPath.c_str(), NK_TEXT_LEFT);

            // Progress + status text
            nk_layout_row_dynamic(ctx, 35, 1);
            if (uninstalling) {
                nk_size cur = progress.load();
                nk_progress(ctx, &cur, 100, nk_false);
                nk_layout_row_dynamic(ctx, 20, 1);
                nk_label(ctx, lastAction.c_str(), NK_TEXT_LEFT);
            } else if (uninstallOk) {
                nk_label(ctx, "Completed. Click OK to exit.", NK_TEXT_LEFT);
                nk_layout_row_dynamic(ctx, 35, 1);
                if (nk_button_label(ctx, "OK")) {
                    scheduleSelfDelete();
                    running = false;
                }
            } else if (uninstallFailed) {
                nk_label(ctx, "Error during uninstallation.", NK_TEXT_LEFT);
                nk_layout_row_dynamic(ctx, 35, 2);
                if (nk_button_label(ctx, "Retry")) startUninstallAsync();
                if (nk_button_label(ctx, "Close")) running = false;
            } else {
                nk_layout_row_dynamic(ctx, 35, 2);
                if (nk_button_label(ctx, "Uninstall")) startUninstallAsync();
                if (nk_button_label(ctx, "Close")) running = false;
            }
        }
        nk_end(ctx);

        nk_sdl_render(NK_ANTI_ALIASING_ON);
        SDL_RenderPresent(renderer);
    }
}

void UninstallerWindow::cleanup() {
    if (worker.joinable()) { uninstalling = false; worker.join(); }
    if (ctx) { nk_sdl_shutdown(); ctx = nullptr; }
    if (renderer) { SDL_DestroyRenderer(renderer); renderer = nullptr; }
    if (window) { SDL_DestroyWindow(window); window = nullptr; }
    SDL_Quit();
}

LRESULT CALLBACK UninstallWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    UninstallerWindow* window = reinterpret_cast<UninstallerWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    switch (uMsg) {
        case WM_SYSCOMMAND:
            if ((wParam & 0xFFF0) == SC_CLOSE) { if (window) window->running = false; return 0; }
            break;
        case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    if (window && window->originalWndProc) return CallWindowProc(window->originalWndProc, hwnd, uMsg, wParam, lParam);
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
