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
m_updateId(0)
{
	m_nameToDiskMonitors.clear();
}

PartitionGroup::~PartitionGroup()
{
}

void PartitionGroup::addVolume(std::string name, DiskMonitor* dm)
{
	m_nameToDiskMonitors[name] = dm;	// if exist, just replace it.
	++m_updateId;
}

int PartitionGroup::getAllFiles(std::map<DWORDLONG, FileInfo*>& allFiles, int updateId)
{
	if (updateId == m_updateId)
	{
		return updateId;
	}
	//just samplely clear;
	allFiles.clear();

	std::map<std::string, DiskMonitor*>::iterator it = m_nameToDiskMonitors.begin();
	while (m_nameToDiskMonitors.end() != it)
	{
		it->second->getAllFiles(allFiles);
		++it;
	}
	return m_updateId;
}

bool PartitionGroup::volumeChange(FileOperation fo, std::string volume, FileInfo* fi)
{
	std::map<std::string, DiskMonitor*>::iterator it = m_nameToDiskMonitors.find(volume);
	if(m_nameToDiskMonitors.end() != it)
	{
		++m_updateId;
		return true;
	}
	return false;
}
