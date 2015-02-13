using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.IO;
using System.Windows.Forms;
using System.Drawing;


//using MS.Internal.Mita.Foundation;

namespace FileComparer
{
    public static class GlobalValues
    {
        public static Dictionary<string, string> m_paras = new Dictionary<string, string>();
        public static ConfigParser ConfigeData = null;
        public static DllCallBackProcessor ServiceHandle = null;

        public static string UseDllName         = "BaseFactory.dll";
        public static string StartBtn_start     = "Start";
        public static string StartBtn_stop      = "Stop";
        public static string StressBtn_start    = "Stress On";
        public static string StressBtn_stop     = "Stress Off";


        public static string VolumeIDsSeparator = "__###__";
        public static string VolumeAllIDs       = "All";


        public static string AllDefualtString   = "AllDefualtString_NOYES";
        public static string PartitionGroupName1 = "PartitionGroupName1";
        public static string PartitionGroupName2 = "PartitionGroupName2";
        public static string AllFilesBtnName    = "AllFiles";
        public static string AllFilesTableName  = "showAllFiles";
        public static string SameFilesTableName = "showSameFiles";
        public static string AllBtnDefualtColor = "AllBtnDefualtColor";

        // parameter name.---------------
        public static string PN_MessageValue		= "Value";
        public static string PN_MessageValue2       = "Value2";
        public static string PN_PartitionGroup		= "ParttGroup";
        public static string PN_ShowName			= "ShowName";
        public static string PN_ShowType            = "ShowType";
        public static string PN_CmpToParttGroup     = "CmpToParttGroup";
        public static string PN_SelectCondition     = "SelectCondition";

        //msg para values.
        public static string PV_ShowType_AllFiles   = "AllFiles";
        public static string PV_ShowType_SameFiles  = "SameFiles";
        public static string PV_ShowType_MoreFiles  = "MoreFiles";

        // msg: to service.-------------
        public static string MSG_LoadAllVolumeIDs   = "LoadAllVolumeIDs";
        public static string MSG_SetCurrentShow     = "SetCurrentShow";         //PN_ShowName, PN_ShowType, PN_PartitionGroup, []
        public static string MSG_SetPartitionMulSel = "SetPartitionMulSel";     //PN_PartitionGroup, PN_MessageValue
        public static string MSG_CancelLoadVolume   = "CancelLoadVolume";       //PN_PartitionGroup
        public static string MSG_UpdateSelectCondition = "UpdateSelectCondition";   //PN_ShowName, PN_MessageValue
        public static string MSG_SortShowItems          = "SortShowItems";      //PN_ShowName,PN_MessageValue
        public static string MSG_ShowMoreItems          = "ShowMoreItems";      //PN_ShowName
        public static string MSG_GetSameFileAllPaths    = "GetSameFileAllPaths";//PN_ShowName,PN_MessageValue       // split by ';'

        //msg: to ui--------------------
        public static string MSG_UpdateShowedNums = "UpdateShowedNums";   //PN_ShowName, PN_MessageValue

        public static void InitDefualtParas()
        {
            m_paras[PV_ShowType_AllFiles] = "All Files";
            m_paras[PV_ShowType_SameFiles] = "Same Files";
            m_paras[PV_ShowType_MoreFiles] = "More Files";
            m_paras[AllBtnDefualtColor] = "DeepSkyBlue";
        }

        public static string get(string paraName)
        {
            if (m_paras.ContainsKey(paraName))
            {
                return m_paras[paraName];
            }
            return AllDefualtString;
        }

        public static Color getColorFromString(string paraName)
        {
            Color retColor = Color.Red;
            retColor = Color.FromName(paraName);
            return retColor;
        }

        public static DataTable getDataTable(string tableName)
        {
            if (null != ConfigeData)
            {
                return ConfigeData.getDataTable(tableName);
            }
            return new DataTable();
        }

    };

    public static class MessageTypes
    {
        public static string DirectCallBack     = "DirectCallBack";
        public static string DestroyBaseFactory = "DestroyBaseFactory";

        public static string MessageAttibuteMessageTypeName = "MessageType";
        public static string MessageAttibuteSplitSign       = "=";
        public static string MessageSplitSign = ",..,";

        public static string MessageAttibuteTabName         = "tabName";

        public static string UniIDColumnName = "RowRefIndex";

        //----------------------to service.
        public static string LoadAllVolumeIDs       = "LoadAllVolumeIDs";
        public static string SetCurrentShow         = "SetCurrentShow";
        public static string SetPartitionMulSel     = "SetPartitionMulSel"; //like:ParttGroup=ParttGroup1/Value=(D:) D, (E:) E
        
        public static string CancelLoadVolume       = "CancelLoadVolume";

        //----------------------to ui.
        public static string ShowStatusMessage      = "ShowStatusMessage";
        public static string VolumeIDsUpdate        = "VolumeIDsUpdate";
        public static string FileListItemClear      = "FileListItemClear";
        public static string FileListItemAdd        = "FileListItemAdd";
        public static string FileListItemUpdate     = "FileListItemUpdate";
        public static string FileListItemRemove     = "FileListItemRemove";
        public static string ShowSameFileAllPaths   = "ShowSameFileAllPaths";
        public static string UpdateLoadingRate      = "UpdateLoadingRate";  //like:loadingRate=%d,.., volume=%s

        public static string getAttributeValue(string attrName, string msg)
        {
            int start = msg.IndexOf(attrName + MessageAttibuteSplitSign);
            if (start < 0)
            {
                return "";
            }
            start = start + attrName.Length + MessageAttibuteSplitSign.Length;

            string val = msg.Substring(start);
            if (val.IndexOf(MessageSplitSign) >= 0)
            {
                val = val.Substring(0, val.IndexOf(MessageSplitSign));
            }
            return val;
        }

    };
    
    public class TableColumn
    {
        public string m_name = "";
        public string m_dataType = "";
        public string m_width = "";
    };
    public class DataTable
    {
        public static string tableBackgroundColor = "tableBackgroundColor";
        public static string columnHeadersBackColor = "columnHeadersBackColor";
        public static string rowsBackColor = "rowsBackColor";
        public static string rowMouseMoveBackColor = "rowMouseMoveBackColor";
        public static string rowMouseLeaveBackColor = "rowMouseLeaveBackColor";
        
        public string m_tableBackgroundColor = ConfigParser.defaultBackColor;
        public string m_columnHeadersBackColor = ConfigParser.defaultBackColor;
        public string m_rowsBackColor = ConfigParser.defaultBackColor;
        public string m_rowMouseMoveBackColor = ConfigParser.defaultBackColor;
        public string m_rowMouseLeaveBackColor = ConfigParser.defaultBackColor;

        public string m_tableName = "";
        public List<TableColumn> m_columns = new List<TableColumn>();
        public DataTable()
        {
            m_columns.Clear();
        }
    };

    public class ConfigParser
    {
        public static string AppTitle = "show_title";
        public static string VolumeSelectLabel = "volume_select_label";
        public static string ReloadVolumeLabel = "reload_volume_label";
        public static string defaultBackColor = "white";
        public static string mainFormBackColor = "mainFormBackColor";
        public static string volumeLabelBackColor = "volumeLabelBackColor";
        public static string volumeSelectBackColor = "volumeSelectBackColor";
        public static string volumeloadingButtonBackColor = "volumeloadingButtonBackColor";

        string  m_configPath;
        string  m_parseContent;
        bool    m_parsedOk;
        XmlReader m_xmlReader;

        Dictionary<string, string> m_paras = null;
        Dictionary<string, DataTable> m_tables = null;
        
        public ConfigParser()
        {
            m_configPath = "";
            m_parseContent = "";
            m_parsedOk = false;

            m_paras = new Dictionary<string, string>();
            m_tables = new Dictionary<string, DataTable>();
        }
        public bool parseFile(string filePath)
        {
            m_parsedOk = false;
            m_configPath = filePath;
            if (!File.Exists(m_configPath))
            {
                MessageBox.Show("file:" + filePath + " not find!!");
                return false;
            }
            m_parseContent = File.ReadAllText(m_configPath);
            return this.parseString(m_parseContent);
        }
        public bool parseString(string str)
        {
            m_parsedOk = false;
            m_parseContent = str;
            m_xmlReader = XmlReader.Create(new StringReader(m_parseContent));
            while (m_xmlReader.Read())
            {
                switch (m_xmlReader.NodeType)
                {
                    case XmlNodeType.Element:
                        if ("data_table" == m_xmlReader.Name)
                        {
                            parseDataTable();
                        }
                        else
                        {
                            parseSpecialNode(m_xmlReader.Name);
                        }
                        break;
                    case XmlNodeType.Text:
                        break;
                    case XmlNodeType.XmlDeclaration:
                    case XmlNodeType.ProcessingInstruction:
                        //(reader.Name, reader.Value);
                        break;
                    case XmlNodeType.Comment:
                        //reader.Value
                        break;
                    case XmlNodeType.EndElement:
                        break;
                }
            }
            m_parsedOk = true;
            return m_parsedOk;
        }

        private bool parseSpecialNode(string nodeName)
        {
            string value = "";
            if (m_xmlReader.HasAttributes)
            {
                value = tryGetAttribute("value");
            }
            if ("" == value)
            {
                m_xmlReader.Read();
                if (XmlNodeType.Text == m_xmlReader.NodeType)
                {
                    value = m_xmlReader.Value;
                }
            }
            m_paras[nodeName] = value;
            return true;
        }
        
        private bool parseDataTable()
        {
            DataTable dt = new DataTable();
            dt.m_tableName = tryGetAttribute("tableName");
            dt.m_tableBackgroundColor = tryGetAttribute(DataTable.tableBackgroundColor, ConfigParser.defaultBackColor);
            dt.m_columnHeadersBackColor = tryGetAttribute(DataTable.columnHeadersBackColor, ConfigParser.defaultBackColor);
            dt.m_rowsBackColor = tryGetAttribute(DataTable.rowsBackColor, ConfigParser.defaultBackColor);
            dt.m_rowMouseMoveBackColor = tryGetAttribute(DataTable.rowMouseMoveBackColor, ConfigParser.defaultBackColor);
            dt.m_rowMouseLeaveBackColor = tryGetAttribute(DataTable.rowMouseLeaveBackColor, ConfigParser.defaultBackColor);

            bool bContinue = true;
            while (bContinue)
            {
                switch (m_xmlReader.NodeType)
                {
                    case XmlNodeType.Element:
                        if ("column" == m_xmlReader.Name)
                        {
                            parseTableCol(dt);
                        }
                        break;
                    case XmlNodeType.EndElement:
                        if ("data_table" == m_xmlReader.Name)
                        {
                            bContinue = false;
                        }
                        break;

                    default:
                        break;
                }
                bContinue = bContinue && m_xmlReader.Read();
            }

            m_tables.Add(dt.m_tableName, dt);
            return true;
        }
        private bool parseTableCol(DataTable dt)
        {
            TableColumn col = new TableColumn();
            col.m_name = tryGetAttribute("name");
            col.m_dataType = tryGetAttribute("data_type");
            col.m_width = tryGetAttribute("width");
            dt.m_columns.Add(col);
            return true;
        }
        private string tryGetAttribute(string name, string defaultValue)
        {
            try
            {
                string value = m_xmlReader.GetAttribute(name);
                if (null != value)
                    return value;
            }
            catch (System.ArgumentOutOfRangeException)
            {
            }
            return defaultValue;
        }

        private string tryGetAttribute(string name)
        {
            return tryGetAttribute(name, "");
        }

        public string getSpecialPara(string paraName, string defaultValue)
        {
            if (m_parsedOk && m_paras.ContainsKey(paraName))
            {
                return m_paras[paraName];
            }
            return defaultValue;
        }

        public DataTable getDataTable(string tableName)
        {
            if (m_parsedOk)
            {
                if (m_tables.ContainsKey(tableName))
                {
                    return m_tables[tableName];
                }
            }
            return new DataTable();
        }

        public static Color getConfigParaColor(string paraName)
        {
            Color retColor = Color.Red;
            retColor = Color.FromName(paraName);
            return retColor;
        }
    }
}
