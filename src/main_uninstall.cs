using System;
using System.Windows;
using MikoInstaller.Utils;

namespace MikoInstaller
{
    public class UninstallProgram
    {
        [STAThread]
        public static void Main(string[] args)
        {
            try
            {
                var app = new Application();
                var uninstallerWizard = new UninstallerWizard();
                app.Run(uninstallerWizard);
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Uninstaller error: {ex.Message}", "Error", MessageBoxButton.OK, MessageBoxImage.Error);
            }
        }
    }
}