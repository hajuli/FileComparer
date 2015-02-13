using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Drawing;

namespace FileComparer
{
    class DataPanelView : Panel
    {
        string m_name = "DataPanelViewNoName";
        TextBox m_selectConTextBox = null;
        UserDataGridView m_UDGVRef = null;
        Label m_showNumsLabel = null;

        public DataPanelView(string name)
        {
            m_name = name;
        }

        public void Init(Control p, DataTable data)
        {
            this.Size = p.Size;
            this.Anchor = p.Anchor;

            {
                TextBox selectTextBox = new TextBox();
                selectTextBox.SetBounds(0, 0, p.ClientSize.Width, 20);
                selectTextBox.Anchor = AnchorStyles.Left | AnchorStyles.Top | AnchorStyles.Right;
                selectTextBox.BackColor = ConfigParser.getConfigParaColor("yellow");
                selectTextBox.Show();
                selectTextBox.Name = "selectTextBox";
                m_selectConTextBox = selectTextBox;
                this.Controls.Add(selectTextBox);

                selectTextBox.TextChanged += new System.EventHandler(this.selectBox_TextChanged);
            }
            {
                Rectangle r = p.ClientRectangle;
                m_UDGVRef = new UserDataGridView(m_name, data);
                m_UDGVRef.SetBounds(0, 0, p.ClientSize.Width, p.ClientSize.Height);
                m_UDGVRef.SetBounds(0, 20, p.ClientSize.Width, p.ClientSize.Height - 38);
                this.Controls.Add(m_UDGVRef);
            }
            {
                m_showNumsLabel = new Label();
                m_showNumsLabel.SetBounds(0, p.ClientSize.Height - 18, p.ClientSize.Width, p.ClientSize.Height - 20);
                m_showNumsLabel.Anchor = AnchorStyles.Left | AnchorStyles.Bottom | AnchorStyles.Right;
                m_showNumsLabel.BackColor = ConfigParser.getConfigParaColor("red");
                m_showNumsLabel.Show();
                m_showNumsLabel.Text = "showed 0 / 0 objects";
                this.Controls.Add(m_showNumsLabel);
            }
        }

        public void selectBox_TextChanged(object sender, EventArgs e)
        {
            TextBox tb = ((TextBox)sender);
            string msg = GlobalValues.PN_ShowName + MessageTypes.MessageAttibuteSplitSign + m_name
                    + MessageTypes.MessageSplitSign
                    + GlobalValues.PN_SelectCondition + MessageTypes.MessageAttibuteSplitSign + tb.Text;
            GlobalValues.ServiceHandle.sendMessage(GlobalValues.MSG_UpdateSelectCondition, msg);
        }

        public void processMessages(string msgType, string msg)
        {
            if (GlobalValues.MSG_UpdateShowedNums == msgType)
            {
                m_showNumsLabel.Text = "showed " + MessageTypes.getAttributeValue(GlobalValues.PN_MessageValue, msg) + " objects";
                return;
            }
            else if (MessageTypes.ShowSameFileAllPaths == msgType)
            {
                string name = MessageTypes.getAttributeValue(GlobalValues.PN_MessageValue, msg);
                string paths = MessageTypes.getAttributeValue(GlobalValues.PN_MessageValue2, msg);
                UserListView ulv = new UserListView();
                if (ulv.showPaths(name, paths))
                {
                    ulv.Show();
                }
                return;
            }
            m_UDGVRef.processMessages(msgType, msg);
        }
    }
}
