#include <iostream>
#include "UninstallerWindow.h"

int main(int argc, char* argv[]) {
    UninstallerWindow app;
    if (!app.initialize()) {
        std::cerr << "Failed to initialize uninstaller!" << std::endl;
        return -1;
    }
    std::cout << "MikoIDE Uninstaller started" << std::endl;
    app.run();
    return 0;
}
