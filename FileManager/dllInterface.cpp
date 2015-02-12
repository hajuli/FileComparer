// stdafx.cpp : source file that includes just the standard includes
// FileManager.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file


#include "dllInterface.h"
#include "MainProcessor.h"


void myAssertFail()
{
	int i = 0;
	int j = 10 / i;
	printf("in myAssertFail\n");
}

int parseMsg(const char* const msgValue, MessageInfo& msgInfo)
{
	std::string msg = msgValue;
	int i = 0;
	while (true && msg.length() > 0)
	{
		bool b = true;
		int pos = msg.find(MessageSplitSign, i);
		std::string para = msg.substr(i);
		if (std::string::npos != pos)
		{
			para = msg.substr(i, pos - i);
			i = pos + MessageSplitSign.length();
			b = false;
		}
		pos = para.find("=");
		if (std::string::npos == pos)
		{
			myAssertFail();
		}
		msgInfo.paras[para.substr(0, pos)] = para.substr(pos + 1);
		if (b)
		{
			break;
		}
	}
	return 0;
}

DllInterface::DllInterface()
{
	ACE::init();
	m_mainProcessor = new MainProcessor();
}


DllInterface::~DllInterface()
{

	ACE::fini();
	delete m_mainProcessor;
	m_mainProcessor = 0;
}

void DllInterface::start(CallBackToOwner notifyFunc)
{
	m_mainProcessor->regisgerCallBack(notifyFunc);
	m_mainProcessor->startProcess();
}

int DllInterface::parseCmd(const char* const msgType, const char* const msgValue)
{
	timeb timestamp;
	ftime( &timestamp );
	printf("%lld, DllInterface got msg: %s, %s\n", timestamp.time, msgType, msgValue);

	MessageInfo msg;
	msg.type = MSG_None;
	msg.paras.clear();
	parseMsg(msgValue, msg);

	if (cmdSetCurrentShow == msgType)
	{
		msg.type = MSG_SetCurrentShow;
		return m_mainProcessor->setCurrentShow(msg);
	}
	if (cmdLoadAllVolumeIDs == msgType)
	{
		return m_mainProcessor->loadAllVolumeIDs();
	}
	if (cmdCancelLoadVolume == msgType)
	{
		return m_mainProcessor->cancelLoadVolume(msg);
	}
	if (cmdUpdateSelectCondition == msgType)
	{
		msg.type = MSG_UpdateSelectCondition;
		return m_mainProcessor->updateSelectCondition(msg);
	}
	if (cmdSortShowItems == msgType)
	{
		msg.type = MSG_SortFileList;
		m_mainProcessor->sortFileList(msg);	// using ui thread sort.
		return 0;
	}

	if (cmdSetPartitionMulSel == msgType)
	{
		msg.type = MSG_SetPartitionGroup;
	}
	else if (cmdShowMoreItems == msgType)
	{
		msg.type = MSG_ShowMoreItems;
	}	
	else if (cmdGetSameFileAllPaths == msgType)
	{
		msg.type = MSG_GetSameFileAllPaths;
	}
	
	m_mainProcessor->addMessage(msg);
	return 0;
}


//--------------------------------------For Dll----------------------------------


int initService(CallBackToOwner callBack)
{
	g_callBackToOwner = callBack;
	DllInterfaceSingleton::instance()->start(callBack);
	return 0;
}

int finiService()
{
	DllInterfaceSingleton::close();
	return 0;
}

int postMessageToService(const char* const msgType, const char* const msgValue)
{
	std::string type = msgType;
	if (type == DirectCallBack)
	{
		return g_callBackToOwner(msgType, msgValue);
	}
	return DllInterfaceSingleton::instance()->parseCmd(msgType, msgValue);
}
