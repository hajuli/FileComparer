// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#include "ThreadWorker.h"
#include "dllInterface.h"
#include "PartitionGroup.h"
#include "FilesDisplayer.h"
#include <vector>
#include <list>
#include <map>

class MainProcessor: public IDiskMonitorEvent, public ThreadWorker
{
public:
	MainProcessor();
	~MainProcessor();

	void regisgerCallBack(CallBackToOwner notifyFunc);
	int loadAllVolumeIDs();

	int addMessage(MessageInfo msg);
	void setPartitionGroup(MessageInfo msg);
	int cancelLoadVolume(MessageInfo msg);
	int setCurrentShow(MessageInfo msg);
	int showFileList(MessageInfo msg);
	int sortFileList(MessageInfo msg);

	virtual bool updateLoadingRate(int rate, const char* vol = 0);
	virtual bool notifyFilesChange(FileOperation, std::string volume, FileInfo*);

private:
	virtual int svc();
	int addNextMessage(MessageInfo msg);
	void mergeMsgValues(MessageInfo& msg);

	int updateSelectCondition(MessageInfo msg);
	void loadVolumeFiles(std::string pgName, std::string volume);
	void createDisplayer(std::string name);
	int getSameFileAllPaths(MessageInfo msg);

	MessageTypes			m_nextMessage[MSG_MaxSize];
	std::string				m_currentShowName;

	ACE_Thread_Mutex		m_lock;
	ACE_Thread_Mutex		m_msgLock;
	CallBackToOwner			m_notifyFunc;

	std::list<MessageInfo>		m_msgList;
	std::map<std::string, MessageInfo>		m_showNameMessages;
	std::map<std::string, PartitionGroup*>	m_partitionGroups;
	std::map<std::string, FilesDisplayer*>	m_filesDisplayers;
	std::map<std::string, std::map<std::string, bool>> m_pgNameToDisplayerName;

	bool			m_bCancelLoadVolume;
	std::map<std::string, DiskMonitor*>	m_nameToDiskMonitors;
};

