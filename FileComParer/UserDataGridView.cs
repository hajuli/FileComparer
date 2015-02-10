using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Diagnostics;

namespace FileComparer
{
    partial class UserDataGridView : DataGridView
    {
        string m_name = "UserDataGridViewNoName";
        DataTable m_config;
        List<string> m_colNames;
        DllCallBackProcessor m_serviceProcessor;
        int m_lastSortedColumnId = -1;

        public UserDataGridView(string name, DataTable config) : base ()
        {
            m_name = name;
            m_colNames = new List<string>();
            m_config = config;
            this.Location = new Point(0, 20);
            this.BorderStyle = BorderStyle.None;
            this.ScrollBars = ScrollBars.Vertical | ScrollBars.Horizontal;
            this.Dock = DockStyle.None;
            this.Anchor = AnchorStyles.Left | AnchorStyles.Top | AnchorStyles.Right | AnchorStyles.Bottom;
            this.BackgroundColor = ConfigParser.getConfigParaColor(m_config.m_tableBackgroundColor);
            this.DoubleBuffered = true; // stopping flash data. maybe call cellpainting too much.

            DataGridViewColumn col = new DataGridViewTextBoxColumn();
            col.Name = MessageTypes.UniIDColumnName;
            col.HeaderText = MessageTypes.UniIDColumnName;
            col.Visible = false;
            col.ReadOnly = true;
            this.Columns.Add(col);
            m_colNames.Add(col.Name);
            int i = 0;
            for (i = 0; i < m_config.m_columns.Count; ++i)
            {
                //DataGridViewColumn col;
                col = new DataGridViewTextBoxColumn();
                
                //col = new DataGridViewComboBoxColumn();
                col.Name = m_config.m_columns[i].m_name;
                col.HeaderText = m_config.m_columns[i].m_name;
                col.Width = Int32.Parse(m_config.m_columns[i].m_width);
                col.ReadOnly = true;
                col.SortMode = DataGridViewColumnSortMode.Programmatic;
                this.Columns.Add(col);
                m_colNames.Add(col.Name);
                if ("name" == col.Name)
                {
                    Padding newPadding = col.DefaultCellStyle.Padding;
                    newPadding.Left = 18;
                    col.DefaultCellStyle.Padding = newPadding;
                }
                if ("size" == col.Name)
                {
                    col.DefaultCellStyle.Alignment = DataGridViewContentAlignment.MiddleRight;
                }
            }
            //this.RowCount = 1;
            this.AllowUserToAddRows = false;
            this.AllowUserToResizeRows = false;
            this.ColumnHeadersHeightSizeMode = DataGridViewColumnHeadersHeightSizeMode.DisableResizing;
            this.RowHeadersWidthSizeMode = DataGridViewRowHeadersWidthSizeMode.DisableResizing;
            this.RowHeadersVisible = false;

            this.EnableHeadersVisualStyles = false; //If you do not set the EnableHeadersVisualStyles flag to False, then the changes you make to the style of the header will not take effect, as the grid will use the style from the current users default theme. from: http://stackoverflow.com/questions/1247800/how-to-change-the-color-of-winform-datagridview-header
            this.ColumnHeadersDefaultCellStyle.BackColor = ConfigParser.getConfigParaColor(m_config.m_columnHeadersBackColor);
            Font ft = new Font(this.ColumnHeadersDefaultCellStyle.Font, FontStyle.Bold);
            this.ColumnHeadersDefaultCellStyle.Font = ft;

            //this.Rows[1].HeaderCell.Value = "Row1";
            //this.Rows[1].HeaderCell.Style.Alignment = DataGridViewContentAlignment.MiddleLeft;

            this.ColumnHeaderMouseClick += new DataGridViewCellMouseEventHandler(this.dgv_ColumnHeaderMouseClick);
            this.CellDoubleClick += new DataGridViewCellEventHandler(this.dgv_CellDoubleClick);
            this.CellPainting += new DataGridViewCellPaintingEventHandler(this.dgv_CellPainting);
            this.CellMouseMove += new DataGridViewCellMouseEventHandler(this.dgv_CellMouseMove);
            this.CellMouseLeave += new DataGridViewCellEventHandler(this.dgv_CellMouseLeave);
        }

        public void processMessages(string msgType, string msg)
        {
            if (MessageTypes.FileListItemClear == msgType)
            {
                this.Rows.Clear();
            }
            if (MessageTypes.FileListItemAdd == msgType)
            {
                dataItemAdd(msg);
            }
            else if (MessageTypes.FileListItemUpdate == msgType)
            {
                dataItemUpdate(msg);
            }
            else if (MessageTypes.FileListItemRemove == msgType)
            {
                dataItemRemove(msg);
            }
            else
            {
                //throw ;
            }
        }

        private void dataItemAdd(string msg)
        {
            string pkey = MessageTypes.getAttributeValue(MessageTypes.UniIDColumnName, msg);
            int i = 0;
            for (i = 0; i < this.Rows.Count; ++i)
            {
                if (pkey == this.Rows[i].Cells[0].Value.ToString())
                {
                    return; // exist this data;
                }
            }
            this.Rows.Add(1);
            this.Rows[this.Rows.Count - 1].Cells[0].Value = pkey;
            this.Rows[this.Rows.Count - 1].DefaultCellStyle.BackColor = ConfigParser.getConfigParaColor(m_config.m_rowsBackColor); ;
            dataItemUpdate(msg);
            //this.Rows[this.Rows.Count - 1]
        }

        private void dataItemUpdate(string msg)
        {
            string pkey = MessageTypes.getAttributeValue(MessageTypes.UniIDColumnName, msg);
            int i = 0;
            for (i = 0; i < this.Rows.Count; ++i)
            {
                if (pkey == this.Rows[i].Cells[0].Value.ToString())
                {
                    break;
                }
            }
            int rowN = i;
            if (i < this.Rows.Count)
            {
                for (i = 1; i < m_colNames.Count; ++i)
                {
                    try
                    {
                        this.Rows[rowN].Cells[i].Value = 
                            MessageTypes.getAttributeValue(m_colNames[i], msg);
                    }
                    catch (Exception)
                    {
                    }
                }
            }
        }
        private void dataItemRemove(string msg)
        {
            string pkey = MessageTypes.getAttributeValue(MessageTypes.UniIDColumnName, msg);
            int i = 0;
            for (i = 0; i < this.Rows.Count; ++i)
            {
                if (pkey == this.Rows[i].Cells[0].Value.ToString())
                {
                    this.Rows.Remove(this.Rows[i]);
                    break;
                }
            }
        }

        public void setServiceProcessor(DllCallBackProcessor pro)
        {
            m_serviceProcessor = pro;
        }

        protected override void OnScroll(ScrollEventArgs e)
        {
            base.OnScroll(e);
            //ScrollBar vb = this.VerticalScrollBar;
            int n = this.DisplayedRowCount(false);
            if (this.FirstDisplayedScrollingRowIndex + n >= this.Rows.Count)
            {
                string msg = GlobalValues.PN_ShowName + MessageTypes.MessageAttibuteSplitSign + m_name
                    + MessageTypes.MessageSplitSign
                    + GlobalValues.PN_MessageValue + MessageTypes.MessageAttibuteSplitSign + "morrre";
                GlobalValues.ServiceHandle.sendMessage(GlobalValues.MSG_ShowMoreItems, msg);
            }
        }

        protected override bool ProcessDialogKey(Keys keyData)
        {
            // this function called before OnKeyDown; "alt + enter" been process by base, can't reach to OnKeyDown.

            if ((Keys.Alt | Keys.Enter) == keyData)
            {
                int rowIndex = this.SelectedCells[0].RowIndex;
                WinHelper.ShowFileProperties(this.Rows[rowIndex].Cells[this.Columns["path"].Index].Value.ToString());
            }
            else
            {
                return base.ProcessDialogKey(keyData);
            }
            return true;
        }

        public void dgv_ColumnHeaderMouseClick(object sender, DataGridViewCellMouseEventArgs e)
        {
            if (this.m_config.m_tableName == "findSameFiles")
                return;
            DataGridViewColumn col = this.Columns[e.ColumnIndex];
            SortOrder so = col.HeaderCell.SortGlyphDirection;
            if (so != SortOrder.Ascending)
            {
                so = SortOrder.Ascending;
            }
            else
            {
                so = SortOrder.Descending;
            }
            col.HeaderCell.SortGlyphDirection = so;
            if (m_lastSortedColumnId > -1 && m_lastSortedColumnId != e.ColumnIndex)
            {
                this.Columns[m_lastSortedColumnId].HeaderCell.SortGlyphDirection = SortOrder.None;
            }
            m_lastSortedColumnId = e.ColumnIndex;
            string msg = GlobalValues.PN_ShowName + MessageTypes.MessageAttibuteSplitSign + m_name
                    + MessageTypes.MessageSplitSign
                    + GlobalValues.PN_MessageValue + MessageTypes.MessageAttibuteSplitSign + col.Name + ":" + so.ToString();
            GlobalValues.ServiceHandle.sendMessage(GlobalValues.MSG_SortShowItems, msg);
        }

        public void dgv_CellDoubleClick(object sender, DataGridViewCellEventArgs e)
        {
            if (e.RowIndex < 0)
                return;
            if (GlobalValues.SameFilesTableName == this.m_config.m_tableName)
            {
                string msg = GlobalValues.PN_ShowName + MessageTypes.MessageAttibuteSplitSign + m_name
                    + MessageTypes.MessageSplitSign
                    + GlobalValues.PN_MessageValue + MessageTypes.MessageAttibuteSplitSign + 
                    this.Rows[e.RowIndex].Cells[this.Columns[MessageTypes.UniIDColumnName].Index].Value.ToString();

                GlobalValues.ServiceHandle.sendMessage(GlobalValues.MSG_GetSameFileAllPaths, msg);
                return;
            }
            //WinHelper.ShowFileProperties(this.Rows[e.RowIndex].Cells[this.Columns["path"].Index].Value.ToString());
            WinHelper.openExplorer(this.Rows[e.RowIndex].Cells[this.Columns["path"].Index].Value.ToString());
        }

        public void dgv_CellPainting(object sender, DataGridViewCellPaintingEventArgs e)
        {
            if (e.ColumnIndex != this.Columns["name"].Index || e.RowIndex == -1)
                return;

            if (DataGridViewElementStates.Selected == (e.State & DataGridViewElementStates.Selected))
                return;

            Rectangle rowBounds = new Rectangle(
                e.CellBounds.Left + 2, e.CellBounds.Top + 3, 16, 16);//e.CellBounds.Height - 4);

            e.PaintBackground(e.CellBounds, false);
            //e.Graphics.FillRectangle(Brushes.White, e.CellBounds);
            string path = this.Rows[e.RowIndex].Cells[this.Columns["path"].Index].Value.ToString();
            string name = e.Value.ToString();

            e.Graphics.DrawIcon(SystemIcon.getPathIcon(path), rowBounds);
            e.Graphics.DrawString(name, this.Font, Brushes.Black, new PointF(e.CellBounds.X + 20, e.CellBounds.Y + 5));
            e.Handled = true;
        }

        public void dgv_CellMouseMove(object sender, DataGridViewCellMouseEventArgs e)
        {
            if (e.RowIndex >= 0)
            {
                this.Rows[e.RowIndex].DefaultCellStyle.BackColor = ConfigParser.getConfigParaColor(m_config.m_rowMouseMoveBackColor);
            }
        }
        public void dgv_CellMouseLeave(object sender, DataGridViewCellEventArgs e)
        {
            if (e.RowIndex >= 0)
            {
                this.Rows[e.RowIndex].DefaultCellStyle.BackColor = ConfigParser.getConfigParaColor(m_config.m_rowMouseLeaveBackColor);
            }
        }

        protected override void OnRowPostPaint(DataGridViewRowPostPaintEventArgs e)
        {
            base.OnRowPostPaint(e);

            //http://msdn.microsoft.com/en-us/library/system.windows.forms.datagridview.rowpostpaint(v=vs.110).aspx
        }

    }

    public class SystemIcon
    {
        public static Icon getPathIcon(string path)
        {
            Icon icon = null;
            try
            {
                icon = Icon.ExtractAssociatedIcon(path);
            }
            catch (Exception)
            {
                icon = GetFolderIcon(true);
            };

            return icon;
        }
        /// <summary>         
        /// ����shell32�ļ���SHGetFileInfo API����         
        /// </summary>         
        /// <param name="pszPath">ָ�����ļ���,���Ϊ""�򷵻��ļ��е�</param>         
        /// <param name="dwFileAttributes">�ļ�����</param>         
        /// <param name="sfi">���ػ�õ��ļ���Ϣ,��һ����¼����</param>         
        /// <param name="cbFileInfo">�ļ���������</param>        
        /// <param name="uFlags">�ļ���Ϣ��ʶ</param>         
        /// <returns>-1ʧ��</returns>         
        [DllImport("shell32", EntryPoint = "SHGetFileInfo", ExactSpelling = false, CharSet = CharSet.Auto, SetLastError = true)]
        private static extern IntPtr SHGetFileInfo(string pszPath, FileAttribute dwFileAttributes, ref SHFileInfo sfi, uint cbFileInfo, SHFileInfoFlags uFlags);

        /// <summary>   
        /// ����ϵͳ���õ�ͼ��   
        /// </summary>   
        /// <param name="lpszFile">�ļ���,ָ����exe�ļ���dll�ļ�����icon</param>   
        /// <param name="nIconIndex">�ļ���ͼ���еĵڼ���,ָ��icon���������Ϊ0���ָ�����ļ��������1��icon</param>   
        /// <param name="phiconLarge">���صĴ�ͼ���ָ��,��ͼ�������Ϊnull��Ϊû�д�ͼ��</param>   
        /// <param name="phiconSmall">���ص�Сͼ���ָ��,Сͼ�������Ϊnull��Ϊû��Сͼ��</param>   
        /// <param name="nIcons">ico����,�Ҽ���ͼ��</param>   
        /// <returns></returns
        [DllImport("shell32.dll")]
        public static extern uint ExtractIconEx(string lpszFile, int nIconIndex, int[] phiconLarge, int[] phiconSmall, uint nIcons);

        [DllImport("User32.dll", EntryPoint = "DestroyIcon")]
        public static extern int DestroyIcon(IntPtr hIcon);

        /// <summary>         
        /// �ļ���Ϣ��ʶö����,����ö�ٶ���ֵǰʡ��SHGFIͶ�꣬����Icon ��������ӦΪSHGFI_ICON         
        /// </summary>         
        [Flags]
        private enum SHFileInfoFlags : uint
        {
            /// <summary>             
            /// �����е���ͼ�����ļ�ͼ�꣬�ñ�ʶ�����Iconͬʱʹ��             
            /// </summary>             
            AddOveylays = 0x20,         // SHGFI_AddOverlays = 0x000000020
            /// <summary>             
            /// ֻ��ȡ�ɲ���FileAttributeָ�����ļ���Ϣ��������д��SHFileInfo�ṹ��dwAttributes���ԣ������ָ���ñ�ʶ����ͬʱ��ȡ�����ļ���Ϣ���ñ�־���ܺ�Icon��ʶͬʱʹ��             
            /// </summary>             
            Attr_Specified = 0x20000,   //  SHGFI_SpecifiedAttributes = 0x000020000
            /// <summary>             
            /// ����ȡ���ļ����Ը��Ƶ�SHFileInfo�ṹ��dwAttributes������             
            /// </summary>             
            Attributes = 0x800,     // SHGFI_Attributes = 0x000000800
            /// <summary>             
            /// ��ȡ�ļ�����ʾ���ƣ����ļ����ƣ������临�Ƶ�SHFileInfo�ṹ��dwAttributes������             
            /// </summary>             
            DisplayName = 0x200,    // SHGFI_DisplayName = 0x000000200
            /// <summary>            
            /// ����ļ��ǿ�ִ���ļ�������������Ϣ��������Ϣ��Ϊ����ֵ����              
            /// </summary>             
            ExeType = 0x2000,       // SHGFI_EXEType = 0x000002000
            /// <summary>             
            /// ���ͼ�����������ͼ�������ص�SHFileInfo�ṹ��hIcon�����У��������ص�iIcon������             
            /// </summary>             
            Icon = 0x100,           // SHGFI_Icon = 0x000000100
            /// <summary>             
            /// ��������ͼ����ļ��������ļ�����ͼ������ͼ�������ţ��Żص�SHFileInfo�ṹ��             
            /// </summary>             
            IconLocation = 0x1000,  // SHGFI_IconLocation = 0x000001000
            /// <summary>             
            /// ��ô�ͼ�꣬�ñ�ʶ�����Icon��ʶͬʱʹ��             
            /// </summary>             
            LargeIcon = 0x0,        // SHGFI_LargeIcon = 0x000000000
            /// <summary>             
            /// ��ȡ���Ӹ����ļ�ͼ�꣬�ñ�ʶ�����Icon��ʶͬʱʹ�á�             
            /// </summary>             
            LinkOverlay = 0x8000,   // SHGFI_LinkOverlay = 0x000008000
            /// <summary>             
            /// ��ȡ�ļ���ʱ��ͼ�꣬�ñ�ʶ�����Icon��SysIconIndexͬʱʹ��             
            /// </summary>             
            OpenIcon = 0x2,         //  SHGFI_OpenIcon = 0x000000002
            /// <summary>             
            /// ��ȡ���Ӹ����ļ�ͼ���������ñ�ʶ�����Icon��ʶͬʱʹ�á�             
            /// </summary>             
            OverlayIndex = 0x40,    // SHGFI_OverlayIndex = 0x000000040
            /// <summary>             
            /// ָʾ�����·����һ��ITEMIDLIST�ṹ���ļ���ַ������һ��·������             
            /// </summary>             
            Pidl = 0x8,             // SHGFI_PIDL = 0x000000008
            /// <summary>             
            /// ��ȡϵͳ�ĸ�����ʾͼ�꣬�ñ�ʶ�����Icon��ʶͬʱʹ�á�             
            /// </summary>             
            Selected = 0x10000,     // SHGFI_SelectedState = 0x000010000
            /// <summary>             
            /// ��ȡ Shell-sized icon ���ñ�־�����Icon��ʶͬʱʹ�á�             
            /// </summary>             
            ShellIconSize = 0x4,    // SHGFI_ShellIconSize = 0x000000004
            /// <summary>             
            /// ���Сͼ�꣬�ñ�ʶ�����Icon��SysIconIndexͬʱʹ�á�             
            /// </summary>             
            SmallIcon = 0x1,       // SHGFI_SmallIcon = 0x000000001
            /// <summary>             
            /// ��ȡϵͳͼ���б�ͼ������������ϵͳͼ���б���             
            /// </summary>             
            SysIconIndex = 0x4000,  // SHGFI_SysIconIndex = 0x000004000
            /// <summary>             
            /// ����ļ����ͣ������ַ�����д��SHFileInfo�ṹ��szTypeName������             
            /// </summary>             
            TypeName = 0x400,       // SHGFI_TypeName = 0x000000400
            /// <summary>             
            /// ָʾ�����pszPathָ����·�������ڣ�SHGetFileInfo�����䲻����ͼȥ�����ļ���ָʾ�������ļ�������ص���Ϣ���ñ�ʶ���ܺ�Attributes��ExeType��Pidlͬʱʹ��             
            /// </summary>             
            UseFileAttributes = 0x10    // SHGFI_UserFileAttributes = 0x000000010,
        }

        /// <summary>         
        /// �ļ�����ö��         
        /// </summary>         
        [Flags]
        private enum FileAttribute
        {
            ReadOnly = 0x00000001,
            Hidden = 0x00000002,
            System = 0x00000004,
            Directory = 0x00000010,     //·����Ϣ             
            Archive = 0x00000020,
            Device = 0x00000040,
            Normal = 0x00000080,        //��ͨ�ļ���Ϣ             
            Temporary = 0x00000100,
            Sparse_File = 0x00000200,
            Reparse_Point = 0x00000400,
            Compressed = 0x00000800,
            Offline = 0x00001000,
            Not_Content_Indexed = 0x00002000,
            Encrypted = 0x00004000
        }

        /// <summary>        
        /// ���巵�ص��ļ���Ϣ�ṹ         
        /// </summary>         
        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto)]
        private struct SHFileInfo
        {
            /// <summary>             
            /// �ļ���ͼ����             
            /// </summary>             
            public IntPtr hIcon;
            /// <summary>             
            /// ͼ���ϵͳ������             
            /// </summary>             
            public IntPtr iIcon;
            /// <summary>             
            /// �ļ�������ֵ,��FileAttributeָ�������ԡ�             
            /// </summary>             
            public uint dwAttributes;
            /// <summary>            
            /// �ļ�����ʾ��,�����64λϵͳ����������Ҫ�ƶ�SizeConst=258����260���ܹ���ʾ������ TypeName             
            /// </summary>             
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 260)]
            public string szDisplayName;
            /// <summary>             
            /// �ļ���������             
            /// </summary>             
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 80)]
            public string szTypeName;
        }

        /// <summary>         
        /// ����ȡ�ļ�ͼ��ʧ�ܵ�Ĭ��ͼ��������         
        /// </summary>         
        public static readonly long ErrorFileIndex = -2;
        /// <summary>         
        /// ����ȡ�ļ���ͼ��ʧ�ܵ�Ĭ��ͼ��������         
        /// </summary>         
        public static readonly long ErrorFolderIndex = -4;
        /// <summary>         
        /// ����ȡ����������ͼ��ʧ�ܵ�Ĭ��ͼ��������         
        /// </summary>         
        public static readonly long ErrorDriverIndex = -8;
        /// <summary>         
        /// ����ȡ��ִ���ļ�ͼ��ʧ�ܵ�Ĭ��ͼ��������         
        /// </summary>         
        public static readonly long ErrorApplicationIndex = -16;

        /// <summary>
        /// ��ȡ�ļ����͵Ĺ���ͼ��
        /// </summary>
        /// <param name="fileName">�ļ����͵���չ�����ļ��ľ���·��</param>
        /// <param name="isSmallIcon">�Ƿ񷵻�Сͼ��</param>
        /// <returns>����һ��Icon���͵��ļ�ͼ�����</returns>
        public static Icon GetFileIcon(string fileName, bool isSmallIcon)
        {
            long imageIndex;
            return GetFileIcon(fileName, isSmallIcon, out imageIndex);
        }

        /// <summary>
        /// ��ȡϵͳ�ļ�ͼ��
        /// </summary>
        /// <param name="fileName">�ļ����͵���չ�����ļ��ľ���·��,�����һ��exe��ִ���ļ������ṩ�������ļ���������·����Ϣ��</param>
        /// <param name="isSmallIcon">�Ƿ񷵻�Сͼ��</param>
        /// <param name="imageIndex">����뷵��ͼ���Ӧ��ϵͳͼ��������</param>
        /// <returns>����һ��Icon���͵��ļ�ͼ�����</returns>
        public static Icon GetFileIcon(string fileName, bool isSmallIcon, out long imageIndex)
        {
            imageIndex = ErrorFileIndex;
            if (String.IsNullOrEmpty(fileName))
                return null;

            SHFileInfo shfi = new SHFileInfo();
            SHFileInfoFlags uFlags = SHFileInfoFlags.Icon | SHFileInfoFlags.ShellIconSize;
            if (isSmallIcon)
                uFlags |= SHFileInfoFlags.SmallIcon;
            else
                uFlags |= SHFileInfoFlags.LargeIcon;
            //FileInfo fi = new FileInfo(fileName);
            //if (fi.Name.ToUpper().EndsWith(".EXE"))
            if (fileName.ToUpper().EndsWith(".EXE"))
                uFlags |= SHFileInfoFlags.ExeType;
            else
                uFlags |= SHFileInfoFlags.UseFileAttributes;

            int iTotal = (int)SHGetFileInfo(fileName, FileAttribute.Normal, ref shfi, (uint)Marshal.SizeOf(shfi), uFlags);
            //��int iTotal = (int)SHGetFileInfo(fileName, 0, ref shfi, (uint)Marshal.SizeOf(shfi), uFlags);
            Icon icon = null;
            if (iTotal > 0)
            {
                icon = Icon.FromHandle(shfi.hIcon).Clone() as Icon;
                imageIndex = shfi.iIcon.ToInt64();
            }
            DestroyIcon(shfi.hIcon); //�ͷ���Դ
            return icon;
        }

        /// <summary>  
        /// ��ȡϵͳ�ļ���Ĭ��ͼ��
        /// </summary>  
        /// <param name="isSmallIcon">�Ƿ񷵻�Сͼ��</param>
        /// <returns>����һ��Icon���͵��ļ���ͼ�����</returns>
        public static Icon GetFolderIcon(bool isSmallIcon)
        {
            long imageIndex;
            return GetFolderIcon(isSmallIcon, out imageIndex);
        }

        /// <summary>  
        /// ��ȡϵͳ�ļ���Ĭ��ͼ��
        /// </summary>  
        /// <param name="isSmallIcon">�Ƿ񷵻�Сͼ��</param>
        /// <param name="imageIndex">����뷵��ͼ���Ӧ��ϵͳͼ��������</param>
        /// <returns>����һ��Icon���͵��ļ���ͼ�����</returns>
        public static Icon GetFolderIcon(bool isSmallIcon, out long imageIndex)
        {
            return GetFolderIcon(Environment.SystemDirectory, isSmallIcon, out imageIndex);
        }

        /// <summary>  
        /// ��ȡϵͳ�ļ���Ĭ��ͼ��
        /// </summary>  
        /// <param name="folderName">�ļ�������,������ȡ�Զ����ļ���ͼ�꣬��ָ���������ļ������ƣ��� F:\test)</param>
        /// <param name="isSmallIcon">�Ƿ񷵻�Сͼ��</param>
        /// <param name="imageIndex">����뷵��ͼ���Ӧ��ϵͳͼ��������</param>
        /// <returns>����һ��Icon���͵��ļ���ͼ�����</returns>
        public static Icon GetFolderIcon(string folderName, bool isSmallIcon, out long imageIndex)
        {
            imageIndex = ErrorFolderIndex;
            if (String.IsNullOrEmpty(folderName))
                return null;

            SHFileInfo shfi = new SHFileInfo();
            SHFileInfoFlags uFlags = SHFileInfoFlags.Icon | SHFileInfoFlags.ShellIconSize | SHFileInfoFlags.UseFileAttributes;
            if (isSmallIcon)
                uFlags |= SHFileInfoFlags.SmallIcon;
            else
                uFlags |= SHFileInfoFlags.LargeIcon;

            int iTotal = (int)SHGetFileInfo(folderName, FileAttribute.Directory, ref shfi, (uint)Marshal.SizeOf(shfi), uFlags);
            //��int iTotal = (int)SHGetFileInfo("", 0, ref shfi, (uint)Marshal.SizeOf(shfi), SHFileInfoFlags.Icon | SHFileInfoFlags.SmallIcon);
            Icon icon = null;
            if (iTotal > 0)
            {
                icon = Icon.FromHandle(shfi.hIcon).Clone() as Icon;
                imageIndex = shfi.iIcon.ToInt64();
            }
            DestroyIcon(shfi.hIcon); //�ͷ���Դ
            return icon;
        }

        /// <summary>         
        /// ��ȡ����������ͼ��
        /// </summary>         
        /// <param name="driverMark">��Ч�Ĵ��̱�ţ���C��D��I�ȵȣ������ִ�Сд</param>         
        /// <param name="isSmallIcon">��ʶ�ǻ�ȡСͼ�껹�ǻ�ȡ��ͼ��</param>         
        /// <param name="imageIndex">����뷵��ͼ���Ӧ��ϵͳͼ��������</param>         
        /// <returns>����һ��Icon���͵Ĵ���������ͼ�����</returns>         
        public static Icon GetDriverIcon(char driverMark, bool isSmallIcon)
        {
            long imageIndex;
            return GetDriverIcon(driverMark, isSmallIcon, out imageIndex);
        }

        /// <summary>         
        /// ��ȡ����������ͼ��
        /// </summary>         
        /// <param name="driverMark">��Ч�Ĵ��̱�ţ���C��D��I�ȵȣ������ִ�Сд</param>         
        /// <param name="isSmallIcon">��ʶ�ǻ�ȡСͼ�껹�ǻ�ȡ��ͼ��</param>         
        /// <param name="imageIndex">����뷵��ͼ���Ӧ��ϵͳͼ��������</param>         
        /// <returns>����һ��Icon���͵Ĵ���������ͼ�����</returns>         
        public static Icon GetDriverIcon(char driverMark, bool isSmallIcon, out long imageIndex)
        {
            imageIndex = ErrorDriverIndex;
            //����Ч�̷������ط�װ�Ĵ���ͼ��             
            if (driverMark < 'a' && driverMark > 'z' && driverMark < 'A' && driverMark > 'Z')
            {
                return null;
            }
            string driverName = driverMark.ToString().ToUpper() + ":\\";

            SHFileInfo shfi = new SHFileInfo();
            SHFileInfoFlags uFlags = SHFileInfoFlags.Icon | SHFileInfoFlags.ShellIconSize | SHFileInfoFlags.UseFileAttributes;
            if (isSmallIcon)
                uFlags |= SHFileInfoFlags.SmallIcon;
            else
                uFlags |= SHFileInfoFlags.LargeIcon;
            int iTotal = (int)SHGetFileInfo(driverName, FileAttribute.Normal, ref shfi, (uint)Marshal.SizeOf(shfi), uFlags);
            //int iTotal = (int)SHGetFileInfo(driverName, 0, ref shfi, (uint)Marshal.SizeOf(shfi), uFlags);
            Icon icon = null;
            if (iTotal > 0)
            {
                icon = Icon.FromHandle(shfi.hIcon).Clone() as Icon;
                imageIndex = shfi.iIcon.ToInt64();
            }
            DestroyIcon(shfi.hIcon); //�ͷ���Դ
            return icon;
        }
    }
}
