using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using MikoInstaller.Hook;
using MikoInstaller.Utils;

namespace MikoInstaller.Components;

public class InstallationCard
{
    private readonly UseState<string> _status;
    private readonly UseState<double> _progress;
    private readonly UseEffect _effects;
    private ProgressBar _progressBar;
    private Label _statusLabel;
    
    public InstallationCard()
    {
        _status = new UseState<string>("Ready");
        _progress = new UseState<double>(0.0);
        _effects = new UseEffect();
        
        // Subscribe to state changes
        _status.StateChanged += OnStatusChanged;
        _progress.StateChanged += OnProgressChanged;
    }
    
    public UIElement Render()
    {
        var cardContent = UIHelper.CreateVerticalStack();
        
        // Title
        var title = UIHelper.CreateLabel("Installation Progress", 16, HorizontalAlignment.Center);
        title.FontWeight = FontWeights.Bold;
        cardContent.Children.Add(title);
        
        // Status
        _statusLabel = UIHelper.CreateLabel(_status.Value, 12, HorizontalAlignment.Center);
        _statusLabel.Margin = new Thickness(0, 10, 0, 10);
        cardContent.Children.Add(_statusLabel);
        
        // Progress bar
        _progressBar = new ProgressBar
        {
            Width = 300,
            Height = 20,
            Value = _progress.Value,
            Maximum = 100,
            Margin = new Thickness(0, 0, 0, 15)
        };
        cardContent.Children.Add(_progressBar);
        
        // Buttons
        var buttonPanel = UIHelper.CreateHorizontalStack();
        buttonPanel.HorizontalAlignment = HorizontalAlignment.Center;
        
        var startButton = UIHelper.CreateButton("Start", 80, 30);
        startButton.Click += OnStartClick;
        
        var resetButton = UIHelper.CreateButton("Reset", 80, 30);
        resetButton.Background = new SolidColorBrush(Color.FromRgb(108, 117, 125));
        resetButton.Click += OnResetClick;
        resetButton.Margin = new Thickness(10, 0, 0, 0);
        
        buttonPanel.Children.Add(startButton);
        buttonPanel.Children.Add(resetButton);
        cardContent.Children.Add(buttonPanel);
        
        return UIHelper.CreateCard(cardContent);
    }
    
    private void OnStartClick(object sender, RoutedEventArgs e)
    {
        _status.SetState("Installing...");
        SimulateInstallation();
    }
    
    private void OnResetClick(object sender, RoutedEventArgs e)
    {
        _status.SetState("Ready");
        _progress.SetState(0.0);
    }
    
    private async void SimulateInstallation()
    {
        for (int i = 0; i <= 100; i += 10)
        {
            await Task.Delay(200);
            _progress.SetState(i);
            
            if (i == 100)
            {
                _status.SetState("Installation Complete!");
            }
        }
    }
    
    private void OnStatusChanged(string newStatus)
    {
        if (_statusLabel != null)
        {
            Application.Current.Dispatcher.Invoke(() =>
            {
                _statusLabel.Content = newStatus;
            });
        }
    }
    
    private void OnProgressChanged(double newProgress)
    {
        if (_progressBar != null)
        {
            Application.Current.Dispatcher.Invoke(() =>
            {
                _progressBar.Value = newProgress;
            });
        }
    }
    
    public void OnUnmount()
    {
        _effects.Cleanup();
        _status.StateChanged -= OnStatusChanged;
        _progress.StateChanged -= OnProgressChanged;
    }
}