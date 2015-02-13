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
m_updateId(100),
m_startUpdateId(100)
{
	m_nameToDiskMonitors.clear();
	m_fileOperationRecords.clear();
}

PartitionGroup::~PartitionGroup()
{
}

void PartitionGroup::addVolume(std::string name, DiskMonitor* dm)
{
	MutexGuard guard(m_lock);
	m_nameToDiskMonitors[name] = dm;	// if exist, just replace it.
	m_updateId += 100;

	m_fileOperationRecords.clear();
	m_startUpdateId = m_updateId;
}

int PartitionGroup::getUpdatedFiles(FilesMapType& allFiles, int updateId, std::vector<FileOperationRecord>& operations)
{
	MutexGuard guard(m_lock);

	// too much update or cleared record map.
	if (m_updateId - updateId >= 100 || updateId < m_startUpdateId)
	{
		allFiles.clear();
		std::map<std::string, DiskMonitor*>::iterator it = m_nameToDiskMonitors.begin();
		while (m_nameToDiskMonitors.end() != it)
		{
			it->second->getAllFiles(allFiles);
			++it;
		}
	}
	else	// just return new added operation files.
	{
		int i = updateId + 1;	//
		while (i <= m_updateId)
		{
			if (m_fileOperationRecords.end() != m_fileOperationRecords.find(i))
			{
				operations.push_back(m_fileOperationRecords[i]);
			}
			++i;
		}
	}
	return m_updateId;
}

bool PartitionGroup::volumeChange(FileOperation fo, std::string volume, FileInfo* fi)
{
	MutexGuard guard(m_lock);
	std::map<std::string, DiskMonitor*>::iterator it = m_nameToDiskMonitors.find(volume);
	if(m_nameToDiskMonitors.end() != it)
	{
		FileOperationRecord r;
		r.operation = fo;
		r.fi = fi;
		m_fileOperationRecords[++m_updateId] = r;
		return true;
	}
	return false;
}
