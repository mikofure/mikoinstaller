using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace MikoInstaller.Components;

public class MainWindow : Window
{
    private StackPanel _mainPanel;
    
    public MainWindow()
    {
        InitializeWindow();
        InitializeComponents();
    }
    
    private void InitializeWindow()
    {
        Title = "MikoInstaller";
        Width = 800;
        Height = 600;
        WindowStartupLocation = WindowStartupLocation.CenterScreen;
        Background = new SolidColorBrush(Color.FromRgb(240, 240, 240));
    }
    
    private void InitializeComponents()
    {
        _mainPanel = new StackPanel
        {
            Orientation = Orientation.Vertical,
            Margin = new Thickness(20),
            HorizontalAlignment = HorizontalAlignment.Stretch,
            VerticalAlignment = VerticalAlignment.Stretch
        };
        
        // Add header
        var header = CreateHeader();
        _mainPanel.Children.Add(header);
        
        // Add content area
        var contentArea = CreateContentArea();
        _mainPanel.Children.Add(contentArea);
        
        // Add footer
        var footer = CreateFooter();
        _mainPanel.Children.Add(footer);
        
        Content = _mainPanel;
    }
    
    private UIElement CreateHeader()
    {
        var headerPanel = new StackPanel
        {
            Orientation = Orientation.Vertical,
            Margin = new Thickness(0, 0, 0, 20)
        };
        
        var titleLabel = new Label
        {
            Content = "MikoInstaller",
            FontSize = 24,
            FontWeight = FontWeights.Bold,
            HorizontalAlignment = HorizontalAlignment.Center,
            Foreground = new SolidColorBrush(Color.FromRgb(51, 51, 51))
        };
        
        var subtitleLabel = new Label
        {
            Content = "Application Installer",
            FontSize = 14,
            HorizontalAlignment = HorizontalAlignment.Center,
            Foreground = new SolidColorBrush(Color.FromRgb(102, 102, 102))
        };
        
        headerPanel.Children.Add(titleLabel);
        headerPanel.Children.Add(subtitleLabel);
        
        return headerPanel;
    }
    
    private UIElement CreateContentArea()
    {
        var contentStack = new StackPanel
        {
            Orientation = Orientation.Vertical
        };
        
        var welcomeText = new TextBlock
        {
            Text = "Welcome to MikoInstaller!\n\nThis application demonstrates WPF with React-like component structure.",
            FontSize = 14,
            TextWrapping = TextWrapping.Wrap,
            Margin = new Thickness(0, 0, 0, 20),
            HorizontalAlignment = HorizontalAlignment.Center,
            TextAlignment = TextAlignment.Center
        };
        
        // Add the InstallationCard component
        var installationCard = new InstallationCard();
        var cardElement = installationCard.Render();
        
        contentStack.Children.Add(welcomeText);
        contentStack.Children.Add(cardElement);
        
        return contentStack;
    }
    
    private UIElement CreateFooter()
    {
        var footerPanel = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            HorizontalAlignment = HorizontalAlignment.Center
        };
        
        var statusLabel = new Label
        {
            Content = "Ready",
            FontSize = 12,
            Foreground = new SolidColorBrush(Color.FromRgb(102, 102, 102))
        };
        
        footerPanel.Children.Add(statusLabel);
        
        return footerPanel;
    }
}