// Thin entry point that uses the split InstallerWindow class.
#include <iostream>
#include "InstallerWindow.h"
#include <string>

int main(int argc, char* argv[]) {
    InstallerWindow app;
    // Parse --config <path> and --progress-file <path>
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--config" && i + 1 < argc) {
            app.setConfigPath(argv[++i]);
        } else if (arg == "--progress-file" && i + 1 < argc) {
            app.setProgressFile(argv[++i]);
        }
    }
    if (!app.initialize()) {
        std::cerr << "Failed to initialize installer!" << std::endl;
        return -1;
    }
    std::cout << "MikoIDE Installer started" << std::endl;
    std::cout << "Window size: 800x533" << std::endl;
    // If progress mode flagged, immediately start install UI state
    // so NSIS-driven progress is visible without clicking Install.
    // performInstallation is private; simulate by pressing Install via public flow not available,
    // instead we rely on NSIS launching UI after it already has config; we'll start showing progress
    // when it sets --progress-file.
    app.run();
    return app.getExitCode();
}