// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#include "superList.h"
#include <string>


class FileInfo : public IListItem
{
public:
    DWORDLONG	FileRefNo;
    DWORDLONG	ParentRefNo;
    DWORD		FileAttributes;
	FILETIME	createDate;
	FILETIME	modifyDate;
	FILETIME	lastAccessTime;
	unsigned int	fileSize;
	std::string Name;
	std::string NameUppered;
	std::string path;
	bool		pathSetted;

	FileInfo();
	
	char* fileSizeAsString()
	{
		static char sizeBuf[32];
		static char tBuf[32];
		memset(sizeBuf, 0, 32);
		memset(tBuf, 0, 32);
		int i = 0, j = 0;
		unsigned int fSize = fileSize;
		do
		{
			if (0 == (i + 1) % 4)
			{
				tBuf[i++] = ',';
			}
			tBuf[i++] = fSize % 10 + '0';
			fSize = fSize / 10;
		}
		while (fSize);
		while (i)
		{
			sizeBuf[j++] = tBuf[--i];
		}
		sizeBuf[j] = ' ';	// add more space for good look.
		return sizeBuf;
	}
};