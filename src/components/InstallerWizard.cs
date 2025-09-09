using System;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Input;
using MikoInstaller.Utils;

public class InstallerWizard : Window
{
    private Grid _mainGrid;
    private Border _contentArea;
    private ProgressBar _progressBar;
    private TextBlock _statusText;
    private Button _installButton;
    private CheckBox _agreeCheckBox;
    private TextBox _pathTextBox;
    private TextBlock _spaceLabel;
    private int _currentScreen = 0; // 0: Welcome, 1: Installing, 2: Completed
    
    public InstallerWizard()
    {
        // Initialize configuration
        Config.Initialize();
        
        InitializeWindow();
        InitializeComponents();
    }
    
    private void InitializeWindow()
        {
            Title = $"{Config.Current.ApplicationName} Installer";
            Width = 800;
            Height = 320;
            WindowStartupLocation = WindowStartupLocation.CenterScreen;
            ResizeMode = ResizeMode.NoResize;
            WindowStyle = WindowStyle.None;
            Background = new SolidColorBrush(Color.FromRgb(32, 32, 32));
        }
    

    
    private void InitializeComponents()
    {
        // Main container with rounded corners and shadow effect
        var mainContainer = new Border
        {
            Background = new SolidColorBrush(Color.FromRgb(32, 32, 32)),
            BorderBrush = new SolidColorBrush(Color.FromRgb(64, 64, 64)),
            BorderThickness = new Thickness(1),
            Effect = new System.Windows.Media.Effects.DropShadowEffect
            {
                Color = Colors.Black,
                Direction = 315,
                ShadowDepth = 5,
                Opacity = 0.3,
                BlurRadius = 10
            }
        };
        
        _mainGrid = new Grid();
        _mainGrid.RowDefinitions.Add(new RowDefinition { Height = GridLength.Auto }); // Titlebar
        _mainGrid.RowDefinitions.Add(new RowDefinition { Height = new GridLength(1, GridUnitType.Star) }); // Content
        _mainGrid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(200) }); // Side panel
        _mainGrid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) }); // Main content
        
        // Custom titlebar
        var titlebar = CreateTitlebar();
        Grid.SetRow(titlebar, 0);
        Grid.SetColumnSpan(titlebar, 2);
        _mainGrid.Children.Add(titlebar);
        
        // Side image panel
        var sidePanel = CreateSidePanel();
        Grid.SetRow(sidePanel, 1);
        Grid.SetColumn(sidePanel, 0);
        _mainGrid.Children.Add(sidePanel);
        
        // Main content area - dynamic screen content
        _contentArea = new Border
        {
            Background = new SolidColorBrush(Color.FromRgb(40, 40, 40)),
            Padding = new Thickness(20)
        };
        Grid.SetRow(_contentArea, 1);
        Grid.SetColumn(_contentArea, 1);
        _mainGrid.Children.Add(_contentArea);
        
        // Show initial welcome screen
        ShowWelcomeScreen();
        
        mainContainer.Child = _mainGrid;
        Content = mainContainer;
    }
    
    private Border CreateTitlebar()
    {
        var titlebarGrid = new Grid();
        titlebarGrid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) });
        titlebarGrid.ColumnDefinitions.Add(new ColumnDefinition { Width = GridLength.Auto });
        
        // Title and icon
        var titlePanel = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            VerticalAlignment = VerticalAlignment.Center,
            Margin = new Thickness(15, 0, 0, 0)
        };
        
        var titleText = new TextBlock
        {
            Text = "MikoIDE Installer",
            FontSize = 14,
            FontWeight = FontWeights.Medium,
            Foreground = new SolidColorBrush(Color.FromRgb(220, 220, 220)),
            VerticalAlignment = VerticalAlignment.Center
        };
        
        titlePanel.Children.Add(titleText);
        Grid.SetColumn(titlePanel, 0);
        titlebarGrid.Children.Add(titlePanel);
        
        // Close button
        var closeButton = new Button
        {
            Content = "✕",
            Width = 45,
            Height = 30,
            Background = Brushes.Transparent,
            Foreground = new SolidColorBrush(Color.FromRgb(180, 180, 180)),
            BorderThickness = new Thickness(0),
            FontSize = 16,
            HorizontalContentAlignment = HorizontalAlignment.Center,
            VerticalContentAlignment = VerticalAlignment.Center
        };
        closeButton.Click += (s, e) => {
            Close();
        };
        
        // Hover effect for close button
        closeButton.MouseEnter += (s, e) => {
            closeButton.Background = Brushes.Red;
            closeButton.Foreground = Brushes.White;
        };
        closeButton.MouseLeave += (s, e) => {
            closeButton.Background = Brushes.Transparent;
            closeButton.Foreground = new SolidColorBrush(Color.FromRgb(180, 180, 180));
        };
        
        Grid.SetColumn(closeButton, 1);
        titlebarGrid.Children.Add(closeButton);
        
        var titlebar = new Border
        {
            Child = titlebarGrid,
            Background = new SolidColorBrush(Color.FromRgb(32, 32, 32)),
            Height = 30,
            BorderBrush = new SolidColorBrush(Color.FromRgb(64, 64, 64)),
            BorderThickness = new Thickness(0, 0, 0, 1)
        };
        
        // Enable window dragging
        titlebar.MouseLeftButtonDown += (s, e) => {
            DragMove();
        };
        
        return titlebar;
    }
    
    private Border CreateSidePanel()
    {
        var sideGrid = new Grid();
        sideGrid.RowDefinitions.Add(new RowDefinition { Height = new GridLength(1, GridUnitType.Star) });
        sideGrid.RowDefinitions.Add(new RowDefinition { Height = GridLength.Auto });
        
        // Main image/logo area
        var logoContainer = new Image
        {
            Source = new BitmapImage(new Uri("pack://application:,,,/assets/banner.png")),
            Stretch = Stretch.Uniform,
            HorizontalAlignment = HorizontalAlignment.Center,
            VerticalAlignment = VerticalAlignment.Center,
        };
        Grid.SetRow(logoContainer, 0);
        sideGrid.Children.Add(logoContainer);
        
        return new Border
        {
            Child = sideGrid,
            Background = new SolidColorBrush(Color.FromRgb(28, 28, 28))
        };
    }
    
    private void ShowWelcomeScreen()
    {
        _currentScreen = 0;
        var mainPanel = new StackPanel
        {
            Orientation = Orientation.Vertical,
            Margin = new Thickness(0)
        };
        
        // Header
        var titleText = new TextBlock
        {
            Text = Config.Current.UIStrings.ExpandString(Config.Current.UIStrings.WelcomeTitle),
            FontSize = 20,
            FontWeight = FontWeights.SemiBold,
            Foreground = new SolidColorBrush(Color.FromRgb(220, 220, 220)),
            Margin = new Thickness(0, 0, 0, 4)
        };
        
        // Installation Path Section
        var pathLabel = new TextBlock
        {
            Text = Config.Current.UIStrings.InstallationPathLabel,
            FontSize = 14,
            FontWeight = FontWeights.Medium,
            Foreground = new SolidColorBrush(Color.FromRgb(200, 200, 200)),
            Margin = new Thickness(0, 0, 0, 10)
        };
        
        var pathInputPanel = new Grid();
        pathInputPanel.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) });
        pathInputPanel.ColumnDefinitions.Add(new ColumnDefinition { Width = GridLength.Auto });
        
        _pathTextBox = new TextBox
        {
            Text = Config.Current.ResolvedDefaultInstallPath,
            Height = 32,
            Background = new SolidColorBrush(Color.FromRgb(50, 50, 50)),
            Foreground = new SolidColorBrush(Color.FromRgb(220, 220, 220)),
            BorderBrush = new SolidColorBrush(Color.FromRgb(80, 80, 80)),
            BorderThickness = new Thickness(1),
            Padding = new Thickness(8, 6, 8, 6),
            FontSize = 12,
            VerticalContentAlignment = VerticalAlignment.Center
        };
        Grid.SetColumn(_pathTextBox, 0);
        
        var browseButton = CreateStyledButton(Config.Current.UIStrings.BrowseButtonText, 80, 32, Color.FromRgb(60, 60, 60), Color.FromRgb(200, 200, 200));
        browseButton.Margin = new Thickness(10, 0, 0, 0);
        browseButton.Click += (s, e) => {
            var selectedPath = WindowsAPI.SelectFolder(_pathTextBox.Text, $"Select {Config.Current.ApplicationName} installation folder:");
            if (!string.IsNullOrEmpty(selectedPath))
            {
                _pathTextBox.Text = selectedPath;
                UpdateSpaceInfo();
            }
        };
        Grid.SetColumn(browseButton, 1);
        
        pathInputPanel.Children.Add(_pathTextBox);
        pathInputPanel.Children.Add(browseButton);
        
        // Checkboxes
        var checkboxPanel = new StackPanel
        {
            Orientation = Orientation.Vertical,
            Margin = new Thickness(0, 20, 0, 0)
        };
        
        var desktopShortcut = new CheckBox
        {
            Content = Config.Current.UIStrings.Checkboxes.DesktopShortcut,
            Foreground = new SolidColorBrush(Color.FromRgb(200, 200, 200)),
            Margin = new Thickness(0, 0, 0, 8),
            FontSize = 12
        };
        
        var autoUpdate = new CheckBox
        {
            Content = Config.Current.UIStrings.Checkboxes.AutoUpdate,
            Foreground = new SolidColorBrush(Color.FromRgb(200, 200, 200)),
            Margin = new Thickness(0, 0, 0, 8),
            FontSize = 12
        };
        
        var baseToolchain = new CheckBox
        {
            Content = Config.Current.UIStrings.Checkboxes.BaseToolchain,
            Foreground = new SolidColorBrush(Color.FromRgb(200, 200, 200)),
            FontSize = 12
        };
        
        checkboxPanel.Children.Add(desktopShortcut);
        checkboxPanel.Children.Add(autoUpdate);
        checkboxPanel.Children.Add(baseToolchain);
        
        // Space info
        _spaceLabel = new TextBlock
        {
            FontSize = 11,
            Foreground = new SolidColorBrush(Color.FromRgb(160, 160, 160)),
            VerticalAlignment = VerticalAlignment.Center
        };
        UpdateSpaceInfo();
        
        // Install Button
        _installButton = CreateStyledButton(Config.Current.UIStrings.InstallButtonText, 100, 35, Color.FromRgb(0, 120, 215), Color.FromRgb(255, 255, 255));
        _installButton.FontSize = 14;
        _installButton.FontWeight = FontWeights.Medium;
        _installButton.HorizontalAlignment = HorizontalAlignment.Right;
        
        _installButton.Click += async (s, e) => {
            await StartInstallation();
        };
        
        // Bottom panel with space between storage info and install button
        var bottomPanel = new Grid
        {
            Margin = new Thickness(0, 30, 0, 0)
        };
        bottomPanel.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) });
        bottomPanel.ColumnDefinitions.Add(new ColumnDefinition { Width = GridLength.Auto });
        
        Grid.SetColumn(_spaceLabel, 0);
        Grid.SetColumn(_installButton, 1);
        
        bottomPanel.Children.Add(_spaceLabel);
        bottomPanel.Children.Add(_installButton);
        
        mainPanel.Children.Add(titleText);
        mainPanel.Children.Add(pathLabel);
        mainPanel.Children.Add(pathInputPanel);
        mainPanel.Children.Add(checkboxPanel);
        mainPanel.Children.Add(bottomPanel);
        
        _contentArea.Child = mainPanel;
    }
    
    private void ShowInstallingScreen()
    {
        _currentScreen = 1;
        var mainPanel = new StackPanel
        {
            Orientation = Orientation.Vertical,
            Margin = new Thickness(0)
        };
        
        var titleText = new TextBlock
        {
            Text = Config.Current.UIStrings.InstallingTitle,
            FontSize = 20,
            FontWeight = FontWeights.SemiBold,
            Foreground = new SolidColorBrush(Color.FromRgb(220, 220, 220)),
            Margin = new Thickness(0, 0, 0, 30)
        };
        
        _statusText = new TextBlock
        {
            Text = Config.Current.UIStrings.PreparingText,
            FontSize = 12,
            Foreground = new SolidColorBrush(Color.FromRgb(180, 180, 180)),
            Margin = new Thickness(0, 0, 0, 20)
        };
        
        _progressBar = new ProgressBar
        {
            Height = 20,
            Background = new SolidColorBrush(Color.FromRgb(50, 50, 50)),
            Foreground = new SolidColorBrush(Color.FromRgb(0, 120, 215)),
            BorderBrush = new SolidColorBrush(Color.FromRgb(80, 80, 80)),
            BorderThickness = new Thickness(1),
            Value = 0
        };
        
        mainPanel.Children.Add(titleText);
        mainPanel.Children.Add(_statusText);
        mainPanel.Children.Add(_progressBar);
        
        _contentArea.Child = mainPanel;
    }
    
    private void ShowCompletedScreen()
    {
        _currentScreen = 2;
        var mainPanel = new StackPanel
        {
            Orientation = Orientation.Vertical,
            Margin = new Thickness(0)
        };
        
        var titleText = new TextBlock
        {
            Text = Config.Current.UIStrings.CompletedTitle,
            FontSize = 20,
            FontWeight = FontWeights.SemiBold,
            Foreground = new SolidColorBrush(Color.FromRgb(220, 220, 220)),
            Margin = new Thickness(0, 0, 0, 30)
        };
        
        var launchCheckbox = new CheckBox
        {
            Content = Config.Current.UIStrings.ExpandString(Config.Current.UIStrings.LaunchAppText),
            Foreground = new SolidColorBrush(Color.FromRgb(200, 200, 200)),
            FontSize = 12,
            IsChecked = true,
            Margin = new Thickness(0, 0, 0, 30)
        };
        
        var closeButton = CreateStyledButton(Config.Current.UIStrings.CloseButtonText, 100, 35, Color.FromRgb(60, 60, 60), Color.FromRgb(200, 200, 200));
        closeButton.FontSize = 14;
        closeButton.HorizontalAlignment = HorizontalAlignment.Right;
        closeButton.Click += (s, e) => {
            Close();
        };
        
        mainPanel.Children.Add(titleText);
        mainPanel.Children.Add(launchCheckbox);
        mainPanel.Children.Add(closeButton);
        
        _contentArea.Child = mainPanel;
    }
    
    private void UpdateSpaceInfo()
    {
        if (_pathTextBox != null && _spaceLabel != null)
        {
            var availableSpace = WindowsAPI.GetAvailableDiskSpace(_pathTextBox.Text);
            var requiredSpaceGB = Config.Current.RequiredDiskSpace / (1024.0 * 1024.0 * 1024.0);
            var availableSpaceGB = availableSpace / (1024.0 * 1024.0 * 1024.0);
            
            var spaceText = $"Total Required: {requiredSpaceGB:F0}MB\nFree Space: {availableSpaceGB:F1}GB";
            var spaceColor = availableSpaceGB >= requiredSpaceGB ? Color.FromRgb(100, 200, 100) : Color.FromRgb(200, 100, 100);
            _spaceLabel.Text = spaceText;
            _spaceLabel.Foreground = new SolidColorBrush(spaceColor);
        }
    }
    
    private async Task StartInstallation()
    {
        var installPath = _pathTextBox.Text;
        if (string.IsNullOrWhiteSpace(installPath))
        {
            MessageBox.Show(Config.Current.UIStrings.ErrorMessages.PathRequired, "Path Required", MessageBoxButton.OK, MessageBoxImage.Warning);
            return;
        }
        
        ShowInstallingScreen();
        
        try
        {
            // Load embedded assets
            _statusText.Text = Config.Current.UIStrings.LoadingFilesText;
            var assets = await Task.Run(() => WindowsAPI.LoadEmbeddedAssetsToMemory());
            
            if (assets.ContainsKey("app.zip"))
            {
                _statusText.Text = Config.Current.UIStrings.ExtractingFilesText;
                var success = await Task.Run(() => WindowsAPI.ExtractAppZipFromMemory(
                    assets["app.zip"], 
                    installPath, 
                    progress => {
                        Dispatcher.Invoke(() => {
                            _progressBar.Value = progress;
                            _statusText.Text = $"{installPath}";
                        });
                    }
                ));
               
               if (success)
               {
                   // Register installation in registry
                    WindowsAPI.CreateRegistryKey(
                        $@"SOFTWARE\{Config.Current.ApplicationName}",
                        "InstallPath",
                        installPath
                    );
                    
                    WindowsAPI.CreateRegistryKey(
                        $@"SOFTWARE\{Config.Current.ApplicationName}",
                        "Version",
                        Config.Current.Version
                    );
                   
                   ShowCompletedScreen();
               }
               else
               {
                   MessageBox.Show(Config.Current.UIStrings.ErrorMessages.InstallationFailed, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                   ShowWelcomeScreen();
               }
           }
           else
           {
               MessageBox.Show(Config.Current.UIStrings.ErrorMessages.PackageNotFound, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
               ShowWelcomeScreen();
           }
           
           // Cleanup memory streams
           foreach (var asset in assets.Values)
           {
               asset?.Dispose();
           }
       }
       catch (Exception ex)
       {
           var errorMessage = Config.Current.UIStrings.ExpandString(Config.Current.UIStrings.ErrorMessages.InstallationError, new { ErrorMessage = ex.Message });
           MessageBox.Show(errorMessage, "Error", MessageBoxButton.OK, MessageBoxImage.Error);
           ShowWelcomeScreen();
       }
    }
    

    
    private Button CreateStyledButton(string text, double width, double height, Color? backgroundColor = null, Color? foregroundColor = null)
    {
        var bgColor = backgroundColor ?? Color.FromRgb(64, 64, 64);
        var fgColor = foregroundColor ?? Color.FromRgb(220, 220, 220);
        
        var button = new Button
        {
            Content = text,
            Width = width,
            Height = height,
            Background = new SolidColorBrush(bgColor),
            Foreground = new SolidColorBrush(fgColor),
            BorderThickness = new Thickness(1),
            BorderBrush = new SolidColorBrush(Color.FromRgb(80, 80, 80)),
            FontSize = 12,
            FontWeight = FontWeights.Normal,
            Cursor = System.Windows.Input.Cursors.Hand
        };
        
        // Hover effects
        var hoverBackground = Color.FromRgb(
            (byte)Math.Min(255, bgColor.R + 20),
            (byte)Math.Min(255, bgColor.G + 20),
            (byte)Math.Min(255, bgColor.B + 20)
        );
        
        button.MouseEnter += (s, e) => {
            if (button.IsEnabled)
                button.Background = new SolidColorBrush(hoverBackground);
        };
        
        button.MouseLeave += (s, e) => {
            if (button.IsEnabled)
                button.Background = new SolidColorBrush(bgColor);
        };
        
        return button;
    }
    

}

// Screen implementations are now in separate files:
// - WelcomeScreen.cs
// - EulaScreen.cs
// - InstallOptionsScreen.cs
// - ProgressScreen.cs
// - CompletionScreen.cs