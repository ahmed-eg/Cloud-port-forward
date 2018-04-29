using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Linq;
using System.ServiceProcess;
using System.Text;
using System.Threading.Tasks;

namespace WindowsServiceAgent
{
    public partial class Service1 : ServiceBase
    {
        public Service1()
        {
            InitializeComponent();
        }
        string[] args2 = null;

        protected override void OnStart(string[] args)
        {

            ManagementAgent.Services.Config.Load();

            if (!ManagementAgent.Services.Config.isLoaded) Stop();

            new Task(CheckService).Start();

        }

        private void CheckService() {


            Process p = null;

            while (true)
            {
                try
                {
                    ManagementAgent.Services.Config.Load();
                    if (ManagementAgent.Services.Config.Autoconnect)
                    {
                        try
                        {
                            p = null;
                            p = Process.GetProcessById(ManagementAgent.Services.Config.ProcessID);
                        }
                        catch { }
                        if (p == null || ManagementAgent.Services.Config.ProcessID == 0)
                        {
                            //if (cbAutoconnect.Checked) btnStartStop_Click(null, null);
                            ManagementAgent.Services.Config.StartProcess();
                        }
                    }
                }
                catch { }
                System.Threading.Thread.Sleep(3000);
            }

        }

        protected override void OnStop()
        {



        }
    }
}
