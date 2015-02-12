// stdafx.cpp : source file that includes just the standard includes
// FileManager.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

#include "DiskMonitor.h"
#include<iostream>
#include<io.h>
#include <winioctl.h>
#include <deque>

#include "PartitionGroup.h"

using namespace std;

bool File_Info_by_NTFS (HANDLE hVol, DWORDLONG FileReferenceNumber,VolNTFSInfoNode volInfo, FileInfo& finfo);

//Converting a WChar string to a Ansi string
std::string WChar2Ansi(LPCWSTR pwszSrc)
{
	int nLen = WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, NULL, 0, NULL, NULL);

	if (nLen<= 0) return std::string("");

	char* pszDst = new char[nLen];
	if (NULL == pszDst) return std::string("");

	WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, pszDst, nLen, NULL, NULL);
	pszDst[nLen -1] = 0;

	std::string strTemp(pszDst);
	delete [] pszDst;

	return strTemp;
}

DiskMonitor::DiskMonitor(std::string volume, IDiskMonitorEvent* eventHandler):
m_diskName(volume),
m_startUsn(0),
m_eventHandler(eventHandler)
{
	m_allFiles.cleanUp();
}

DiskMonitor::~DiskMonitor()
{
	FileInfo* p = (FileInfo*)m_allFiles.pop();
	while (p)
	{
		delete p;
		p = (FileInfo*)m_allFiles.pop();
	}
	m_allFiles.cleanUp();
}

bool DiskMonitor::loadAllFiles()
{
	bool enumed = EnumUsnRecord(m_diskName.c_str(), m_allFiles);
	if (enumed)
	{
		constructFileFullPath(m_allFiles);
	}
	return enumed;
}

void DiskMonitor::constructFileFullPath(DuLinkList& files)
{
	m_allFilesMap.clear();
	FileInfo* p = (FileInfo*)files.head();
	while (p)
	{
		if (m_allFilesMap.end() != m_allFilesMap.find(p->FileRefNo))
		{
			FileInfo* tmp = m_allFilesMap[p->FileRefNo];
			myAssertFail();
			std::string name = tmp->Name;
		}
		m_allFilesMap[p->FileRefNo] = p;
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
			it = m_allFilesMap.find(cur->ParentRefNo);
			if (m_allFilesMap.end() == it)
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

void DiskMonitor::getAllFiles(std::map<DWORDLONG, FileInfo*>& allFiles)
{
	FileInfo* p = (FileInfo*)m_allFiles.head();
	while (p)
	{
		if (allFiles.end() != allFiles.find(p->FileRefNo))
		{
			//myAssertFail();	//no assert, different volume have same ref no.
		}
		allFiles[p->FileRefNo] = p;	//need update.
		p = (FileInfo*)m_allFiles.next(p);
	}
}

void printUsnRecord(PUSN_RECORD& usnRecord)
{
	printf( "USN: %I64x\n", usnRecord->Usn );
	printf( "FileReferenceNumber: %I64x\n", usnRecord->FileReferenceNumber);
	printf( "ParentFileReferenceNumber: %I64x\n", usnRecord->ParentFileReferenceNumber);
	printf("File name: %.*S\n", usnRecord->FileNameLength/2, usnRecord->FileName );
	printf( "Reason: %x\n", usnRecord->Reason );
	printf( "TimeStamp: %lld\n", usnRecord->TimeStamp.QuadPart / 10 / 1000 / 1000 ); // 100 ns
	printf( "\n" );
}

void setUsnRecord(PUSN_RECORD& usnRecord, FileInfo& fInfo)
{
	fInfo.FileRefNo = usnRecord->FileReferenceNumber;
    fInfo.ParentRefNo = usnRecord->ParentFileReferenceNumber;
    fInfo.FileAttributes = usnRecord->FileAttributes;
	WCHAR fileName[MAX_PATH] = {0};
    memcpy( fileName, usnRecord->FileName, usnRecord->FileNameLength );
    fileName[usnRecord->FileNameLength/2] = L'';
	fInfo.Name = WChar2Ansi(fileName);
	
	fInfo.NameUppered = fInfo.Name.c_str();
	transform(fInfo.NameUppered.begin(), fInfo.NameUppered.end(), fInfo.NameUppered.begin(), toupper);
}

#define BUF_LEN 4096
int DiskMonitor::svc()
{
	while (m_bRunning)
	{
		if(false == m_bRunning)
		{
			break;
		}
		//////=======================
		HANDLE hVol;
		CHAR Buffer[BUF_LEN];

		READ_USN_JOURNAL_DATA ReadData = {0, 0xFFFFFFFF, 1, 0, 0};
		PUSN_RECORD UsnRecord;  

		DWORD dwBytes;
		DWORD dwRetBytes;

		hVol = CreateFile((std::string("\\\\.\\") + m_diskName).c_str(),
			GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

		if( hVol == INVALID_HANDLE_VALUE )
		{
			printf("CreateFile failed (%d)\n", GetLastError());
			return 0;
		}

		ReadData.StartUsn = m_startUsn;
		ReadData.UsnJournalID = m_UsnJournalID;

		DWORD ReasonMask = 0;
		ReadData.ReasonMask = USN_REASON_FILE_CREATE | USN_REASON_FILE_DELETE | USN_REASON_RENAME_OLD_NAME | USN_REASON_RENAME_NEW_NAME;
		ReadData.Timeout = 5;
		ReadData.BytesToWaitFor = BUF_LEN;

		printf( "Journal ID: %I64x\n", ReadData.UsnJournalID );
		printf( "StartUsn: %I64x\n\n", ReadData.StartUsn );

		for(int I=0; I<=1000; I++)
		{
			memset( Buffer, 0, BUF_LEN );

			if( !DeviceIoControl( hVol, FSCTL_READ_USN_JOURNAL, &ReadData, sizeof(ReadData), &Buffer, BUF_LEN, &dwBytes, NULL) )
			{
				printf( "Read journal failed (%d)\n", GetLastError());
				return 0;
			}

			dwRetBytes = dwBytes - sizeof(USN);
			UsnRecord = (PUSN_RECORD)(((PUCHAR)Buffer) + sizeof(USN));  

			printf( "****************************************\n");
			while( dwRetBytes > 0 )
			{
				static int nfiles = 0;
				++nfiles;
				printf( "changed times %d\n", nfiles);
				printUsnRecord(UsnRecord);
				
				FileInfo* fi = new FileInfo();
				setUsnRecord(UsnRecord, *fi);
				if (m_allFilesMap.end() == m_allFilesMap.find(fi->ParentRefNo))
				{
					//root dir
					//myAssertFail();
					fi->path = m_diskName + "\\" + fi->Name;
				}
				else
				{
					fi->path = m_allFilesMap[fi->ParentRefNo]->path + "\\" + fi->Name;
				}
				File_Info_by_NTFS(hVol, fi->FileRefNo, m_volInfo, *fi);


				if (USN_REASON_FILE_CREATE & UsnRecord->Reason)
				{
					m_allFiles.InsertItem( fi );
					m_allFilesMap[fi->FileRefNo] = fi;
					m_eventHandler->notifyFilesChange(File_Create, m_diskName, fi);
				}
				else if (USN_REASON_FILE_DELETE & UsnRecord->Reason)
				{
					FileInfo* oldFile = m_allFilesMap[fi->FileRefNo];	// maybe judge first.
					m_allFiles.removeItem(oldFile);		//no delete,,just keep data.
					m_eventHandler->notifyFilesChange(File_Delete, m_diskName, fi);
				}
				else if (USN_REASON_RENAME_NEW_NAME & UsnRecord->Reason)
				{
					FileInfo* oldFile = m_allFilesMap[fi->FileRefNo];	// maybe judge first.
					m_allFiles.removeItem(oldFile);	//delete old file info. insert new. save new.
					delete oldFile;
					m_allFiles.InsertItem( fi );
					m_allFilesMap[fi->FileRefNo] = fi;
					m_eventHandler->notifyFilesChange(File_Rename, m_diskName, fi);
				}
				else 
				{
					myAssertFail();
					delete fi;
				}

				dwRetBytes -= UsnRecord->RecordLength;

				// Find the next record
				UsnRecord = (PUSN_RECORD)(((PCHAR)UsnRecord) + 
					UsnRecord->RecordLength); 
			}
			// Update starting USN for next call
			ReadData.StartUsn = *(USN *)&Buffer; 
		}
		/////////////======================
	}
	return 0;
}

void DiskMonitor::loadAllVolumeIDs(std::vector<std::string>& ids)
{
	WCHAR tDrivers[26*4+1] = {};
	GetLogicalDriveStringsW(26*4+1, tDrivers);
	WCHAR fileSysBuf[8];
	DWORD dwDri; //0~25

	WCHAR szRootName[40];
	WCHAR szVolumeName[32];
	int iFilterRoot=0;
	for(WCHAR *p = tDrivers; *p != '\0'; p += 4)
	{	   
		if(*p >= L'a') *p -= 32;//
		dwDri = *p - L'A';
		if(DRIVE_FIXED == GetDriveTypeW(p) || DRIVE_REMOVABLE == GetDriveTypeW(p))
		{    
			DWORD dwMaxComLen, dwFileSysFlag;
			GetVolumeInformationW(p, szVolumeName, 32, NULL, &dwMaxComLen, &dwFileSysFlag, fileSysBuf, 8);
			if(fileSysBuf[0] == L'N' && fileSysBuf[1] == L'T' && fileSysBuf[2] == L'F' && fileSysBuf[3] == L'S')
			{
				swprintf(szRootName, L"(%c:) %s", *p, szVolumeName);
				ids.push_back(WChar2Ansi(szRootName));

				WCHAR szVolumePath[10];
				swprintf(szVolumePath, L"\\\\.\\%c:", *p);
				wcout<<szVolumePath<<endl;
			}
		}
	}
}

// drvname with ':' ending, eg: "E:"
bool DiskMonitor::EnumUsnRecord( const char* drvname, DuLinkList & fileList)
{
    bool ret = false;
	int counts = 0;

    char FileSystemName[MAX_PATH+1];
    DWORD MaximumComponentLength;
    if( GetVolumeInformationA( (std::string(drvname)+"\\").c_str(),0,0,0,&MaximumComponentLength,0,FileSystemName,MAX_PATH+1)
        && 0==strcmp(FileSystemName,"NTFS") ) // 判断是否为 NTFS 格式
    {
        HANDLE hVol = CreateFileA( (std::string("\\\\.\\")+drvname).c_str() // 需要管理员权限，无奈
            , GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if( hVol != INVALID_HANDLE_VALUE )
        {

			int usnCount = 0;
			GetVolumeDetail(hVol, usnCount);

            DWORD br;
            CREATE_USN_JOURNAL_DATA cujd = { 0, 0 };
            //jun if( DeviceIoControl( hVol, FSCTL_CREATE_USN_JOURNAL, &cujd, sizeof(cujd), NULL, 0, &br, NULL ) ) // 如果创建过，且没有用FSCTL_DELETE_USN_JOURNAL关闭，则可以跳过这一步
            {
                USN_JOURNAL_DATA qujd;
                if( DeviceIoControl( hVol, FSCTL_QUERY_USN_JOURNAL, NULL, 0, &qujd, sizeof(qujd), &br, NULL ) )
                {
                    char buffer[0x1000]; // 缓冲区越大则DeviceIoControl调用次数越少，即效率越高
                    DWORD BytesReturned;
                    //{ // 使用FSCTL_READ_USN_JOURNAL可以只搜索指定change reason的记录，比如下面的代码只搜索被删除的文件信息，但即便rujd.ReasonMask设为-1，也列不出所有文件
                    //    READ_USN_JOURNAL_DATA rujd = { 0, USN_REASON_FILE_DELETE, 0, 0, 0, qujd.UsnJournalID };
                    //    for( ; DeviceIoControl(hVol,FSCTL_READ_USN_JOURNAL,&rujd,sizeof(rujd),buffer,_countof(buffer),&BytesReturned,NULL); rujd.StartUsn=*(USN*)&buffer )
                    //    {
                    //        DWORD dwRetBytes = BytesReturned - sizeof(USN);
                    //        PUSN_RECORD UsnRecord = (PUSN_RECORD)((PCHAR)buffer+sizeof(USN));
                    //        if( dwRetBytes==0 )
                    //        {
                    //            ret = true;
                    //            break;
                    //        }
                    //
                    //        while( dwRetBytes > 0 )
                    //        {
                    //            printf( "FRU %016I64x, PRU %016I64x, %.*S", UsnRecord->FileReferenceNumber, UsnRecord->ParentFileReferenceNumber
                    //                , UsnRecord->FileNameLength/2, UsnRecord->FileName );
                    //
                    //            dwRetBytes -= UsnRecord->RecordLength;
                    //            UsnRecord = (PUSN_RECORD)( (PCHAR)UsnRecord + UsnRecord->RecordLength );
                    //        }
                    //    }
                    //}
                    { // 使用FSCTL_ENUM_USN_DATA可以列出所有存在的文件信息，但UsnRecord->Reason等信息是无效的
                        MFT_ENUM_DATA med = { 0, 0, qujd.NextUsn -1 }; // jun add, maybe NextUsn -1;
						m_startUsn = qujd.NextUsn;
						m_UsnJournalID = qujd.UsnJournalID;
                        for( ; DeviceIoControl(hVol,FSCTL_ENUM_USN_DATA,&med,sizeof(med),buffer,_countof(buffer),&BytesReturned,NULL); med.StartFileReferenceNumber=*(USN*)&buffer )
                        {
                            DWORD dwRetBytes = BytesReturned - sizeof(USN);
                            PUSN_RECORD UsnRecord = (PUSN_RECORD)((PCHAR)buffer+sizeof(USN));

                            while( dwRetBytes > 0 )
                            {
								FileInfo* fi = new FileInfo();
								setUsnRecord(UsnRecord, *fi);				
								fi->path = drvname;
								File_Info_by_NTFS(hVol, fi->FileRefNo, m_volInfo, *fi);
                                fileList.InsertItem( fi );
                                dwRetBytes -= UsnRecord->RecordLength;
                                UsnRecord = (PUSN_RECORD)( (PCHAR)UsnRecord + UsnRecord->RecordLength );

								counts++;
								if (0 == (counts % 500) && m_eventHandler)
								{
									if (false == m_eventHandler->updateLoadingRate(counts * 100 / usnCount))
									{
										return false; // canceled or error.
									}
								}
                            }
                        }
                        ret = GetLastError()==ERROR_HANDLE_EOF;
                    }
                    DELETE_USN_JOURNAL_DATA dujd = { qujd.UsnJournalID, USN_DELETE_FLAG_DELETE };
                    //jun DeviceIoControl( hVol, FSCTL_DELETE_USN_JOURNAL, &dujd, sizeof(dujd), NULL, 0, &br, NULL ); // 关闭USN记录。如果是别人的电脑，当然可以不关^_^
                }
            }
        }
        CloseHandle( hVol );
    }
	printf("load from %s , enum usn counts:%d\n", drvname, counts);
    return ret;
}

bool DiskMonitor::GetVolumeDetail( HANDLE hVol, int& usnCount)
{
    bool ret = false;
    DWORD BytesReturned;
    NTFS_VOLUME_DATA_BUFFER nvdb;
    if( DeviceIoControl( hVol, FSCTL_GET_NTFS_VOLUME_DATA, NULL, 0
        , &nvdb, sizeof(nvdb), &BytesReturned, NULL ) ) // 仅是事例，没有作优化 1.作为递归调用，这一步应当提取出来 2.如果多次调用，DirectoryFileReferenceNumber没必要被重复获取
    {
		m_volInfo.BytesPerFileRecordBuf = sizeof(NTFS_FILE_RECORD_OUTPUT_BUFFER) + nvdb.BytesPerFileRecordSegment - 1;
		m_volInfo.BytesPerCluster = nvdb.BytesPerCluster;
		
		usnCount = nvdb.MftValidDataLength.QuadPart / nvdb.BytesPerFileRecordSegment;
    }
	else
	{
		m_volInfo.BytesPerFileRecordBuf = 1024;
        m_volInfo.BytesPerCluster = 4096;
		usnCount = nvdb.MftValidDataLength.QuadPart / m_volInfo.BytesPerFileRecordBuf;
	}
    return ret;
}

__forceinline ULONG RunLength (PUCHAR run)
    {
        return (*run & 0x0f) + ( (*run >> 4) & 0x0f) + 1;
    }

    __forceinline LONGLONG RunLCN (PUCHAR run)
    {
        UCHAR n1 = *run & 0x0f;
        UCHAR n2 = (*run >> 4) & 0x0f;

        if (0 == n2) return 0;

        LONGLONG lcn = (CHAR) (run[n1 + n2]);
        LONG i = 0;

        for (i = n1 + n2 - 1; i > n1; --i)
            lcn = (lcn << 8) + run[i];

        return lcn;
    }

    __forceinline ULONGLONG RunCount (PUCHAR run)
    {
        UCHAR n =  *run & 0xf;
        ULONGLONG count = 0;
        ULONG i = 0;

        for (i = n; i > 0; i--)
            count = (count << 8) + run[i];

        return count;
    }

	
// ref to: https://code.google.com/p/bingo-file-search/source/browse/trunk/Bingo/core/NTFS.cpp?r=89
// and https://code.google.com/p/bingo-file-search/source/browse/trunk/Bingo/core/NTFS.h?r=89

bool File_Info_by_NTFS (HANDLE hVol, DWORDLONG FileReferenceNumber,VolNTFSInfoNode volInfo, FileInfo& finfo)
{
	DWORD BytesReturned;
    
    NTFS_FILE_RECORD_INPUT_BUFFER nfrib;
    nfrib.FileReferenceNumber.QuadPart = FileReferenceNumber;
    size_t len = volInfo.BytesPerFileRecordBuf;
    static NTFS_FILE_RECORD_OUTPUT_BUFFER* nfrob = (PNTFS_FILE_RECORD_OUTPUT_BUFFER)operator new(len);

    if (DeviceIoControl (hVol, FSCTL_GET_NTFS_FILE_RECORD, &nfrib, sizeof (NTFS_FILE_RECORD_INPUT_BUFFER),
                         nfrob, len , &BytesReturned, NULL) == 0)
    {
        return false;
    }

    PFILE_RECORD_HEADER pfrh = (PFILE_RECORD_HEADER) ( (NTFS_FILE_RECORD_OUTPUT_BUFFER*) nfrob)->FileRecordBuffer;
    PATTRIBUTE pa = (PATTRIBUTE) ( (char*) pfrh + pfrh->AttributesOffset);

    while (pa->AttributeType != AttributeStandardInformation && pa->AttributeType != AttributeEnd)
        pa = (PATTRIBUTE) ( (char*) pa + pa->Length);

    if (pa->AttributeType == AttributeEnd)
    {
        //delete  (nfrob);
        return false;
    }

    PSTANDARD_INFORMATION psi = PSTANDARD_INFORMATION ( (char*) pa + PRESIDENT_ATTRIBUTE (pa)->ValueOffset);

	FILETIME tmpT;
    memcpy (&tmpT, &psi->CreationTime, sizeof (ULONGLONG));
	FileTimeToLocalFileTime(&tmpT, &finfo.createDate);

	memcpy (&tmpT, &psi->ChangeTime, sizeof (ULONGLONG));
	FileTimeToLocalFileTime(&tmpT, &finfo.modifyDate);

	memcpy (&tmpT, &psi->LastAccessTime, sizeof (ULONGLONG));
	FileTimeToLocalFileTime(&tmpT, &finfo.lastAccessTime);

	unsigned int* pOutSize = &finfo.fileSize;

    while (pa->AttributeType != AttributeEnd && pa->AttributeType < AttributeData)
        pa = (PATTRIBUTE) ( (char*) pa + pa->Length);

	if (pa->AttributeType == AttributeEnd || pa->AttributeType > AttributeData)
    {
        // rescan to locate AttributeAttributeList
        for (pa = (PATTRIBUTE) ( (char*) pfrh + pfrh->AttributesOffset)
                  ; pa->AttributeType < AttributeAttributeList
                ; pa = (PATTRIBUTE) ( (char*) pa + pa->Length)
            );

        if (pa->AttributeType > AttributeAttributeList)
        {
            *pOutSize = 0;
            //delete  (nfrob);
            return true;
        }

		typedef struct
		{
			ATTRIBUTE_TYPE AttributeType;
			USHORT Length;
			UCHAR NameLength;
			UCHAR NameOffset;
			ULONGLONG LowVcn;
			ULONGLONG FileReferenceNumber;
			USHORT AttributeNumber;
			USHORT AlignmentOrReserved[3];
		} ATTRIBUTE_LIST, *PATTRIBUTE_LIST;

        if (pa->Nonresident)
        {
            PATTRIBUTE_LIST pal;
            PNONRESIDENT_ATTRIBUTE pna = PNONRESIDENT_ATTRIBUTE (pa);
            PBYTE pRun = (PBYTE) pa + pna->RunArrayOffset;
            ULONGLONG Lcn = RunLCN (pRun);
            ULONGLONG nCount = RunCount (pRun);
            assert (nCount <= pna->HighVcn - pna->LowVcn + 1);
            LARGE_INTEGER file_offset;
            file_offset.QuadPart = Lcn * volInfo.BytesPerCluster;
            SetFilePointerEx (hVol, file_offset, NULL, FILE_BEGIN);
            PBYTE   pBuffferRead = (PBYTE)operator new(volInfo.BytesPerCluster);
            DWORD   dwRead = 0;
            ReadFile (hVol, pBuffferRead, volInfo.BytesPerCluster, &dwRead, NULL);
            PBYTE   pBufferEnd = pBuffferRead + dwRead;

			int intimes = 0;
			PATTRIBUTE_LIST oldPal = 0;
            for (pal = PATTRIBUTE_LIST (pBuffferRead);
                    pal->AttributeType != AttributeData;
                    pal = PATTRIBUTE_LIST (PBYTE (pal) + pal->Length)
                )
			{
				++intimes;
				if (oldPal == pal)
				{
					break;
				}
				oldPal = pal;
			}

            if (pal->AttributeType == AttributeData)
            {
                nfrib.FileReferenceNumber.QuadPart = 0xffffffffffff & pal->FileReferenceNumber;

                if (DeviceIoControl (hVol, FSCTL_GET_NTFS_FILE_RECORD , &nfrib, sizeof (NTFS_FILE_RECORD_INPUT_BUFFER),
                                     nfrob, volInfo.BytesPerFileRecordBuf, &BytesReturned, NULL) == 0)
                {
                    *pOutSize = 0;
                    //delete (nfrob);
                    delete (pBuffferRead);
                    return true;
                }

                pfrh = (PFILE_RECORD_HEADER) ( (NTFS_FILE_RECORD_OUTPUT_BUFFER*) nfrob)->FileRecordBuffer;
                pa = (PATTRIBUTE) ( (char*) pfrh + pfrh->AttributesOffset);

                while (pa->AttributeType != AttributeData)
                    pa = (PATTRIBUTE) ( (char*) pa + pa->Length);

                if (pa->Nonresident)  *pOutSize = PNONRESIDENT_ATTRIBUTE (pa)->DataSize;
                else *pOutSize = PRESIDENT_ATTRIBUTE (pa)->ValueLength;
            }
            else
            {
                *pOutSize = 0;
                //delete (nfrob);
                delete (pBuffferRead);
                return true;
            }

            delete (pBuffferRead);
        }
        else
        {
            PATTRIBUTE_LIST pal;

            for (pal = PATTRIBUTE_LIST ( (PBYTE) pa + PRESIDENT_ATTRIBUTE (pa)->ValueOffset);
                    pal->AttributeType != AttributeEnd && pal->AttributeType < AttributeData;
                    pal = PATTRIBUTE_LIST (PBYTE (pal) + pal->Length)
                );

            if (pal->AttributeType == AttributeData)
            {
                nfrib.FileReferenceNumber.QuadPart = 0xffffffffffff & pal->FileReferenceNumber;

                if (DeviceIoControl (hVol, FSCTL_GET_NTFS_FILE_RECORD , &nfrib, sizeof (NTFS_FILE_RECORD_INPUT_BUFFER),
                                     nfrob, volInfo.BytesPerFileRecordBuf, &BytesReturned, NULL) == 0)
                {
                    *pOutSize = 0;
                    //delete (nfrob);
                    return true;
                }

                pfrh = (PFILE_RECORD_HEADER) ( (NTFS_FILE_RECORD_OUTPUT_BUFFER*) nfrob)->FileRecordBuffer;
                pa = (PATTRIBUTE) ( (char*) pfrh + pfrh->AttributesOffset);

                while (pa->AttributeType != AttributeData)
                    pa = (PATTRIBUTE) ( (char*) pa + pa->Length);

                if (pa->Nonresident) *pOutSize = PNONRESIDENT_ATTRIBUTE (pa)->DataSize;
                else *pOutSize = PRESIDENT_ATTRIBUTE (pa)->ValueLength;
            }
            else
            {
                *pOutSize = 0;
                //delete (nfrob);
                return true;
            }
        }
    }
    else
    {
        if (pa->Nonresident) *pOutSize = PNONRESIDENT_ATTRIBUTE (pa)->DataSize;
        else *pOutSize = PRESIDENT_ATTRIBUTE (pa)->ValueLength;
    }

    //delete (nfrob);
    return true;
}
