using System;
using System.IO;
using System.IO.Compression;
using System.Runtime.InteropServices;
using System.Text;
using Microsoft.Win32;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace MikoInstaller.Utils
{
    public static class WindowsAPI
    {
        #region Folder Dialog
        
        /// <summary>
        /// Opens a folder browser dialog to select installation directory
        /// </summary>
        /// <param name="initialPath">Initial path to show in dialog</param>
        /// <param name="description">Description text for the dialog</param>
        /// <returns>Selected folder path or null if cancelled</returns>
        public static string SelectFolder(string initialPath = "", string description = "Select installation folder:")
        {
            var dialog = new FolderBrowserDialog
            {
                Description = description,
                SelectedPath = initialPath
            };
            
            return dialog.ShowDialog() == DialogResult.OK ? dialog.SelectedPath : null;
        }
        
        #region Win32 Progress Dialog
        
        [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        private static extern IntPtr CreateWindowEx(
            uint dwExStyle,
            string lpClassName,
            string lpWindowName,
            uint dwStyle,
            int x, int y, int nWidth, int nHeight,
            IntPtr hWndParent,
            IntPtr hMenu,
            IntPtr hInstance,
            IntPtr lpParam);
            
        [DllImport("user32.dll", SetLastError = true)]
        private static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);
        
        [DllImport("user32.dll", SetLastError = true)]
        private static extern bool UpdateWindow(IntPtr hWnd);
        
        [DllImport("user32.dll", SetLastError = true)]
        private static extern bool DestroyWindow(IntPtr hWnd);
        
        [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        private static extern bool SetWindowText(IntPtr hWnd, string lpString);
        
        [DllImport("user32.dll", SetLastError = true)]
        private static extern bool SetWindowPos(IntPtr hWnd, IntPtr hWndInsertAfter, int x, int y, int cx, int cy, uint uFlags);
        
        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr GetModuleHandle(string lpModuleName);
        
        private const uint WS_OVERLAPPED = 0x00000000;
        private const uint WS_CAPTION = 0x00C00000;
        private const uint WS_SYSMENU = 0x00080000;
        private const uint WS_VISIBLE = 0x10000000;
        private const int SW_SHOW = 5;
        private const uint SWP_NOMOVE = 0x0002;
        private const uint SWP_NOSIZE = 0x0001;
        private static readonly IntPtr HWND_TOPMOST = new IntPtr(-1);
        
        /// <summary>
        /// Shows a simple Win32 progress dialog
        /// </summary>
        /// <param name="title">Dialog title</param>
        /// <param name="message">Initial message</param>
        /// <param name="cancellationToken">Token to cancel the dialog</param>
        /// <returns>Handle to the dialog window</returns>
        public static IntPtr ShowProgressDialog(string title, string message, CancellationToken cancellationToken = default)
        {
            try
            {
                var hInstance = GetModuleHandle(null);
                
                var hWnd = CreateWindowEx(
                    0,
                    "#32770", // Dialog class
                    title,
                    WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
                    (GetSystemMetrics(0) - 400) / 2, // Center horizontally
                    (GetSystemMetrics(1) - 150) / 2, // Center vertically
                    400, 150,
                    IntPtr.Zero,
                    IntPtr.Zero,
                    hInstance,
                    IntPtr.Zero);
                    
                if (hWnd != IntPtr.Zero)
                {
                    ShowWindow(hWnd, SW_SHOW);
                    UpdateWindow(hWnd);
                    SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                }
                
                return hWnd;
            }
            catch
            {
                return IntPtr.Zero;
            }
        }
        
        /// <summary>
        /// Updates the progress dialog message
        /// </summary>
        /// <param name="hWnd">Dialog window handle</param>
        /// <param name="message">New message</param>
        public static void UpdateProgressDialog(IntPtr hWnd, string message)
        {
            if (hWnd != IntPtr.Zero)
            {
                SetWindowText(hWnd, message);
                UpdateWindow(hWnd);
            }
        }
        
        /// <summary>
        /// Closes the progress dialog
        /// </summary>
        /// <param name="hWnd">Dialog window handle</param>
        public static void CloseProgressDialog(IntPtr hWnd)
        {
            if (hWnd != IntPtr.Zero)
            {
                DestroyWindow(hWnd);
            }
        }
        
        [DllImport("user32.dll")]
        private static extern int GetSystemMetrics(int nIndex);
        
        #endregion
        
        #endregion
        
        #region Disk Space Calculator
        
        [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Auto)]
        [return: MarshalAs(UnmanagedType.Bool)]
        static extern bool GetDiskFreeSpaceEx(string lpDirectoryName,
            out ulong lpFreeBytesAvailable,
            out ulong lpTotalNumberOfBytes,
            out ulong lpTotalNumberOfFreeBytes);

        public static bool GetDiskSpace(string path, out long freeBytes, out long totalBytes)
        {
            freeBytes = 0;
            totalBytes = 0;
            
            try
            {
                if (GetDiskFreeSpaceEx(path, out ulong freeBytesAvailable, out ulong totalNumberOfBytes, out ulong _))
                {
                    freeBytes = (long)freeBytesAvailable;
                    totalBytes = (long)totalNumberOfBytes;
                    return true;
                }
            }
            catch
            {
                // Fall back to .NET method if Win32 API fails
                try
                {
                    var drive = new DriveInfo(Path.GetPathRoot(path));
                    freeBytes = drive.AvailableFreeSpace;
                    totalBytes = drive.TotalSize;
                    return true;
                }
                catch
                {
                    return false;
                }
            }
            
            return false;
        }


        
        /// <summary>
        /// Gets available disk space for the specified path
        /// </summary>
        /// <param name="path">Path to check disk space for</param>
        /// <returns>Available space in bytes, or -1 if error</returns>
        public static long GetAvailableDiskSpace(string path)
        {
            try
            {
                string rootPath = Path.GetPathRoot(path) ?? "C:\\";
                
                if (GetDiskFreeSpaceEx(rootPath, out ulong freeBytesAvailable, out _, out _))
                {
                    return (long)freeBytesAvailable;
                }
                
                return -1;
            }
            catch
            {
                return -1;
            }
        }

        // Registry Operations
        public static bool CreateRegistryKey(string keyPath, string valueName, object value)
        {
            try
            {
                using (var key = Registry.LocalMachine.CreateSubKey(keyPath))
                {
                    if (key != null)
                    {
                        key.SetValue(valueName, value);
                        return true;
                    }
                }
            }
            catch
            {
                // Try current user if local machine fails
                try
                {
                    using (var key = Registry.CurrentUser.CreateSubKey(keyPath))
                    {
                        if (key != null)
                        {
                            key.SetValue(valueName, value);
                            return true;
                        }
                    }
                }
                catch
                {
                    return false;
                }
            }
            return false;
        }

        public static object GetRegistryValue(string keyPath, string valueName, object defaultValue = null)
        {
            try
            {
                using (var key = Registry.LocalMachine.OpenSubKey(keyPath))
                {
                    if (key != null)
                    {
                        return key.GetValue(valueName, defaultValue);
                    }
                }
            }
            catch
            {
                // Try current user if local machine fails
                try
                {
                    using (var key = Registry.CurrentUser.OpenSubKey(keyPath))
                    {
                        if (key != null)
                        {
                            return key.GetValue(valueName, defaultValue);
                        }
                    }
                }
                catch
                {
                    return defaultValue;
                }
            }
            return defaultValue;
        }

        public static bool DeleteRegistryKey(string keyPath)
        {
            try
            {
                Registry.LocalMachine.DeleteSubKeyTree(keyPath, false);
                return true;
            }
            catch
            {
                try
                {
                    Registry.CurrentUser.DeleteSubKeyTree(keyPath, false);
                    return true;
                }
                catch
                {
                    return false;
                }
            }
        }

        public static bool DeleteRegistryValue(string keyPath, string valueName)
        {
            try
            {
                using (var key = Registry.LocalMachine.OpenSubKey(keyPath, true))
                {
                    if (key != null)
                    {
                        key.DeleteValue(valueName, false);
                        return true;
                    }
                }
            }
            catch
            {
                try
                {
                    using (var key = Registry.CurrentUser.OpenSubKey(keyPath, true))
                    {
                        if (key != null)
                        {
                            key.DeleteValue(valueName, false);
                            return true;
                        }
                    }
                }
                catch
                {
                    return false;
                }
            }
            return false;
        }
        
        /// <summary>
        /// Formats bytes to human readable format (KB, MB, GB)
        /// </summary>
        /// <param name="bytes">Number of bytes</param>
        /// <returns>Formatted string</returns>
        public static string FormatBytes(long bytes)
        {
            if (bytes < 0) return "Unknown";
            
            string[] suffixes = { "B", "KB", "MB", "GB", "TB" };
            int suffixIndex = 0;
            double size = bytes;
            
            while (size >= 1024 && suffixIndex < suffixes.Length - 1)
            {
                size /= 1024;
                suffixIndex++;
            }
            
            return $"{size:F1} {suffixes[suffixIndex]}";
        }
        
        /// <summary>
        /// Checks if there's enough disk space for installation
        /// </summary>
        /// <param name="path">Installation path</param>
        /// <param name="requiredBytes">Required space in bytes</param>
        /// <returns>True if enough space available</returns>
        public static bool HasEnoughDiskSpace(string path, long requiredBytes)
        {
            long availableSpace = GetAvailableDiskSpace(path);
            return availableSpace >= requiredBytes;
        }
        
        #endregion
        
        #region Registry Operations
        
        /// <summary>
        /// Creates registry entry for installed application
        /// </summary>
        /// <param name="appName">Application name</param>
        /// <param name="installPath">Installation path</param>
        /// <param name="version">Application version</param>
        /// <param name="uninstallCommand">Uninstall command</param>
        public static bool CreateUninstallEntry(string appName, string installPath, string version, string uninstallCommand)
        {
            try
            {
                string keyPath = $@"SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{appName}";
                
                using var key = Registry.LocalMachine.CreateSubKey(keyPath);
                if (key != null)
                {
                    key.SetValue("DisplayName", appName);
                    key.SetValue("InstallLocation", installPath);
                    key.SetValue("DisplayVersion", version);
                    key.SetValue("UninstallString", uninstallCommand);
                    key.SetValue("Publisher", "MikoIDE");
                    key.SetValue("InstallDate", DateTime.Now.ToString("yyyyMMdd"));
                    key.SetValue("NoModify", 1, RegistryValueKind.DWord);
                    key.SetValue("NoRepair", 1, RegistryValueKind.DWord);
                    
                    return true;
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Registry error: {ex.Message}");
            }
            
            return false;
        }
        
        /// <summary>
        /// Removes registry entry for uninstalled application
        /// </summary>
        /// <param name="appName">Application name</param>
        public static bool RemoveUninstallEntry(string appName)
        {
            try
            {
                string keyPath = $@"SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{appName}";
                Registry.LocalMachine.DeleteSubKey(keyPath, false);
                return true;
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Registry removal error: {ex.Message}");
                return false;
            }
        }
        
        /// <summary>
        /// Checks if application is already installed
        /// </summary>
        /// <param name="appName">Application name</param>
        /// <returns>True if already installed</returns>
        public static bool IsApplicationInstalled(string appName)
        {
            try
            {
                string keyPath = $@"SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{appName}";
                using var key = Registry.LocalMachine.OpenSubKey(keyPath);
                return key != null;
            }
            catch
            {
                return false;
            }
        }
        
        #endregion
        
        #region File Extraction/Unzip
        
        /// <summary>
        /// Extracts a ZIP archive to the specified directory
        /// </summary>
        /// <param name="zipFilePath">Path to ZIP file</param>
        /// <param name="extractPath">Directory to extract to</param>
        /// <param name="overwrite">Whether to overwrite existing files</param>
        /// <returns>True if extraction successful</returns>
        public static bool ExtractZipFile(string zipFilePath, string extractPath, bool overwrite = true)
        {
            try
            {
                if (!File.Exists(zipFilePath))
                    return false;
                
                Directory.CreateDirectory(extractPath);
                
                using var archive = ZipFile.OpenRead(zipFilePath);
                foreach (var entry in archive.Entries)
                {
                    string destinationPath = Path.Combine(extractPath, entry.FullName);
                    
                    // Create directory if needed
                    string directoryPath = Path.GetDirectoryName(destinationPath);
                    if (!string.IsNullOrEmpty(directoryPath))
                    {
                        Directory.CreateDirectory(directoryPath);
                    }
                    
                    // Skip directories
                    if (entry.FullName.EndsWith("/") || entry.FullName.EndsWith("\\"))
                        continue;
                    
                    // Extract file
                    if (overwrite || !File.Exists(destinationPath))
                    {
                        entry.ExtractToFile(destinationPath, overwrite);
                    }
                }
                
                return true;
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Extraction error: {ex.Message}");
                return false;
            }
        }
        
        /// <summary>
        /// Gets the total uncompressed size of a ZIP archive
        /// </summary>
        /// <param name="zipFilePath">Path to ZIP file</param>
        /// <returns>Total uncompressed size in bytes, or -1 if error</returns>
        public static long GetZipUncompressedSize(string zipFilePath)
        {
            try
            {
                if (!File.Exists(zipFilePath))
                    return -1;
                
                using var archive = ZipFile.OpenRead(zipFilePath);
                long totalSize = 0;
                
                foreach (var entry in archive.Entries)
                {
                    totalSize += entry.Length;
                }
                
                return totalSize;
            }
            catch
            {
                return -1;
            }
        }
        
        /// <summary>
        /// Loads embedded resources into memory for faster access
        /// </summary>
        /// <returns>Dictionary containing resource names and their memory streams</returns>
        public static Dictionary<string, MemoryStream> LoadEmbeddedAssetsToMemory()
        {
            var assets = new Dictionary<string, MemoryStream>();
            var assembly = System.Reflection.Assembly.GetExecutingAssembly();
            
            try
            {
                // Load app.zip
                var appZipStream = assembly.GetManifestResourceStream("mikoinstaller.assets.app.zip");
                if (appZipStream != null)
                {
                    var appZipMemory = new MemoryStream();
                    appZipStream.CopyTo(appZipMemory);
                    appZipMemory.Position = 0;
                    assets["app.zip"] = appZipMemory;
                    appZipStream.Dispose();
                }
                
                // Load banner.png
                var bannerStream = assembly.GetManifestResourceStream("mikoinstaller.assets.banner.png");
                if (bannerStream != null)
                {
                    var bannerMemory = new MemoryStream();
                    bannerStream.CopyTo(bannerMemory);
                    bannerMemory.Position = 0;
                    assets["banner.png"] = bannerMemory;
                    bannerStream.Dispose();
                }
            }
            catch (Exception)
            {
                // Return what we have, even if some assets failed to load
            }
            
            return assets;
        }
        
        /// <summary>
        /// Extracts app.zip from memory stream to the specified directory
        /// </summary>
        /// <param name="appZipMemoryStream">Memory stream containing the app.zip data</param>
        /// <param name="extractPath">Directory to extract to</param>
        /// <param name="progressCallback">Callback for progress updates (0-100)</param>
        /// <param name="overwrite">Whether to overwrite existing files</param>
        /// <returns>True if extraction was successful</returns>
        public static bool ExtractAppZipFromMemory(MemoryStream appZipMemoryStream, string extractPath,
            Action<int> progressCallback = null, bool overwrite = true)
        {
            try
            {
                appZipMemoryStream.Position = 0;
                
                using (var archive = new ZipArchive(appZipMemoryStream, ZipArchiveMode.Read, true))
                {
                    var totalEntries = archive.Entries.Count;
                    var processedEntries = 0;
                    
                    foreach (var entry in archive.Entries)
                    {
                        var destinationPath = Path.Combine(extractPath, entry.FullName);
                        
                        if (entry.FullName.EndsWith("/"))
                        {
                            Directory.CreateDirectory(destinationPath);
                        }
                        else
                        {
                            Directory.CreateDirectory(Path.GetDirectoryName(destinationPath));
                            
                            if (!overwrite && File.Exists(destinationPath))
                            {
                                processedEntries++;
                                continue;
                            }
                            
                            entry.ExtractToFile(destinationPath, overwrite);
                        }
                        
                        processedEntries++;
                        progressCallback?.Invoke((processedEntries * 100) / totalEntries);
                    }
                }
                
                return true;
            }
            catch (Exception)
            {
                return false;
            }
        }
        
        /// <summary>
        /// Extracts the embedded app.zip resource to the specified directory
        /// </summary>
        /// <param name="extractPath">Directory to extract to</param>
        /// <param name="progressCallback">Callback for progress updates (0-100)</param>
        /// <param name="overwrite">Whether to overwrite existing files</param>
        /// <returns>True if extraction was successful</returns>
        public static bool ExtractEmbeddedAppZip(string extractPath,
            Action<int> progressCallback = null, bool overwrite = true)
        {
            try
            {
                var assembly = System.Reflection.Assembly.GetExecutingAssembly();
                var resourceName = "mikoinstaller.assets.app.zip";
                
                using (var stream = assembly.GetManifestResourceStream(resourceName))
                {
                    if (stream == null)
                    {
                        return false;
                    }
                    
                    using (var archive = new ZipArchive(stream, ZipArchiveMode.Read))
                    {
                        var totalEntries = archive.Entries.Count;
                        var processedEntries = 0;
                        
                        foreach (var entry in archive.Entries)
                        {
                            var destinationPath = Path.Combine(extractPath, entry.FullName);
                            
                            if (entry.FullName.EndsWith("/"))
                            {
                                Directory.CreateDirectory(destinationPath);
                            }
                            else
                            {
                                Directory.CreateDirectory(Path.GetDirectoryName(destinationPath));
                                
                                if (!overwrite && File.Exists(destinationPath))
                                {
                                    processedEntries++;
                                    continue;
                                }
                                
                                entry.ExtractToFile(destinationPath, overwrite);
                            }
                            
                            processedEntries++;
                            progressCallback?.Invoke((processedEntries * 100) / totalEntries);
                        }
                    }
                }
                
                return true;
            }
            catch (Exception)
            {
                return false;
            }
        }
        
        /// <summary>
        /// Extracts a ZIP file with progress reporting
        /// </summary>
        /// <param name="zipFilePath">Path to the ZIP file</param>
        /// <param name="extractPath">Directory to extract to</param>
        /// <param name="progressCallback">Callback for progress updates (0-100)</param>
        /// <param name="overwrite">Whether to overwrite existing files</param>
        /// <returns>True if extraction was successful</returns>
        public static bool ExtractZipFileWithProgress(string zipFilePath, string extractPath,
            Action<int> progressCallback = null, bool overwrite = true)
        {
            try
            {
                if (!File.Exists(zipFilePath))
                    return false;
                
                Directory.CreateDirectory(extractPath);
                
                using var archive = ZipFile.OpenRead(zipFilePath);
                int totalEntries = archive.Entries.Count;
                int currentEntry = 0;
                
                foreach (var entry in archive.Entries)
                {
                    string destinationPath = Path.Combine(extractPath, entry.FullName);
                    
                    // Create directory if needed
                    string directoryPath = Path.GetDirectoryName(destinationPath);
                    if (!string.IsNullOrEmpty(directoryPath))
                    {
                        Directory.CreateDirectory(directoryPath);
                    }
                    
                    // Skip directories
                    if (entry.FullName.EndsWith("/") || entry.FullName.EndsWith("\\"))
                    {
                        currentEntry++;
                        if (progressCallback != null)
                            progressCallback.Invoke((currentEntry * 100) / totalEntries);
                        continue;
                    }
                    
                    // Extract file
                    if (overwrite || !File.Exists(destinationPath))
                    {
                        entry.ExtractToFile(destinationPath, overwrite);
                    }
                    
                    currentEntry++;
                    if (progressCallback != null)
                        progressCallback.Invoke((currentEntry * 100) / totalEntries);
                }
                
                return true;
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Extraction error: {ex.Message}");
                return false;
            }
        }
        
        #endregion
    }
}