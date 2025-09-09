using System;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using MikoInstaller.Components;
using MikoInstaller.Utils;

namespace MikoInstaller;

public class Program
{
    [STAThread]
    public static void Main(string[] args)
    {
        var app = new Application();
        
        // Show progress dialog for asset extraction
        var progressDialog = WindowsAPI.ShowProgressDialog("Miko Installer", "Initializing installer...");
        
        try
        {
            // Load embedded assets to memory in background
            Task.Run(() =>
            {
                WindowsAPI.UpdateProgressDialog(progressDialog, "Loading assets...");
                Thread.Sleep(500); // Brief delay to show progress
                
                var assets = WindowsAPI.LoadEmbeddedAssetsToMemory();
                
                WindowsAPI.UpdateProgressDialog(progressDialog, "Preparing installer interface...");
                Thread.Sleep(300);
                
                // Close progress dialog and show main installer
                app.Dispatcher.Invoke(() =>
                {
                    WindowsAPI.CloseProgressDialog(progressDialog);
                    
                    // Create and show installer wizard
                    var installerWizard = new InstallerWizard();
                    
                    // Set application properties
                    app.MainWindow = installerWizard;
                    app.ShutdownMode = ShutdownMode.OnMainWindowClose;
                    
                    // Show window
                    installerWizard.Show();
                });
            });
        }
        catch (Exception ex)
        {
            WindowsAPI.CloseProgressDialog(progressDialog);
            System.Windows.MessageBox.Show($"Failed to initialize installer: {ex.Message}", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            return;
        }
        
        app.Run();
    }
}
