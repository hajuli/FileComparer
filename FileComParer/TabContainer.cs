using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Drawing;

namespace FileComparer
{
    class TabContainer
    {
        DataArea m_configeData = null;
        TabControl m_tabControl;
        List<string> m_tabNames;
        List<UserDataGridView> m_UDGVRefs;
        List<Label> m_showNumsLabels = null;
        List<TextBox> m_selectConTextBoxs = null;

        DllCallBackProcessor    m_serviceProcessor;


        public TabContainer(TabControl tabControl, DataArea tabs)
        {
            m_configeData = tabs;
            m_tabNames = new List<string>();
            m_UDGVRefs = new List<UserDataGridView>();
            m_showNumsLabels = new List<Label>();
            m_selectConTextBoxs = new List<TextBox>();
            m_tabControl = tabControl;
            m_tabControl.Controls.Clear();  //remove example tabpage.

            m_tabControl.DrawMode = TabDrawMode.OwnerDrawFixed;
            m_tabControl.DrawItem += new DrawItemEventHandler(m_tabControl_DrawItem);

            int i = 0;
            for (i = 0; i < m_configeData.m_tabs.Count; ++i)
            {
                TabArea tab = m_configeData.m_tabs[i];
                TabPage tp = new TabPage(tab.m_displayName);
                tp.Name = tab.m_tabName;
                tp.Anchor = AnchorStyles.Left | AnchorStyles.Top | AnchorStyles.Right | AnchorStyles.Bottom;
                m_tabControl.Controls.Add(tp);
                m_tabNames.Add(tab.m_tabName);

                tp.BackColor = ConfigParser.getConfigParaColor(m_configeData.m_tabPageBackColor);
                
                //if (tp.Name == "showAllFiles")
                {
                    TextBox selectTextBox = new TextBox();
                    selectTextBox.SetBounds(0, 0, tp.ClientSize.Width, 20);
                    selectTextBox.Anchor = AnchorStyles.Left | AnchorStyles.Top | AnchorStyles.Right;
                    selectTextBox.BackColor = ConfigParser.getConfigParaColor(m_configeData.m_selectConditionBackColor);
                    selectTextBox.Show();
                    selectTextBox.Name = tab.m_tabName;
                    m_selectConTextBoxs.Add(selectTextBox);
                    tp.Controls.Add(selectTextBox);

                    selectTextBox.TextChanged += new System.EventHandler(this.selectBox_TextChanged);
                }
                {
                    Rectangle r = tp.ClientRectangle;
                    UserDataGridView dgv = new UserDataGridView(tab.m_table);
                    dgv.SetBounds(0, 0, tp.ClientSize.Width, tp.ClientSize.Height);
                    dgv.SetBounds(0, 20, tp.ClientSize.Width, tp.ClientSize.Height - 38);
                    m_UDGVRefs.Add(dgv);
                    tp.Controls.Add(dgv);
                }
                {
                    Label countLabel = new Label();
                    countLabel.SetBounds(0, tp.ClientSize.Height - 18, tp.ClientSize.Width, tp.ClientSize.Height - 20);
                    countLabel.Anchor = AnchorStyles.Left | AnchorStyles.Bottom | AnchorStyles.Right;
                    countLabel.BackColor = ConfigParser.getConfigParaColor(m_configeData.m_showCountLabelBackColor);
                    countLabel.Show();
                    countLabel.Text = "showed 0 / 0 objects";
                    m_showNumsLabels.Add(countLabel);
                    tp.Controls.Add(countLabel); 
                }
            }

            //UserListView ulv = new UserListView();
            //ulv.showPaths(@"abcd,E:\test_code\FileComparer\FileManager;abcd,E:\test_code\FileComparer\FileManager\fileManager.cpp");
            //ulv.Show();
        }

        void m_tabControl_DrawItem(object sender, DrawItemEventArgs e)
        {
            //from: http://www.vbforums.com/showthread.php?355093-Setting-selected-tabpage-header-text-to-bold

            TabPage currentab = m_tabControl.TabPages[e.Index];
            SolidBrush textbrush = new SolidBrush(Color.Black);
            Rectangle itemrect = m_tabControl.GetTabRect(e.Index);
            StringFormat sf = new StringFormat();
            sf.Alignment = StringAlignment.Center;
            sf.LineAlignment = StringAlignment.Center;


            if (e.Index + 1 == m_tabControl.TabPages.Count)
            {
                Rectangle rect = itemrect;
                rect.Width = currentab.Width;
                e.Graphics.FillRectangle(
                    new SolidBrush(m_tabControl.Parent.BackColor),
                    rect);
            }
            e.Graphics.FillRectangle(
                new SolidBrush(ConfigParser.getConfigParaColor(m_configeData.m_tabPageBackColor)),
                itemrect);
            if (Convert.ToBoolean(e.State & DrawItemState.Selected))
            {
                Font f = new Font(m_tabControl.Font.Name, m_tabControl.Font.Size, FontStyle.Bold);
                e.Graphics.DrawString(currentab.Text, f, textbrush, itemrect, sf);
            }
            else
            {
                e.Graphics.DrawString(currentab.Text, e.Font, textbrush, itemrect, sf);
            }
            textbrush.Dispose();
        }

        public void selectBox_TextChanged(object sender, EventArgs e)
        {
            TextBox tb = ((TextBox)sender);
            m_serviceProcessor.sendMessage(MessageTypes.UpdateSelectCondition, tb.Name + "_" + tb.Text);
        }

        public void processMessages(string msgType, string msg)
        {
            string tabName = MessageTypes.getAttributeValue(
                MessageTypes.MessageAttibuteTabName, msg);

            int i = 0;
            for (i = 0; i < m_tabNames.Count; ++i)
            {
                if (tabName == m_tabNames[i])
                {
                    if (MessageTypes.UpdateShowedNums == msgType)
                    {
                        m_showNumsLabels[i].Text = "showed " + MessageTypes.getAttributeValue("value", msg) + " objects";
                        return ;
                    }
                    m_UDGVRefs[i].processMessages(msgType, msg);
                    return;
                }
            }
            if (MessageTypes.ShowSameFileAllPaths == msgType)
            {
                UserListView ulv = new UserListView();
                if (ulv.showPaths(msg))
                {
                    ulv.Show();
                }
            }
        }

        public void focusToSelectCondition()
        {
            int i = m_tabControl.SelectedIndex;
            if (0 <= i && i < m_selectConTextBoxs.Count)
            {
                m_selectConTextBoxs[i].Focus();
                m_selectConTextBoxs[i].SelectAll();
            }
        }

        public void setServiceProcessor(DllCallBackProcessor pro)
        {
            m_serviceProcessor = pro;
            int i = 0;
            for (i = 0; i < m_tabNames.Count; ++i)
            {
                m_UDGVRefs[i].setServiceProcessor(pro);
            }
        }
    }
}
