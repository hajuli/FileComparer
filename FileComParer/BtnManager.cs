using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Drawing;

namespace FileComparer
{
    class BtnManager
    {
        Control m_mainPanel = null;
        Control m_allFilesBtn = null;
        Control m_sameFilesBtn = null;
        Control m_moreFilesBtn = null;
        DataPanelView m_allFilesPanel = null;

        Button m_currentButton = null;
        Dictionary<string, string> m_btnClickMsg = null;
        Dictionary<string, DataPanelView> m_dataPanels = null;

        public BtnManager(Control panel)
        {
            m_mainPanel = panel;
            m_mainPanel.BackColor = Color.Gray;
            m_mainPanel.Anchor = AnchorStyles.Left | AnchorStyles.Right | AnchorStyles.Top | AnchorStyles.Bottom;

            m_btnClickMsg = new Dictionary<string, string>();
            m_dataPanels = new Dictionary<string, DataPanelView>();
        }

        public void addButton(Button btn, string showType, string pgName, string cmpPgName)
        {
            string name = pgName + showType;
            if (m_btnClickMsg.ContainsKey(name))
            {
                MessageBox.Show("exist button:" + name);
                return;
            }
            btn.Name = name;
            string moreMsg = "";
            DataTable dt = null;
            bool addClickMsg = false;

            if (GlobalValues.PV_ShowType_AllFiles == showType)
            {
                dt = GlobalValues.getDataTable(GlobalValues.AllFilesTableName);
                addClickMsg = true;
            }
            else if (GlobalValues.PV_ShowType_SameFiles == showType)
            {
                dt = GlobalValues.getDataTable(GlobalValues.SameFilesTableName);
                addClickMsg = true;
            }
            else if (GlobalValues.PV_ShowType_MoreFiles == showType)
            {
                moreMsg = MessageTypes.MessageSplitSign
                    + GlobalValues.PN_CmpToParttGroup + MessageTypes.MessageAttibuteSplitSign + cmpPgName;
                dt = GlobalValues.getDataTable(GlobalValues.AllFilesTableName);
                addClickMsg = true;
            }
            if (addClickMsg)
            {
                string clickMsg = GlobalValues.PN_ShowName + MessageTypes.MessageAttibuteSplitSign + name
                    + MessageTypes.MessageSplitSign
                    + GlobalValues.PN_ShowType + MessageTypes.MessageAttibuteSplitSign + showType
                    + MessageTypes.MessageSplitSign
                    + GlobalValues.PN_PartitionGroup + MessageTypes.MessageAttibuteSplitSign + pgName
                    + moreMsg;
                m_btnClickMsg.Add(name, clickMsg);

                btn.Name = name;
                btn.Text = GlobalValues.get(showType);
                btn.Click += new EventHandler(AllFilesBtn_Click);

                DataPanelView dataPanel = new DataPanelView(name);
                dataPanel.Init(m_mainPanel, dt);
                m_dataPanels.Add(btn.Name, dataPanel);

                if (null == m_currentButton)
                {
                    //btn.PerformClick();
                    AllFilesBtn_Click(btn, EventArgs.Empty);
                }
            }
        }

        private void AllFilesBtn_Click(object sender, EventArgs e)
        {
            Button btn = (Button)sender;
            string name = btn.Name;
            if (m_dataPanels.ContainsKey(btn.Name) && m_currentButton != btn)
            {
                if (null != m_currentButton)
                {
                    m_currentButton.BackColor = GlobalValues.getColorFromString(
                        GlobalValues.get(GlobalValues.AllBtnDefualtColor));
                }
                m_currentButton = btn;
                btn.BackColor = Color.Yellow;
                m_mainPanel.Controls.Clear();
                m_mainPanel.Controls.Add(m_dataPanels[btn.Name]);

                GlobalValues.ServiceHandle.sendMessage(GlobalValues.MSG_SetCurrentShow, m_btnClickMsg[name]);
            }
        }

        public void processMessages(string msgType, string msg)
        {
            string name = MessageTypes.getAttributeValue(GlobalValues.PN_ShowName, msg);
            if (m_dataPanels.ContainsKey(name))
            {
                m_dataPanels[name].processMessages(msgType, msg);
            }
        }
    }
}
