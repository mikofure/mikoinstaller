#include <iostream>
#include "gui/installer_window.h"
#include "config/app_config.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_BUTTON_TRIGGER_ON_RELEASE
#define NK_IMPLEMENTATION
#define NK_SDL_RENDERER_IMPLEMENTATION
#include "framework/nuklear.h"
#include "framework/nuklear_sdl_renderer.h"

int main(int argc, char* argv[]) {
    InstallerWindow app;
    
    if (!app.Initialize()) {
        std::cerr << "Failed to initialize installer!" << std::endl;
        return -1;
    }
    
    std::cout << "MikoIDE Installer started" << std::endl;
    std::cout << "Window size: " << WINDOW_WIDTH << "x" << WINDOW_HEIGHT << std::endl;
    
    app.Run();
    
    return 0;
}