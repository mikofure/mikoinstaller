#pragma once
#include <SDL.h>
#include <SDL_syswm.h>
#include <string>
#include <windows.h>

// Ensure Nuklear declarations for fonts and vertex buffers are visible
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include "framework/nuklear.h"
#include "format.h"
#include <atomic>
#include <thread>

class InstallerWindow {
public:
    InstallerWindow();
    ~InstallerWindow();

    bool initialize();
    void run();
    void cleanup();
    void setConfigPath(const std::string& path) { configPath = path; }
    int getExitCode() const { return exitCode; }
    void cancel() { exitCode = 2; running = false; }
    void setProgressFile(const std::string& path) { progressFile = path; progressMode = !path.empty(); }

    // Public state accessed by WindowProc
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

    // Install workflow state
    bool isInstalling = false;
    bool installDone = false;
    std::atomic<float> installProgress = 0.0f; // 0..1
    Uint32 installStartTicks = 0;
    Uint32 installDurationMs = 0; // simulated duration

    std::thread worker;
    std::atomic<bool> workerFinished = false;

    // Window control state
    bool isMaximized;
    RECT windowRect;
    RECT normalRect;

    // Config & process
    std::string configPath;
    int exitCode = 1; // 0=success, non-zero=cancel/error
    bool progressMode = false;
    std::string progressFile;

private:
    std::string getExpandedInstallPath();
    void setupCustomStyle();
    bool loadBackgroundImage();
    void handleWindowControls(int mouseX, int mouseY, bool clicked);
    void openFolderDialog();
    void performInstallation();
    void startExtractionAsync();
    bool hasEmbeddedPayload() const;
};

// Window procedure to handle native Windows messages
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
