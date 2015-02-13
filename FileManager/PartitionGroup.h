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

class PartitionGroup
{
public:
	PartitionGroup(std::string name);
	~PartitionGroup();

	void addVolume(std::string name, DiskMonitor* dm);
	void clearAllVolume(){m_nameToDiskMonitors.clear(); ++m_updateId;};
	int  getUpdatedFiles(FilesMapType& allFiles, int updateId, std::vector<FileOperationRecord>& operations);
	
	bool volumeChange(FileOperation fo, std::string volume, FileInfo* fi);
private:

	std::string				m_name;
	int						m_updateId;
	int						m_startUpdateId;
	ACE_Thread_Mutex		m_lock;
	std::map<std::string, DiskMonitor*>	m_nameToDiskMonitors;

	std::map<int, FileOperationRecord>	m_fileOperationRecords;
};
