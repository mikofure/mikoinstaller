using System;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Input;
using MikoInstaller.Utils;

public class UninstallerWizard : Window
{
    private Grid _mainGrid;
    private Border _contentArea;
    private ProgressBar _progressBar;
    private TextBlock _statusText;
    private Button _uninstallButton;
    private CheckBox _agreeCheckBox;
    private TextBox _pathTextBox;
    private TextBlock _spaceLabel;
    private int _currentScreen = 0; // 0: Welcome, 1: Uninstalling, 2: Completed
    private static BitmapImage _cachedBannerImage; // Cache for banner image
    
    public UninstallerWizard()
    {
        InitializeWindow();
        InitializeComponents();
        
        // Initialize configuration asynchronously to avoid blocking UI
        Task.Run(() => Config.Initialize());
    }
    
    private void InitializeWindow()
        {
            Title = $"{Config.Current.ApplicationName} Uninstaller";
            Width = 800;
            Height = 320;
            WindowStartupLocation = WindowStartupLocation.CenterScreen;
            ResizeMode = ResizeMode.NoResize;
            WindowStyle = WindowStyle.None;
            Background = new SolidColorBrush(Color.FromRgb(32, 32, 32));
        }
    

    
    private void InitializeComponents()
    {
        // Main container - simplified for better performance
        var mainContainer = new Border
        {
            Background = new SolidColorBrush(Color.FromRgb(32, 32, 32)),
            BorderBrush = new SolidColorBrush(Color.FromRgb(64, 64, 64)),
            BorderThickness = new Thickness(1)
            // Removed DropShadowEffect for faster rendering
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
        
        // Main image/logo area with lazy loading
        var logoContainer = new Image
        {
            Stretch = Stretch.Uniform,
            HorizontalAlignment = HorizontalAlignment.Center,
            VerticalAlignment = VerticalAlignment.Center,
        };
        
        // Load image asynchronously to avoid blocking UI
        Task.Run(() => {
            if (_cachedBannerImage == null)
            {
                try
                {
                    _cachedBannerImage = new BitmapImage();
                    _cachedBannerImage.BeginInit();
                    _cachedBannerImage.UriSource = new Uri("pack://application:,,,/assets/banner.png");
                    _cachedBannerImage.CacheOption = BitmapCacheOption.OnLoad;
                    _cachedBannerImage.EndInit();
                    _cachedBannerImage.Freeze(); // Make it thread-safe
                }
                catch
                {
                    _cachedBannerImage = null;
                }
            }
            
            Dispatcher.Invoke(() => {
                if (_cachedBannerImage != null)
                {
                    logoContainer.Source = _cachedBannerImage;
                }
            });
        });
        
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
            Text = $"Are you sure you want to uninstall {Config.Current.ApplicationName}?",
            FontSize = 18,
            FontWeight = FontWeights.SemiBold,
            Foreground = new SolidColorBrush(Color.FromRgb(220, 220, 220)),
            Margin = new Thickness(0, 0, 0, 30),
            TextWrapping = TextWrapping.Wrap
        };
        
        // Checkboxes
        var checkboxPanel = new StackPanel
        {
            Orientation = Orientation.Vertical,
            Margin = new Thickness(0, 20, 0, 0)
        };
        
        var deleteUserData = new CheckBox
        {
            Content = "Delete user data",
            Foreground = new SolidColorBrush(Color.FromRgb(200, 200, 200)),
            Margin = new Thickness(0, 0, 0, 8),
            FontSize = 12
        };
        
        checkboxPanel.Children.Add(deleteUserData);
        
        // Button Panel
        var buttonPanel = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            HorizontalAlignment = HorizontalAlignment.Right,
            Margin = new Thickness(0, 40, 0, 0)
        };
        
        // Cancel Button
        var cancelButton = CreateStyledButton("Cancel", 100, 35, Color.FromRgb(80, 80, 80), Color.FromRgb(220, 220, 220));
        cancelButton.FontSize = 14;
        cancelButton.FontWeight = FontWeights.Medium;
        cancelButton.Margin = new Thickness(0, 0, 10, 0);
        cancelButton.Click += (s, e) => {
            Application.Current.Shutdown();
        };
        
        // Uninstall Button
        _uninstallButton = CreateStyledButton("Uninstall", 100, 35, Color.FromRgb(200, 60, 60), Color.FromRgb(255, 255, 255));
        _uninstallButton.FontSize = 14;
        _uninstallButton.FontWeight = FontWeights.Medium;
        
        _uninstallButton.Click += async (s, e) => {
            await StartUninstallation();
        };
        
        buttonPanel.Children.Add(cancelButton);
        buttonPanel.Children.Add(_uninstallButton);
        
        // Keep borderless style
        
        mainPanel.Children.Add(titleText);
        mainPanel.Children.Add(checkboxPanel);
        mainPanel.Children.Add(buttonPanel);
        
        _contentArea.Child = mainPanel;
    }
    
    private void ShowUninstallingScreen()
    {
        _currentScreen = 1;
        var mainPanel = new StackPanel
        {
            Orientation = Orientation.Vertical,
            Margin = new Thickness(0)
        };
        
        var titleText = new TextBlock
        {
            Text = "Uninstalling...",
            FontSize = 20,
            FontWeight = FontWeights.SemiBold,
            Foreground = new SolidColorBrush(Color.FromRgb(220, 220, 220)),
            Margin = new Thickness(0, 0, 0, 30)
        };
        
        _statusText = new TextBlock
        {
            Text = "Preparing uninstallation...",
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
            Text = $"{Config.Current.ApplicationName} has been successfully uninstalled",
            FontSize = 20,
            FontWeight = FontWeights.SemiBold,
            Foreground = new SolidColorBrush(Color.FromRgb(220, 220, 220)),
            Margin = new Thickness(0, 0, 0, 30)
        };
        
        var messageText = new TextBlock
        {
            Text = "The application and its files have been removed from your system.",
            FontSize = 12,
            Foreground = new SolidColorBrush(Color.FromRgb(180, 180, 180)),
            Margin = new Thickness(0, 0, 0, 30),
            TextWrapping = TextWrapping.Wrap
        };
        
        var closeButton = CreateStyledButton("Close", 100, 35, Color.FromRgb(60, 60, 60), Color.FromRgb(200, 200, 200));
        closeButton.FontSize = 14;
        closeButton.HorizontalAlignment = HorizontalAlignment.Right;
        closeButton.Click += (s, e) => {
            Close();
        };
        
        mainPanel.Children.Add(titleText);
        mainPanel.Children.Add(messageText);
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
    
    private async Task StartUninstallation()
    {
        var installPath = _pathTextBox.Text;
        if (string.IsNullOrWhiteSpace(installPath))
        {
            MessageBox.Show("Please select the installation path to remove.", "Path Required", MessageBoxButton.OK, MessageBoxImage.Warning);
            return;
        }
        
        ShowUninstallingScreen();
        
        try
        {
            _statusText.Text = "Removing application files...";
            
            var success = await Task.Run(() => {
                try
                {
                    // Remove installation directory
                    if (System.IO.Directory.Exists(installPath))
                    {
                        var totalFiles = System.IO.Directory.GetFiles(installPath, "*", System.IO.SearchOption.AllDirectories).Length;
                        var deletedFiles = 0;
                        
                        foreach (var file in System.IO.Directory.GetFiles(installPath, "*", System.IO.SearchOption.AllDirectories))
                        {
                            try
                            {
                                System.IO.File.Delete(file);
                                deletedFiles++;
                                var progress = (double)deletedFiles / totalFiles * 100;
                                Dispatcher.Invoke(() => {
                                    _progressBar.Value = progress;
                                    _statusText.Text = $"Removing: {System.IO.Path.GetFileName(file)}";
                                });
                            }
                            catch { /* Continue if file can't be deleted */ }
                        }
                        
                        // Remove empty directories
                        System.IO.Directory.Delete(installPath, true);
                    }
                    
                    return true;
                }
                catch
                {
                    return false;
                }
            });
            
            if (success)
            {
                // Remove registry entries
                _statusText.Text = "Cleaning registry...";
                WindowsAPI.DeleteRegistryKey($@"SOFTWARE\{Config.Current.ApplicationName}");
                
                ShowCompletedScreen();
            }
            else
            {
                MessageBox.Show("Uninstallation failed. Some files may still remain.", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
                ShowWelcomeScreen();
            }
        }
        catch (Exception ex)
        {
            MessageBox.Show($"Uninstallation error: {ex.Message}", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
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