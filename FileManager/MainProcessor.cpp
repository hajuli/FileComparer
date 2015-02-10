// stdafx.cpp : source file that includes just the standard includes
// MainProcessor.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file


#include "MainProcessor.h"
#include <algorithm>

MainProcessor::MainProcessor():
m_bRunning(false),
m_semaphore(0),
m_queueLock(NULL),
m_currentShowName(DefaultInitString)
{
	m_nextMessage[MSG_None] = MSG_None;
	m_nextMessage[MSG_SetPartitionGroup] = MSG_ShowFileList;
	m_nextMessage[MSG_SetCurrentShow] = MSG_ShowFileList;
	m_nextMessage[MSG_ShowFileList] = MSG_None;
	m_nextMessage[MSG_UpdateSelectCondition] = MSG_ShowFileList;
	m_nextMessage[MSG_SortFileList] = MSG_ShowFileList;
	m_nextMessage[MSG_ShowMoreItems] = MSG_None;
	m_nextMessage[MSG_ShowSameFileList] = MSG_None;
	m_nextMessage[MSG_ShowMoreSameItems] = MSG_None;
	m_nextMessage[MSG_GetSameFileAllPaths] = MSG_None;
	m_nextMessage[MSG_SetLoadVolume] = MSG_None;
	m_nextMessage[MSG_FindSameFile] = MSG_None;
	m_nextMessage[MSG_UpdateSameFilesSelectCondition] = MSG_None;
}


MainProcessor::~MainProcessor()
{
	stopProcess();
}

void MainProcessor::startProcess()
{
	m_bRunning = true; 
	this->activate();
}

void MainProcessor::stopProcess()
{
	if(true == m_bRunning)
	{
		m_bRunning = false;
		m_semaphore.release();
		this->wait();
	}
}

int MainProcessor::svc()
{
	while(m_bRunning)
	{
		m_semaphore.acquire();
		if(false == m_bRunning)
		{
			break;
		}
		if (m_msgList.empty())
		{
			continue;
		}
		MessageInfo msg = m_msgList.front();
		m_msgList.pop_front();
		
		if (MSG_SetPartitionGroup == msg.type)
		{
			setPartitionGroup(msg);
		}
		else if (MSG_ShowFileList == msg.type)
		{
			showFileList(msg);
		}
		else if (MSG_UpdateSelectCondition == msg.type)
		{
		}
		else if (MSG_UpdateSameFilesSelectCondition == msg.type)
		{
		}
		else if (MSG_SortFileList == msg.type)
		{
		}
		else if (MSG_ShowMoreItems == msg.type)
		{
		}
		else if (MSG_ShowSameFileList == msg.type)
		{
		}
		else if (MSG_ShowMoreSameItems == msg.type)
		{
		}
		else if (MSG_GetSameFileAllPaths == msg.type)
		{
			getSameFileAllPaths(msg);
		}
		else if (MSG_SetLoadVolume == msg.type)
		{
		}
		else if (MSG_FindSameFile == msg.type)
		{
		}
	}
	return 0;
}

void MainProcessor::regisgerCallBack(CallBackToOwner notifyFunc)
{
	m_notifyFunc = notifyFunc;
}

int MainProcessor::addMessage(MessageInfo msg)
{
	m_msgList.push_back(msg);
	m_semaphore.release();
	return 0;
}

int MainProcessor::addNextMessage(MessageInfo msg)
{
	MessageInfo newMsg = msg;
	newMsg.type = m_nextMessage[msg.type];
	if (MSG_None != newMsg.type)
	{
		m_msgList.push_back(newMsg);
		m_semaphore.release();
	}
	return 0;
}

void MainProcessor::createDisplayer(std::string name)
{
	if (m_filesDisplayers.end() != m_filesDisplayers.find(name)
		|| m_showNameMessages.end() == m_showNameMessages.find(name))
	{
		return;
	}
	
	MessageInfo msg = m_showNameMessages[name];
	if ( msg.hasPara(PN_PartitionGroup)
		&& m_partitionGroups.end() != m_partitionGroups.find(msg.getPara(PN_PartitionGroup)))
	{
		PartitionGroup* pg = m_partitionGroups[msg.getPara(PN_PartitionGroup)];
		std::string type = msg.getPara(PN_ShowType);
		if (PV_ShowType_AllFiles == type)
		{
			m_filesDisplayers[name] = new AllFilesDisplayer(name, pg, m_notifyFunc);
			m_pgNameToDisplayerName[msg.getPara(PN_PartitionGroup)][name] = true;
		}
		else if (PV_ShowType_SameFiles == type)
		{
			m_filesDisplayers[name] = new SameFilesDisplayer(name, pg, m_notifyFunc);
			m_pgNameToDisplayerName[msg.getPara(PN_PartitionGroup)][name] = true;
		}
		else if (PV_ShowType_MoreFiles == type)
		{
			if (msg.hasPara(PN_CmpToParttGroup)
				&& m_partitionGroups.end() != m_partitionGroups.find(msg.getPara(PN_CmpToParttGroup)) )
			{
				PartitionGroup* cmp2pg = m_partitionGroups[msg.getPara(PN_CmpToParttGroup)];
				m_filesDisplayers[name] = new MoreFilesDisplayer(name, pg, cmp2pg, m_notifyFunc);

				m_pgNameToDisplayerName[msg.getPara(PN_PartitionGroup)][name] = true;
				m_pgNameToDisplayerName[msg.getPara(PN_CmpToParttGroup)][name] = true;
			}
		}
	}
}

int MainProcessor::loadAllVolumeIDs()
{
	DiskReader diskReader("", 0);
	std::vector<std::string> ids;
	diskReader.loadAllVolumeIDs(ids);
	std::string vols = "";
	for (int i = 0; i < ids.size(); ++i)
	{
		if (i)
		{
			vols = vols + VolumeIDsSeparator;
		}
		vols = vols + ids[i];
	}
	m_notifyFunc(cmdVolumeIDsUpdate.c_str(), vols.c_str());
	return 0;
}

void MainProcessor::setPartitionGroup(MessageInfo msg)
{
	std::string name = msg.getPara(PN_PartitionGroup);
	if ("" == name)
	{
		return;
	}
	if (m_partitionGroups.end() == m_partitionGroups.find(name))
	{
		PartitionGroup* p = new PartitionGroup(name);
		p->regisgerCallBack(m_notifyFunc);
		m_partitionGroups[name] = p;
	}
	PartitionGroup* p = m_partitionGroups[name];
	p->loadVolumeFiles(msg.getPara(PN_MessageValue));

	addNextMessage(msg);
}

int MainProcessor::setCurrentShow(MessageInfo msg)
{
	if (msg.hasPara(PN_ShowName))
	{
		std::string name = msg.getPara(PN_ShowName);
		if (m_showNameMessages.end() == m_showNameMessages.find(name))
		{
			m_showNameMessages[name] = msg;	// just using first msg.
		}
		//aways set new show Name.
		m_currentShowName = name;
		addNextMessage(msg);
	}
	return 0;
}

int MainProcessor::updateSelectCondition(MessageInfo msg)
{
	std::string name = msg.getPara(PN_ShowName);
	if (m_filesDisplayers.end() != m_filesDisplayers.find(name))
	{
		FilesDisplayer* fd = m_filesDisplayers[name];
		fd->updateSelectCondition(msg.getPara(PN_MessageValue));
	
		addNextMessage(msg);
	}
	return 0;
}

int MainProcessor::sortFileList(MessageInfo msg)
{
	std::string name = msg.getPara(PN_ShowName);
	if (m_filesDisplayers.end() != m_filesDisplayers.find(name))
	{
		FilesDisplayer* fd = m_filesDisplayers[name];
		fd->sortFileList(msg.getPara(PN_MessageValue));
	
		addNextMessage(msg);
	}
	return 0;
}

int MainProcessor::getSameFileAllPaths(MessageInfo msg)
{
	std::string name = msg.getPara(PN_ShowName);
	if (m_filesDisplayers.end() != m_filesDisplayers.find(name))
	{
		FilesDisplayer* fd = m_filesDisplayers[name];
		fd->getSameFileAllPaths(msg.getPara(PN_MessageValue));
	}
	return 0;
}

int MainProcessor::showFileList(MessageInfo msg)
{
	if (msg.hasPara(PN_ShowName) && m_currentShowName != msg.getPara(PN_ShowName))
	{
		return 0;
	}
	if (m_filesDisplayers.end() == m_filesDisplayers.find(m_currentShowName))
	{
		createDisplayer(m_currentShowName);
	}
	if (m_filesDisplayers.end() != m_filesDisplayers.find(m_currentShowName))
	{
		m_filesDisplayers[m_currentShowName]->show();
	}
	return 0;
}


int MainProcessor::cancelLoadVolume(MessageInfo msg)
{
	std::string name = msg.getPara(PN_PartitionGroup);
	if ("" == name)
	{
		return 0;
	}
	if (m_partitionGroups.end() == m_partitionGroups.find(name))
	{
		return 0;
	}
	PartitionGroup* p = m_partitionGroups[name];
	p->cancelLoadVolume();

	return 0;
}
