using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace FileComparer
{
    public class MultiSelComboBox : ComboBox
    {
        public CheckedListBox lst = new CheckedListBox();

        public List<string> itemsList = new List<string>();
        private Dictionary<string, bool> checkedValues = new Dictionary<string, bool>();

        private string m_lastMulSelText;
        string m_parentName = "noParentName";
        MainForm m_mainForm = null;
        public MultiSelComboBox()
        {
            itemsList.Clear();
            checkedValues.Clear();
            m_lastMulSelText = "";
        }

        public void Init(string parentName, MainForm form)
        {
            m_parentName = parentName;
            m_mainForm = form;
            this.DrawMode = DrawMode.Normal;//DrawMode.OwnerDrawFixed;//只有设置这个属性为OwnerDrawFixed才可能让重画起作用
            this.IntegralHeight = false;
            this.DoubleBuffered = true;
            this.DroppedDown = false;
            this.DropDownHeight = 1;
            this.DropDownStyle = ComboBoxStyle.DropDownList;
            this.BackColor = Color.Blue;
            this.ForeColor = Color.Yellow;

            lst.CheckOnClick = true;
            lst.BorderStyle = BorderStyle.Fixed3D;
            lst.Visible = false;
            lst.LostFocus += new EventHandler(lst_LostFocus);
            lst.ItemCheck += new ItemCheckEventHandler(lst_ItemCheck);
            lst.BackColor = Color.LightBlue;
        }

        private void onMulSelChange(string newPartt)
        {
            if (m_lastMulSelText == newPartt)
            {
                return;
            }
            m_lastMulSelText = newPartt;

            string msg = GlobalValues.PN_PartitionGroup + MessageTypes.MessageAttibuteSplitSign + m_parentName;
            msg = msg + MessageTypes.MessageSplitSign + "Value=" + newPartt;

            GlobalValues.ServiceHandle.sendMessage(GlobalValues.MSG_SetPartitionMulSel, msg);

            Form_showLoadingRate rateView = new Form_showLoadingRate();
            rateView.setLoadingRate(0);
            rateView.cancelClicked = false;

            m_mainForm.showLoadingRate(rateView);

            if (rateView.cancelClicked)
            {
                string cancelMsg = GlobalValues.PN_PartitionGroup + MessageTypes.MessageAttibuteSplitSign + m_parentName;
                GlobalValues.ServiceHandle.sendMessage(GlobalValues.MSG_CancelLoadVolume, cancelMsg);
            }
        }

        #region override

        protected override void OnMouseDown(MouseEventArgs e)
        {
            this.DroppedDown = false;

        }
        protected override void OnMouseUp(MouseEventArgs e)
        {
            this.DroppedDown = false;
        }
        protected override void OnDropDown(EventArgs e)
        {
            lst.Visible = !lst.Visible;
            if (!lst.Visible)
            {
                //this.Focus();
                onMulSelChange(this.Text);
                return;
            }
            lst.Focus();
            lst.ItemHeight = this.ItemHeight;
            lst.BorderStyle = BorderStyle.FixedSingle;
            lst.Size = new Size(this.DropDownWidth, this.ItemHeight * (this.MaxDropDownItems - 1) - (int)this.ItemHeight / 2);
            lst.Location = new Point(this.Left, this.Top + this.ItemHeight + 6);
            lst.BeginUpdate();
            lst.Items.Clear();
            for (int i = 0; i < this.itemsList.Count; i++)
            {
                lst.Items.Add(this.itemsList[i]);
                if (this.checkedValues.ContainsKey(this.itemsList[i]))
                {
                    lst.SetItemChecked(i, true);
                }
            }
            lst.EndUpdate();
            if (!this.Parent.Controls.Contains(lst))
            {
                this.Parent.Controls.Add(lst);
            }
            lst.BringToFront();
        }
        #endregion

        private void lst_LostFocus(object sender, EventArgs e)
        {
            //lst.Visible = false;
        }

        void lst_ItemCheck(object sender, ItemCheckEventArgs e)
        {
            if (e.NewValue == CheckState.Checked)
            {
                if (!this.checkedValues.ContainsKey(this.itemsList[e.Index]))
                {
                    this.checkedValues.Add(this.itemsList[e.Index], true);
                }
            }
            else if (e.NewValue == CheckState.Unchecked)
            {
                if (this.checkedValues.ContainsKey(this.itemsList[e.Index]))
                {
                    this.checkedValues.Remove(this.itemsList[e.Index]);
                }
            }
            string text = "";
            for (int i = 0; i < this.itemsList.Count; i++)
            {
                if (this.checkedValues.ContainsKey(this.itemsList[i]))
                {
                    if ("" != text)
                        text = text + "; ";
                    text = text + this.itemsList[i];
                }
            }
            this.Items.Clear();
            this.Items.Add(text);
            this.SelectedIndex = 0;
            this.Text = text;
        }

    }
    // ref: http://blog.sina.com.cn/s/blog_49e6d37f01007z5b.html
    // ref: http://www.cnblogs.com/greatverve/archive/2011/07/19/ComboBox-Multi-Pic.html
}
