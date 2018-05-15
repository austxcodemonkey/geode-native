using System;
using System.Diagnostics;
using System.Net;
using System.IO;
using System.Net.Sockets;
using Xunit;
using System.Collections.Generic;

public class GeodeServer : IDisposable
{
    #region Properties/Fields

    public int LocatorPort { get; private set; }

    #endregion

    #region Public methods

    public void init(List<string> regionNames, bool readSerialized = false)
    {
        try
        {
            //Clean up previous server dirs
            foreach (var dir in new DirectoryInfo(Environment.CurrentDirectory).GetDirectories())
            {
                dir.Delete(true);
            }
        }
        catch
        {
            //Ignored
        }

        LocatorPort = FreeTcpPort();
        var locatorJmxPort = FreeTcpPort();

        var readSerializedStr = readSerialized ? "--read-serialized=true" : "--read-serialized=false";

        var gfshCommand = " -e \"start locator --bind-address=localhost --port=" + LocatorPort +
                    " --J=-Dgemfire.jmx-manager-port=" + locatorJmxPort + " --http-service-port=0\"" +
                    " -e \"connect --locator=localhost[" + LocatorPort + "]\"" +
                    " -e \"configure pdx " + readSerializedStr + "\"" +
                    " -e \"start server --bind-address=localhost --server-port=0 --log-level=all\"";
        foreach (var regionName in regionNames)
        {
          gfshCommand += " -e \"create region --name=" + regionName + " --type=PARTITION\"";
        }

    var gfsh = new Process
    {
      StartInfo =
            {
                FileName = Config.GeodeGfsh,
                Arguments = gfshCommand,
                WindowStyle = ProcessWindowStyle.Hidden,
                UseShellExecute = false,
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                CreateNoWindow = true
            }
        };

        gfsh.OutputDataReceived += (sender, args) =>
        {
            if (null != args.Data)
                Debug.WriteLine("GeodeServer: " + args.Data);
        };

        gfsh.ErrorDataReceived += (sender, args) =>
        {
            if (null != args.Data)
                Debug.WriteLine("GeodeServer: ERROR: " + args.Data);
        };

        gfsh.Start();
        gfsh.BeginOutputReadLine();
        gfsh.BeginErrorReadLine();
        gfsh.WaitForExit(60000 * 10);

        Debug.WriteLine("GeodeServer Start: gfsh.HasExited = {0}, gfsh.ExitCode = {1}",
            gfsh.HasExited, gfsh.ExitCode);

        Assert.True(gfsh.HasExited);
        Assert.Equal(0, gfsh.ExitCode);
    }

    public GeodeServer(List<string> regionNames, bool readSerialized = false)
    {
      init(regionNames, readSerialized);
    }
    public GeodeServer(string regionName = "testRegion", bool readSerialized = false)
    {
      init(new List<string> { regionName, "testRegion1" }, readSerialized);
    }

    public void Dispose()
    {
        try
        {
            var gfsh = new Process
            {
                StartInfo =
                {
                    FileName = Config.GeodeGfsh,
                    Arguments = "-e \"connect --locator=localhost[" + LocatorPort +
                                "]\" -e \"shutdown --include-locators true\" ",
                    WindowStyle = ProcessWindowStyle.Hidden,
                    UseShellExecute = false,
                    RedirectStandardOutput = true,
                    RedirectStandardError = true,
                    CreateNoWindow = true
                }
            };

            gfsh.OutputDataReceived += (sender, args) =>
            {
                if (null != args.Data)
                    Debug.WriteLine("GeodeServer: " + args.Data);
            };

            gfsh.ErrorDataReceived += (sender, args) =>
            {
                if (null != args.Data)
                    Debug.WriteLine("GeodeServer: ERROR: " + args.Data);
            };

            gfsh.Start();
            gfsh.BeginOutputReadLine();
            gfsh.BeginErrorReadLine();
            gfsh.WaitForExit(30000);
        }
        catch
        {
            // ignored
        }
    }

    #endregion

    #region Private Methods

    private static int FreeTcpPort()
    {
        var tcpListner = new TcpListener(IPAddress.Loopback, 0);
        tcpListner.Start();
        var port = ((IPEndPoint)tcpListner.LocalEndpoint).Port;
        tcpListner.Stop();
        return port;
    }

    #endregion
}
