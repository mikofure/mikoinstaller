#include <iostream>
#include "config/app_config.h"

// Include just the header for main.cpp - no implementation
#include "nuklear.h"
#include "framework/nuklear_sdl_renderer.h"

#include "gui/installer_window.h"

int main(int argc, char* argv[]) {
    std::cout << "DEBUG: Starting main function" << std::endl;
    
    InstallerWindow app;
    std::cout << "DEBUG: InstallerWindow created" << std::endl;
    
    if (!app.Initialize()) {
        std::cerr << "Failed to initialize installer!" << std::endl;
        return -1;
    }
    
    std::cout << "MikoIDE Installer started" << std::endl;
    std::cout << "Window size: " << WINDOW_WIDTH << "x" << WINDOW_HEIGHT << std::endl;
    
    app.Run();
    
    return 0;
}