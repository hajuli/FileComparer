// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#include "ace/Thread_Semaphore.h"
#include "ace/Thread_Mutex.h"
#include "dllInterface.h"
#include "diskReader.h"
#include "fileInfo.h"
#include <vector>
#include <list>
#include <map>


const int SHOW_FILE_ITEM_NUM = 100;

class DiskReader;
class ThreadWorker;

class PartitionGroup: public IDiskReadEvent
{
public:
	PartitionGroup(std::string name);
	~PartitionGroup();

	void regisgerCallBack(CallBackToOwner notifyFunc);

	int setSelectCondition(std::string newCondition);
	int loadAllVolumeIDs();
	void showMoreItems();

	void showFileList(int fromIndex = 0);
	void sortFileList(std::string sortKeys);

	int addMessage(MessageInfo msg);

	void findSameFiles();
	void showSameFiles(int fromIndex);
	int cancelLoadVolume() {m_bCancelLoadVolume = true; return 0;};

	virtual bool updateLoadingRate(int rate, const char* vol = 0);

	void loadVolumeFiles(std::string volume);	// eg "E:"

	int  getAllFiles(std::map<DWORDLONG, FileInfo*>& allFiles, int updateId);

private:

	void reSetData();

	void updateSelectCondition();
	void updateSameFilesSelect();
	
	void constructFileFullPath(DuLinkList & files);

	std::string				m_name;
	volatile bool			m_bRunning;
	volatile bool			m_bCancelLoadVolume;
	int						m_updateId;

	ACE_Thread_Semaphore	m_semaphore;
	ACE_Thread_Mutex		m_queueLock;
	CallBackToOwner			m_notifyFunc;

	DiskReader				m_diskReader;
	DuLinkList				m_fileList;
	std::list<MessageInfo>		m_msgList;

	std::vector<FileInfo*>	m_selectedFiles;
	int						m_ShowedItemCount;

	std::string				m_selectCondition;
	std::string				m_newSelectCondition;

	ThreadWorker*			m_findSameFileThread;
};
