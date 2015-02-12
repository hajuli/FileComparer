// stdafx.cpp : source file that includes just the standard includes
// FileManager.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file


#include "fileInfo.h"

LONGLONG FileInfo::global_uuid = 10000;

FileInfo::FileInfo()
:fileSize(0),
pathSetted(false)
{
	uuid = InterlockedIncrement64(&global_uuid);
};
