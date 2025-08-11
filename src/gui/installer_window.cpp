#include "installer_window.h"
#include "window_utils.h"
#include "../config/app_config.h"
#include "../fonts/InterVariable.h"
#include "../images/banner.h"
#include "../framework/nuklear_sdl_renderer.h"
#include <iostream>
#include <commdlg.h>
#include <shlobj.h>
#include <shellapi.h>

InstallerWindow::InstallerWindow() 
    : window(nullptr)
    , renderer(nullptr)
    , ctx(nullptr)
    , font(nullptr)
    , iconFont(nullptr)
    , backgroundTexture(nullptr)
    , hwnd(nullptr)
    , originalWndProc(nullptr)
    , running(false)
    , installPath(WindowUtils::GetDefaultInstallPath())
    , addToPath(nk_true)
    , assignFileExtension(nk_true)
    , isMaximized(false)
    , windowRect{}
    , normalRect{}
{
}

InstallerWindow::~InstallerWindow() {
    Cleanup();
}

bool InstallerWindow::Initialize() {
    if (!InitializeSDL()) return false;
    if (!InitializeWindow()) return false;
    if (!InitializeRenderer()) return false;
    if (!LoadBackgroundImage()) return false;
    if (!InitializeNuklear()) return false;
    
    SetupWindowsIntegration();
    SetupCustomStyle();
    
    // Initialize window state
    isMaximized = false;
    GetWindowRect(hwnd, &normalRect);
    
    running = true;
    return true;
}

bool InstallerWindow::InitializeSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}

bool InstallerWindow::InitializeWindow() {
    window = SDL_CreateWindow(APP_TITLE,
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            WINDOW_WIDTH, WINDOW_HEIGHT,
                            SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS);
    
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}

bool InstallerWindow::InitializeRenderer() {
    int flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
    renderer = SDL_CreateRenderer(window, -1, flags);
    
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    // Scale renderer for High-DPI displays
    int render_w, render_h;
    int window_w, window_h;
    float scale_x, scale_y;
    SDL_GetRendererOutputSize(renderer, &render_w, &render_h);
    SDL_GetWindowSize(window, &window_w, &window_h);
    scale_x = (float)(render_w) / (float)(window_w);
    scale_y = (float)(render_h) / (float)(window_h);
    SDL_RenderSetScale(renderer, scale_x, scale_y);
    
    return true;
}

bool InstallerWindow::InitializeNuklear() {
    ctx = nk_sdl_init(window, renderer);
    
    // Load custom fonts
    struct nk_font_atlas *atlas;
    struct nk_font_config config = nk_font_config(0);
    
    nk_sdl_font_stash_begin(&atlas);
    
    // Load InterVariable font from memory
    config.oversample_h = 2;
    config.oversample_v = 2;
    config.pixel_snap = true;
    font = nk_font_atlas_add_from_memory(atlas, (void*)InterVariable_ttf, sizeof(InterVariable_ttf), 16, &config);
    
    // Load Segoe Fluent Icons for window controls (optional)
    struct nk_font_config iconConfig = nk_font_config(0);
    iconConfig.oversample_h = 1;
    iconConfig.oversample_v = 1;
    iconConfig.pixel_snap = true;
    // Use default range for now
    iconFont = nk_font_atlas_add_default(atlas, 16, &iconConfig);
    
    nk_sdl_font_stash_end();
    
    if (font) {
        nk_style_set_font(ctx, &font->handle);
    }
    
    return true;
}

bool InstallerWindow::LoadBackgroundImage() {
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

void InstallerWindow::SetupWindowsIntegration() {
    // Get native window handle
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if (SDL_GetWindowWMInfo(window, &wmInfo)) {
        hwnd = wmInfo.info.win.window;
        
        // Store reference to this instance for window procedure
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        
        // Subclass the window to handle WM_SYSCOMMAND messages
        originalWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProc)));
        
        WindowUtils::ApplyNativeEffects(hwnd);
        WindowUtils::SetupWindowStyle(hwnd);
    }
}

void InstallerWindow::SetupCustomStyle() {
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

void InstallerWindow::HandleWindowControls(int mouseX, int mouseY, bool clicked) {
    // Define button areas in the custom titlebar
    int buttonWidth = CONTROL_BUTTON_WIDTH;
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

void InstallerWindow::OpenFolderDialog() {
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
            installPath = std::string(narrowPath) + "\\" + DEFAULT_INSTALL_SUBDIR;
        }
        
        IMalloc* imalloc = 0;
        if (SUCCEEDED(SHGetMalloc(&imalloc))) {
            imalloc->Free(pidl);
            imalloc->Release();
        }
    }
}

void InstallerWindow::PerformInstallation() {
    std::cout << "Installing " << DEFAULT_INSTALL_SUBDIR << " to: " << installPath << std::endl;
    std::cout << "Add to PATH: " << (addToPath ? "Yes" : "No") << std::endl;
    std::cout << "Assign file extension: " << (assignFileExtension ? "Yes" : "No") << std::endl;
    
    // Here you would implement the actual installation logic
    // For now, just show a message
    std::cout << "Installation completed successfully!" << std::endl;
    
    running = false;
}

void InstallerWindow::HandleEvents() {
    SDL_Event e;
    bool dragging = false;
    int dragStartX = 0, dragStartY = 0;
    
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
                    HandleWindowControls(mouseX, mouseY, true);
                    
                    // Start dragging if not on control buttons
                    if (mouseX < WINDOW_WIDTH - (CONTROL_BUTTON_COUNT * CONTROL_BUTTON_WIDTH + 2)) {
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
        
        nk_sdl_handle_event(&e);
    }
    nk_sdl_handle_grab();
    nk_input_end(ctx);
}

void InstallerWindow::RenderTitleBar() {
    // Custom transparent titlebar
    struct nk_style_window titlebar_style = ctx->style.window;
    ctx->style.window.fixed_background = nk_style_item_color(nk_rgba(0, 0, 0, 0)); // Fully transparent
    ctx->style.window.padding = nk_vec2(10, 4);
    
    if (nk_begin(ctx, "Titlebar", nk_rect(0, 0, WINDOW_WIDTH, TITLEBAR_HEIGHT),
                NK_WINDOW_NO_SCROLLBAR)) {
        
        nk_layout_row_begin(ctx, NK_STATIC, TITLEBAR_HEIGHT - 4, 1);
        
        // App icon and title
        nk_layout_row_push(ctx, WINDOW_WIDTH - (CONTROL_BUTTON_COUNT * CONTROL_BUTTON_WIDTH + 2));
        nk_label(ctx, APP_TITLE, NK_TEXT_LEFT);
        
        nk_layout_row_end(ctx);
    }
    nk_end(ctx);
    
    // Restore original window style
    ctx->style.window = titlebar_style;
}

void InstallerWindow::RenderInstallerPanel() {
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
            OpenFolderDialog();
        }
        nk_layout_row_end(ctx);
        
        // Checkboxes row
        nk_layout_row_dynamic(ctx, 25, 2);
        nk_checkbox_label(ctx, ("add " + std::string(DEFAULT_INSTALL_SUBDIR) + " to environment path").c_str(), &addToPath);
        nk_checkbox_label(ctx, "assign file extension", &assignFileExtension);
        
        // Install button row
        nk_layout_row_dynamic(ctx, 35, 1);
        if (nk_button_label(ctx, "Install")) {
            PerformInstallation();
        }
    }
    nk_end(ctx);
}

void InstallerWindow::RenderFrame() {
    // Clear background with black color
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    // Render background image (full height, overlapped by titlebar)
    if (backgroundTexture) {
        SDL_Rect imageRect = {0, 0, WINDOW_WIDTH, IMAGE_HEIGHT + TITLEBAR_HEIGHT};
        SDL_RenderCopy(renderer, backgroundTexture, NULL, &imageRect);
    }
    
    RenderTitleBar();
    RenderInstallerPanel();
    
    // Render
    nk_sdl_render(NK_ANTI_ALIASING_ON);
    SDL_RenderPresent(renderer);
}

void InstallerWindow::Run() {
    while (running) {
        HandleEvents();
        RenderFrame();
    }
}

void InstallerWindow::Cleanup() {
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
