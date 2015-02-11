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

class PartitionGroup: public IDiskMonitorEvent
{
public:
	PartitionGroup(std::string name);
	~PartitionGroup();

	void regisgerCallBack(CallBackToOwner notifyFunc);

	int cancelLoadVolume() {m_bCancelLoadVolume = true; return 0;};

	virtual bool updateLoadingRate(int rate, const char* vol = 0);
	virtual bool notifyFilesChange(FileOperation, FileInfo*);

	void loadVolumeFiles(std::string volume);	// eg "(E:)"

	int  getAllFiles(std::map<DWORDLONG, FileInfo*>& allFiles, int updateId);

private:

	void reSetData();

	void constructFileFullPath(DuLinkList & files);

	std::string				m_name;
	volatile bool			m_bCancelLoadVolume;
	int						m_updateId;

	CallBackToOwner			m_notifyFunc;

	DiskMonitor				m_diskReader;
	DuLinkList				m_fileList;
};
