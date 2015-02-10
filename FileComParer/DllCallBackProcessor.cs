using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Runtime.InteropServices;

namespace FileComparer
{
    public delegate int DllCallBackHandler(string msgType, string msgContent);

    public class DllCallBackProcessor
    {
        MainForm    m_observerForm;
        private DllCallBackHandler m_dllCallBackHandler;

        public DllCallBackProcessor(MainForm observer)
        {
            m_observerForm = observer;
        }

        [DllImport("FileManager.dll", EntryPoint = "initService", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Winapi)]
        public static extern int initService([In] IntPtr callback);
        [DllImport("FileManager.dll", EntryPoint = "postMessageToService", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Winapi)]
        public static extern int postMessageToService(string msgType, string msgContent);

        public void registerToDll()
        {
            m_dllCallBackHandler = new DllCallBackHandler(this.handleDllCallBack);
            IntPtr ptr = Marshal.GetFunctionPointerForDelegate(m_dllCallBackHandler);
            try
            {
                initService(ptr);
                sendTestMessage(MessageTypes.ShowStatusMessage, "today is a nice day.");
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        public bool sendMessage(string msgType, string msgContent)
        {
            bool ret = true;
            try
            {
                postMessageToService(msgType, msgContent);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
                ret = false;
            }
            return ret;
        }

        public int handleDllCallBack(string msgType, string msgContent)
        {
            try
            {
                if (MessageTypes.DirectCallBack == msgType)
                {
                    processTestMessage(msgContent);
                }
                else
                {
                    m_observerForm.processCallBackMessage(msgType, msgContent);
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.ToString());
            }
            return 0;
        }

        public void unRegisterToDll()
        {
            sendMessage(MessageTypes.DestroyBaseFactory, "now");
        }

        public void sendTestMessage(string msgType, string msgContent)
        {
            string newContent = MessageTypes.MessageAttibuteMessageTypeName +
                MessageTypes.MessageAttibuteSplitSign +
                msgType + MessageTypes.MessageSplitSign + msgContent;

            sendMessage(MessageTypes.DirectCallBack, newContent);
        }

        private void processTestMessage(string msgContent)
        {
            string type = MessageTypes.getAttributeValue(
                MessageTypes.MessageAttibuteMessageTypeName, msgContent);

            int contentIdx = msgContent.IndexOf(MessageTypes.MessageSplitSign) +
                MessageTypes.MessageSplitSign.Length;

            handleDllCallBack(type, msgContent.Substring(contentIdx));
        }

    }
}
