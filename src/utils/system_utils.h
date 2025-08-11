#pragma once

#include <string>
#include <vector>

namespace Utils {
    // File system utilities
    bool CreateDirectoryRecursive(const std::string& path);
    bool FileExists(const std::string& path);
    bool DirectoryExists(const std::string& path);
    
    // String utilities  
    std::string ToLower(const std::string& str);
    std::string ToUpper(const std::string& str);
    std::vector<std::string> Split(const std::string& str, char delimiter);
    std::string Trim(const std::string& str);
    
    // System utilities
    std::string GetSystemInfo();
    bool IsAdmin();
    bool RunAsAdmin(const std::string& program, const std::string& args = "");
    
    // Registry utilities (Windows specific)
    bool SetRegistryValue(const std::string& keyPath, const std::string& valueName, 
                         const std::string& value);
    std::string GetRegistryValue(const std::string& keyPath, const std::string& valueName);
    
    // Environment utilities
    bool AddToPath(const std::string& path);
    bool RemoveFromPath(const std::string& path);
    std::string GetEnvironmentVariable(const std::string& name);
    bool SetEnvironmentVariable(const std::string& name, const std::string& value);
}
