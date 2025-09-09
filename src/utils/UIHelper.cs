using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace MikoInstaller.Utils;

public static class UIHelper
{
    public static Button CreateButton(string content, double width = 100, double height = 30)
    {
        return new Button
        {
            Content = content,
            Width = width,
            Height = height,
            FontSize = 12,
            Background = new SolidColorBrush(Color.FromRgb(0, 120, 215)),
            Foreground = Brushes.White,
            BorderThickness = new Thickness(0),
            Cursor = System.Windows.Input.Cursors.Hand
        };
    }
    
    public static Label CreateLabel(string content, double fontSize = 12, HorizontalAlignment alignment = HorizontalAlignment.Left)
    {
        return new Label
        {
            Content = content,
            FontSize = fontSize,
            HorizontalAlignment = alignment,
            Foreground = new SolidColorBrush(Color.FromRgb(51, 51, 51))
        };
    }
    
    public static TextBlock CreateTextBlock(string text, double fontSize = 12, TextWrapping wrapping = TextWrapping.NoWrap)
    {
        return new TextBlock
        {
            Text = text,
            FontSize = fontSize,
            TextWrapping = wrapping,
            Foreground = new SolidColorBrush(Color.FromRgb(51, 51, 51))
        };
    }
    
    public static Border CreateCard(UIElement content, double cornerRadius = 8)
    {
        return new Border
        {
            Background = new SolidColorBrush(Color.FromRgb(48, 48, 48)),
            CornerRadius = new CornerRadius(cornerRadius),
            Padding = new Thickness(15),
            Margin = new Thickness(0, 10, 0, 0),
            BorderBrush = new SolidColorBrush(Color.FromRgb(64, 64, 64)),
            BorderThickness = new Thickness(1),
            Child = content
        };
    }
    
    public static StackPanel CreateVerticalStack(double spacing = 10)
    {
        return new StackPanel
        {
            Orientation = Orientation.Vertical,
            Margin = new Thickness(0, 0, 0, spacing)
        };
    }
    
    public static StackPanel CreateHorizontalStack(double spacing = 10)
    {
        return new StackPanel
        {
            Orientation = Orientation.Horizontal,
            Margin = new Thickness(0, 0, spacing, 0)
        };
    }
}