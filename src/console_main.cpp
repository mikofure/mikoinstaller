#include <iostream>
#include <string>
#include "config/app_config.h"
#include "utils/system_utils.h"
#include "installer/installation_manager.h"

int main(int argc, char* argv[]) {
    std::cout << "=== " << APP_TITLE << " ===" << std::endl;
    std::cout << "Version: " << APP_VERSION << std::endl;
    std::cout << std::endl;

    // Basic console installer for testing
    std::string installPath;
    char choice;
    bool addToPath = true;
    bool assignFileExtension = true;

    std::cout << "Default installation path: " << Utils::GetEnvVariable("LOCALAPPDATA") + "\\" + DEFAULT_INSTALL_SUBDIR << std::endl;
    std::cout << "Use default path? (y/n): ";
    std::cin >> choice;

    if (choice == 'y' || choice == 'Y') {
        installPath = Utils::GetEnvVariable("LOCALAPPDATA") + "\\" + DEFAULT_INSTALL_SUBDIR;
    } else {
        std::cout << "Enter installation path: ";
        std::cin.ignore();
        std::getline(std::cin, installPath);
    }

    std::cout << "Add to PATH? (y/n): ";
    std::cin >> choice;
    addToPath = (choice == 'y' || choice == 'Y');

    std::cout << "Assign file extension? (y/n): ";
    std::cin >> choice;
    assignFileExtension = (choice == 'y' || choice == 'Y');

    std::cout << std::endl;
    std::cout << "Installation Summary:" << std::endl;
    std::cout << "  Path: " << installPath << std::endl;
    std::cout << "  Add to PATH: " << (addToPath ? "Yes" : "No") << std::endl;
    std::cout << "  File association: " << (assignFileExtension ? "Yes" : "No") << std::endl;
    std::cout << std::endl;

    std::cout << "Proceed with installation? (y/n): ";
    std::cin >> choice;

    if (choice == 'y' || choice == 'Y') {
        std::cout << "Starting installation..." << std::endl;
        
        InstallationManager installer;
        InstallationOptions options;
        options.installPath = installPath;
        options.addToPath = addToPath;
        options.assignFileExtension = assignFileExtension;
        options.createDesktopShortcut = true;
        options.createStartMenuShortcut = true;

        // Set progress callback
        installer.SetProgressCallback([](InstallationStep step, int percentage, const std::string& message) {
            std::cout << "[" << percentage << "%] " << message << std::endl;
        });

        if (installer.Install(options)) {
            std::cout << std::endl << "Installation completed successfully!" << std::endl;
        } else {
            std::cout << std::endl << "Installation failed: " << installer.GetLastError() << std::endl;
            return 1;
        }
    } else {
        std::cout << "Installation cancelled." << std::endl;
    }

    std::cout << "Press Enter to exit...";
    std::cin.ignore();
    std::cin.get();
    return 0;
}
