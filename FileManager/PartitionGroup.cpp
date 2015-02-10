// stdafx.cpp : source file that includes just the standard includes
// PartitionGroup.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file


#include "PartitionGroup.h"
#include <algorithm>

PartitionGroup::PartitionGroup(std::string name):
m_name(name),
m_bRunning(false),
m_semaphore(0),
m_queueLock(NULL),
m_updateId(0),
m_diskReader("", this)
{
	m_fileList.cleanUp();
	m_msgList.clear();

	m_selectedFiles.clear();
	m_ShowedItemCount = 0;

	m_selectCondition = SelectedConditionDefault;
	m_newSelectCondition = "";


	//m_findSameFileThread = new ThreadWorker(this);
}

PartitionGroup::~PartitionGroup()
{
}

void PartitionGroup::reSetData()
{
	static char* buf = "destroy loaded items.";
	m_notifyFunc(cmdShowStatusMessage.c_str(), buf);
	FileInfo* p = (FileInfo*)m_fileList.pop();
	while (p)
	{
		delete p;
		p = (FileInfo*)m_fileList.pop();
	}
	m_fileList.cleanUp();
	m_msgList.clear();
	
	++m_updateId;

}

void PartitionGroup::regisgerCallBack(CallBackToOwner notifyFunc)
{
	m_notifyFunc = notifyFunc;
}

void PartitionGroup::loadVolumeFiles(std::string volume)
{
	reSetData();
	m_bCancelLoadVolume = false;
	std::vector<std::string> ids;
	if (VolumeAllIDs == volume)
	{
		m_diskReader.loadAllVolumeIDs(ids);
	}
	else
	{
		ids.push_back(volume);
	}
	char buf[500];
	std::string showVol = "";

	for (int id = 0; id < ids.size() && false == m_bCancelLoadVolume; ++id)
	{
		std::string vol = ids[id];
		if (std::string::npos != vol.find("(") && std::string::npos != vol.find(")", vol.find("(")))
		{
			vol = vol.substr(vol.find("(") + 1);
			vol = vol.substr(0, vol.find(")"));
		}
		sprintf(buf, "start loading files from volume %s", vol.c_str());
		m_notifyFunc(cmdShowStatusMessage.c_str(), buf);

		updateLoadingRate(0, vol.c_str());
	
		DuLinkList files;
		m_diskReader.EnumUsnRecord(vol.c_str(), files);
		constructFileFullPath(files);

		FileInfo* p = (FileInfo*)files.pop();
		while (p)
		{
			p->NameUppered = p->Name.c_str();
			transform(p->NameUppered.begin(), p->NameUppered.end(), p->NameUppered.begin(), toupper);
			m_fileList.InsertItem(p);
			p = (FileInfo*)files.pop();
		}
		if (!showVol.empty())
		{
			showVol = showVol + ",";
		}
		showVol = showVol + vol;

	}

	if (m_bCancelLoadVolume)
	{
		reSetData();
		sprintf(buf, "cancel load volume %s", showVol.c_str());
		m_notifyFunc(cmdShowStatusMessage.c_str(), buf);
		updateLoadingRate(100);
		return;
	}
	updateLoadingRate(100);

	sprintf(buf, "finished to load %d files from volume %s", m_fileList.size(), showVol.c_str());
	m_notifyFunc(cmdShowStatusMessage.c_str(), buf);
}

void PartitionGroup::constructFileFullPath(DuLinkList& files)
{
	int nSize = files.size();
	std::map<DWORDLONG, FileInfo*>	fileMap;
	fileMap.clear();
	FileInfo* p = (FileInfo*)files.head();
	while (p)
	{
		fileMap[p->FileRefNo] = p;
		p = (FileInfo*)files.next(p);
	}

	FileInfo* stack[64];
	std::map<DWORDLONG, FileInfo*>::const_iterator it;
	int stackCount = 0;
	FileInfo* cur = 0;
	FileInfo* par = 0;

	p = (FileInfo*)files.head();
	while (p)
	{
		cur = p;
		p = (FileInfo*)files.next(p);
		if (cur->pathSetted)
		{
			continue;
		}
		stack[stackCount++] = cur;
		while (stackCount)
		{
			cur = stack[stackCount - 1];
			it = fileMap.find(cur->ParentRefNo);
			if (fileMap.end() == it)
			{
				cur->path = cur->path + "\\" + cur->Name;
				cur->pathSetted = true;
				--stackCount;
			}
			else
			{
				par = it->second;
				if (par->pathSetted)
				{
					cur->path = par->path + "\\" + cur->Name;
					cur->pathSetted = true;
					--stackCount;
				}
				else
				{
					stack[stackCount++] = par;
				}
			}
		}
	}
}

void PartitionGroup::showFileList(int fromIndex)
{
	char buf[1024] = {0};
	sprintf(buf, "tabName=%s%s", UiTabName_showAllFiles.c_str(), MessageSplitSign.c_str());
	if (0 == fromIndex)
	{
		m_notifyFunc(cmdFileListItemClear.c_str(), buf);
	}
	int i = 0, j = 0, nSize = m_selectedFiles.size();
	for (i = fromIndex, j = 0; i < nSize && j < SHOW_FILE_ITEM_NUM; ++i, ++j)
	{
		FileInfo& f = *m_selectedFiles[i];
		SYSTEMTIME cDate;
		FileTimeToSystemTime(&f.createDate, &cDate);
		SYSTEMTIME mDate;
		FileTimeToSystemTime(&f.modifyDate, &mDate);
		SYSTEMTIME lwDate;
		FileTimeToSystemTime(&f.lastAccessTime, &lwDate);
		memset(buf, 0, 1024);
		sprintf(buf,
			"tabName=%s,.., RowRefIndex=%I64u,.., ParentRefNum=%I64u,.., ParentRefNo=%I64u,.., name=%s,.., FileAttributes=%d,.., size=%s,.., createDate=%d-%02d-%02d %02d:%02d:%02d,.., modifyDate=%d-%02d-%02d %02d:%02d:%02d,.., lastAccessDate=%d-%02d-%02d %02d:%02d:%02d,.., path=%s,..,",
			UiTabName_showAllFiles.c_str(),
			f.FileRefNo, f.ParentRefNo, f.ParentRefNo, f.Name.c_str(), f.FileAttributes, f.fileSizeAsString(),
			cDate.wYear, cDate.wMonth, cDate.wDay, cDate.wHour, cDate.wMinute, cDate.wSecond,
			mDate.wYear, mDate.wMonth, mDate.wDay, mDate.wHour, mDate.wMinute, mDate.wSecond,
			lwDate.wYear, lwDate.wMonth, lwDate.wDay, lwDate.wHour, lwDate.wMinute, lwDate.wSecond,
			f.path.c_str()); 
		m_notifyFunc(cmdFileListItemAdd.c_str(), buf);
	}
	m_ShowedItemCount = i;
	memset(buf, 0, 1024);
	sprintf(buf, "tabName=%s,.., value=%d / %d", UiTabName_showAllFiles.c_str(), m_ShowedItemCount, nSize);
	m_notifyFunc(cmdUpdateShowedNums.c_str(), buf);
}


int PartitionGroup::getAllFiles(std::map<DWORDLONG, FileInfo*>& allFiles, int updateId)
{
	if (updateId == m_updateId)
	{
		return updateId;
	}
	FileInfo* p = (FileInfo*)m_fileList.head();
	while (p)
	{
		if (allFiles.end() != allFiles.find(p->FileRefNo))
		{
			int i = 0;
			int ast = 2 / i;
		}
		allFiles[p->FileRefNo] = p;
		p = (FileInfo*)m_fileList.next(p);
	}
	return m_updateId;
}

bool PartitionGroup::updateLoadingRate(int rate, const char* vol)
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
