using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.IO;

namespace MikoInstaller.Components;

public enum InstallationType
{
    Standard,
    Custom,
    Minimal
}

public enum InstallerOperation
{
    Install,
    Uninstall,
    Repair,
    Update
}

public enum InstallationLevel
{
    User,
    System
}

public class InstallationParams
{
    public string TargetDirectory { get; set; } = "C:\\Program Files\\MikoInstaller";
    public InstallationType InstallationType { get; set; } = InstallationType.Standard;
    public InstallerOperation Operation { get; set; } = InstallerOperation.Install;
    public InstallationLevel Level { get; set; } = InstallationLevel.System;
    public bool CreateDesktopShortcut { get; set; } = true;
    public bool CreateStartMenuShortcut { get; set; } = true;
    public bool RegisterFileAssociations { get; set; } = true;
    public bool SendUsageStatistics { get; set; } = false;
    public bool EulaAccepted { get; set; } = false;
    public bool LaunchAfterInstall { get; set; } = true;
    public List<string> SelectedComponents { get; set; } = new List<string>();
    public Dictionary<string, object> CustomSettings { get; set; } = new Dictionary<string, object>();
}

public class InstallationProgress
{
    public double OverallProgress { get; set; } = 0.0;
    public string CurrentStage { get; set; } = "";
    public string CurrentFile { get; set; } = "";
    public int FilesProcessed { get; set; } = 0;
    public int TotalFiles { get; set; } = 0;
    public long BytesProcessed { get; set; } = 0;
    public long TotalBytes { get; set; } = 0;
    public TimeSpan ElapsedTime { get; set; } = TimeSpan.Zero;
    public TimeSpan EstimatedTimeRemaining { get; set; } = TimeSpan.Zero;
    public bool IsCompleted { get; set; } = false;
    public bool HasError { get; set; } = false;
    public string ErrorMessage { get; set; } = "";
    public List<string> LogEntries { get; set; } = new List<string>();
}

public class InstallerState : INotifyPropertyChanged
{
    private InstallationParams _params;
    private InstallationProgress _progress;
    private Dictionary<string, object> _sessionData;
    private DateTime _installationStartTime;
    private bool _isInstalling;
    
    public event PropertyChangedEventHandler PropertyChanged;
    public event EventHandler<InstallationProgress> ProgressChanged;
    public event EventHandler<string> LogEntryAdded;
    
    public InstallerState()
    {
        _params = new InstallationParams();
        _progress = new InstallationProgress();
        _sessionData = new Dictionary<string, object>();
        _isInstalling = false;
    }
    
    public InstallationParams Parameters
    {
        get => _params;
        set
        {
            _params = value;
            OnPropertyChanged();
        }
    }
    
    public InstallationProgress Progress
    {
        get => _progress;
        private set
        {
            _progress = value;
            OnPropertyChanged();
            ProgressChanged?.Invoke(this, _progress);
        }
    }
    
    public bool IsInstalling
    {
        get => _isInstalling;
        private set
        {
            _isInstalling = value;
            OnPropertyChanged();
        }
    }
    
    public DateTime InstallationStartTime => _installationStartTime;
    
    // Session data management
    public void SetData(string key, object value)
    {
        _sessionData[key] = value;
        OnPropertyChanged(nameof(SessionData));
    }
    
    public T GetData<T>(string key, T defaultValue = default(T))
    {
        if (_sessionData.TryGetValue(key, out var value) && value is T typedValue)
        {
            return typedValue;
        }
        return defaultValue;
    }
    
    public bool HasData(string key)
    {
        return _sessionData.ContainsKey(key);
    }
    
    public Dictionary<string, object> SessionData => new Dictionary<string, object>(_sessionData);
    
    // Progress management
    public void UpdateProgress(double overallProgress, string currentStage = null, string currentFile = null)
    {
        var newProgress = new InstallationProgress
        {
            OverallProgress = Math.Max(0, Math.Min(100, overallProgress)),
            CurrentStage = currentStage ?? _progress.CurrentStage,
            CurrentFile = currentFile ?? _progress.CurrentFile,
            FilesProcessed = _progress.FilesProcessed,
            TotalFiles = _progress.TotalFiles,
            BytesProcessed = _progress.BytesProcessed,
            TotalBytes = _progress.TotalBytes,
            ElapsedTime = _isInstalling ? DateTime.Now - _installationStartTime : _progress.ElapsedTime,
            EstimatedTimeRemaining = CalculateEstimatedTimeRemaining(overallProgress),
            IsCompleted = overallProgress >= 100,
            HasError = _progress.HasError,
            ErrorMessage = _progress.ErrorMessage,
            LogEntries = new List<string>(_progress.LogEntries)
        };
        
        Progress = newProgress;
    }
    
    public void UpdateFileProgress(int filesProcessed, int totalFiles, long bytesProcessed = 0, long totalBytes = 0)
    {
        var newProgress = new InstallationProgress
        {
            OverallProgress = _progress.OverallProgress,
            CurrentStage = _progress.CurrentStage,
            CurrentFile = _progress.CurrentFile,
            FilesProcessed = filesProcessed,
            TotalFiles = totalFiles,
            BytesProcessed = bytesProcessed,
            TotalBytes = totalBytes,
            ElapsedTime = _isInstalling ? DateTime.Now - _installationStartTime : _progress.ElapsedTime,
            EstimatedTimeRemaining = _progress.EstimatedTimeRemaining,
            IsCompleted = _progress.IsCompleted,
            HasError = _progress.HasError,
            ErrorMessage = _progress.ErrorMessage,
            LogEntries = new List<string>(_progress.LogEntries)
        };
        
        Progress = newProgress;
    }
    
    public void SetError(string errorMessage)
    {
        var newProgress = new InstallationProgress
        {
            OverallProgress = _progress.OverallProgress,
            CurrentStage = "Installation Failed",
            CurrentFile = _progress.CurrentFile,
            FilesProcessed = _progress.FilesProcessed,
            TotalFiles = _progress.TotalFiles,
            BytesProcessed = _progress.BytesProcessed,
            TotalBytes = _progress.TotalBytes,
            ElapsedTime = _progress.ElapsedTime,
            EstimatedTimeRemaining = TimeSpan.Zero,
            IsCompleted = false,
            HasError = true,
            ErrorMessage = errorMessage,
            LogEntries = new List<string>(_progress.LogEntries)
        };
        
        AddLogEntry($"ERROR: {errorMessage}");
        Progress = newProgress;
        IsInstalling = false;
    }
    
    public void AddLogEntry(string message)
    {
        var timestamp = DateTime.Now.ToString("HH:mm:ss.fff");
        var logEntry = $"[{timestamp}] {message}";
        
        _progress.LogEntries.Add(logEntry);
        LogEntryAdded?.Invoke(this, logEntry);
        
        // Limit log entries to prevent memory issues
        if (_progress.LogEntries.Count > 1000)
        {
            _progress.LogEntries.RemoveAt(0);
        }
    }
    
    // Installation lifecycle
    public void StartInstallation()
    {
        if (_isInstalling) return;
        
        _installationStartTime = DateTime.Now;
        IsInstalling = true;
        
        var initialProgress = new InstallationProgress
        {
            OverallProgress = 0,
            CurrentStage = "Initializing installation...",
            CurrentFile = "",
            FilesProcessed = 0,
            TotalFiles = EstimateTotalFiles(),
            BytesProcessed = 0,
            TotalBytes = EstimateTotalBytes(),
            ElapsedTime = TimeSpan.Zero,
            EstimatedTimeRemaining = TimeSpan.Zero,
            IsCompleted = false,
            HasError = false,
            ErrorMessage = "",
            LogEntries = new List<string>()
        };
        
        Progress = initialProgress;
        AddLogEntry("Installation started");
        AddLogEntry($"Target directory: {_params.TargetDirectory}");
        AddLogEntry($"Installation type: {_params.InstallationType}");
        AddLogEntry($"Operation: {_params.Operation}");
    }
    
    public void CompleteInstallation()
    {
        UpdateProgress(100, "Installation completed successfully!");
        IsInstalling = false;
        AddLogEntry("Installation completed successfully");
    }
    
    public void CancelInstallation()
    {
        if (!_isInstalling) return;
        
        IsInstalling = false;
        AddLogEntry("Installation cancelled by user");
        
        var cancelledProgress = new InstallationProgress
        {
            OverallProgress = _progress.OverallProgress,
            CurrentStage = "Installation cancelled",
            CurrentFile = "",
            FilesProcessed = _progress.FilesProcessed,
            TotalFiles = _progress.TotalFiles,
            BytesProcessed = _progress.BytesProcessed,
            TotalBytes = _progress.TotalBytes,
            ElapsedTime = _progress.ElapsedTime,
            EstimatedTimeRemaining = TimeSpan.Zero,
            IsCompleted = false,
            HasError = false,
            ErrorMessage = "",
            LogEntries = new List<string>(_progress.LogEntries)
        };
        
        Progress = cancelledProgress;
    }
    
    // Validation
    public List<string> ValidateInstallationParameters()
    {
        var errors = new List<string>();
        
        if (string.IsNullOrWhiteSpace(_params.TargetDirectory))
        {
            errors.Add("Target directory cannot be empty");
        }
        
        if (!_params.EulaAccepted)
        {
            errors.Add("License agreement must be accepted");
        }
        
        try
        {
            var targetDir = new DirectoryInfo(_params.TargetDirectory);
            if (!targetDir.Parent.Exists)
            {
                errors.Add("Parent directory of target path does not exist");
            }
        }
        catch (Exception ex)
        {
            errors.Add($"Invalid target directory: {ex.Message}");
        }
        
        // Check disk space
        try
        {
            var drive = new DriveInfo(Path.GetPathRoot(_params.TargetDirectory));
            var requiredSpace = EstimateTotalBytes();
            if (drive.AvailableFreeSpace < requiredSpace * 1.1) // 10% buffer
            {
                errors.Add($"Insufficient disk space. Required: {FormatBytes(requiredSpace)}, Available: {FormatBytes(drive.AvailableFreeSpace)}");
            }
        }
        catch (Exception ex)
        {
            errors.Add($"Could not check disk space: {ex.Message}");
        }
        
        return errors;
    }
    
    // Helper methods
    private TimeSpan CalculateEstimatedTimeRemaining(double currentProgress)
    {
        if (!_isInstalling || currentProgress <= 0) return TimeSpan.Zero;
        
        var elapsed = DateTime.Now - _installationStartTime;
        var progressRatio = currentProgress / 100.0;
        var estimatedTotal = elapsed.TotalSeconds / progressRatio;
        var remaining = estimatedTotal - elapsed.TotalSeconds;
        
        return TimeSpan.FromSeconds(Math.Max(0, remaining));
    }
    
    private int EstimateTotalFiles()
    {
        return _params.InstallationType switch
        {
            InstallationType.Minimal => 25,
            InstallationType.Standard => 50,
            InstallationType.Custom => _params.SelectedComponents.Count * 10,
            _ => 50
        };
    }
    
    private long EstimateTotalBytes()
    {
        return _params.InstallationType switch
        {
            InstallationType.Minimal => 15 * 1024 * 1024, // 15 MB
            InstallationType.Standard => 45 * 1024 * 1024, // 45 MB
            InstallationType.Custom => _params.SelectedComponents.Count * 5 * 1024 * 1024, // 5 MB per component
            _ => 45 * 1024 * 1024
        };
    }
    
    private string FormatBytes(long bytes)
    {
        string[] suffixes = { "B", "KB", "MB", "GB", "TB" };
        int counter = 0;
        decimal number = bytes;
        while (Math.Round(number / 1024) >= 1)
        {
            number /= 1024;
            counter++;
        }
        return $"{number:n1} {suffixes[counter]}";
    }
    
    protected virtual void OnPropertyChanged([CallerMemberName] string propertyName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }
}