#include "installation_manager.h"
#include "../utils/system_utils.h"
#include "../config/app_config.h"
#include <iostream>
#include <fstream>
#include <windows.h>
#include <shlobj.h>
#include <comdef.h>

InstallationManager::InstallationManager() 
    : currentStep(InstallationStep::Preparing)
    , installing(false)
    , cancelled(false)
{
}

InstallationManager::~InstallationManager() {
}

void InstallationManager::SetProgressCallback(ProgressCallback callback) {
    progressCallback = callback;
}

bool InstallationManager::Install(const InstallationOptions& opts) {
    if (installing) {
        SetError("Installation already in progress");
        return false;
    }
    
    options = opts;
    installing = true;
    cancelled = false;
    currentStep = InstallationStep::Preparing;
    
    try {
        if (!PrepareInstallation()) return false;
        if (cancelled) return false;
        
        if (!ExtractFiles()) return false;
        if (cancelled) return false;
        
        if (!CreateDirectories()) return false;
        if (cancelled) return false;
        
        if (!CopyFiles()) return false;
        if (cancelled) return false;
        
        if (!UpdateRegistry()) return false;
        if (cancelled) return false;
        
        if (!CreateShortcuts()) return false;
        if (cancelled) return false;
        
        if (!UpdateEnvironment()) return false;
        if (cancelled) return false;
        
        if (!FinalizeInstallation()) return false;
        if (cancelled) return false;
        
        currentStep = InstallationStep::Complete;
        ReportProgress(currentStep, 100, "Installation completed successfully!");
        
    } catch (const std::exception& e) {
        SetError(std::string("Installation failed: ") + e.what());
        currentStep = InstallationStep::Error;
        installing = false;
        return false;
    }
    
    installing = false;
    return true;
}

void InstallationManager::Cancel() {
    cancelled = true;
}

bool InstallationManager::PrepareInstallation() {
    ReportProgress(InstallationStep::Preparing, 0, "Preparing installation...");
    
    // Check if target directory exists and is writable
    if (!Utils::DirectoryExists(options.installPath)) {
        if (!Utils::CreateDirectoryRecursive(options.installPath)) {
            SetError("Cannot create installation directory: " + options.installPath);
            return false;
        }
    }
    
    // Check available disk space (basic check)
    ULARGE_INTEGER freeBytesAvailable;
    if (GetDiskFreeSpaceExA(options.installPath.c_str(), &freeBytesAvailable, NULL, NULL)) {
        // Assume we need at least 100MB for installation
        if (freeBytesAvailable.QuadPart < 100 * 1024 * 1024) {
            SetError("Insufficient disk space for installation");
            return false;
        }
    }
    
    ReportProgress(InstallationStep::Preparing, 100, "Preparation complete");
    return true;
}

bool InstallationManager::ExtractFiles() {
    ReportProgress(InstallationStep::ExtractingFiles, 0, "Extracting installation files...");
    
    // In a real installer, you would extract files from a compressed archive
    // For this example, we'll simulate the process
    
    std::vector<std::string> filesToExtract = {
        "mikoide.exe",
        "mikoide.dll",
        "config.json",
        "templates/template.miko",
        "libs/runtime.dll"
    };
    
    for (size_t i = 0; i < filesToExtract.size(); ++i) {
        if (cancelled) return false;
        
        int progress = (int)((i + 1) * 100 / filesToExtract.size());
        ReportProgress(InstallationStep::ExtractingFiles, progress, 
                      "Extracting " + filesToExtract[i] + "...");
        
        // Simulate extraction time
        Sleep(100);
    }
    
    return true;
}

bool InstallationManager::CreateDirectories() {
    ReportProgress(InstallationStep::CreatingDirectories, 0, "Creating directories...");
    
    std::vector<std::string> directories = {
        options.installPath,
        options.installPath + "\\bin",
        options.installPath + "\\lib",
        options.installPath + "\\templates",
        options.installPath + "\\config"
    };
    
    for (size_t i = 0; i < directories.size(); ++i) {
        if (cancelled) return false;
        
        if (!Utils::CreateDirectoryRecursive(directories[i])) {
            SetError("Failed to create directory: " + directories[i]);
            return false;
        }
        
        int progress = (int)((i + 1) * 100 / directories.size());
        ReportProgress(InstallationStep::CreatingDirectories, progress, 
                      "Created " + directories[i]);
    }
    
    return true;
}

bool InstallationManager::CopyFiles() {
    ReportProgress(InstallationStep::CopyingFiles, 0, "Copying files...");
    
    // In a real installer, you would copy files from a temporary location
    // or extract them from embedded resources
    
    std::vector<std::pair<std::string, std::string>> filesToCopy = {
        {"mikoide.exe", options.installPath + "\\bin\\mikoide.exe"},
        {"mikoide.dll", options.installPath + "\\bin\\mikoide.dll"},
        {"config.json", options.installPath + "\\config\\config.json"},
        {"template.miko", options.installPath + "\\templates\\template.miko"},
        {"runtime.dll", options.installPath + "\\lib\\runtime.dll"}
    };
    
    for (size_t i = 0; i < filesToCopy.size(); ++i) {
        if (cancelled) return false;
        
        const auto& [source, dest] = filesToCopy[i];
        
        // For demonstration, create empty files
        std::ofstream file(dest);
        if (!file.is_open()) {
            SetError("Failed to create file: " + dest);
            return false;
        }
        file.close();
        
        int progress = (int)((i + 1) * 100 / filesToCopy.size());
        ReportProgress(InstallationStep::CopyingFiles, progress, 
                      "Copied " + source + " to " + dest);
        
        Sleep(50); // Simulate copy time
    }
    
    return true;
}

bool InstallationManager::UpdateRegistry() {
    ReportProgress(InstallationStep::UpdatingRegistry, 0, "Updating registry...");
    
    // Register application in Windows
    std::string appRegPath = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" + std::string(DEFAULT_INSTALL_SUBDIR);
    
    if (!Utils::SetRegistryValue(appRegPath, "DisplayName", std::string(APP_NAME))) {
        SetError("Failed to update registry");
        return false;
    }
    
    ReportProgress(InstallationStep::UpdatingRegistry, 50, "Setting application info...");
    
    Utils::SetRegistryValue(appRegPath, "DisplayVersion", APP_VERSION);
    Utils::SetRegistryValue(appRegPath, "InstallLocation", options.installPath);
    Utils::SetRegistryValue(appRegPath, "UninstallString", options.installPath + "\\uninstall.exe");
    Utils::SetRegistryValue(appRegPath, "Publisher", "MikoIDE Team");
    
    // Register file associations if requested
    if (options.assignFileExtension) {
        ReportProgress(InstallationStep::UpdatingRegistry, 75, "Registering file associations...");
        
        Utils::SetRegistryValue("SOFTWARE\\Classes\\.miko", "", "MikoIDE.File");
        Utils::SetRegistryValue("SOFTWARE\\Classes\\MikoIDE.File", "", "MikoIDE File");
        Utils::SetRegistryValue("SOFTWARE\\Classes\\MikoIDE.File\\shell\\open\\command", "", 
                               "\"" + options.installPath + "\\bin\\mikoide.exe\" \"%1\"");
    }
    
    ReportProgress(InstallationStep::UpdatingRegistry, 100, "Registry update complete");
    return true;
}

bool InstallationManager::CreateShortcuts() {
    ReportProgress(InstallationStep::CreatingShortcuts, 0, "Creating shortcuts...");
    
    // Create start menu shortcut
    if (options.createStartMenuShortcut) {
        char startMenuPath[MAX_PATH];
        if (SHGetFolderPathA(NULL, CSIDL_STARTMENU, NULL, 0, startMenuPath) == S_OK) {
            std::string shortcutPath = std::string(startMenuPath) + "\\Programs\\" + DEFAULT_INSTALL_SUBDIR + ".lnk";
            std::string targetPath = options.installPath + "\\bin\\mikoide.exe";
            
            if (!CreateShortcut(shortcutPath, targetPath, "", APP_NAME)) {
                SetError("Failed to create start menu shortcut");
                return false;
            }
        }
        
        ReportProgress(InstallationStep::CreatingShortcuts, 50, "Created start menu shortcut");
    }
    
    // Create desktop shortcut
    if (options.createDesktopShortcut) {
        char desktopPath[MAX_PATH];
        if (SHGetFolderPathA(NULL, CSIDL_DESKTOP, NULL, 0, desktopPath) == S_OK) {
            std::string shortcutPath = std::string(desktopPath) + "\\" + DEFAULT_INSTALL_SUBDIR + ".lnk";
            std::string targetPath = options.installPath + "\\bin\\mikoide.exe";
            
            if (!CreateShortcut(shortcutPath, targetPath, "", APP_NAME)) {
                SetError("Failed to create desktop shortcut");
                return false;
            }
        }
        
        ReportProgress(InstallationStep::CreatingShortcuts, 100, "Created desktop shortcut");
    }
    
    return true;
}

bool InstallationManager::UpdateEnvironment() {
    ReportProgress(InstallationStep::UpdatingEnvironment, 0, "Updating environment...");
    
    if (options.addToPath) {
        std::string binPath = options.installPath + "\\bin";
        if (!Utils::AddToPath(binPath)) {
            SetError("Failed to add to PATH environment variable");
            return false;
        }
        
        ReportProgress(InstallationStep::UpdatingEnvironment, 100, "Added to PATH");
    }
    
    return true;
}

bool InstallationManager::FinalizeInstallation() {
    ReportProgress(InstallationStep::Finalizing, 0, "Finalizing installation...");
    
    // Create uninstaller
    std::string uninstallerPath = options.installPath + "\\uninstall.exe";
    std::ofstream uninstaller(uninstallerPath);
    if (uninstaller.is_open()) {
        uninstaller.close();
    }
    
    // Refresh shell icons and file associations
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
    
    ReportProgress(InstallationStep::Finalizing, 100, "Installation finalized");
    return true;
}

void InstallationManager::ReportProgress(InstallationStep step, int percentage, const std::string& message) {
    currentStep = step;
    if (progressCallback) {
        progressCallback(step, percentage, message);
    }
}

void InstallationManager::SetError(const std::string& error) {
    lastError = error;
    currentStep = InstallationStep::Error;
    installing = false;
}

bool InstallationManager::CreateShortcut(const std::string& shortcutPath, const std::string& targetPath, 
                                       const std::string& arguments, const std::string& description) {
    // This is a simplified implementation
    // In a real application, you would use COM interfaces to create proper shortcuts
    
    // For now, just create an empty file to represent the shortcut
    std::ofstream shortcut(shortcutPath);
    if (!shortcut.is_open()) {
        return false;
    }
    
    shortcut << "[Shortcut]" << std::endl;
    shortcut << "Target=" << targetPath << std::endl;
    shortcut << "Arguments=" << arguments << std::endl;
    shortcut << "Description=" << description << std::endl;
    shortcut.close();
    
    return true;
}
