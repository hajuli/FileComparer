using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Text;
using System.Windows.Forms;
using System.Drawing;

namespace FileComparer
{
    public partial class UserListView : Component
    {
        Form m_form = new Form();
        public UserListView()
        {
            InitializeComponent();
            m_form.Controls.Add(this.listView1);
            m_form.MaximizeBox = false;
            m_form.MinimizeBox = false;
            m_form.KeyDown += new KeyEventHandler(Form_KeyDown);    //not work,, inside control got the key focus.
            m_form.Size = new Size(800, 400);

            listView1.Size = m_form.ClientSize;
            listView1.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right | AnchorStyles.Bottom;
            ColumnHeader col = new ColumnHeader();
            col.Width = 200;
            col.Text = "name";
            col.TextAlign = HorizontalAlignment.Left;
            listView1.Columns.Add(col);
            ColumnHeader col2 = new ColumnHeader();
            col2.Width = listView1.Size.Width - 200 - 24;
            col2.Text = "path";
            listView1.Columns.Add(col2);
            
            listView1.View = View.Details;
            listView1.SmallImageList = new ImageList();
            listView1.SmallImageList.ImageSize = new Size(16, 16);

            listView1.KeyDown += new KeyEventHandler(Form_KeyDown);
        }

        public UserListView(IContainer container)
        {
            container.Add(this);

            InitializeComponent();
        }

        public void Form_KeyDown(object sender, KeyEventArgs e)
        {
            if (Keys.Escape == e.KeyCode)
            {
                m_form.Close();
            }
        }

        private void listView1_MouseDoubleClick(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            ListViewHitTestInfo info = listView1.HitTest(e.X, e.Y);
            //WinHelper.ShowFileProperties(info.Item.SubItems[1].Text);
            WinHelper.openExplorer(info.Item.SubItems[1].Text);
        }

        public bool showPaths(string paths)
        {
            bool ret = false;
            char[] cs = new char[1];
            cs[0] = ';';
            string[] cs2 = new string[1];
            cs2[0] = MessageTypes.MessageSplitSign;
            string[] files = paths.Split(cs);
            for (int i = 0; i < files.Length; ++i)
            {
                string[] values = files[i].Split(cs2, StringSplitOptions.None);
                if (values.Length == 2)
                {
                    ListViewItem it = new ListViewItem();
                    it.Text = values[0];
                    it.SubItems.Add(values[1]);
                    it.ImageIndex = i;
                    listView1.Items.Add(it);

                    listView1.SmallImageList.Images.Add(SystemIcon.getPathIcon(values[1]));
                    if (false == ret)
                    {
                        m_form.Icon = SystemIcon.getPathIcon(values[1]);
                        m_form.Text = values[0];
                    }
                    ret = true;
                }
            }
            return ret;
        }
        public void Show()
        {
            m_form.Show();
        }
    }
}
