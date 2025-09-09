using System;
using System.IO;
using System.Xml;
using System.Xml.Serialization;
using System.Collections.Generic;

namespace MikoInstaller.Utils
{
    /// <summary>
    /// Configuration class for installer settings
    /// </summary>
    [XmlRoot("InstallerConfig")]
    public class Config
    {
        [XmlElement("ApplicationName")]
        public string ApplicationName { get; set; } = "MikoIDE";
        
        [XmlElement("Version")]
        public string Version { get; set; } = "1.0.0";
        
        [XmlElement("DefaultInstallPath")]
        public string DefaultInstallPath { get; set; } = @"$ProgramFiles\MikoIDE";
        
        /// <summary>
        /// Gets the resolved default install path with environment variables expanded
        /// </summary>
        public string ResolvedDefaultInstallPath => ExpandEnvironmentVariables(DefaultInstallPath);
        
        [XmlElement("RequiredDiskSpace")]
        public long RequiredDiskSpace { get; set; } = 100 * 1024 * 1024; // 100MB
        
        [XmlElement("RegistryPath")]
        public string RegistryPath { get; set; } = @"SOFTWARE\MikoIDE";
        
        [XmlElement("WindowTitle")]
        public string WindowTitle { get; set; } = "MikoInstaller Setup";
        
        [XmlElement("WindowWidth")]
        public int WindowWidth { get; set; } = 800;
        
        [XmlElement("WindowHeight")]
        public int WindowHeight { get; set; } = 320;
        
        [XmlElement("LicenseText")]
        public string LicenseText { get; set; } = "I agree to the license terms and conditions";
        
        [XmlArray("EmbeddedAssets")]
        [XmlArrayItem("Asset")]
        public List<EmbeddedAsset> EmbeddedAssets { get; set; } = new List<EmbeddedAsset>
        {
            new EmbeddedAsset { Name = "app.zip", ResourcePath = "mikoinstaller.assets.app.zip" },
            new EmbeddedAsset { Name = "banner.png", ResourcePath = "mikoinstaller.assets.banner.png" }
        };
        
        [XmlArray("RegistryEntries")]
        [XmlArrayItem("Entry")]
        public List<RegistryEntry> RegistryEntries { get; set; } = new List<RegistryEntry>
        {
            new RegistryEntry { Key = "InstallPath", ValueType = "String" },
            new RegistryEntry { Key = "Version", ValueType = "String" },
            new RegistryEntry { Key = "InstallDate", ValueType = "String" }
        };
        
        [XmlElement("UIStrings")]
        public UIStrings UIStrings { get; set; } = new UIStrings();
        
        /// <summary>
        /// Loads configuration from XML file
        /// </summary>
        /// <param name="filePath">Path to XML config file</param>
        /// <returns>Config instance</returns>
        public static Config LoadFromXml(string filePath)
        {
            try
            {
                if (!File.Exists(filePath))
                {
                    var defaultConfig = new Config();
                    defaultConfig.SaveToXml(filePath);
                    return defaultConfig;
                }
                
                var serializer = new XmlSerializer(typeof(Config));
                using (var reader = new FileStream(filePath, FileMode.Open))
                {
                    return (Config)serializer.Deserialize(reader);
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Failed to load config: {ex.Message}");
                return new Config(); // Return default config on error
            }
        }
        
        /// <summary>
        /// Saves configuration to XML file
        /// </summary>
        /// <param name="filePath">Path to save XML config file</param>
        public void SaveToXml(string filePath)
        {
            try
            {
                var directory = Path.GetDirectoryName(filePath);
                if (!string.IsNullOrEmpty(directory) && !Directory.Exists(directory))
                {
                    Directory.CreateDirectory(directory);
                }
                
                var serializer = new XmlSerializer(typeof(Config));
                var settings = new XmlWriterSettings
                {
                    Indent = true,
                    IndentChars = "  "
                };
                
                using (var writer = XmlWriter.Create(filePath, settings))
                {
                    serializer.Serialize(writer, this);
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Failed to save config: {ex.Message}");
            }
        }
        
        /// <summary>
        /// Loads configuration from embedded resource
        /// </summary>
        /// <param name="resourceName">Name of embedded XML resource</param>
        /// <returns>Config instance</returns>
        public static Config LoadFromEmbeddedResource(string resourceName = "mikoinstaller.config.xml")
        {
            try
            {
                var assembly = System.Reflection.Assembly.GetExecutingAssembly();
                using (var stream = assembly.GetManifestResourceStream(resourceName))
                {
                    if (stream != null)
                    {
                        var serializer = new XmlSerializer(typeof(Config));
                        return (Config)serializer.Deserialize(stream);
                    }
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine($"Failed to load embedded config: {ex.Message}");
            }
            
            return new Config(); // Return default config on error
        }
        
        /// <summary>
        /// Gets the current configuration instance (singleton pattern)
        /// </summary>
        public static Config Current { get; private set; } = new Config();
        
        /// <summary>
        /// Initializes the configuration from file or embedded resource
        /// </summary>
        /// <param name="configPath">Optional path to config file</param>
        public static void Initialize(string configPath = null)
        {
            if (!string.IsNullOrEmpty(configPath) && File.Exists(configPath))
            {
                Current = LoadFromXml(configPath);
            }
            else
            {
                Current = LoadFromEmbeddedResource();
            }
        }
        
        /// <summary>
        /// Expands custom environment variables in a path string
        /// </summary>
        /// <param name="path">Path containing environment variables</param>
        /// <returns>Expanded path</returns>
        public static string ExpandEnvironmentVariables(string path)
        {
            if (string.IsNullOrEmpty(path))
                return path;
                
            var expandedPath = path;
            
            // Replace custom environment variables
            expandedPath = expandedPath.Replace("$ProgramFiles", Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles));
            expandedPath = expandedPath.Replace("$ProgramFiles32", Environment.GetFolderPath(Environment.SpecialFolder.ProgramFilesX86));
            expandedPath = expandedPath.Replace("$LocalAppData", Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData));
            expandedPath = expandedPath.Replace("$CommonApp", Environment.GetFolderPath(Environment.SpecialFolder.CommonApplicationData));
            expandedPath = expandedPath.Replace("$CommonApp32", Environment.GetFolderPath(Environment.SpecialFolder.CommonApplicationData));
            
            // Also expand standard Windows environment variables
            expandedPath = Environment.ExpandEnvironmentVariables(expandedPath);
            
            return expandedPath;
        }
    }
    
    /// <summary>
    /// Represents an embedded asset configuration
    /// </summary>
    public class EmbeddedAsset
    {
        [XmlAttribute("name")]
        public string Name { get; set; }
        
        [XmlAttribute("resourcePath")]
        public string ResourcePath { get; set; }
    }
    
    /// <summary>
    /// Represents a registry entry configuration
    /// </summary>
    public class RegistryEntry
    {
        [XmlAttribute("key")]
        public string Key { get; set; }
        
        [XmlAttribute("valueType")]
        public string ValueType { get; set; }
        
        [XmlText]
        public string DefaultValue { get; set; }
    }
    
    /// <summary>
    /// UI strings configuration for localization
    /// </summary>
    public class UIStrings
    {
        [XmlElement("WelcomeTitle")]
        public string WelcomeTitle { get; set; } = "Welcome to {ApplicationName} Setup";
        
        [XmlElement("InstallPathLabel")]
        public string InstallPathLabel { get; set; } = "Installation Path:";
        
        [XmlElement("InstallationPathLabel")]
        public string InstallationPathLabel { get; set; } = "Installation Path:";
        
        [XmlElement("BrowseButtonText")]
        public string BrowseButtonText { get; set; } = "Browse";
        
        [XmlElement("InstallButtonText")]
        public string InstallButtonText { get; set; } = "Install";
        
        [XmlElement("InstallingTitle")]
        public string InstallingTitle { get; set; } = "Installing...";
        
        [XmlElement("PreparingText")]
        public string PreparingText { get; set; } = "Preparing installation...";
        
        [XmlElement("LoadingFilesText")]
        public string LoadingFilesText { get; set; } = "Loading installation files...";
        
        [XmlElement("ExtractingFilesText")]
        public string ExtractingFilesText { get; set; } = "Extracting files...";
        
        [XmlElement("CompletedTitle")]
        public string CompletedTitle { get; set; } = "Installation Completed!";
        
        [XmlElement("LaunchAppText")]
        public string LaunchAppText { get; set; } = "Launch {ApplicationName}";
        
        [XmlElement("CloseButtonText")]
        public string CloseButtonText { get; set; } = "Close";
        
        [XmlElement("Checkboxes")]
        public CheckboxStrings Checkboxes { get; set; } = new CheckboxStrings();
        
        [XmlElement("ErrorMessages")]
        public ErrorMessages ErrorMessages { get; set; } = new ErrorMessages();
        
        /// <summary>
        /// Expands placeholders in strings with actual values
        /// </summary>
        /// <param name="text">Text containing placeholders</param>
        /// <param name="parameters">Optional parameters for expansion</param>
        /// <returns>Expanded text</returns>
        public string ExpandString(string text, object parameters = null)
        {
            if (string.IsNullOrEmpty(text))
                return text;
                
            var expanded = text;
            
            // Replace application name placeholder
            expanded = expanded.Replace("{ApplicationName}", Config.Current.ApplicationName);
            
            // Replace custom parameters if provided
            if (parameters != null)
            {
                var props = parameters.GetType().GetProperties();
                foreach (var prop in props)
                {
                    var value = prop.GetValue(parameters)?.ToString() ?? "";
                    expanded = expanded.Replace($"{{{prop.Name}}}", value);
                }
            }
            
            return expanded;
        }
    }
    
    /// <summary>
    /// Checkbox text strings
    /// </summary>
    public class CheckboxStrings
    {
        [XmlElement("DesktopShortcut")]
        public string DesktopShortcut { get; set; } = "Create desktop shortcut";
        
        [XmlElement("AutoUpdate")]
        public string AutoUpdate { get; set; } = "Enable automatic updates";
        
        [XmlElement("BaseToolchain")]
        public string BaseToolchain { get; set; } = "Install base toolchain";
    }
    
    /// <summary>
    /// Error message strings
    /// </summary>
    public class ErrorMessages
    {
        [XmlElement("PathRequired")]
        public string PathRequired { get; set; } = "Please select an installation path.";
        
        [XmlElement("InstallationFailed")]
        public string InstallationFailed { get; set; } = "Installation failed. Please try again.";
        
        [XmlElement("PackageNotFound")]
        public string PackageNotFound { get; set; } = "Installation package not found.";
        
        [XmlElement("InstallationError")]
        public string InstallationError { get; set; } = "Installation error: {ErrorMessage}";
    }
}