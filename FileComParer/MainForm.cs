using System;
using System.IO;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.Threading;

namespace FileComparer
{
    public partial class MainForm : Form
    {
        // This delegate enables asynchronous calls for setting 
        // the text property on a TextBox control. 
        delegate void SetTextCallback(string text);
        delegate void ProcessMsgCallback(string msgType, string msg);

        //TabContainer    m_tabContainer;
        System.Windows.Forms.Timer m_myTimer;
        DllCallBackProcessor    m_dllProcessor;
        ConfigParser            m_configs = null;
        Form_showLoadingRate    m_loadingRate = null;
        Form                    m_grayBack = null;
        BtnManager              m_parttGroup = null;

        string m_selectedVolume = "";

        [System.Runtime.InteropServices.DllImport("user32.dll")]
        private static extern IntPtr SendMessage(IntPtr hWnd, int msg, IntPtr wp, IntPtr lp);
        // size: (1000, 600);
        public MainForm()
        {
            GlobalValues.InitDefualtParas();
            InitializeComponent(); //Thread.Sleep(10 * 1000);
            //WinHelper.sendKeysMessage("windbg", (IntPtr)(Keys.Alt | Keys.D));
            //WinHelper.sendKeysMessage("windbg", (IntPtr)(Keys.B));
            //WinHelper.sendKeysMessage("notepad++", (IntPtr)(Keys.Alt | Keys.F));
            //WinHelper.sendKeysMessage("notepad++", (IntPtr)( Keys.O));

            this.Size = new Size(1000, 600);
            this.CurrentTimeLab.Text = "";
            m_configs = new ConfigParser();
            m_configs.parseFile(@"configUI.xml");
            GlobalValues.ConfigeData = m_configs;
            this.Text = m_configs.getSpecialPara(ConfigParser.AppTitle, "SameFileFinder");
            this.BackColor = ConfigParser.getConfigParaColor(
                m_configs.getSpecialPara(ConfigParser.mainFormBackColor, ConfigParser.defaultBackColor));

            this.FormStatusLabel.Text = "main Form init ok";

            m_dllProcessor = new DllCallBackProcessor(this);
            m_dllProcessor.registerToDll();
            GlobalValues.ServiceHandle = m_dllProcessor;


            m_myTimer = new System.Windows.Forms.Timer();
            m_myTimer.Tick += new EventHandler(this.TimerEventProcessor);
            m_myTimer.Interval = 100;
            m_myTimer.Start();

            this.partitionMulSel1.Name = GlobalValues.PartitionGroupName1;
            this.partitionMulSel2.Name = GlobalValues.PartitionGroupName2;
            this.partitionMulSel1.Init(GlobalValues.PartitionGroupName1, this);
            this.partitionMulSel2.Init(GlobalValues.PartitionGroupName2, this);



            m_parttGroup = new BtnManager(this.mainPanel);

            m_parttGroup.addButton(this.allFilesBtn1, GlobalValues.PV_ShowType_AllFiles, GlobalValues.PartitionGroupName1, "");
            m_parttGroup.addButton(this.allFilesBtn2, GlobalValues.PV_ShowType_AllFiles, GlobalValues.PartitionGroupName2, "");

            m_parttGroup.addButton(this.sameFilesBtn1, GlobalValues.PV_ShowType_SameFiles, GlobalValues.PartitionGroupName1, "");
            m_parttGroup.addButton(this.sameFilesBtn2, GlobalValues.PV_ShowType_SameFiles, GlobalValues.PartitionGroupName2, "");

            m_parttGroup.addButton(this.moreFilesBtn1, GlobalValues.PV_ShowType_MoreFiles, GlobalValues.PartitionGroupName1, GlobalValues.PartitionGroupName2);
            m_parttGroup.addButton(this.moreFilesBtn2, GlobalValues.PV_ShowType_MoreFiles, GlobalValues.PartitionGroupName2, GlobalValues.PartitionGroupName1);

            m_dllProcessor.sendMessage(GlobalValues.MSG_LoadAllVolumeIDs, "");
        }

        // This is the method to run when the timer is raised.
        private void TimerEventProcessor(Object myObject, EventArgs myEventArgs)
        {
            this.CurrentTimeLab.Text = DateTime.Now.ToString("yyyy-mm-dd HH:mm:ss");
        }
        protected override bool ProcessDialogKey(Keys keyData)
        {
            // this function called before OnKeyDown; "alt + enter" been process by base, can't reach to OnKeyDown.

            if (((Keys.Alt | Keys.D) == keyData) ||
                ((Keys.Control | Keys.F) == keyData))
            {
                //m_tabContainer.focusToSelectCondition();
                return true;
            }
            return base.ProcessDialogKey(keyData);
        }

        public void updateStatusMessage(string msg)
        {
            this.FormStatusLabel.Text = msg;
        }


        public void updateVolumeIDs(string msg)
        {
            int selectIndex = 0;
            this.partitionMulSel1.itemsList.Clear();
            this.partitionMulSel2.itemsList.Clear();
            string[] ids = msg.Split(new string[]{GlobalValues.VolumeIDsSeparator}, StringSplitOptions.RemoveEmptyEntries);
            for (int i = 0; i < ids.Length; ++i)
            {
                this.partitionMulSel1.itemsList.Add(ids[i]);
                this.partitionMulSel2.itemsList.Add(ids[i]);
                if (m_selectedVolume == ids[i])
                {
                    selectIndex = i + 1;
                }
            }
        }
        
        public void processCallBackMessage(string msgType, string msg)
        {
            // InvokeRequired required compares the thread ID of the 
            // calling thread to the thread ID of the creating thread. 
            // If these threads are different, it returns true. 
            if (this.InvokeRequired)
            {
                ProcessMsgCallback d = new ProcessMsgCallback(processCallBackMessage);
                this.BeginInvoke(d, new object[] { msgType, msg });
                return;
            }
            else
            {
                if (MessageTypes.ShowStatusMessage == msgType)
                {
                    updateStatusMessage(msg);
                }
                else if (MessageTypes.VolumeIDsUpdate == msgType)
                {
                    updateVolumeIDs(msg);
                }
                else if (MessageTypes.UpdateLoadingRate == msgType)
                {
                    updateLoadingRate(msg);
                }
                else if (null != m_parttGroup)
                {
                    m_parttGroup.processMessages(msgType, msg);
                }
            }
        }

        public void sendMsgToDll(string msgType, string msg)
        {
            m_dllProcessor.sendMessage(msgType, msg);
        }

        private void updateLoadingRate(string msg)
        {
            if (null != m_loadingRate)
            {
                string rate = MessageTypes.getAttributeValue("loadingRate", msg);
                m_loadingRate.setLoadingPath(MessageTypes.getAttributeValue("volume", msg));
                m_loadingRate.setLoadingRate(int.Parse(rate));
                if ("100" == rate)
                {
                    m_loadingRate.Close();
                }
            }
        }

        public void showLoadingRate(Form_showLoadingRate rateView)
        {
            // sometime this error, so new one everytime.

            //An unhandled exception of type 'System.ObjectDisposedException' occurred in System.Windows.Forms.dll
            //Additional information: Cannot access a disposed object.

            m_grayBack = new Form();    //new a new everytime.
            m_grayBack.BackColor = Color.Gray;
            m_grayBack.FormBorderStyle = FormBorderStyle.None;
            m_grayBack.Opacity = 0.20;
            m_grayBack.ShowIcon = false;
            m_grayBack.ShowInTaskbar = false;
            m_grayBack.StartPosition = FormStartPosition.Manual;
            m_grayBack.Location = this.Location;
            m_grayBack.Size = this.Size;
            m_grayBack.Show(this);

            m_loadingRate = rateView;
            m_loadingRate.ShowDialog(this);

            m_grayBack.Close();
            this.Show();
        }

        private void button_about_Click(object sender, EventArgs e)
        {
            AboutBox1 about = new AboutBox1();
            //about.Show();
            about.ShowDialog(this);
            
        }
    }
}
