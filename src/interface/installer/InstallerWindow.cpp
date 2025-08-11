#include "InstallerWindow.h"

#include <iostream>
#include <algorithm>
#include <commdlg.h>
#include <shlobj.h>
#include <shellapi.h>
#include <dwmapi.h>
#include <fstream>
#include <filesystem>
#include <vector>
#include <cstdint>
#include <sstream>
#ifdef HAVE_LZMA
#include <lzma.h>
#endif
#include "../../fonts/InterVariable.h"
#include "../../images/banner.h"
#include "../../framework/nuklear_sdl_renderer.h"

#pragma comment(lib, "dwmapi.lib")

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
#define WINDOW_HEIGHT 570
#define IMAGE_HEIGHT 365
#define PANEL_HEIGHT 168
#define TITLEBAR_HEIGHT 32

InstallerWindow::InstallerWindow() : window(nullptr), renderer(nullptr), ctx(nullptr), running(false),
                                     installPath(getExpandedInstallPath()),
                                     addToPath(nk_true), assignFileExtension(nk_true),
                                     font(nullptr), iconFont(nullptr), hwnd(nullptr), backgroundTexture(nullptr),
                                     isMaximized(false) {}

InstallerWindow::~InstallerWindow() {
    cleanup();
}

std::string InstallerWindow::getExpandedInstallPath() {
    char* localAppData = nullptr;
    size_t len = 0;
    errno_t err = _dupenv_s(&localAppData, &len, "LOCALAPPDATA");
    if (err == 0 && localAppData != nullptr) {
        std::string path = std::string(localAppData) + "\\MikoIDE";
        free(localAppData);
        return path;
    } else {
        return "C:\\Users\\Default\\AppData\\Local\\MikoIDE";
    }
}

bool InstallerWindow::initialize() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow("MikoIDE Installer",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              WINDOW_WIDTH, WINDOW_HEIGHT,
                              SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS);
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if (SDL_GetWindowWMInfo(window, &wmInfo)) {
        hwnd = wmInfo.info.win.window;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        originalWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProc)));

        MARGINS margins = {-1, -1, -1, -1};
        DwmExtendFrameIntoClientArea(hwnd, &margins);

        BOOL darkMode = TRUE;
        DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkMode, sizeof(darkMode));

        DWM_WINDOW_CORNER_PREFERENCE cornerPreference = DWMWCP_ROUND;
        DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &cornerPreference, sizeof(cornerPreference));

        DWM_SYSTEMBACKDROP_TYPE backdropType = DWMSBT_MAINWINDOW;
        DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdropType, sizeof(backdropType));

        COLORREF captionColor = RGB(32, 32, 32);
        DwmSetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, &captionColor, sizeof(captionColor));

        COLORREF borderColor = RGB(64, 64, 64);
        DwmSetWindowAttribute(hwnd, DWMWA_BORDER_COLOR, &borderColor, sizeof(borderColor));

        BOOL disableTransitions = FALSE;
        DwmSetWindowAttribute(hwnd, DWMWA_TRANSITIONS_FORCEDISABLED, &disableTransitions, sizeof(disableTransitions));

        BOOL enableComposition = TRUE;
        DwmEnableComposition(enableComposition);

        LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
        style |= WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU;
        SetWindowLongPtr(hwnd, GWL_STYLE, style);
    }

    int flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
    renderer = SDL_CreateRenderer(window, -1, flags);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

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

    if (!loadBackgroundImage()) {
        std::cerr << "Failed to load background image" << std::endl;
        return false;
    }

    ctx = nk_sdl_init(window, renderer);

    {
        struct nk_font_atlas *atlas;
        struct nk_font_config config = nk_font_config(0);

        nk_sdl_font_stash_begin(&atlas);
        config.oversample_h = 2;
        config.oversample_v = 2;
        config.pixel_snap = true;
        font = nk_font_atlas_add_from_memory(atlas, (void*)InterVariable_ttf, sizeof(InterVariable_ttf), 16, &config);

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

    setupCustomStyle();

    isMaximized = false;
    GetWindowRect(hwnd, &normalRect);

    running = true;
    if (progressMode && !progressFile.empty()) {
        isInstalling = true;
        installDone = false;
        installProgress.store(0.0f);
    }
    exitCode = 1; // not completed yet
    return true;
}

void InstallerWindow::setupCustomStyle() {
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
    s->button.border = 0;
    s->button.rounding = 4;
    s->button.padding = nk_vec2(8, 8);

    s->text.color = nk_rgba(255, 255, 255, 255);

    s->edit.normal = nk_style_item_color(nk_rgba(40, 40, 40, 255));
    s->edit.hover = nk_style_item_color(nk_rgba(50, 50, 50, 255));
    s->edit.active = nk_style_item_color(nk_rgba(60, 60, 60, 255));
    s->edit.text_normal = nk_rgba(255, 255, 255, 255);
    s->edit.text_hover = nk_rgba(255, 255, 255, 255);
    s->edit.text_active = nk_rgba(255, 255, 255, 255);
    s->edit.border = 1;
    s->edit.border_color = nk_rgba(100, 100, 100, 255);
    s->edit.rounding = 4;

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

bool InstallerWindow::loadBackgroundImage() {
    SDL_RWops* rw = SDL_RWFromConstMem(__image_bmp, __image_bmp_len);
    if (!rw) {
        std::cerr << "Failed to create RWops from binary data: " << SDL_GetError() << std::endl;
        return false;
    }
    SDL_Surface* surface = SDL_LoadBMP_RW(rw, 1);
    if (!surface) {
        std::cerr << "Failed to load BMP from binary data: " << SDL_GetError() << std::endl;
        return false;
    }
    backgroundTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!backgroundTexture) {
        std::cerr << "Failed to create texture: " << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}

void InstallerWindow::handleWindowControls(int mouseX, int mouseY, bool clicked) {
    int buttonWidth = 46;
    int buttonHeight = TITLEBAR_HEIGHT;

    SDL_Rect closeRect = {WINDOW_WIDTH - buttonWidth, 0, buttonWidth, buttonHeight};
    SDL_Rect maxRect = {WINDOW_WIDTH - buttonWidth * 2, 0, buttonWidth, buttonHeight};
    SDL_Rect minRect = {WINDOW_WIDTH - buttonWidth * 3, 0, buttonWidth, buttonHeight};

    if (clicked) {
        if (mouseX >= closeRect.x && mouseX < closeRect.x + closeRect.w &&
            mouseY >= closeRect.y && mouseY < closeRect.y + closeRect.h) {
            PostMessage(hwnd, WM_SYSCOMMAND, SC_CLOSE, 0);
        }
        else if (mouseX >= minRect.x && mouseX < minRect.x + minRect.w &&
                 mouseY >= minRect.y && mouseY < minRect.y + minRect.h) {
            PostMessage(hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
        }
        else if (mouseX >= maxRect.x && mouseX < maxRect.x + maxRect.w &&
                 mouseY >= maxRect.y && mouseY < maxRect.y + maxRect.h) {
            if (isMaximized) {
                PostMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
                isMaximized = false;
            } else {
                PostMessage(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
                isMaximized = true;
            }
            // Create shortcuts if main exe exists
            std::filesystem::path mainExe = std::filesystem::path(installPath) / "MikoIDE.exe";
            if (std::filesystem::exists(mainExe)) {
                // Start Menu
                PWSTR startMenuPath = nullptr;
                if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Programs, 0, NULL, &startMenuPath))) {
                    std::filesystem::path lnkDir = std::filesystem::path(startMenuPath) / "MikoIDE";
                    std::error_code ecMk;
                    std::filesystem::create_directories(lnkDir, ecMk);
                    std::filesystem::path lnkPath = lnkDir / "MikoIDE.lnk";
                    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
                    IShellLinkW* psl;
                    if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (LPVOID*)&psl))) {
                        psl->SetPath(std::wstring(mainExe.wstring()).c_str());
                        psl->SetWorkingDirectory(std::wstring(std::filesystem::path(installPath).wstring()).c_str());
                        IPersistFile* ppf;
                        if (SUCCEEDED(psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf))) {
                            ppf->Save(std::wstring(lnkPath.wstring()).c_str(), TRUE);
                            ppf->Release();
                        }
                        psl->Release();
                    }
                    CoUninitialize();
                    CoTaskMemFree(startMenuPath);
                }
                // Desktop
                PWSTR desktopPath = nullptr;
                if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Desktop, 0, NULL, &desktopPath))) {
                    std::filesystem::path lnkPath = std::filesystem::path(desktopPath) / "MikoIDE.lnk";
                    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
                    IShellLinkW* psl;
                    if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (LPVOID*)&psl))) {
                        psl->SetPath(std::wstring(mainExe.wstring()).c_str());
                        psl->SetWorkingDirectory(std::wstring(std::filesystem::path(installPath).wstring()).c_str());
                        IPersistFile* ppf;
                        if (SUCCEEDED(psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf))) {
                            ppf->Save(std::wstring(lnkPath.wstring()).c_str(), TRUE);
                            ppf->Release();
                        }
                        psl->Release();
                    }
                    CoUninitialize();
                    CoTaskMemFree(desktopPath);
                }
            }
        }
    }
}

void InstallerWindow::openFolderDialog() {
    BROWSEINFOW bi = { 0 };
    bi.lpszTitle = L"Select Installation Directory";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

    LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);

    if (pidl != 0) {
        wchar_t path[MAX_PATH];
        if (SHGetPathFromIDListW(pidl, path)) {
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

void InstallerWindow::performInstallation() {
    // Begin installation; if self-contained payload exists, extraction happens here.
    installProgress.store(0.0f);

    std::cout << "Installing MikoIDE to: " << installPath << std::endl;
    if (hasEmbeddedPayload()) {
        std::cout << "Found embedded payload. Extracting..." << std::endl;
        // Minimal streamed copy to buffer to simulate extraction work
        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);
        std::ifstream f(exePath, std::ios::binary);
        if (f) {
            f.seekg(0, std::ios::end);
            auto size = f.tellg();
            f.seekg(size - (std::streamoff)(8*3), std::ios::beg);
            uint64_t blob_size=0, meta_size=0, magic_off=0;
            f.read(reinterpret_cast<char*>(&blob_size), 8);
            f.read(reinterpret_cast<char*>(&meta_size), 8);
            f.read(reinterpret_cast<char*>(&magic_off), 8);
            f.seekg(magic_off + MIKO_MAGIC_LEN, std::ios::beg);
            char algo[MIKO_ALGO_LEN];
            f.read(algo, MIKO_ALGO_LEN);
            std::vector<uint8_t> blob(blob_size);
            f.read(reinterpret_cast<char*>(blob.data()), blob.size());
            std::vector<uint8_t> tar;
#ifdef HAVE_LZMA
            if (memcmp(algo, "LZMA", 4) == 0) {
                lzma_stream strm = LZMA_STREAM_INIT;
                if (lzma_auto_decoder(&strm, UINT64_MAX, 0) == LZMA_OK) {
                    strm.next_in = blob.data();
                    strm.avail_in = (size_t)blob.size();
                    std::vector<uint8_t> chunk(256 * 1024);
                    lzma_ret ret = LZMA_OK;
                    while (ret == LZMA_OK) {
                        tar.resize(tar.size() + chunk.size());
                        strm.next_out = tar.data() + (tar.size() - chunk.size());
                        strm.avail_out = (size_t)chunk.size();
                        ret = lzma_code(&strm, LZMA_RUN);
                        size_t produced = chunk.size() - strm.avail_out;
                        tar.resize(tar.size() - chunk.size() + produced);
                        // UI: bump progress a bit to avoid perceived freeze during long decompression
                        float cur = installProgress.load();
                        installProgress.store(std::min(0.90f, cur + 0.02f));
                    }
                    if (ret != LZMA_STREAM_END) {
                        tar.clear();
                    }
                    lzma_end(&strm);
                }
            }
#endif
            // Fallback: assume already raw tar if decoding failed
            if (tar.empty()) {
                tar.assign(blob.begin(), blob.end());
            }
            // Extract tar entries
            std::error_code ec;
            std::filesystem::create_directories(installPath, ec);
            auto rd = [&](const void* p, size_t n)->uint64_t { return 0; };
            // Minimal tar read loop
            size_t off = 0; size_t total = tar.size();
            auto clamp = [](long long v, long long lo, long long hi){ return v<lo?lo:(v>hi?hi:v); };
            while (off + 512 <= total) {
                const uint8_t* hdr = tar.data() + off;
                bool empty = true; for (int i=0;i<512;i++){ if(hdr[i]){ empty=false; break;} }
                if (empty) break;
                char name[101]={0}; memcpy(name, hdr, 100);
                char size_oct[13]={0}; memcpy(size_oct, hdr+124, 12);
                size_t fsize = strtoull(size_oct, nullptr, 8);
                size_t data_off = off + 512;
                std::filesystem::path out = std::filesystem::path(installPath) / name;
                std::string nstr(name);
                if (!nstr.empty() && nstr.back()=='/') {
                    std::filesystem::create_directories(out, ec);
                } else {
                    std::filesystem::create_directories(out.parent_path(), ec);
                    std::ofstream fo(out, std::ios::binary);
                    if (fo) {
                        fo.write((const char*)(tar.data()+data_off), fsize);
                    }
                }
                size_t blocks = (fsize + 511)/512;
                off = data_off + blocks*512;
                // update progress
                installProgress.store((float)clamp((long long)off*100LL/(long long)total, 0, 100) / 100.0f);
                // Progress is atomic and UI polls it on the main thread.
            }
        }
    } else {
        std::cout << "No embedded payload found; running in config UI mode." << std::endl;
    }
}

void InstallerWindow::startExtractionAsync() {
    if (worker.joinable()) {
        try { worker.join(); } catch(...) {}
    }
    workerFinished.store(false);
    worker = std::thread([this]() {
        try {
            performInstallation();
        } catch (...) {
            // TODO: capture error state
        }
        workerFinished.store(true);
    });
}

bool InstallerWindow::hasEmbeddedPayload() const {
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    std::ifstream f(exePath, std::ios::binary);
    if (!f) return false;
    f.seekg(0, std::ios::end);
    auto size = f.tellg();
    if (size < 8*3 + MIKO_MAGIC_LEN) return false;
    // Read trailer
    f.seekg(size - (std::streamoff)(8*3), std::ios::beg);
    uint64_t blob_size=0, meta_size=0, magic_off=0;
    f.read(reinterpret_cast<char*>(&blob_size), 8);
    f.read(reinterpret_cast<char*>(&meta_size), 8);
    f.read(reinterpret_cast<char*>(&magic_off), 8);
    if (magic_off + MIKO_MAGIC_LEN + MIKO_ALGO_LEN + blob_size + meta_size + 8*3 != (uint64_t)size) return false;
    f.seekg(magic_off, std::ios::beg);
    char mg[MIKO_MAGIC_LEN];
    f.read(mg, MIKO_MAGIC_LEN);
    if (memcmp(mg, MIKO_MAGIC, MIKO_MAGIC_LEN) != 0) return false;
    return true;
}

void InstallerWindow::run() {
    SDL_Event e;
    bool dragging = false;
    int dragStartX = 0, dragStartY = 0;

    while (running) {
        nk_input_begin(ctx);
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                exitCode = 2; // canceled/closed
                running = false;
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    int mouseX = e.button.x;
                    int mouseY = e.button.y;
                    if (mouseY < TITLEBAR_HEIGHT) {
                        handleWindowControls(mouseX, mouseY, true);
                        if (mouseX < WINDOW_WIDTH - 138) {
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

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (backgroundTexture) {
            SDL_Rect imageRect = {0, 0, WINDOW_WIDTH, IMAGE_HEIGHT + TITLEBAR_HEIGHT};
            SDL_RenderCopy(renderer, backgroundTexture, NULL, &imageRect);
        }

        struct nk_style_window titlebar_style = ctx->style.window;
        ctx->style.window.fixed_background = nk_style_item_color(nk_rgba(0, 0, 0, 0));
        ctx->style.window.padding = nk_vec2(10, 4);

        if (nk_begin(ctx, "Titlebar", nk_rect(0, 0, WINDOW_WIDTH, TITLEBAR_HEIGHT),
                    NK_WINDOW_NO_SCROLLBAR)) {
            nk_layout_row_begin(ctx, NK_STATIC, TITLEBAR_HEIGHT - 4, 3);
            nk_layout_row_push(ctx, WINDOW_WIDTH - 138 - 0);
            nk_label(ctx, "MikoIDE Installer 0.1.2", NK_TEXT_LEFT);
            nk_layout_row_end(ctx);
        }
        nk_end(ctx);

        ctx->style.window = titlebar_style;

        int panelY = IMAGE_HEIGHT + TITLEBAR_HEIGHT;
        if (nk_begin(ctx, "Installer Panel", nk_rect(0, panelY, WINDOW_WIDTH, PANEL_HEIGHT),
                    NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND)) {
            if (!isInstalling && !installDone) {
                nk_layout_row_dynamic(ctx, 25, 1);
                nk_label(ctx, "Install location", NK_TEXT_LEFT);

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

                nk_layout_row_dynamic(ctx, 25, 2);
                nk_checkbox_label(ctx, "add MikoIDE to environment path", &addToPath);
                nk_checkbox_label(ctx, "assign file extension", &assignFileExtension);

                nk_layout_row_dynamic(ctx, 35, 1);
                if (nk_button_label(ctx, "Install")) {
                    // Start non-blocking extraction on a worker thread
                    isInstalling = true;
                    installDone = false;
                    installStartTicks = SDL_GetTicks();
                    installProgress.store(0.0f);
                    workerFinished.store(false);
                    startExtractionAsync();
                }
            } else if (isInstalling && !installDone) {
                // Update progress
                if (progressMode && !progressFile.empty()) {
                    // Expect a simple INI-like file with lines: Progress=0..100 and Done=0/1
                    std::ifstream f(progressFile);
                    if (f.is_open()) {
                        std::string line;
                        int pct = -1; int done = 0;
                        while (std::getline(f, line)) {
                            if (line.rfind("Progress=", 0) == 0) {
                                try { pct = std::stoi(line.substr(9)); } catch (...) {}
                            } else if (line.rfind("Done=", 0) == 0) {
                                try { done = std::stoi(line.substr(5)); } catch (...) {}
                            }
                        }
                        if (pct >= 0) installProgress.store(std::max(0, std::min(100, pct)) / 100.0f);
                        if (done) {
                            workerFinished.store(true);
                        }
                    }
                }

                nk_layout_row_dynamic(ctx, 25, 1);
                nk_label(ctx, "Installing...", NK_TEXT_LEFT);
                nk_layout_row_dynamic(ctx, 22, 1);
                // Progress bar (text displays percent)
                int p = (int)(installProgress.load() * 100.0f + 0.5f);
                nk_progress(ctx, (nk_size*)&p, 100, 0);
                if (workerFinished.load()) {
                    isInstalling = false;
                    installDone = true;
                    installDurationMs = SDL_GetTicks() - installStartTicks;
                    std::cout << "Installation completed successfully!" << std::endl;
                }
            } else if (installDone) {
                nk_layout_row_dynamic(ctx, 25, 1);
                nk_label(ctx, "MikoIDE has been installed.", NK_TEXT_LEFT);
                nk_layout_row_dynamic(ctx, 35, 1);
                if (nk_button_label(ctx, "OK")) {
                    // Persist selection for NSIS via provided --config path or fallback
                    std::filesystem::path iniPath;
                    if (!configPath.empty()) {
                        iniPath = std::filesystem::path(configPath);
                    } else {
                        char exePath[MAX_PATH];
                        GetModuleFileNameA(NULL, exePath, MAX_PATH);
                        std::filesystem::path pth(exePath);
                        iniPath = pth.parent_path() / "install_config.ini";
                    }
                    std::ofstream ini(iniPath.string(), std::ios::trunc);
                    if (ini.is_open()) {
                        ini << "[Install]\n";
                        ini << "Dir=" << installPath << "\n";
                        ini.close();
                    }
                    exitCode = 0; // success
                    running = false;
                }
            }
        }
        nk_end(ctx);

        nk_sdl_render(NK_ANTI_ALIASING_ON);
        SDL_RenderPresent(renderer);
    }
}

void InstallerWindow::cleanup() {
    if (worker.joinable()) {
        try { worker.join(); } catch(...) {}
    }
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

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    InstallerWindow* window = reinterpret_cast<InstallerWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    switch (uMsg) {
        case WM_SYSCOMMAND: {
            switch (wParam & 0xFFF0) {
                case SC_CLOSE:
                    if (window) {
                        window->cancel();
                    }
                    return 0;
                case SC_MINIMIZE:
                case SC_MAXIMIZE:
                case SC_RESTORE:
                    break;
            }
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    if (window && window->originalWndProc) {
        return CallWindowProc(window->originalWndProc, hwnd, uMsg, wParam, lParam);
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
