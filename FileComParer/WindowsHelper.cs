
using System;
using System.Runtime.InteropServices;
using System.Diagnostics;
using System.Windows.Forms;

namespace FileComparer
{
    public class WinHelper
    {
        [DllImport("shell32.dll", CharSet = CharSet.Auto)]
        static extern bool ShellExecuteEx(ref SHELLEXECUTEINFO lpExecInfo);

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto)]
        public struct SHELLEXECUTEINFO
        {
            public int cbSize;
            public uint fMask;
            public IntPtr hwnd;
            [MarshalAs(UnmanagedType.LPTStr)]
            public string lpVerb;
            [MarshalAs(UnmanagedType.LPTStr)]
            public string lpFile;
            [MarshalAs(UnmanagedType.LPTStr)]
            public string lpParameters;
            [MarshalAs(UnmanagedType.LPTStr)]
            public string lpDirectory;
            public int nShow;
            public IntPtr hInstApp;
            public IntPtr lpIDList;
            [MarshalAs(UnmanagedType.LPTStr)]
            public string lpClass;
            public IntPtr hkeyClass;
            public uint dwHotKey;
            public IntPtr hIcon;
            public IntPtr hProcess;
        }

        private const int SW_SHOW = 5;
        private const uint SEE_MASK_INVOKEIDLIST = 12;
        public static bool ShowFileProperties(string Filename)
        {
            SHELLEXECUTEINFO info = new SHELLEXECUTEINFO();
            info.cbSize = System.Runtime.InteropServices.Marshal.SizeOf(info);
            info.lpVerb = "properties";
            info.lpFile = Filename;
            info.nShow = SW_SHOW;
            info.fMask = SEE_MASK_INVOKEIDLIST;
            return ShellExecuteEx(ref info);
        }

        private void test(object sender, EventArgs e)
        {
            string path = @"C:\Users\test\Documents\test.text";
            ShowFileProperties(path);
        }

        [DllImport("user32.dll")]
        public static extern IntPtr PostMessage(IntPtr hWnd, uint Msg, IntPtr wParam, IntPtr lParam);
        [DllImport("User32.dll")]
        public static extern int SendMessage(IntPtr hWnd, uint wMsg, IntPtr wParam, IntPtr lParam);
        [DllImport("user32.dll")]
        private static extern bool ShowWindow(IntPtr hWnd, uint windowStyle);
        [DllImport("user32.dll")]
        private static extern bool SetWindowPos(IntPtr hWnd,
          IntPtr hWndInsertAfter, int x, int y, int cx, int cy, uint uFlags);
        [DllImport("user32.dll")]
        private static extern bool SetForegroundWindow(IntPtr hWnd);
        public static void sendKeysMessage(string processName, IntPtr keyValue)
        {
            const uint WM_KEYDOWN = 0x100;
            const uint WM_KEYUP = 0x0101;

            IntPtr hWnd;
            //string processName = "putty";
            Process[] processList = Process.GetProcesses();

            foreach (Process P in processList)
            {
                if (P.ProcessName.Equals(processName))
                {
                    //int i = KeyCodes.WMessages;
                    hWnd = P.MainWindowHandle;
                    //PostMessage(edit, WM_KEYDOWN, (IntPtr)(Keys.Control | Keys.Pause), IntPtr.Zero);
                    
                    //SendMessage(hWnd, WM_KEYDOWN, keyValue, IntPtr.Zero);
                    //ShowWindow(hWnd, 1);
                    //SetWindowPos(hWnd, new IntPtr(-1), 0, 0, 0, 0, 3);
                    //SetForegroundWindow(hWnd);
                    PostMessage(hWnd, WM_KEYDOWN, keyValue, IntPtr.Zero);
                }
            }
        }

        public static void openExplorer(string path)
        {
            if (path.Length > 1 && path.IndexOf("\"") < 0)
            {
                path = "\"" + path + "\"";
            }
            Process.Start("Explorer.exe ", "/select, " + path);
        }
    }
}
