using System;
using System.Collections.Generic;
using System.Linq;

namespace MikoInstaller.Components;

public enum InstallationStage
{
    Initializing,
    ValidatingSystem,
    ExtractingFiles,
    InstallingCore,
    ConfiguringSystem,
    CreatingShortcuts,
    RegisteringAssociations,
    FinalizingInstallation,
    Completed
}

public class StageInfo
{
    public InstallationStage Stage { get; set; }
    public string DisplayName { get; set; }
    public double WeightPercentage { get; set; }
    public TimeSpan EstimatedDuration { get; set; }
    public List<string> Tasks { get; set; } = new List<string>();
}

public class ProgressCalculator
{
    private readonly Dictionary<InstallationStage, StageInfo> _stages;
    private readonly Dictionary<InstallationType, Dictionary<InstallationStage, double>> _typeWeights;
    
    public ProgressCalculator()
    {
        _stages = InitializeStages();
        _typeWeights = InitializeTypeWeights();
    }
    
    private Dictionary<InstallationStage, StageInfo> InitializeStages()
    {
        return new Dictionary<InstallationStage, StageInfo>
        {
            [InstallationStage.Initializing] = new StageInfo
            {
                Stage = InstallationStage.Initializing,
                DisplayName = "Initializing installation...",
                WeightPercentage = 5.0,
                EstimatedDuration = TimeSpan.FromSeconds(2),
                Tasks = new List<string>
                {
                    "Checking system requirements",
                    "Validating installation parameters",
                    "Preparing installation environment"
                }
            },
            [InstallationStage.ValidatingSystem] = new StageInfo
            {
                Stage = InstallationStage.ValidatingSystem,
                DisplayName = "Validating system requirements...",
                WeightPercentage = 8.0,
                EstimatedDuration = TimeSpan.FromSeconds(3),
                Tasks = new List<string>
                {
                    "Checking Windows version compatibility",
                    "Verifying disk space availability",
                    "Checking user permissions",
                    "Scanning for conflicting software"
                }
            },
            [InstallationStage.ExtractingFiles] = new StageInfo
            {
                Stage = InstallationStage.ExtractingFiles,
                DisplayName = "Extracting installation files...",
                WeightPercentage = 25.0,
                EstimatedDuration = TimeSpan.FromSeconds(8),
                Tasks = new List<string>
                {
                    "Extracting core application files",
                    "Extracting resource files",
                    "Extracting library dependencies",
                    "Extracting configuration templates"
                }
            },
            [InstallationStage.InstallingCore] = new StageInfo
            {
                Stage = InstallationStage.InstallingCore,
                DisplayName = "Installing core components...",
                WeightPercentage = 35.0,
                EstimatedDuration = TimeSpan.FromSeconds(12),
                Tasks = new List<string>
                {
                    "Installing main executable",
                    "Installing system libraries",
                    "Installing plugins and extensions",
                    "Setting up application data directories",
                    "Configuring application settings"
                }
            },
            [InstallationStage.ConfiguringSystem] = new StageInfo
            {
                Stage = InstallationStage.ConfiguringSystem,
                DisplayName = "Configuring system integration...",
                WeightPercentage = 12.0,
                EstimatedDuration = TimeSpan.FromSeconds(4),
                Tasks = new List<string>
                {
                    "Updating system registry",
                    "Configuring Windows services",
                    "Setting up environment variables",
                    "Configuring firewall exceptions"
                }
            },
            [InstallationStage.CreatingShortcuts] = new StageInfo
            {
                Stage = InstallationStage.CreatingShortcuts,
                DisplayName = "Creating shortcuts and menu entries...",
                WeightPercentage = 5.0,
                EstimatedDuration = TimeSpan.FromSeconds(2),
                Tasks = new List<string>
                {
                    "Creating desktop shortcut",
                    "Creating Start Menu entries",
                    "Creating Quick Launch shortcut"
                }
            },
            [InstallationStage.RegisteringAssociations] = new StageInfo
            {
                Stage = InstallationStage.RegisteringAssociations,
                DisplayName = "Registering file associations...",
                WeightPercentage = 6.0,
                EstimatedDuration = TimeSpan.FromSeconds(3),
                Tasks = new List<string>
                {
                    "Registering file type associations",
                    "Updating default program settings",
                    "Configuring context menu entries"
                }
            },
            [InstallationStage.FinalizingInstallation] = new StageInfo
            {
                Stage = InstallationStage.FinalizingInstallation,
                DisplayName = "Finalizing installation...",
                WeightPercentage = 4.0,
                EstimatedDuration = TimeSpan.FromSeconds(2),
                Tasks = new List<string>
                {
                    "Cleaning up temporary files",
                    "Updating installation registry",
                    "Verifying installation integrity"
                }
            },
            [InstallationStage.Completed] = new StageInfo
            {
                Stage = InstallationStage.Completed,
                DisplayName = "Installation completed successfully!",
                WeightPercentage = 0.0,
                EstimatedDuration = TimeSpan.Zero,
                Tasks = new List<string>()
            }
        };
    }
    
    private Dictionary<InstallationType, Dictionary<InstallationStage, double>> InitializeTypeWeights()
    {
        return new Dictionary<InstallationType, Dictionary<InstallationStage, double>>
        {
            [InstallationType.Minimal] = new Dictionary<InstallationStage, double>
            {
                [InstallationStage.Initializing] = 8.0,
                [InstallationStage.ValidatingSystem] = 10.0,
                [InstallationStage.ExtractingFiles] = 20.0,
                [InstallationStage.InstallingCore] = 45.0,
                [InstallationStage.ConfiguringSystem] = 8.0,
                [InstallationStage.CreatingShortcuts] = 4.0,
                [InstallationStage.RegisteringAssociations] = 3.0,
                [InstallationStage.FinalizingInstallation] = 2.0
            },
            [InstallationType.Standard] = new Dictionary<InstallationStage, double>
            {
                [InstallationStage.Initializing] = 5.0,
                [InstallationStage.ValidatingSystem] = 8.0,
                [InstallationStage.ExtractingFiles] = 25.0,
                [InstallationStage.InstallingCore] = 35.0,
                [InstallationStage.ConfiguringSystem] = 12.0,
                [InstallationStage.CreatingShortcuts] = 5.0,
                [InstallationStage.RegisteringAssociations] = 6.0,
                [InstallationStage.FinalizingInstallation] = 4.0
            },
            [InstallationType.Custom] = new Dictionary<InstallationStage, double>
            {
                [InstallationStage.Initializing] = 4.0,
                [InstallationStage.ValidatingSystem] = 7.0,
                [InstallationStage.ExtractingFiles] = 30.0,
                [InstallationStage.InstallingCore] = 32.0,
                [InstallationStage.ConfiguringSystem] = 15.0,
                [InstallationStage.CreatingShortcuts] = 4.0,
                [InstallationStage.RegisteringAssociations] = 5.0,
                [InstallationStage.FinalizingInstallation] = 3.0
            }
        };
    }
    
    public double Calculate(InstallationStage currentStage, double stageProgress, InstallationType installationType = InstallationType.Standard)
    {
        if (currentStage == InstallationStage.Completed)
            return 100.0;
        
        var weights = _typeWeights.ContainsKey(installationType) ? _typeWeights[installationType] : _typeWeights[InstallationType.Standard];
        
        // Calculate progress from completed stages
        double completedProgress = 0.0;
        var stageOrder = GetStageOrder();
        var currentStageIndex = stageOrder.IndexOf(currentStage);
        
        for (int i = 0; i < currentStageIndex; i++)
        {
            var stage = stageOrder[i];
            completedProgress += weights.ContainsKey(stage) ? weights[stage] : _stages[stage].WeightPercentage;
        }
        
        // Add progress from current stage
        var currentStageWeight = weights.ContainsKey(currentStage) ? weights[currentStage] : _stages[currentStage].WeightPercentage;
        var currentStageProgress = (stageProgress / 100.0) * currentStageWeight;
        
        return Math.Min(100.0, completedProgress + currentStageProgress);
    }
    
    public StageInfo GetStageInfo(InstallationStage stage)
    {
        return _stages.ContainsKey(stage) ? _stages[stage] : _stages[InstallationStage.Initializing];
    }
    
    public List<InstallationStage> GetStageOrder()
    {
        return new List<InstallationStage>
        {
            InstallationStage.Initializing,
            InstallationStage.ValidatingSystem,
            InstallationStage.ExtractingFiles,
            InstallationStage.InstallingCore,
            InstallationStage.ConfiguringSystem,
            InstallationStage.CreatingShortcuts,
            InstallationStage.RegisteringAssociations,
            InstallationStage.FinalizingInstallation,
            InstallationStage.Completed
        };
    }
    
    public InstallationStage GetNextStage(InstallationStage currentStage)
    {
        var stageOrder = GetStageOrder();
        var currentIndex = stageOrder.IndexOf(currentStage);
        
        if (currentIndex >= 0 && currentIndex < stageOrder.Count - 1)
        {
            return stageOrder[currentIndex + 1];
        }
        
        return InstallationStage.Completed;
    }
    
    public InstallationStage GetPreviousStage(InstallationStage currentStage)
    {
        var stageOrder = GetStageOrder();
        var currentIndex = stageOrder.IndexOf(currentStage);
        
        if (currentIndex > 0)
        {
            return stageOrder[currentIndex - 1];
        }
        
        return InstallationStage.Initializing;
    }
    
    public TimeSpan EstimateTotalDuration(InstallationType installationType = InstallationType.Standard)
    {
        var weights = _typeWeights.ContainsKey(installationType) ? _typeWeights[installationType] : _typeWeights[InstallationType.Standard];
        var totalSeconds = 0.0;
        
        foreach (var stage in GetStageOrder())
        {
            if (stage == InstallationStage.Completed) continue;
            
            var stageInfo = _stages[stage];
            var weight = (weights.ContainsKey(stage) ? weights[stage] : stageInfo.WeightPercentage) / 100.0;
            totalSeconds += stageInfo.EstimatedDuration.TotalSeconds * (1.0 + weight); // Weight affects duration
        }
        
        return TimeSpan.FromSeconds(totalSeconds);
    }
    
    public TimeSpan EstimateRemainingDuration(InstallationStage currentStage, double stageProgress, InstallationType installationType = InstallationType.Standard)
    {
        if (currentStage == InstallationStage.Completed)
            return TimeSpan.Zero;
        
        var weights = _typeWeights.ContainsKey(installationType) ? _typeWeights[installationType] : _typeWeights[InstallationType.Standard];
        var remainingSeconds = 0.0;
        var stageOrder = GetStageOrder();
        var currentStageIndex = stageOrder.IndexOf(currentStage);
        
        // Add remaining time for current stage
        var currentStageInfo = _stages[currentStage];
        var currentStageWeight = (weights.ContainsKey(currentStage) ? weights[currentStage] : currentStageInfo.WeightPercentage) / 100.0;
        var currentStageRemainingProgress = (100.0 - stageProgress) / 100.0;
        remainingSeconds += currentStageInfo.EstimatedDuration.TotalSeconds * currentStageRemainingProgress * (1.0 + currentStageWeight);
        
        // Add time for remaining stages
        for (int i = currentStageIndex + 1; i < stageOrder.Count; i++)
        {
            var stage = stageOrder[i];
            if (stage == InstallationStage.Completed) continue;
            
            var stageInfo = _stages[stage];
            var weight = (weights.ContainsKey(stage) ? weights[stage] : stageInfo.WeightPercentage) / 100.0;
            remainingSeconds += stageInfo.EstimatedDuration.TotalSeconds * (1.0 + weight);
        }
        
        return TimeSpan.FromSeconds(Math.Max(0, remainingSeconds));
    }
    
    public List<string> GetCurrentStageTasks(InstallationStage stage)
    {
        return _stages.ContainsKey(stage) ? _stages[stage].Tasks : _stages[InstallationStage.Initializing].Tasks;
    }
    
    public string GetRandomTaskForStage(InstallationStage stage)
    {
        var tasks = GetCurrentStageTasks(stage);
        if (tasks.Count == 0) return "Processing...";
        
        var random = new Random();
        return tasks[random.Next(tasks.Count)];
    }
    
    public double GetStageWeight(InstallationStage stage, InstallationType installationType = InstallationType.Standard)
    {
        var weights = _typeWeights.ContainsKey(installationType) ? _typeWeights[installationType] : _typeWeights[InstallationType.Standard];
        return weights.ContainsKey(stage) ? weights[stage] : _stages[stage].WeightPercentage;
    }
}