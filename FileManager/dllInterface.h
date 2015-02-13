// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#include <string>
#include <map>
#include "ace/OS.h"
#include "ace/ace.h"
#include "ace/singleton.h"

#define SHOW_FILE_ITEM_NUM 100

static const std::string VolumeIDsSeparator	= "__###__";
static const std::string MessageSplitSign	= ",..,";
static const std::string VolumeAllIDs		= "All";
static const std::string DefaultInitString	= "DefaultInitString ~!@#$%^&*()_+";
static const std::string SelectedConditionDefault = "no selected condition, all file match. ~!@#$%^&*()_+";

static const char* DirectCallBack = "DirectCallBack";

static const std::string UiTabName_showAllFiles		= "showAllFiles";
static const std::string UiTabName_findSameFiles	= "findSameFiles";

static const std::string UiName_AllFiles	= "AllFiles";
static const std::string UiName_SameFiles	= "SameFiles";

// msg para names
static const std::string PN_MessageValue		= "Value";
static const std::string PN_MessageValue2		= "Value2";
static const std::string PN_PartitionGroup		= "ParttGroup";
static const std::string PN_ShowName			= "ShowName";	//need be unique.
static const std::string PN_ShowType			= "ShowType";	//[AllFiles, SameFiles, MoreFiles]
static const std::string PN_CmpToParttGroup		= "CmpToParttGroup";
static const std::string PN_SelectCondition		= "SelectCondition";

//msg para values.
static const std::string PV_ShowType_AllFiles	= "AllFiles";
static const std::string PV_ShowType_SameFiles	= "SameFiles";
static const std::string PV_ShowType_MoreFiles	= "MoreFiles";

// to service.
static const std::string cmdLoadAllVolumeIDs		= "LoadAllVolumeIDs";
static const std::string cmdSetCurrentShow			= "SetCurrentShow";
static const std::string cmdSetPartitionMulSel		= "SetPartitionMulSel";
static const std::string cmdUpdateSelectCondition	= "UpdateSelectCondition";
static const std::string cmdShowMoreItems			= "ShowMoreItems";
static const std::string cmdSortShowItems			= "SortShowItems";
static const std::string cmdGetSameFileAllPaths		= "GetSameFileAllPaths";
static const std::string cmdCancelLoadVolume		= "CancelLoadVolume";

// to ui
static const std::string cmdShowStatusMessage	= "ShowStatusMessage";
static const std::string cmdVolumeIDsUpdate		= "VolumeIDsUpdate";
static const std::string cmdFileListItemClear	= "FileListItemClear";	//ShowFullName,
static const std::string cmdFileListItemAdd		= "FileListItemAdd";
static const std::string cmdFileListItemUpdate	= "FileListItemUpdate";
static const std::string cmdFileListItemRemove	= "FileListItemRemove";
static const std::string cmdShowSameFileAllPaths= "ShowSameFileAllPaths";
static const std::string cmdUpdateShowedNums	= "UpdateShowedNums";
static const std::string cmdUpdateLoadingRate	= "UpdateLoadingRate";


//============================================================================

enum MessageTypes{
	MSG_None = 1,
	MSG_SetPartitionGroup,	//PN_PartitionGroup, PN_MessageValue, 
	MSG_SetCurrentShow,		//PN_ShowName, PN_ShowType, PN_PartitionGroup, [PN_CmpToParttGroup]
	MSG_ShowFileList,
	MSG_UpdateSelectCondition,	//PN_ShowName, PN_MessageValue, 
	MSG_SortFileList,			//PN_ShowName, PN_MessageValue, 
	MSG_ShowMoreItems,			//PN_ShowName,
	MSG_GetSameFileAllPaths,	//PN_ShowName, PN_MessageValue,
	MSG_MaxSize,
};


void myAssertFail();
int  printStackTrace();

struct MessageInfo
{
	MessageTypes type;
	std::string value;
	std::map<std::string, std::string> paras;
	
	MessageInfo()
	{
		type = MSG_None;
		value = "";
		paras.clear();
	}
	std::string getPara(std::string name)
	{
		std::string value = "";
		if (paras.end() != paras.find(name))
		{
			value = paras[name];
		}
		return value;
	}
	bool hasPara(std::string name)
	{
		bool ret = false;
		if (paras.end() != paras.find(name))
		{
			ret = true;
		}
		return ret;
	}
	//simplely update exist values and add new values.
	bool mergeMsgValues(MessageInfo& msg)
	{
		std::map<std::string, std::string>::iterator it = msg.paras.begin();
		while (msg.paras.end() != it)
		{
			paras[it->first] = it->second;
			++it;
		}
		return true;
	}
	void print()
	{
		std::string typeStr = "MSG_None";
		switch (type)
		{
		case MSG_SetPartitionGroup:
			typeStr = "MSG_SetPartitionGroup";
			break;
		case MSG_SetCurrentShow:
			typeStr = "MSG_SetCurrentShow";
			break;
		case MSG_ShowFileList:
			typeStr = "MSG_ShowFileList";
			break;
		case MSG_UpdateSelectCondition:
			typeStr = "MSG_UpdateSelectCondition";
			break;
		case MSG_SortFileList:
			typeStr = "MSG_SortFileList";
			break;
		case MSG_ShowMoreItems:
			typeStr = "MSG_ShowMoreItems";
			break;
		case MSG_GetSameFileAllPaths:
			typeStr = "MSG_GetSameFileAllPaths";
			break;
		case MSG_MaxSize:
			typeStr = "MSG_MaxSize";
			break;
		}
		printf("MessageInfo type:%s, para size:%d\n", typeStr.c_str(), paras.size());
	}
};
//============================================================================

typedef int (__stdcall* CallBackToOwner) (const char* msgType, const char* msgContent);

static CallBackToOwner g_callBackToOwner;

class MainProcessor;
class DllInterface
{
public:
	explicit DllInterface();
	~DllInterface();
	void start(CallBackToOwner notifyFunc = NULL);

	int parseCmd(const char* const msgType, const char* const msgValue);	
private:
	MainProcessor* m_mainProcessor;
};

typedef ACE_Unmanaged_Singleton<DllInterface, ACE_Thread_Mutex> DllInterfaceSingleton;


extern "C" __declspec(dllexport) int initService(CallBackToOwner callback);
extern "C" __declspec(dllexport) int finiService();
extern "C" __declspec(dllexport) int postMessageToService(const char* const msgType, const char* const msgValue);
