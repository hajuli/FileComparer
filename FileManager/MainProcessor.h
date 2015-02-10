// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#include "ace/Thread_Semaphore.h"
#include "ace/Thread_Mutex.h"
#include "ace/Task.h"
#include "dllInterface.h"
#include "PartitionGroup.h"
#include "FilesDisplayer.h"
#include <vector>
#include <list>
#include <map>

class MainProcessor: public ACE_Task_Base
{
public:
	MainProcessor();
	~MainProcessor();

	void startProcess();
	void stopProcess();
	void regisgerCallBack(CallBackToOwner notifyFunc);
	int loadAllVolumeIDs();

	int addMessage(MessageInfo msg);
	int addNextMessage(MessageInfo msg);
	void setPartitionGroup(MessageInfo msg);
	int cancelLoadVolume(MessageInfo msg);
	int setCurrentShow(MessageInfo msg);
	int updateSelectCondition(MessageInfo msg);
	int showFileList(MessageInfo msg);
	int sortFileList(MessageInfo msg);

private:
	virtual int svc();

	void createDisplayer(std::string name);
	int getSameFileAllPaths(MessageInfo msg);

	MessageTypes			m_nextMessage[MSG_MaxSize];
	volatile bool			m_bRunning;
	std::string				m_currentShowName;

	ACE_Thread_Semaphore	m_semaphore;
	ACE_Thread_Mutex		m_queueLock;
	CallBackToOwner			m_notifyFunc;

	std::list<MessageInfo>		m_msgList;
	std::map<std::string, MessageInfo>		m_showNameMessages;
	std::map<std::string, PartitionGroup*>	m_partitionGroups;
	std::map<std::string, FilesDisplayer*>	m_filesDisplayers;
	std::map<std::string, std::map<std::string, bool>> m_pgNameToDisplayerName;

};


class ThreadWorker : public ACE_Task_Base
{
public:
	ThreadWorker(PartitionGroup* owner):
		m_semaphore(0), 
		m_bRunning(false)
	{
		m_owner = owner;
	};
	virtual int svc()
	{
		while (m_bRunning)
		{
			m_semaphore.acquire();
			if(false == m_bRunning)
			{
				break;
			}
			m_owner->findSameFiles();
		}
		return 0;
	};
	void startProcess()
	{
		m_bRunning = true;
		this->activate();
	};
	void stopProcess()
	{
		m_bRunning = false;
		m_semaphore.release();
		this->wait();
	};
private:
	PartitionGroup* m_owner;
	volatile bool			m_bRunning;
	ACE_Thread_Semaphore	m_semaphore;
};
