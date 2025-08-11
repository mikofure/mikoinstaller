#pragma once

#include <string>
#include <vector>
#include <functional>

struct InstallationOptions {
    std::string installPath;
    bool addToPath;
    bool assignFileExtension;
    bool createDesktopShortcut;
    bool createStartMenuShortcut;
};

enum class InstallationStep {
    Preparing,
    ExtractingFiles,
    CreatingDirectories,
    CopyingFiles,
    UpdatingRegistry,
    CreatingShortcuts,
    UpdatingEnvironment,
    Finalizing,
    Complete,
    Error
};

class InstallationManager {
public:
    using ProgressCallback = std::function<void(InstallationStep step, int percentage, const std::string& message)>;
    
    InstallationManager();
    ~InstallationManager();
    
    // Main installation method
    bool Install(const InstallationOptions& options);
    
    // Set progress callback for UI updates
    void SetProgressCallback(ProgressCallback callback);
    
    // Get current installation step
    InstallationStep GetCurrentStep() const { return currentStep; }
    
    // Get last error message
    std::string GetLastError() const { return lastError; }
    
    // Check if installation is in progress
    bool IsInstalling() const { return installing; }
    
    // Cancel installation (if possible)
    void Cancel();

private:
    InstallationOptions options;
    ProgressCallback progressCallback;
    InstallationStep currentStep;
    std::string lastError;
    bool installing;
    bool cancelled;
    
    // Installation steps
    bool PrepareInstallation();
    bool ExtractFiles();
    bool CreateDirectories();
    bool CopyFiles();
    bool UpdateRegistry();
    bool CreateShortcuts();
    bool UpdateEnvironment();
    bool FinalizeInstallation();
    
    // Helper methods
    void ReportProgress(InstallationStep step, int percentage, const std::string& message);
    void SetError(const std::string& error);
    bool CreateShortcut(const std::string& shortcutPath, const std::string& targetPath, 
                       const std::string& arguments = "", const std::string& description = "");
};
