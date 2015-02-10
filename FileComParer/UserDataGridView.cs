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
        /// 引用shell32文件的SHGetFileInfo API方法         
        /// </summary>         
        /// <param name="pszPath">指定的文件名,如果为""则返回文件夹的</param>         
        /// <param name="dwFileAttributes">文件属性</param>         
        /// <param name="sfi">返回获得的文件信息,是一个记录类型</param>         
        /// <param name="cbFileInfo">文件的类型名</param>        
        /// <param name="uFlags">文件信息标识</param>         
        /// <returns>-1失败</returns>         
        [DllImport("shell32", EntryPoint = "SHGetFileInfo", ExactSpelling = false, CharSet = CharSet.Auto, SetLastError = true)]
        private static extern IntPtr SHGetFileInfo(string pszPath, FileAttribute dwFileAttributes, ref SHFileInfo sfi, uint cbFileInfo, SHFileInfoFlags uFlags);

        /// <summary>   
        /// 返回系统设置的图标   
        /// </summary>   
        /// <param name="lpszFile">文件名,指定从exe文件或dll文件引入icon</param>   
        /// <param name="nIconIndex">文件的图表中的第几个,指定icon的索引如果为0则从指定的文件中引入第1个icon</param>   
        /// <param name="phiconLarge">返回的大图标的指针,大图标句柄如果为null则为没有大图标</param>   
        /// <param name="phiconSmall">返回的小图标的指针,小图标句柄如果为null则为没有小图标</param>   
        /// <param name="nIcons">ico个数,找几个图标</param>   
        /// <returns></returns
        [DllImport("shell32.dll")]
        public static extern uint ExtractIconEx(string lpszFile, int nIconIndex, int[] phiconLarge, int[] phiconSmall, uint nIcons);

        [DllImport("User32.dll", EntryPoint = "DestroyIcon")]
        public static extern int DestroyIcon(IntPtr hIcon);

        /// <summary>         
        /// 文件信息标识枚举类,所有枚举定义值前省略SHGFI投标，比如Icon 完整名称应为SHGFI_ICON         
        /// </summary>         
        [Flags]
        private enum SHFileInfoFlags : uint
        {
            /// <summary>             
            /// 允许有叠加图案的文件图标，该标识必须和Icon同时使用             
            /// </summary>             
            AddOveylays = 0x20,         // SHGFI_AddOverlays = 0x000000020
            /// <summary>             
            /// 只获取由参数FileAttribute指定的文件信息，并将其写入SHFileInfo结构的dwAttributes属性，如果不指定该标识，将同时获取所有文件信息。该标志不能和Icon标识同时使用             
            /// </summary>             
            Attr_Specified = 0x20000,   //  SHGFI_SpecifiedAttributes = 0x000020000
            /// <summary>             
            /// 将获取的文件属性复制到SHFileInfo结构的dwAttributes属性中             
            /// </summary>             
            Attributes = 0x800,     // SHGFI_Attributes = 0x000000800
            /// <summary>             
            /// 获取文件的显示名称（长文件名称），将其复制到SHFileInfo结构的dwAttributes属性中             
            /// </summary>             
            DisplayName = 0x200,    // SHGFI_DisplayName = 0x000000200
            /// <summary>            
            /// 如果文件是可执行文件，将检索其信息，并将信息作为返回值返回              
            /// </summary>             
            ExeType = 0x2000,       // SHGFI_EXEType = 0x000002000
            /// <summary>             
            /// 获得图标和索引，将图标句柄返回到SHFileInfo结构的hIcon属性中，索引返回到iIcon属性中             
            /// </summary>             
            Icon = 0x100,           // SHGFI_Icon = 0x000000100
            /// <summary>             
            /// 检索包含图标的文件，并将文件名，图标句柄，图标索引号，放回到SHFileInfo结构中             
            /// </summary>             
            IconLocation = 0x1000,  // SHGFI_IconLocation = 0x000001000
            /// <summary>             
            /// 获得大图标，该标识必须和Icon标识同时使用             
            /// </summary>             
            LargeIcon = 0x0,        // SHGFI_LargeIcon = 0x000000000
            /// <summary>             
            /// 获取链接覆盖文件图标，该标识必须和Icon标识同时使用。             
            /// </summary>             
            LinkOverlay = 0x8000,   // SHGFI_LinkOverlay = 0x000008000
            /// <summary>             
            /// 获取文件打开时的图标，该标识必须和Icon或SysIconIndex同时使用             
            /// </summary>             
            OpenIcon = 0x2,         //  SHGFI_OpenIcon = 0x000000002
            /// <summary>             
            /// 获取链接覆盖文件图标索引，该标识必须和Icon标识同时使用。             
            /// </summary>             
            OverlayIndex = 0x40,    // SHGFI_OverlayIndex = 0x000000040
            /// <summary>             
            /// 指示传入的路径是一个ITEMIDLIST结构的文件地址而不是一个路径名。             
            /// </summary>             
            Pidl = 0x8,             // SHGFI_PIDL = 0x000000008
            /// <summary>             
            /// 获取系统的高亮显示图标，该标识必须和Icon标识同时使用。             
            /// </summary>             
            Selected = 0x10000,     // SHGFI_SelectedState = 0x000010000
            /// <summary>             
            /// 获取 Shell-sized icon ，该标志必须和Icon标识同时使用。             
            /// </summary>             
            ShellIconSize = 0x4,    // SHGFI_ShellIconSize = 0x000000004
            /// <summary>             
            /// 获得小图标，该标识必须和Icon或SysIconIndex同时使用。             
            /// </summary>             
            SmallIcon = 0x1,       // SHGFI_SmallIcon = 0x000000001
            /// <summary>             
            /// 获取系统图像列表图标索引，返回系统图像列表句柄             
            /// </summary>             
            SysIconIndex = 0x4000,  // SHGFI_SysIconIndex = 0x000004000
            /// <summary>             
            /// 获得文件类型，类型字符串被写入SHFileInfo结构的szTypeName属性中             
            /// </summary>             
            TypeName = 0x400,       // SHGFI_TypeName = 0x000000400
            /// <summary>             
            /// 指示如果由pszPath指定的路径不存在，SHGetFileInfo方法变不会试图去操作文件。指示返回与文件类型相关的信息。该标识不能和Attributes、ExeType和Pidl同时使用             
            /// </summary>             
            UseFileAttributes = 0x10    // SHGFI_UserFileAttributes = 0x000000010,
        }

        /// <summary>         
        /// 文件属性枚举         
        /// </summary>         
        [Flags]
        private enum FileAttribute
        {
            ReadOnly = 0x00000001,
            Hidden = 0x00000002,
            System = 0x00000004,
            Directory = 0x00000010,     //路径信息             
            Archive = 0x00000020,
            Device = 0x00000040,
            Normal = 0x00000080,        //普通文件信息             
            Temporary = 0x00000100,
            Sparse_File = 0x00000200,
            Reparse_Point = 0x00000400,
            Compressed = 0x00000800,
            Offline = 0x00001000,
            Not_Content_Indexed = 0x00002000,
            Encrypted = 0x00004000
        }

        /// <summary>        
        /// 定义返回的文件信息结构         
        /// </summary>         
        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto)]
        private struct SHFileInfo
        {
            /// <summary>             
            /// 文件的图标句柄             
            /// </summary>             
            public IntPtr hIcon;
            /// <summary>             
            /// 图标的系统索引号             
            /// </summary>             
            public IntPtr iIcon;
            /// <summary>             
            /// 文件的属性值,由FileAttribute指定的属性。             
            /// </summary>             
            public uint dwAttributes;
            /// <summary>            
            /// 文件的显示名,如果是64位系统，您可能需要制定SizeConst=258而非260才能够显示完整的 TypeName             
            /// </summary>             
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 260)]
            public string szDisplayName;
            /// <summary>             
            /// 文件的类型名             
            /// </summary>             
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 80)]
            public string szTypeName;
        }

        /// <summary>         
        /// 当读取文件图标失败的默认图标索引号         
        /// </summary>         
        public static readonly long ErrorFileIndex = -2;
        /// <summary>         
        /// 当读取文件夹图标失败的默认图标索引号         
        /// </summary>         
        public static readonly long ErrorFolderIndex = -4;
        /// <summary>         
        /// 当读取磁盘驱动器图标失败的默认图标索引号         
        /// </summary>         
        public static readonly long ErrorDriverIndex = -8;
        /// <summary>         
        /// 当读取可执行文件图标失败的默认图标索引号         
        /// </summary>         
        public static readonly long ErrorApplicationIndex = -16;

        /// <summary>
        /// 获取文件类型的关联图标
        /// </summary>
        /// <param name="fileName">文件类型的扩展名或文件的绝对路径</param>
        /// <param name="isSmallIcon">是否返回小图标</param>
        /// <returns>返回一个Icon类型的文件图标对象</returns>
        public static Icon GetFileIcon(string fileName, bool isSmallIcon)
        {
            long imageIndex;
            return GetFileIcon(fileName, isSmallIcon, out imageIndex);
        }

        /// <summary>
        /// 获取系统文件图标
        /// </summary>
        /// <param name="fileName">文件类型的扩展名或文件的绝对路径,如果是一个exe可执行文件，请提供完整的文件名（包含路径信息）</param>
        /// <param name="isSmallIcon">是否返回小图标</param>
        /// <param name="imageIndex">输出与返回图标对应的系统图标索引号</param>
        /// <returns>返回一个Icon类型的文件图标对象</returns>
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
            //或int iTotal = (int)SHGetFileInfo(fileName, 0, ref shfi, (uint)Marshal.SizeOf(shfi), uFlags);
            Icon icon = null;
            if (iTotal > 0)
            {
                icon = Icon.FromHandle(shfi.hIcon).Clone() as Icon;
                imageIndex = shfi.iIcon.ToInt64();
            }
            DestroyIcon(shfi.hIcon); //释放资源
            return icon;
        }

        /// <summary>  
        /// 获取系统文件夹默认图标
        /// </summary>  
        /// <param name="isSmallIcon">是否返回小图标</param>
        /// <returns>返回一个Icon类型的文件夹图标对象</returns>
        public static Icon GetFolderIcon(bool isSmallIcon)
        {
            long imageIndex;
            return GetFolderIcon(isSmallIcon, out imageIndex);
        }

        /// <summary>  
        /// 获取系统文件夹默认图标
        /// </summary>  
        /// <param name="isSmallIcon">是否返回小图标</param>
        /// <param name="imageIndex">输出与返回图标对应的系统图标索引号</param>
        /// <returns>返回一个Icon类型的文件夹图标对象</returns>
        public static Icon GetFolderIcon(bool isSmallIcon, out long imageIndex)
        {
            return GetFolderIcon(Environment.SystemDirectory, isSmallIcon, out imageIndex);
        }

        /// <summary>  
        /// 获取系统文件夹默认图标
        /// </summary>  
        /// <param name="folderName">文件夹名称,如果想获取自定义文件夹图标，请指定完整的文件夹名称（如 F:\test)</param>
        /// <param name="isSmallIcon">是否返回小图标</param>
        /// <param name="imageIndex">输出与返回图标对应的系统图标索引号</param>
        /// <returns>返回一个Icon类型的文件夹图标对象</returns>
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
            //或int iTotal = (int)SHGetFileInfo("", 0, ref shfi, (uint)Marshal.SizeOf(shfi), SHFileInfoFlags.Icon | SHFileInfoFlags.SmallIcon);
            Icon icon = null;
            if (iTotal > 0)
            {
                icon = Icon.FromHandle(shfi.hIcon).Clone() as Icon;
                imageIndex = shfi.iIcon.ToInt64();
            }
            DestroyIcon(shfi.hIcon); //释放资源
            return icon;
        }

        /// <summary>         
        /// 获取磁盘驱动器图标
        /// </summary>         
        /// <param name="driverMark">有效的磁盘标号，如C、D、I等等，不区分大小写</param>         
        /// <param name="isSmallIcon">标识是获取小图标还是获取大图标</param>         
        /// <param name="imageIndex">输出与返回图标对应的系统图标索引号</param>         
        /// <returns>返回一个Icon类型的磁盘驱动器图标对象</returns>         
        public static Icon GetDriverIcon(char driverMark, bool isSmallIcon)
        {
            long imageIndex;
            return GetDriverIcon(driverMark, isSmallIcon, out imageIndex);
        }

        /// <summary>         
        /// 获取磁盘驱动器图标
        /// </summary>         
        /// <param name="driverMark">有效的磁盘标号，如C、D、I等等，不区分大小写</param>         
        /// <param name="isSmallIcon">标识是获取小图标还是获取大图标</param>         
        /// <param name="imageIndex">输出与返回图标对应的系统图标索引号</param>         
        /// <returns>返回一个Icon类型的磁盘驱动器图标对象</returns>         
        public static Icon GetDriverIcon(char driverMark, bool isSmallIcon, out long imageIndex)
        {
            imageIndex = ErrorDriverIndex;
            //非有效盘符，返回封装的磁盘图标             
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
            DestroyIcon(shfi.hIcon); //释放资源
            return icon;
        }
    }
}
