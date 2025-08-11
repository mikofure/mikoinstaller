#include "system_utils.h"
#include <windows.h>
#include <shlobj.h>
#include <filesystem>
#include <algorithm>
#include <sstream>

namespace Utils {
    
    bool CreateDirectoryRecursive(const std::string& path) {
        try {
            return std::filesystem::create_directories(path);
        } catch (const std::exception&) {
            return false;
        }
    }
    
    bool FileExists(const std::string& path) {
        return std::filesystem::exists(path) && std::filesystem::is_regular_file(path);
    }
    
    bool DirectoryExists(const std::string& path) {
        return std::filesystem::exists(path) && std::filesystem::is_directory(path);
    }
    
    std::string ToLower(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }
    
    std::string ToUpper(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        return result;
    }
    
    std::vector<std::string> Split(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::stringstream ss(str);
        std::string token;
        
        while (std::getline(ss, token, delimiter)) {
            tokens.push_back(token);
        }
        
        return tokens;
    }
    
    std::string Trim(const std::string& str) {
        size_t start = str.find_first_not_of(" \t\n\r\f\v");
        if (start == std::string::npos) return "";
        
        size_t end = str.find_last_not_of(" \t\n\r\f\v");
        return str.substr(start, end - start + 1);
    }
    
    std::string GetSystemInfo() {
        OSVERSIONINFOEX osvi;
        ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        
        // Note: GetVersionEx is deprecated, but still works for basic info
        if (GetVersionEx((OSVERSIONINFO*)&osvi)) {
            return "Windows " + std::to_string(osvi.dwMajorVersion) + "." + 
                   std::to_string(osvi.dwMinorVersion);
        }
        
        return "Windows (Unknown Version)";
    }
    
    bool IsAdmin() {
        BOOL isAdmin = FALSE;
        PSID adminGroup = NULL;
        
        SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
        if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                   DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
            CheckTokenMembership(NULL, adminGroup, &isAdmin);
            FreeSid(adminGroup);
        }
        
        return isAdmin == TRUE;
    }
    
    bool RunAsAdmin(const std::string& program, const std::string& args) {
        SHELLEXECUTEINFOA sei = { sizeof(sei) };
        sei.lpVerb = "runas";
        sei.lpFile = program.c_str();
        sei.lpParameters = args.empty() ? NULL : args.c_str();
        sei.hwnd = NULL;
        sei.nShow = SW_NORMAL;
        
        return ShellExecuteExA(&sei) == TRUE;
    }
    
    bool SetRegistryValue(const std::string& keyPath, const std::string& valueName, 
                         const std::string& value) {
        HKEY hKey;
        DWORD disposition;
        
        if (RegCreateKeyExA(HKEY_CURRENT_USER, keyPath.c_str(), 0, NULL, 
                           REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &disposition) == ERROR_SUCCESS) {
            
            LONG result = RegSetValueExA(hKey, valueName.c_str(), 0, REG_SZ, 
                                       (const BYTE*)value.c_str(), (DWORD)value.length() + 1);
            RegCloseKey(hKey);
            return result == ERROR_SUCCESS;
        }
        
        return false;
    }
    
    std::string GetRegistryValue(const std::string& keyPath, const std::string& valueName) {
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_CURRENT_USER, keyPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            char buffer[1024];
            DWORD bufferSize = sizeof(buffer);
            DWORD type;
            
            if (RegQueryValueExA(hKey, valueName.c_str(), NULL, &type, 
                               (LPBYTE)buffer, &bufferSize) == ERROR_SUCCESS && type == REG_SZ) {
                RegCloseKey(hKey);
                return std::string(buffer);
            }
            
            RegCloseKey(hKey);
        }
        
        return "";
    }
    
    bool AddToPath(const std::string& path) {
        std::string currentPath = GetEnvironmentVariable("PATH");
        if (currentPath.find(path) != std::string::npos) {
            return true; // Already in PATH
        }
        
        std::string newPath = currentPath + ";" + path;
        return SetEnvironmentVariable("PATH", newPath);
    }
    
    bool RemoveFromPath(const std::string& path) {
        std::string currentPath = GetEnvironmentVariable("PATH");
        size_t pos = currentPath.find(path);
        
        if (pos != std::string::npos) {
            // Remove the path and any adjacent semicolons
            currentPath.erase(pos, path.length());
            
            // Clean up semicolons
            if (pos > 0 && currentPath[pos - 1] == ';' && 
                pos < currentPath.length() && currentPath[pos] == ';') {
                currentPath.erase(pos, 1);
            }
            
            return SetEnvironmentVariable("PATH", currentPath);
        }
        
        return true; // Not found, nothing to remove
    }
    
    std::string GetEnvironmentVariable(const std::string& name) {
        char buffer[32768]; // Windows max environment variable size
        DWORD result = GetEnvironmentVariableA(name.c_str(), buffer, sizeof(buffer));
        
        if (result > 0 && result < sizeof(buffer)) {
            return std::string(buffer);
        }
        
        return "";
    }
    
    bool SetEnvironmentVariable(const std::string& name, const std::string& value) {
        return SetEnvironmentVariableA(name.c_str(), value.c_str()) != 0;
    }
}
