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
	printStackTrace();
}

//==========================
#include <process.h>
#include <iostream>
#include <Windows.h>
#include "dbghelp.h"

#define TRACE_MAX_STACK_FRAMES 1024
#define TRACE_MAX_FUNCTION_NAME_LENGTH 1024

int printStackTrace()
{
    void *stack[TRACE_MAX_STACK_FRAMES];
    HANDLE process = GetCurrentProcess();
    SymInitialize(process, NULL, TRUE);
    WORD numberOfFrames = CaptureStackBackTrace(0, TRACE_MAX_STACK_FRAMES, stack, NULL);
    SYMBOL_INFO *symbol = (SYMBOL_INFO *)malloc(sizeof(SYMBOL_INFO)+(TRACE_MAX_FUNCTION_NAME_LENGTH - 1) * sizeof(TCHAR));
    symbol->MaxNameLen = TRACE_MAX_FUNCTION_NAME_LENGTH;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    DWORD displacement;
    IMAGEHLP_LINE64 *line = (IMAGEHLP_LINE64 *)malloc(sizeof(IMAGEHLP_LINE64));
    line->SizeOfStruct = sizeof(IMAGEHLP_LINE64);
    for (int i = 0; i < numberOfFrames; i++)
    {
        DWORD64 address = (DWORD64)(stack[i]);
        SymFromAddr(process, address, NULL, symbol);
        if (SymGetLineFromAddr64(process, address, &displacement, line))
        {
            printf("\t%d at %s in %s: line: %lu: address: 0x%0X\n", numberOfFrames - i -1, symbol->Name, line->FileName, line->LineNumber, symbol->Address);
        }
        else
        {
            printf("\tSymGetLineFromAddr64 returned error code %lu.\n", GetLastError());
            printf("\t%d at %s, address 0x%0X.\n", numberOfFrames - i - 1, symbol->Name, symbol->Address);
        }
    }
    return 0;
}
//ref: https://msdn.microsoft.com/en-us/library/windows/desktop/bb204633(v=vs.85).aspx
//ref: http://stackoverflow.com/questions/22467604/how-can-you-use-capturestackbacktrace-to-capture-the-exception-stack-not-the-ca
//==========================

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
	if (cmdSortShowItems == msgType)
	{
		msg.type = MSG_SortFileList;
		m_mainProcessor->sortFileList(msg);	// using ui thread sort.
		return 0;
	}

	if (cmdUpdateSelectCondition == msgType)
	{
		msg.type = MSG_UpdateSelectCondition;
	}
	else if (cmdSetPartitionMulSel == msgType)
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
