using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace ManagementAgent
{
    public partial class MainForm : Form
    {

        UInt32 last_rec = 0;
        UInt32 last_sent = 0;

        bool isUsetStop = false;

        public MainForm()
        {
            InitializeComponent();
        }

        private void MainForm_Load(object sender, EventArgs e)
        {
            
            LoadConfig();
            
        }

        private void timerStatus_Tick(object sender, EventArgs e)
        {

            Services.Config.Load();
            Process p = null;
            try
            {
                p = Process.GetProcessById(Services.Config.ProcessID);
            }
            catch { }
            if (p == null || Services.Config.ProcessID == 0)
            {
                lblStatus.Text = "Stoped";
                lblStatus.BackColor = Color.OrangeRed;
                btnStartStop.Text = "Start";
                txtUpTime.Text = "00:00:00";

                lblConnectionStatus.Text = "Disconnected";
                lblConnectionStatus.BackColor = Color.OrangeRed;

                if (!isUsetStop && cbAutoconnect.Checked) btnStartStop_Click(null, null);

            }
            else
            {
                lblStatus.Text = "Runing";
                lblStatus.BackColor = Color.GreenYellow;
                btnStartStop.Text = "Stop";
                var ts = new TimeSpan(DateTime.Now.Ticks) - new TimeSpan(Services.Config.StartTime);
                txtUpTime.Text = ts.TotalHours.ToString("00") + ":" + ts.Minutes.ToString("00") + ":" + ts.Seconds.ToString("00");

                Services.Memory.OpenMem();
                if (Services.Memory.Connected > 0)
                {
                    lblConnectionStatus.Text = "Connected";
                    lblConnectionStatus.BackColor = Color.GreenYellow;
                    //lblDataSent.Text = GetBytesCount(Services.Memory.DataSent);
                    //lblDataRec.Text = GetBytesCount(Services.Memory.DataRec);
                    lblClients.Text = Services.Memory.CLients.ToString();
                    CalcSpeed();
                }
                else
                {
                    lblConnectionStatus.Text = "Disconnected";
                    lblConnectionStatus.BackColor = Color.OrangeRed;
                }
            }

            
        }

        private string GetBytesCount(UInt32 _bytes) {

            if (_bytes < 1024) {
                return _bytes.ToString() + " B";
            }
            if (_bytes < (1024*1024))
            {
                return (_bytes/1024).ToString("#.0") + " KB";
            }
            if (_bytes < (1024 * 1024 * 1024))
            {
                return (_bytes / (1024*1024)).ToString("#.0") + " MB";
            }
            return (_bytes / (1024 * 1024 * 1024)).ToString("#.0") + " GB";
        }

        private string GetBytesSpeed(UInt32 _bytes)
        {

            if (_bytes < 1024)
            {
                return _bytes.ToString() + " Bps";
            }
            if (_bytes < (1024 * 1024))
            {
                return (_bytes / 1024).ToString("#.0") + " KBps";
            }
            if (_bytes < (1024 * 1024 * 1024))
            {
                return (_bytes / (1024 * 1024)).ToString("#.0") + " MBps";
            }
            return (_bytes / (1024 * 1024 * 1024)).ToString("#.0") + " GBpS";
        }

        private void CalcSpeed()
        {

            if (last_rec > Services.Memory.DataRec)
            {
                last_rec = Services.Memory.DataRec;
            }
            if (last_sent > Services.Memory.DataSent)
            {
                last_sent = Services.Memory.DataSent;
            }

            var send_speed = (Services.Memory.DataSent - last_sent) * 2; // read each half second
            var rec_speed = (Services.Memory.DataRec - last_rec) * 2;// read each half second

            last_rec = Services.Memory.DataRec;
            last_sent = Services.Memory.DataSent;
            lblDataSent.Text = GetBytesSpeed(send_speed);
            lblDataRec.Text = GetBytesSpeed(rec_speed);

        }
        
        private void LoadConfig() {

            Services.Config.Load();
            txtServerIP.Text = Services.Config.CloudServer;
            txtPort.Text = Services.Config.CloudPort;
            txtPassword.Text = Services.Config.Password;
            txtEndpointServer.Text = Services.Config.EndpointServer;
            txtEndpointPort.Text = Services.Config.EndpointPort;
            cbAutoconnect.Checked = Services.Config.Autoconnect;
        }

        private void SaveConfig()
        {
            Services.Config.CloudServer  = txtServerIP.Text;
            Services.Config.CloudPort = txtPort.Text;
            Services.Config.Password = txtPassword.Text;
            Services.Config.EndpointServer= txtEndpointServer.Text;
            Services.Config.EndpointPort = txtEndpointPort.Text;
            Services.Config.Save();
        }

        private void groupBox2_Enter(object sender, EventArgs e)
        {

        }

        private void btnStartStop_Click(object sender, EventArgs e)
        {

            switch (btnStartStop.Text)
            {

                case "Start":                    
                   Services.Config.StartProcess();
                    isUsetStop = false;
                    break;
                case "Stop":
                    isUsetStop = true;
                    Services.Config.StopProcess();
                    break;
                default:
                    break;

            }


        }

       
             
        private void btnSave_Click(object sender, EventArgs e)
        {
            SaveConfig();
        }

        private void btnClose_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void cbAutoconnect_CheckedChanged(object sender, EventArgs e)
        {
            try
            {
                Services.Config.Autoconnect = cbAutoconnect.Checked;
                Services.Config.Save();
            }
            catch { }
        }

    }
}
