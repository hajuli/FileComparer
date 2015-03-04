// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#include "superList.h"
#include <string>
#include <map>


class FileInfo : public IListItem
{
public:
	LONGLONG	uuid;	// using uuid, 不同磁盘分区的FileRefNo可能一样， 经过验证， 很多是一样的。 所以使用uuid来区分。
    DWORDLONG	FileRefNo;
    DWORDLONG	ParentRefNo;
    DWORD		FileAttributes;
	FILETIME	createDate;
	FILETIME	modifyDate;
	FILETIME	lastAccessTime;
	unsigned int	fileSize;
	std::string Name;
	std::string NameUppered;
	std::string fullPath;
	bool		pathSetted;
	
	DuLinkList	children;

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
	void updateChildFullPath()
	{
		FileInfo* p = (FileInfo*)children.head();
		while (p)
		{
			p->fullPath = fullPath + "\\" + p->Name;
			if (0 < p->children.size())
			{
				p->updateChildFullPath();
			}
			p = (FileInfo*)children.next(p);
		}
	}
private:
	static LONGLONG global_uuid;
};

typedef std::map<LONGLONG, FileInfo*> FilesMapType;	//using uuid or FileRefNo as key. 
