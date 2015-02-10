// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#include "dllInterface.h"

class FolderReader
{
public:
	FolderReader(std::string path);
	~FolderReader();

	void getFiles(std::string path, bool withSub = false);

private:
	std::string m_folderName;
};
