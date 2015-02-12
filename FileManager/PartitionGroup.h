// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#include "ace/Thread_Semaphore.h"
#include "ace/Thread_Mutex.h"
#include "dllInterface.h"
#include "diskMonitor.h"
#include "fileInfo.h"
#include <vector>
#include <list>
#include <map>


const int SHOW_FILE_ITEM_NUM = 100;

class DiskReader;
class ThreadWorker;

class PartitionGroup
{
public:
	PartitionGroup(std::string name);
	~PartitionGroup();

	void addVolume(std::string name, DiskMonitor* dm);
	void clearAllVolume(){m_nameToDiskMonitors.clear(); ++m_updateId;};
	int  getAllFiles(std::map<DWORDLONG, FileInfo*>& allFiles, int updateId);
	
	bool volumeChange(FileOperation fo, std::string volume, FileInfo* fi);
private:

	std::string				m_name;
	int						m_updateId;
	std::map<std::string, DiskMonitor*>	m_nameToDiskMonitors;
};
