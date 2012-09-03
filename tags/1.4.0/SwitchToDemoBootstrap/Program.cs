using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Resources;
using System.Windows.Forms;

static class Program
{
    /// <summary>
    /// The main entry point for the application.
    /// </summary>
    [STAThread]
    static void Main()
    {
        var baseDir = Path.Combine(
            Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),
            "SwitchToDemo");
        Directory.CreateDirectory(baseDir);

        var exePath = Path.Combine(baseDir, "SwitchToDemo.exe");

        using (var stream = typeof(Program).Assembly.GetManifestResourceStream("SwitchToDemoBootstrap.SwitchToDemo.exe"))
        {
            var buff = new byte[stream.Length];
            stream.Read(buff, 0, buff.Length);
            try
            {
                File.WriteAllBytes(exePath, buff);
            }
            catch (System.IO.IOException) { }
        }

        Process.Start(exePath);
    }
}
