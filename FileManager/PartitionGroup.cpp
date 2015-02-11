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
m_updateId(0),
m_diskReader("", this)
{
	m_fileList.cleanUp();
}

PartitionGroup::~PartitionGroup()
{
}

void PartitionGroup::reSetData()
{
	char buf[500];
	sprintf(buf, "destroy loaded %d items.", m_fileList.size());
	m_notifyFunc(cmdShowStatusMessage.c_str(), buf);
	FileInfo* p = (FileInfo*)m_fileList.pop();
	while (p)
	{
		delete p;
		p = (FileInfo*)m_fileList.pop();
	}
	m_fileList.cleanUp();
	
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

	for (int id = 0; id < ids.size() && false == m_bCancelLoadVolume; ++id)
	{
		std::string vol = ids[id];
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
	
		DuLinkList files;
		m_diskReader.EnumUsnRecord(vol.c_str(), files);
		m_diskReader.startProcess();

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

int PartitionGroup::getAllFiles(std::map<DWORDLONG, FileInfo*>& allFiles, int updateId)
{
	if (updateId == m_updateId)
	{
		return updateId;
	}
	//just samplely clear;
	allFiles.clear();
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

bool PartitionGroup::notifyFilesChange(FileOperation, FileInfo*)
{
	return true;
}
