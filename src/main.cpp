#include <SDL.h>
#include <SDL_syswm.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#include <shellapi.h>
#include <dwmapi.h>
#include <fstream>
#include <filesystem>
#include "fonts/InterVariable.h"
#include "images/banner.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_RENDERER_IMPLEMENTATION
#include "framework/nuklear.h"
#include "framework/nuklear_sdl_renderer.h"

#pragma comment(lib, "dwmapi.lib")

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

// Resource IDs (removed IDR_IMAGE_BACKGROUND as we now load from file)

// Window constants
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 570
#define IMAGE_HEIGHT 365
#define PANEL_HEIGHT 168
#define TITLEBAR_HEIGHT 32

// Forward declaration for window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

class InstallerWindow {
public:
    bool running;
    WNDPROC originalWndProc;
    
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    struct nk_context* ctx;
    struct nk_font* font;
    struct nk_font* iconFont;
    HWND hwnd;
    SDL_Texture* backgroundTexture;
    
    // UI state
    std::string installPath;
    nk_bool addToPath;
    nk_bool assignFileExtension;
    
    // Window control state
    bool isMaximized;
    RECT windowRect;
    RECT normalRect;
    
public:
    InstallerWindow() : window(nullptr), renderer(nullptr), ctx(nullptr), running(false),
                       installPath(getExpandedInstallPath()),
                       addToPath(nk_true), assignFileExtension(nk_true) {}
    
    ~InstallerWindow() {
        cleanup();
    }
    
    std::string getExpandedInstallPath() {
        char* localAppData = nullptr;
        size_t len = 0;
        
        // Get %LOCALAPPDATA% environment variable
        errno_t err = _dupenv_s(&localAppData, &len, "LOCALAPPDATA");
        
        if (err == 0 && localAppData != nullptr) {
            std::string path = std::string(localAppData) + "\\MikoIDE";
            free(localAppData);
            return path;
        } else {
            // Fallback if environment variable is not available
            return "C:\\Users\\Default\\AppData\\Local\\MikoIDE";
        }
    }
    
    bool initialize() {
        // Initialize SDL
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
            return false;
        }
        
        // Create borderless window
        window = SDL_CreateWindow("MikoIDE Installer",
                                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                WINDOW_WIDTH, WINDOW_HEIGHT,
                                SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS);
        
        if (!window) {
            std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            return false;
        }
        
        // Get native window handle
        SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version);
        if (SDL_GetWindowWMInfo(window, &wmInfo)) {
            hwnd = wmInfo.info.win.window;
            
            // Store reference to this instance for window procedure
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
            
            // Subclass the window to handle WM_SYSCOMMAND messages
            originalWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProc)));
            
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
            
            // Set window style to support native animations while keeping borderless appearance
            LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
            style |= WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU;
            SetWindowLongPtr(hwnd, GWL_STYLE, style);
        }
        
        // Create renderer
        int flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
        renderer = SDL_CreateRenderer(window, -1, flags);
        
        if (!renderer) {
            std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
            return false;
        }
        
        // Scale renderer for High-DPI displays
        {
            int render_w, render_h;
            int window_w, window_h;
            float scale_x, scale_y;
            SDL_GetRendererOutputSize(renderer, &render_w, &render_h);
            SDL_GetWindowSize(window, &window_w, &window_h);
            scale_x = (float)(render_w) / (float)(window_w);
            scale_y = (float)(render_h) / (float)(window_h);
            SDL_RenderSetScale(renderer, scale_x, scale_y);
        }
        
        // Load background image
        if (!loadBackgroundImage()) {
            std::cerr << "Failed to load background image" << std::endl;
            return false;
        }
        
        // Initialize Nuklear
        ctx = nk_sdl_init(window, renderer);
        
        // Load custom fonts
        {
            struct nk_font_atlas *atlas;
            struct nk_font_config config = nk_font_config(0);
            
            nk_sdl_font_stash_begin(&atlas);
            
            // Load InterVariable font from memory
            config.oversample_h = 2;
            config.oversample_v = 2;
            config.pixel_snap = true;
            font = nk_font_atlas_add_from_memory(atlas, (void*)InterVariable_ttf, sizeof(InterVariable_ttf), 16, &config);
            
            // Load Segoe Fluent Icons for window controls
            struct nk_font_config iconConfig = nk_font_config(0);
            iconConfig.oversample_h = 1;
            iconConfig.oversample_v = 1;
            iconConfig.pixel_snap = true;
            iconConfig.range = nk_font_default_glyph_ranges();
            iconFont = nk_font_atlas_add_from_file(atlas, "C:\\Windows\\Fonts\\SegoeFluentIcons.ttf", 16, &iconConfig);
            
            nk_sdl_font_stash_end();
            
            if (font) {
                nk_style_set_font(ctx, &font->handle);
            }
        }
        
        // Customize UI styling
        setupCustomStyle();
        
        // Initialize window state
        isMaximized = false;
        GetWindowRect(hwnd, &normalRect);
        
        // Window starts visible for native behavior
        // ShowWindow(hwnd, SW_SHOW); // Already shown by SDL
        
        running = true;
        return true;
    }
    
    void setupCustomStyle() {
        struct nk_style *s = &ctx->style;
        
        // Set window background to black
        s->window.background = nk_rgba(0, 0, 0, 255);
        s->window.fixed_background = nk_style_item_color(nk_rgba(0, 0, 0, 255));
        
        // Set window padding to 17px
        s->window.padding = nk_vec2(17, 17);
        s->window.group_padding = nk_vec2(17, 17);
        
        // Button styling - white background, black text, no border
        s->button.normal = nk_style_item_color(nk_rgba(255, 255, 255, 255));
        s->button.hover = nk_style_item_color(nk_rgba(240, 240, 240, 255));
        s->button.active = nk_style_item_color(nk_rgba(220, 220, 220, 255));
        s->button.text_normal = nk_rgba(0, 0, 0, 255);
        s->button.text_hover = nk_rgba(0, 0, 0, 255);
        s->button.text_active = nk_rgba(0, 0, 0, 255);
        s->button.border = 0;
        s->button.rounding = 4;
        s->button.padding = nk_vec2(8, 8);
        
        // Text and label colors for dark theme
        s->text.color = nk_rgba(255, 255, 255, 255);
        
        // Input field styling
        s->edit.normal = nk_style_item_color(nk_rgba(40, 40, 40, 255));
        s->edit.hover = nk_style_item_color(nk_rgba(50, 50, 50, 255));
        s->edit.active = nk_style_item_color(nk_rgba(60, 60, 60, 255));
        s->edit.text_normal = nk_rgba(255, 255, 255, 255);
        s->edit.text_hover = nk_rgba(255, 255, 255, 255);
        s->edit.text_active = nk_rgba(255, 255, 255, 255);
        s->edit.border = 1;
        s->edit.border_color = nk_rgba(100, 100, 100, 255);
        s->edit.rounding = 4;
        
        // Checkbox styling
        s->checkbox.normal = nk_style_item_color(nk_rgba(40, 40, 40, 255));
        s->checkbox.hover = nk_style_item_color(nk_rgba(50, 50, 50, 255));
        s->checkbox.active = nk_style_item_color(nk_rgba(60, 60, 60, 255));
        s->checkbox.cursor_normal = nk_style_item_color(nk_rgba(255, 255, 255, 255));
        s->checkbox.cursor_hover = nk_style_item_color(nk_rgba(240, 240, 240, 255));
        s->checkbox.text_normal = nk_rgba(255, 255, 255, 255);
        s->checkbox.text_hover = nk_rgba(255, 255, 255, 255);
        s->checkbox.text_active = nk_rgba(255, 255, 255, 255);
        s->checkbox.border = 1;
        s->checkbox.border_color = nk_rgba(100, 100, 100, 255);
    }
    
    bool loadBackgroundImage() {
        // Load image from binary data in banner.h
        SDL_RWops* rw = SDL_RWFromConstMem(__image_bmp, __image_bmp_len);
        if (!rw) {
            std::cerr << "Failed to create RWops from binary data: " << SDL_GetError() << std::endl;
            return false;
        }
        
        // Load surface from BMP binary data
        SDL_Surface* surface = SDL_LoadBMP_RW(rw, 1);
        if (!surface) {
            std::cerr << "Failed to load BMP from binary data: " << SDL_GetError() << std::endl;
            return false;
        }
        
        // Create texture from surface
        backgroundTexture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        
        if (!backgroundTexture) {
            std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
            return false;
        }
        
        return true;
    }
    
    void handleWindowControls(int mouseX, int mouseY, bool clicked) {
        // Define button areas in the custom titlebar
        int buttonWidth = 46;
        int buttonHeight = TITLEBAR_HEIGHT;
        
        // Close button (rightmost)
        SDL_Rect closeRect = {WINDOW_WIDTH - buttonWidth, 0, buttonWidth, buttonHeight};
        // Maximize button
        SDL_Rect maxRect = {WINDOW_WIDTH - buttonWidth * 2, 0, buttonWidth, buttonHeight};
        // Minimize button
        SDL_Rect minRect = {WINDOW_WIDTH - buttonWidth * 3, 0, buttonWidth, buttonHeight};
        
        if (clicked) {
            if (mouseX >= closeRect.x && mouseX < closeRect.x + closeRect.w &&
                mouseY >= closeRect.y && mouseY < closeRect.y + closeRect.h) {
                // Use native close with animation
                PostMessage(hwnd, WM_SYSCOMMAND, SC_CLOSE, 0);
            }
            else if (mouseX >= minRect.x && mouseX < minRect.x + minRect.w &&
                     mouseY >= minRect.y && mouseY < minRect.y + minRect.h) {
                // Use native minimize with animation
                PostMessage(hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
            }
            else if (mouseX >= maxRect.x && mouseX < maxRect.x + maxRect.w &&
                     mouseY >= maxRect.y && mouseY < maxRect.y + maxRect.h) {
                // Use native maximize/restore with animation
                if (isMaximized) {
                    PostMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
                    isMaximized = false;
                } else {
                    PostMessage(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
                    isMaximized = true;
                }
            }
        }
    }
    
    // void toggleMaximize() {
    //     if (isMaximized) {
    //         SetWindowPos(hwnd, NULL, normalRect.left, normalRect.top,
    //                     normalRect.right - normalRect.left,
    //                     normalRect.bottom - normalRect.top,
    //                     SWP_NOZORDER | SWP_NOACTIVATE);
    //         isMaximized = false;
    //     } else {
    //         GetWindowRect(hwnd, &normalRect);
    //         MONITORINFO mi = { sizeof(mi) };
    //         if (GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &mi)) {
    //             SetWindowPos(hwnd, NULL, mi.rcWork.left, mi.rcWork.top,
    //                         mi.rcWork.right - mi.rcWork.left,
    //                         mi.rcWork.bottom - mi.rcWork.top,
    //                         SWP_NOZORDER | SWP_NOACTIVATE);
    //             isMaximized = true;
    //         }
    //     }
    // }
    
    void openFolderDialog() {
        BROWSEINFOW bi = { 0 };
        bi.lpszTitle = L"Select Installation Directory";
        bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
        
        LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);
        
        if (pidl != 0) {
            wchar_t path[MAX_PATH];
            if (SHGetPathFromIDListW(pidl, path)) {
                // Convert wide string to narrow string
                char narrowPath[MAX_PATH];
                WideCharToMultiByte(CP_UTF8, 0, path, -1, narrowPath, MAX_PATH, NULL, NULL);
                installPath = std::string(narrowPath) + "\\MikoIDE";
            }
            
            IMalloc* imalloc = 0;
            if (SUCCEEDED(SHGetMalloc(&imalloc))) {
                imalloc->Free(pidl);
                imalloc->Release();
            }
        }
    }
    
    void performInstallation() {
        std::cout << "Installing MikoIDE to: " << installPath << std::endl;
        std::cout << "Add to PATH: " << (addToPath ? "Yes" : "No") << std::endl;
        std::cout << "Assign file extension: " << (assignFileExtension ? "Yes" : "No") << std::endl;
        
        // Here you would implement the actual installation logic
        // For now, just show a message
        std::cout << "Installation completed successfully!" << std::endl;
        
        running = false;
    }
    
    // Animation helper functions
    // Custom animation functions removed - using native Windows animations instead
    
    void run() {
        // Window is already shown by SDL, using native animations
        
        SDL_Event e;
        bool dragging = false;
        int dragStartX = 0, dragStartY = 0;
        
        while (running) {
            // Handle events
            nk_input_begin(ctx);
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    running = false;
                }
                else if (e.type == SDL_MOUSEBUTTONDOWN) {
                    if (e.button.button == SDL_BUTTON_LEFT) {
                        int mouseX = e.button.x;
                        int mouseY = e.button.y;
                        
                        // Handle window controls
                        if (mouseY < TITLEBAR_HEIGHT) {
                            handleWindowControls(mouseX, mouseY, true);
                            
                            // Start dragging if not on control buttons
                            if (mouseX < WINDOW_WIDTH - 138) { // 3 buttons * 46px
                                dragging = true;
                                dragStartX = mouseX;
                                dragStartY = mouseY;
                            }
                        }
                    }
                }
                else if (e.type == SDL_MOUSEBUTTONUP) {
                    if (e.button.button == SDL_BUTTON_LEFT) {
                        dragging = false;
                    }
                }
                else if (e.type == SDL_MOUSEMOTION && dragging) {
                    int currentX, currentY;
                    SDL_GetWindowPosition(window, &currentX, &currentY);
                    SDL_SetWindowPosition(window, 
                                        currentX + e.motion.x - dragStartX,
                                        currentY + e.motion.y - dragStartY);
                }
                // else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT && e.button.clicks == 2) {
                //     // Double-click to maximize/restore
                //     if (e.button.y < TITLEBAR_HEIGHT && e.button.x < WINDOW_WIDTH - 138) {
                //         toggleMaximize();
                //     }
                // }
                
                nk_sdl_handle_event(&e);
            }
            nk_sdl_handle_grab();
            nk_input_end(ctx);
            
            // Clear background with black color
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            
            // Render background image (full height, overlapped by titlebar)
            if (backgroundTexture) {
                SDL_Rect imageRect = {0, 0, WINDOW_WIDTH, IMAGE_HEIGHT + TITLEBAR_HEIGHT};
                SDL_RenderCopy(renderer, backgroundTexture, NULL, &imageRect);
            }
            
            // Custom transparent titlebar
            struct nk_style_window titlebar_style = ctx->style.window;
            ctx->style.window.fixed_background = nk_style_item_color(nk_rgba(0, 0, 0, 0)); // Fully transparent
            ctx->style.window.padding = nk_vec2(10, 4);
            
            if (nk_begin(ctx, "Titlebar", nk_rect(0, 0, WINDOW_WIDTH, TITLEBAR_HEIGHT),
                        NK_WINDOW_NO_SCROLLBAR)) {
                
                nk_layout_row_begin(ctx, NK_STATIC, TITLEBAR_HEIGHT - 4, 3);
                
                // App icon and title
                nk_layout_row_push(ctx, WINDOW_WIDTH - 138 - 0);
                nk_label(ctx, "MikoIDE Installer 0.1.2", NK_TEXT_LEFT);
                
                // // Window control buttons
                // nk_layout_row_push(ctx, 46);
                // if (nk_button_label(ctx, "-")) { // Minimize
                //     ShowWindow(hwnd, SW_MINIMIZE);
                // }
                // nk_layout_row_push(ctx, 46);
                // if (nk_button_label(ctx, "X")) { // Close
                //     running = false;
                // }
                nk_layout_row_end(ctx);
            }
            nk_end(ctx);
            
            // Restore original window style
            ctx->style.window = titlebar_style;
            
            // Installer panel at bottom (full width)
            int panelY = IMAGE_HEIGHT + TITLEBAR_HEIGHT;
            if (nk_begin(ctx, "Installer Panel", nk_rect(0, panelY, WINDOW_WIDTH, PANEL_HEIGHT),
                        NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND)) {
                        
                // Install location label
                nk_layout_row_dynamic(ctx, 25, 1);
                nk_label(ctx, "Install location", NK_TEXT_LEFT);
                        
                // Install location input and Choose button
                nk_layout_row_begin(ctx, NK_DYNAMIC, 35, 2);
                nk_layout_row_push(ctx, 0.75f);
                static char pathBuffer[512];
                strncpy(pathBuffer, installPath.c_str(), sizeof(pathBuffer) - 1);
                pathBuffer[sizeof(pathBuffer) - 1] = '\0';
                nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, pathBuffer, sizeof(pathBuffer), nk_filter_default);
                installPath = std::string(pathBuffer);
                        
                nk_layout_row_push(ctx, 0.25f);
                if (nk_button_label(ctx, "Choose")) {
                    openFolderDialog();
                }
                nk_layout_row_end(ctx);
                
                // Checkboxes row
                nk_layout_row_dynamic(ctx, 25, 2);
                nk_checkbox_label(ctx, "add MikoIDE to environment path", &addToPath);
                nk_checkbox_label(ctx, "assign file extension", &assignFileExtension);
                
                // Install button row
                nk_layout_row_dynamic(ctx, 35, 1);
                if (nk_button_label(ctx, "Install")) {
                    performInstallation();
                }
            }

            nk_end(ctx);
            
            // Render
            nk_sdl_render(NK_ANTI_ALIASING_ON);
            
            SDL_RenderPresent(renderer);
        }
    }
    
    void cleanup() {
        if (backgroundTexture) {
            SDL_DestroyTexture(backgroundTexture);
            backgroundTexture = nullptr;
        }
        
        if (ctx) {
            nk_sdl_shutdown();
            ctx = nullptr;
        }
        
        if (renderer) {
            SDL_DestroyRenderer(renderer);
            renderer = nullptr;
        }
        
        if (window) {
            SDL_DestroyWindow(window);
            window = nullptr;
        }
        
        SDL_Quit();
    }
};

// Window procedure to handle native Windows messages
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    InstallerWindow* window = reinterpret_cast<InstallerWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    
    switch (uMsg) {
        case WM_SYSCOMMAND: {
            switch (wParam & 0xFFF0) {
                case SC_CLOSE:
                    if (window) {
                        window->running = false;
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
    if (window && window->originalWndProc) {
        return CallWindowProc(window->originalWndProc, hwnd, uMsg, wParam, lParam);
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int main(int argc, char* argv[]) {
    InstallerWindow app;
    
    if (!app.initialize()) {
        std::cerr << "Failed to initialize installer!" << std::endl;
        return -1;
    }
    
    std::cout << "MikoIDE Installer started" << std::endl;
    std::cout << "Window size: 800x533" << std::endl;
    
    app.run();
    
    return 0;
}