// stdafx.cpp : source file that includes just the standard includes
// MainProcessor.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file


#include "MainProcessor.h"
#include <algorithm>

MainProcessor::MainProcessor():
m_lock(NULL),
m_msgLock(NULL),
m_bCancelLoadVolume(false),
m_currentShowName(DefaultInitString)
{
	m_nameToDiskMonitors.clear();

	m_nextMessage[MSG_None] = MSG_None;
	m_nextMessage[MSG_SetPartitionGroup] = MSG_ShowFileList;
	m_nextMessage[MSG_SetCurrentShow] = MSG_ShowFileList;
	m_nextMessage[MSG_ShowFileList] = MSG_None;
	m_nextMessage[MSG_UpdateSelectCondition] = MSG_ShowFileList;
	m_nextMessage[MSG_SortFileList] = MSG_ShowFileList;
	m_nextMessage[MSG_ShowMoreItems] = MSG_None;
	m_nextMessage[MSG_GetSameFileAllPaths] = MSG_None;
	m_nextMessage[MSG_SetLoadVolume] = MSG_None;
	m_nextMessage[MSG_FindSameFile] = MSG_None;
	m_nextMessage[MSG_UpdateSameFilesSelectCondition] = MSG_None;
}


MainProcessor::~MainProcessor()
{
	stopProcess();
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
		{
			MutexGuard guard(m_msgLock);
			m_msgList.pop_front();
		}

		MutexGuard guard(m_lock);
		
		if (MSG_SetPartitionGroup == msg.type)
		{
			setPartitionGroup(msg);
		}
		else if (MSG_ShowFileList == msg.type || MSG_ShowMoreItems == msg.type)
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
	if (MSG_None != msg.type)
	{
		MutexGuard guard(m_msgLock);
		m_msgList.push_back(msg);
		m_semaphore.release();
	}
	return 0;
}

int MainProcessor::addNextMessage(MessageInfo msg)
{
	MessageInfo newMsg = msg;
	newMsg.type = m_nextMessage[msg.type];
	addMessage(newMsg);
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
	std::vector<std::string> ids;
	DiskMonitor::loadAllVolumeIDs(ids);
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
		m_partitionGroups[name] = p;
	}
	PartitionGroup* p = m_partitionGroups[name];
	p->clearAllVolume();
	loadVolumeFiles(name, msg.getPara(PN_MessageValue));

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
	m_bCancelLoadVolume = true;
	return 0;
}

void MainProcessor::loadVolumeFiles(std::string pgName, std::string volume)
{
	m_bCancelLoadVolume = false;
	std::vector<std::string> ids;
	if (VolumeAllIDs == volume)
	{
		DiskMonitor::loadAllVolumeIDs(ids);
	}
	else
	{
		int i = 0;
		while ( i < volume.size())
		{
			if(std::string::npos != volume.find(";", i))
			{
				int j = volume.find(";", i);
				ids.push_back(volume.substr(i, j - i));
				i = j + 1;
			}
			else
			{
				ids.push_back(volume.substr(i));
				break;
			}
		}
	}
	char buf[500];
	std::string showVol = "";
	std::string vol;

	for (int id = 0; id < ids.size() && false == m_bCancelLoadVolume; ++id)
	{
		vol = ids[id];
		if (std::string::npos != vol.find("(") && std::string::npos != vol.find(")", vol.find("(")))
		{
			vol = vol.substr(vol.find("(") + 1);
			vol = vol.substr(0, vol.find(")"));
		}
		else
		{
			printf("error, not find (x:) from string %s\n", vol);
			continue;
		}
		sprintf(buf, "start loading files from volume %s", vol.c_str());
		m_notifyFunc(cmdShowStatusMessage.c_str(), buf);

		updateLoadingRate(0, vol.c_str());
	
		if (m_nameToDiskMonitors.end() != m_nameToDiskMonitors.find(vol))
		{
			m_partitionGroups[pgName]->addVolume(vol, m_nameToDiskMonitors.find(vol)->second);
			continue;
		}
		DiskMonitor* dm = new DiskMonitor(vol, this);
		if (dm->loadAllFiles())
		{
			dm->startProcess();
			m_nameToDiskMonitors[vol] = dm;
			m_partitionGroups[pgName]->addVolume(vol, dm);
			sprintf(buf, "load %d files from volume %s", dm->getAllFilesCount(), vol.c_str());
			m_notifyFunc(cmdShowStatusMessage.c_str(), buf);
		}
		else
		{
			delete dm;
			dm = 0;
			sprintf(buf, "cancel or fail to load volume %s", showVol.c_str());
			m_notifyFunc(cmdShowStatusMessage.c_str(), buf);
		}
		
		if (!showVol.empty())
		{
			showVol = showVol + ",";
		}
		showVol = showVol + vol;

	}
	updateLoadingRate(100);

}

bool MainProcessor::updateLoadingRate(int rate, const char* vol)
{
	char buf[1024] = {0};
	if (vol)
	{
		sprintf(buf, "loadingRate=%d,.., volume=%s", rate, vol);
	}
	else
	{
		sprintf(buf, "loadingRate=%d", rate);
	}
	m_notifyFunc(cmdUpdateLoadingRate.c_str(), buf);

	if (m_bCancelLoadVolume)
		return false;	//failed.
	return true;
}

bool MainProcessor::notifyFilesChange(FileOperation fo, std::string volume, FileInfo* fi)
{
	MutexGuard guard(m_lock);
	std::map<std::string, PartitionGroup*>::iterator it = m_partitionGroups.begin();
	while (m_partitionGroups.end() != it)
	{
		PartitionGroup* pg = it->second;
		if (pg->volumeChange(fo, volume, fi))
		{
			if (m_pgNameToDisplayerName[it->first].end() != m_pgNameToDisplayerName[it->first].find(m_currentShowName))
			{
				MessageInfo msg;
				msg.type = MSG_ShowFileList;
				addMessage(msg);
			}
		}
		++it;
	}
	return true;
}
