using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;

namespace ManagementAgent.Services
{
    public class Config
    {

        public static string ProcessName = "CloudTcpForwarderAgent.exe";
        public static string Password { get; set; }
        public static string CloudServer { get; set; }
        public static string CloudPort { get; set; }
        public static string EndpointServer { get; set; }
        public static string EndpointPort { get; set; }
        public static int ProcessID { get; set; }
        public static long StartTime { get; set; }
        public static bool Autoconnect { get; set; }

        public static bool isLoaded = false;

        public static void Load()
        {
            try
            {
               // if (isLoaded) return;

                string l = "";
                using (StreamReader sr = new StreamReader(GetFileName()))
                {

                    while (!sr.EndOfStream)
                    {
                        l = sr.ReadLine();
                        try
                        {
                            if (!l.StartsWith("#"))
                            {
                                ProcessLine(l);
                            }
                        }// inside loop in each line
                        catch { }

                    }
                    isLoaded = true;
                }
            }
            catch { }
        }

        private static void ProcessLine(string l)
        {
            var lin = l.Split('=');
            switch (lin[0].Trim())
            {
                case "CloudServer":
                    CloudServer = lin[1].Trim();
                    break;
                case "CloudPort":
                    CloudPort = lin[1].Trim();
                    break;
                case "EndpointServer":
                    EndpointServer = lin[1].Trim();
                    break;
                case "EndpointPort":
                    EndpointPort = lin[1].Trim();
                    break;
                case "Password":
                    Password = lin[1].Trim();
                    break;
                case "ProcessID":
                    ProcessID = int.Parse(lin[1].Trim());
                    break;
                case "StartTime":
                    StartTime = long.Parse(lin[1].Trim());
                    break;
                case "Autoconnect":
                    Autoconnect = bool.Parse(lin[1].Trim());
                    break;
                default:
                    break;
            }
        }

        public static bool Save()
        {
            try
            {
                using (StreamWriter sw = new StreamWriter(GetFileName(), false))
                {
                    sw.WriteLine("CloudServer=" + CloudServer);
                    sw.WriteLine("CloudPort=" + CloudPort);
                    sw.WriteLine("EndpointServer=" + EndpointServer);
                    sw.WriteLine("EndpointPort=" + EndpointPort);
                    sw.WriteLine("Password=" + Password);
                    sw.WriteLine("ProcessID=" + ProcessID);
                    sw.WriteLine("StartTime=" + StartTime);
                    sw.WriteLine("Autoconnect=" + Autoconnect);
                }
                return true;
            }
            catch
            {
                return false;
            }
        }

        private static string GetFileName()
        {

            var path = System.Reflection.Assembly.GetEntryAssembly().Location;
            var exefilename = path.Split('\\').Last();

            return path.Replace(exefilename, "") + "config.ini";

        }

        public static string StartProcess()
        {
            try
            {
                var p = new ProcessStartInfo();
                var arg = new StringBuilder(CloudServer).Append(" ").Append(CloudPort).Append(" ")
                   .Append(EndpointServer).Append(" ").Append(EndpointPort).Append(" ");

                p.Arguments = arg.ToString();
                p.FileName = Services.Config.ProcessName;
                p.CreateNoWindow = true;

                //p.RedirectStandardOutput = true;

                p.UseShellExecute = false; // dont show command line

                var sp = new Process();
                sp.StartInfo = p;
                //sp.OutputDataReceived += new DataReceivedEventHandler(Sp_OutputDataReceived);
                // sp.BeginOutputReadLine();
                sp.Start();
                ProcessID = sp.Id;
                StartTime = DateTime.Now.Ticks;
                Save();
                return "OK";
            }
            catch (Exception ex) { return (ex.ToString()); }
        }

        public static string StopProcess()
        {
            try
            {
                var p = Process.GetProcessById(ProcessID);
                if (p != null)
                {
                    p.Kill();
                    p.Dispose();
                }
                return "OK";
            }
            catch (Exception ex) { return (ex.ToString()); }
        }
    }
}

 
