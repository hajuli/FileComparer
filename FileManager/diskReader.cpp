// stdafx.cpp : source file that includes just the standard includes
// FileManager.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

#include "DiskReader.h"
#include<iostream>
#include<io.h>
#include <winioctl.h>
#include <deque>

#include "PartitionGroup.h"

using namespace std;

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

DiskReader::DiskReader(std::string path, IDiskReadEvent* eventHandler):
m_diskName(path),
m_eventHandler(eventHandler)
{
}


DiskReader::~DiskReader()
{
}

void DiskReader::loadAllVolumeIDs(std::vector<std::string>& ids)
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
// take a look: http://zuoqiang.iteye.com/blog/1181641

// from: http://msdn.microsoft.com/en-us/library/windows/desktop/aa365736(v=vs.85).aspx

#define BUF_LEN 4096

void DiskReader::getFiles()
{
	//EnumUsnRecord("E");
	HANDLE hVol;
	CHAR Buffer[BUF_LEN];

	USN_JOURNAL_DATA JournalData;
	READ_USN_JOURNAL_DATA ReadData = {0, 0xFFFFFFFF, FALSE, 0, 0};
	PUSN_RECORD UsnRecord;  

	DWORD dwBytes;
	DWORD dwRetBytes;
	int I;

	hVol = CreateFile( TEXT("\\\\.\\E:"), 
		GENERIC_READ | GENERIC_WRITE, 
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if( hVol == INVALID_HANDLE_VALUE )
	{
		printf("CreateFile failed (%d)\n", GetLastError());
		return;
	}

	if( !DeviceIoControl( hVol, 
		FSCTL_QUERY_USN_JOURNAL, 
		NULL,
		0,
		&JournalData,
		sizeof(JournalData),
		&dwBytes,
		NULL) )
	{
		printf( "Query journal failed (%d)\n", GetLastError());
		return;
	}

	ReadData.UsnJournalID = JournalData.UsnJournalID;

	printf( "Journal ID: %I64x\n", JournalData.UsnJournalID );
	printf( "FirstUsn: %I64x\n\n", JournalData.FirstUsn );

	for(I=0; I<=10; I++)
	{
		memset( Buffer, 0, BUF_LEN );

		if( !DeviceIoControl( hVol, 
			FSCTL_READ_USN_JOURNAL, 
			&ReadData,
			sizeof(ReadData),
			&Buffer,
			BUF_LEN,
			&dwBytes,
			NULL) )
		{
			printf( "Read journal failed (%d)\n", GetLastError());
			return;
		}

		dwRetBytes = dwBytes - sizeof(USN);

		// Find the first record
		UsnRecord = (PUSN_RECORD)(((PUCHAR)Buffer) + sizeof(USN));  

		printf( "****************************************\n");

		// This loop could go on for a long time, given the current buffer size.
		while( dwRetBytes > 0 )
		{
			printf( "USN: %I64x\n", UsnRecord->Usn );
			printf("File name: %.*S\n", 
				UsnRecord->FileNameLength/2, 
				UsnRecord->FileName );
			printf( "Reason: %x\n", UsnRecord->Reason );
			printf( "Reason: %d\n", UsnRecord->TimeStamp.QuadPart / 1000 / 1000 / 1000 );
			printf( "\n" );

			dwRetBytes -= UsnRecord->RecordLength;

			// Find the next record
			UsnRecord = (PUSN_RECORD)(((PCHAR)UsnRecord) + 
				UsnRecord->RecordLength); 
		}
		// Update starting USN for next call
		ReadData.StartUsn = *(USN *)&Buffer; 
	}

	CloseHandle(hVol);
}
bool File_Info_by_NTFS (HANDLE hVol, DWORDLONG FileReferenceNumber,VolNTFSInfoNode volInfo, FileInfo& finfo);
//bool EnumUsnRecord( const char* drvname, std::deque<FileInfo>& con )
// drvname with ':' ending, eg: "E:"
bool DiskReader::EnumUsnRecord( const char* drvname, DuLinkList & fileList)
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
                        MFT_ENUM_DATA med = { 0, 0, qujd.NextUsn };
                        for( ; DeviceIoControl(hVol,FSCTL_ENUM_USN_DATA,&med,sizeof(med),buffer,_countof(buffer),&BytesReturned,NULL); med.StartFileReferenceNumber=*(USN*)&buffer )
                        {
                            DWORD dwRetBytes = BytesReturned - sizeof(USN);
                            PUSN_RECORD UsnRecord = (PUSN_RECORD)((PCHAR)buffer+sizeof(USN));

                            while( dwRetBytes > 0 )
                            {
								FileInfo* fi = new FileInfo();
                                FileInfo& finf = *fi;
                                finf.FileRefNo = UsnRecord->FileReferenceNumber;
                                finf.ParentRefNo = UsnRecord->ParentFileReferenceNumber;
                                finf.FileAttributes = UsnRecord->FileAttributes;
								WCHAR fileName[MAX_PATH];
                                memcpy( fileName, UsnRecord->FileName, UsnRecord->FileNameLength );
                                fileName[UsnRecord->FileNameLength/2] = L'';
								finf.Name = WChar2Ansi(fileName);
								finf.path = drvname;
								//GetFileDetail(hVol, finf.FileRefNo, finf);
								File_Info_by_NTFS(hVol, finf.FileRefNo, m_volInfo, finf);
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
	printf("counts:%d\n", counts);
    return ret;
}

bool DiskReader::GetVolumeDetail( HANDLE hVol, int& usnCount)
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

void GetFullPathByFileReferenceNumber( HANDLE hVol, DWORDLONG FileReferenceNumber );
bool DiskReader::GetFileDetail(HANDLE hVol, DWORDLONG FileReferenceNumber, FileInfo& finfo)
{
    if( (FileReferenceNumber&0x0000FFFFFFFFFFFF) == 5 )
        return true;
	//GetFullPathByFileReferenceNumber(hVol, FileReferenceNumber);
    bool ret = false;
    DWORD BytesReturned;
    
    NTFS_FILE_RECORD_INPUT_BUFFER nfrib;
    nfrib.FileReferenceNumber.QuadPart = FileReferenceNumber;
    size_t len = m_volInfo.BytesPerFileRecordBuf;
    NTFS_FILE_RECORD_OUTPUT_BUFFER* nfrob = (PNTFS_FILE_RECORD_OUTPUT_BUFFER)operator new(len);
    
	if( DeviceIoControl( hVol, FSCTL_GET_NTFS_FILE_RECORD, &nfrib, sizeof(nfrib)
        , nfrob, len, &BytesReturned, NULL ) )
    {
        if( (nfrib.FileReferenceNumber.QuadPart&0x0000FFFFFFFFFFFF) == nfrob->FileReferenceNumber.QuadPart ) // a 48-bit index and a 16-bit sequence number
        {
            PFILE_RECORD_HEADER frh = (PFILE_RECORD_HEADER)nfrob->FileRecordBuffer;
            for( PATTRIBUTE attr=(PATTRIBUTE)((LPBYTE)frh+frh->AttributesOffset); attr->AttributeType!=-1; attr=(PATTRIBUTE)((LPBYTE)attr+attr->Length) )
            {
                if( attr->AttributeType == AttributeFileName )
                {
                    PFILENAME_ATTRIBUTE fna = (PFILENAME_ATTRIBUTE)( (LPBYTE)attr + PRESIDENT_ATTRIBUTE(attr)->ValueOffset );
                    if( (fna->NameType&1) == 1 ) // long name
                    {
                        //if( GetFullPathByFileReferenceNumber( hVol, name->DirectoryFileReferenceNumber ) )
                        {
                            //printf( "\%.*S\n", name->NameLength, name->Name );
							finfo.createDate = *(FILETIME*)&fna->CreationTime;
							finfo.modifyDate = *(FILETIME*)&fna->ChangeTime;
							finfo.fileSize = fna->DataSize;
                            ret = true;
                        }
                    }
                }
				else if( attr->AttributeType == AttributeStandardInformation )
				{
					PSTANDARD_INFORMATION si = (PSTANDARD_INFORMATION)( (LPBYTE)attr + PRESIDENT_ATTRIBUTE(attr)->ValueOffset );
					ULONG a = si->FileAttributes;
				}
            }
        }
    }
    operator delete( nfrob );

    return ret;
}

//#include <ntstatus.h>
// 有点慢， 不用。
void GetFullPathByFileReferenceNumber( HANDLE hVol, DWORDLONG FileReferenceNumber )
{
    typedef ULONG (__stdcall *PNtCreateFile)(
        PHANDLE FileHandle,
        ULONG DesiredAccess,
        PVOID ObjectAttributes,
        PVOID IoStatusBlock,
        PLARGE_INTEGER AllocationSize,
        ULONG FileAttributes,
        ULONG ShareAccess,
        ULONG CreateDisposition,
        ULONG CreateOptions,
        PVOID EaBuffer,
        ULONG EaLength );
    static PNtCreateFile NtCreatefile = (PNtCreateFile)GetProcAddress( GetModuleHandleW(L"ntdll.dll"), "NtCreateFile" );

    typedef struct _UNICODE_STRING {
        USHORT Length, MaximumLength;
        PWCH Buffer;
    } UNICODE_STRING, *PUNICODE_STRING;
    UNICODE_STRING fidstr = { 8, 8, (PWSTR)&FileReferenceNumber };

    typedef struct _OBJECT_ATTRIBUTES {
        ULONG Length;
        HANDLE RootDirectory;
        PUNICODE_STRING ObjectName;
        ULONG Attributes;
        PVOID SecurityDescriptor;
        PVOID SecurityQualityOfService;
    } OBJECT_ATTRIBUTES;
    const ULONG OBJ_CASE_INSENSITIVE = 0x00000040UL;
    OBJECT_ATTRIBUTES oa = { sizeof(OBJECT_ATTRIBUTES), hVol, &fidstr, OBJ_CASE_INSENSITIVE, 0, 0 };
   
    HANDLE hFile;
    ULONG iosb[2];
    const ULONG FILE_OPEN_BY_FILE_ID = 0x00002000UL;
    const ULONG FILE_OPEN            = 0x00000001UL;
    ULONG status = NtCreatefile( &hFile, GENERIC_ALL, &oa, iosb, NULL, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ|FILE_SHARE_WRITE, FILE_OPEN, FILE_OPEN_BY_FILE_ID, NULL, 0 );
    if( status == 0 )
    {
		typedef LONG  NTSTATUS;
        typedef struct _IO_STATUS_BLOCK {
            union {
                NTSTATUS Status;
                PVOID Pointer;
            };
            ULONG_PTR Information;
        } IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;
        typedef enum _FILE_INFORMATION_CLASS {
            // ……
			FileBothDirectoryInformation = 3,
			FileStandardInformation = 5,
            FileNameInformation = 9
            // ……
        } FILE_INFORMATION_CLASS, *PFILE_INFORMATION_CLASS;
        typedef NTSTATUS (__stdcall *PNtQueryInformationFile)(
            HANDLE FileHandle,
            PIO_STATUS_BLOCK IoStatusBlock,
            PVOID FileInformation,
            DWORD Length,
            FILE_INFORMATION_CLASS FileInformationClass );

        static PNtQueryInformationFile NtQueryInformationFile = (PNtQueryInformationFile)GetProcAddress( GetModuleHandleW(L"ntdll.dll"), "NtQueryInformationFile" );

        typedef struct _OBJECT_NAME_INFORMATION {
            UNICODE_STRING Name;
        } OBJECT_NAME_INFORMATION, *POBJECT_NAME_INFORMATION;
		typedef struct _FILE_NAME_INFORMATION {
		  ULONG FileNameLength;
		  WCHAR FileName[1];
		} FILE_NAME_INFORMATION, *PFILE_NAME_INFORMATION;
		// from: http://msdn.microsoft.com/en-us/library/windows/hardware/ff540235(v=vs.85).aspx
		typedef struct _FILE_BOTH_DIR_INFORMATION {
		  ULONG         NextEntryOffset;
		  ULONG         FileIndex;
		  LARGE_INTEGER CreationTime;
		  LARGE_INTEGER LastAccessTime;
		  LARGE_INTEGER LastWriteTime;
		  LARGE_INTEGER ChangeTime;
		  LARGE_INTEGER EndOfFile;
		  LARGE_INTEGER AllocationSize;
		  ULONG         FileAttributes;
		  ULONG         FileNameLength;
		  ULONG         EaSize;
		  CCHAR         ShortNameLength;
		  WCHAR         ShortName[12];
		  WCHAR         FileName[1];
		} FILE_BOTH_DIR_INFORMATION, *PFILE_BOTH_DIR_INFORMATION;

		typedef struct _FILE_STANDARD_INFORMATION {
		  LARGE_INTEGER AllocationSize;
		  LARGE_INTEGER EndOfFile;
		  ULONG         NumberOfLinks;
		  BOOLEAN       DeletePending;
		  BOOLEAN       Directory;
		} FILE_STANDARD_INFORMATION, *PFILE_STANDARD_INFORMATION;
		
        IO_STATUS_BLOCK IoStatus;
        //size_t allocSize = sizeof(FILE_STANDARD_INFORMATION) + MAX_PATH*sizeof(WCHAR);
		size_t allocSize = sizeof(FILE_STANDARD_INFORMATION);
        PFILE_STANDARD_INFORMATION pfni = (PFILE_STANDARD_INFORMATION)operator new(allocSize);
        status = NtQueryInformationFile(hFile, &IoStatus, pfni, allocSize, FileStandardInformation);
        if( status == 0 )
        {
            //printf( "%.*S", pfni->FileNameLength/2, &pfni->FileName );
			//wprintf(L"%s",pfni->FileName);
        }
        operator delete(pfni);

        CloseHandle(hFile);
    }
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

/*
#include <vector>
#include <algorithm>
#include <cstdio>

struct FileNode
{
    WCHAR name[MAX_PATH];
    DWORD FileAttributes;
    std::vector<FileNode> subs;

    FileNode( const WCHAR* filename, DWORD fileattr ) : FileAttributes(fileattr)
    {
        wcscpy( name, filename );
    }
};

// tree output.
int TreeOutPut()
{
    std::deque<FileInfo> con;// list不利于折半查找
    //EnumUsnRecord( "D", con );

    // 整理成树
    struct foo1
    {
        bool operator()( const FileInfo& a, const FileInfo& b ) const
        {
            if( a.ParentRefNo != b.ParentRefNo )
                return a.ParentRefNo<b.ParentRefNo;
            if( (a.FileAttributes&FILE_ATTRIBUTE_DIRECTORY) != (b.FileAttributes&FILE_ATTRIBUTE_DIRECTORY) )
                return (a.FileAttributes&FILE_ATTRIBUTE_DIRECTORY) > (b.FileAttributes&FILE_ATTRIBUTE_DIRECTORY);
            return _wcsicmp(a.Name,b.Name)<0;
        }
    };
    std::sort( con.begin(), con.end(), foo1() );
    FileNode root( L"D:\\", 0 );
    std::deque< std::pair<DWORDLONG,std::vector<FileNode>*> > tmp;
    tmp.push_back( std::make_pair(0x5000000000005,&root.subs) );
    for( ; !tmp.empty(); )
    {
        DWORDLONG ParentRefNo = tmp.front().first;
        std::vector<FileNode>& subs = *tmp.front().second;
        tmp.pop_front();

        struct foo2 {
            bool operator()( DWORDLONG prn, const FileInfo& fi ) const { return prn < fi.ParentRefNo; }
            bool operator()( const FileInfo& fi, DWORDLONG prn ) const { return fi.ParentRefNo < prn; }
            bool operator()( const FileInfo& a, const FileInfo& b ) const { return a.ParentRefNo < b.ParentRefNo; }
        };
        std::pair<std::deque<FileInfo>::iterator,std::deque<FileInfo>::iterator> r = std::equal_range( con.begin(), con.end(), ParentRefNo, foo2() );
        subs.reserve( std::distance(r.first,r.second) );
        for( std::deque<FileInfo>::iterator itor=r.first; itor!=r.second; ++itor )
        {
            FileNode fn( itor->Name, itor->FileAttributes );
            subs.push_back( fn );
            tmp.push_front( std::make_pair(itor->FileRefNo, &subs.back().subs) ); // 深度优先
        }
    }
    con.clear();

    // 输出树
    setlocale( LC_CTYPE, "chs" );
    std::vector< std::pair<std::vector<FileNode>::iterator,std::vector<FileNode>::iterator> > path;
    printf( "%s", "D:" );
    path.push_back( std::make_pair(root.subs.begin(),root.subs.end()) );
    for( ; !path.empty(); )
    {
        if( path.back().first != path.back().second )
        {
            printf( "%*s%S", path.size()*2, "", path.back().first->name );
            path.push_back( std::make_pair(path.back().first->subs.begin(),path.back().first->subs.end()) );
        }
        else
        {
            path.pop_back();
            if( path.empty() ) break;
            ++path.back().first;
        }
    }

    return 0;
}

#include <windows.h>

// 根据 FileReferenceNumber 直接获得全路径 的方法二
//使用 NtCreatefile 和 NtQueryInformationFile ，但要求这个文件必须存在（in-used）
void GetFullPathByFileReferenceNumber( HANDLE hVol, DWORDLONG FileReferenceNumber )
{
    typedef ULONG (__stdcall *PNtCreateFile)(
        PHANDLE FileHandle,
        ULONG DesiredAccess,
        PVOID ObjectAttributes,
        PVOID IoStatusBlock,
        PLARGE_INTEGER AllocationSize,
        ULONG FileAttributes,
        ULONG ShareAccess,
        ULONG CreateDisposition,
        ULONG CreateOptions,
        PVOID EaBuffer,
        ULONG EaLength );
    PNtCreateFile NtCreatefile = (PNtCreateFile)GetProcAddress( GetModuleHandle(L"ntdll.dll"), "NtCreateFile" );

    typedef struct _UNICODE_STRING {
        USHORT Length, MaximumLength;
        PWCH Buffer;
    } UNICODE_STRING, *PUNICODE_STRING;
    UNICODE_STRING fidstr = { 8, 8, (PWSTR)&FileReferenceNumber };

    typedef struct _OBJECT_ATTRIBUTES {
        ULONG Length;
        HANDLE RootDirectory;
        PUNICODE_STRING ObjectName;
        ULONG Attributes;
        PVOID SecurityDescriptor;
        PVOID SecurityQualityOfService;
    } OBJECT_ATTRIBUTES;
    const ULONG OBJ_CASE_INSENSITIVE = 0x00000040UL;
    OBJECT_ATTRIBUTES oa = { sizeof(OBJECT_ATTRIBUTES), hVol, &fidstr, OBJ_CASE_INSENSITIVE, 0, 0 };
   
    HANDLE hFile;
    ULONG iosb[2];
    const ULONG FILE_OPEN_BY_FILE_ID = 0x00002000UL;
    const ULONG FILE_OPEN            = 0x00000001UL;
    ULONG status = NtCreatefile( &hFile, GENERIC_ALL, &oa, iosb, NULL, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ|FILE_SHARE_WRITE, FILE_OPEN, FILE_OPEN_BY_FILE_ID, NULL, 0 );
    if( status == 0 )
    {
        typedef NTSTATUS (__stdcall *PNtQueryInformationFile)(
            HANDLE FileHandle,
            PIO_STATUS_BLOCK IoStatusBlock,
            PVOID FileInformation,
            DWORD Length,
            FILE_INFORMATION_CLASS FileInformationClass );

        typedef struct _IO_STATUS_BLOCK {
            union {
                NTSTATUS Status;
                PVOID Pointer;
            };
            ULONG_PTR Information;
        } IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;
        typedef enum _FILE_INFORMATION_CLASS {
            // ……
            FileNameInformation = 9
            // ……
        } FILE_INFORMATION_CLASS, *PFILE_INFORMATION_CLASS;
        PNtQueryInformationFile NtQueryInformationFile = (PNtQueryInformationFile)GetProcAddress( GetModuleHandle(L"ntdll.dll"), "NtQueryInformationFile" );

        typedef struct _OBJECT_NAME_INFORMATION {
            UNICODE_STRING Name;
        } OBJECT_NAME_INFORMATION, *POBJECT_NAME_INFORMATION;
        IO_STATUS_BLOCK IoStatus;
        size_t allocSize = sizeof(OBJECT_NAME_INFORMATION) + MAX_PATH*sizeof(WCHAR);
        POBJECT_NAME_INFORMATION pfni = (POBJECT_NAME_INFORMATION)operator new(allocSize);
        status = NtQueryInformationFile(hFile, &IoStatus, pfni, allocSize, FileNameInformation);
        if( status == 0 )
        {
            printf( "%.*S", pfni->Name.Length/2, &pfni->Name.Buffer );
        }
        operator delete(pfni);

        CloseHandle(hFile);
    }
}

// 根据 FileReferenceNumber 直接获得全路径 的方法三
//使用 FSCTL_GET_NTFS_FILE_RECORD，但要求这个文件必须存在（in-used）
typedef struct {
    ULONG Type;
    USHORT UsaOffset;
    USHORT UsaCount;
    USN Usn;
} NTFS_RECORD_HEADER, *PNTFS_RECORD_HEADER;

typedef struct {
    NTFS_RECORD_HEADER Ntfs;
    USHORT SequenceNumber;
    USHORT LinkCount;
    USHORT AttributesOffset;
    USHORT Flags;               // 0x0001 = InUse, 0x0002 = Directory
    ULONG BytesInUse;
    ULONG BytesAllocated;
    ULONGLONG BaseFileRecord;
    USHORT NextAttributeNumber;
} FILE_RECORD_HEADER, *PFILE_RECORD_HEADER;

typedef enum {
    AttributeStandardInformation = 0x10,
    AttributeAttributeList = 0x20,
    AttributeFileName = 0x30,
    AttributeObjectId = 0x40,
    AttributeSecurityDescriptor = 0x50,
    AttributeVolumeName = 0x60,
    AttributeVolumeInformation = 0x70,
    AttributeData = 0x80,
    AttributeIndexRoot = 0x90,
    AttributeIndexAllocation = 0xA0,
    AttributeBitmap = 0xB0,
    AttributeReparsePoint = 0xC0,
    AttributeEAInformation = 0xD0,
    AttributeEA = 0xE0,
    AttributePropertySet = 0xF0,
    AttributeLoggedUtilityStream = 0x100
} ATTRIBUTE_TYPE, *PATTRIBUTE_TYPE;

typedef struct {
    ATTRIBUTE_TYPE AttributeType;
    ULONG Length;
    BOOLEAN Nonresident;
    UCHAR NameLength;
    USHORT NameOffset;
    USHORT Flags;               // 0x0001 = Compressed
    USHORT AttributeNumber;
} ATTRIBUTE, *PATTRIBUTE;

typedef struct {
    ATTRIBUTE Attribute;
    ULONGLONG LowVcn;
    ULONGLONG HighVcn;
    USHORT RunArrayOffset;
    UCHAR CompressionUnit;
    UCHAR AlignmentOrReserved[5];
    ULONGLONG AllocatedSize;
    ULONGLONG DataSize;
    ULONGLONG InitializedSize;
    ULONGLONG CompressedSize;    // Only when compressed
} NONRESIDENT_ATTRIBUTE, *PNONRESIDENT_ATTRIBUTE;

typedef struct {
    ATTRIBUTE Attribute;
    ULONG ValueLength;
    USHORT ValueOffset;
    USHORT Flags;               // 0x0001 = Indexed
} RESIDENT_ATTRIBUTE, *PRESIDENT_ATTRIBUTE;

typedef struct {
    ULONGLONG CreationTime;
    ULONGLONG ChangeTime;
    ULONGLONG LastWriteTime;
    ULONGLONG LastAccessTime;
    ULONG FileAttributes;
    ULONG AlignmentOrReservedOrUnknown[3];
    ULONG QuotaId;                        // NTFS 3.0 only
    ULONG SecurityId;                     // NTFS 3.0 only
    ULONGLONG QuotaCharge;                // NTFS 3.0 only
    USN Usn;                              // NTFS 3.0 only
} STANDARD_INFORMATION, *PSTANDARD_INFORMATION;

typedef struct {
    ULONGLONG DirectoryFileReferenceNumber;
    ULONGLONG CreationTime;   // Saved when filename last changed
    ULONGLONG ChangeTime;     // ditto
    ULONGLONG LastWriteTime;  // ditto
    ULONGLONG LastAccessTime; // ditto
    ULONGLONG AllocatedSize;  // ditto
    ULONGLONG DataSize;       // ditto
    ULONG FileAttributes;     // ditto
    ULONG AlignmentOrReserved;
    UCHAR NameLength;
    UCHAR NameType;           // 0x01 = Long, 0x02 = Short
    WCHAR Name[1];
} FILENAME_ATTRIBUTE, *PFILENAME_ATTRIBUTE;

bool GetFullPathByFileReferenceNumber( HANDLE hVol, DWORDLONG FileReferenceNumber )
{
    if( (FileReferenceNumber&0x0000FFFFFFFFFFFF) == 5 )
        return true;

    bool ret = false;
    DWORD BytesReturned;
    NTFS_VOLUME_DATA_BUFFER nvdb;
    if( DeviceIoControl( hVol, FSCTL_GET_NTFS_VOLUME_DATA, NULL, 0
        , &nvdb, sizeof(nvdb), &BytesReturned, NULL ) ) // 仅是事例，没有作优化 1.作为递归调用，这一步应当提取出来 2.如果多次调用，DirectoryFileReferenceNumber没必要被重复获取
    {
        NTFS_FILE_RECORD_INPUT_BUFFER nfrib;
        nfrib.FileReferenceNumber.QuadPart = FileReferenceNumber;
        size_t len = sizeof(NTFS_FILE_RECORD_OUTPUT_BUFFER)+nvdb.BytesPerFileRecordBuf-1;
        NTFS_FILE_RECORD_OUTPUT_BUFFER* nfrob = (PNTFS_FILE_RECORD_OUTPUT_BUFFER)operator new(len);
        if( DeviceIoControl( hVol, FSCTL_GET_NTFS_FILE_RECORD, &nfrib, sizeof(nfrib)
            , nfrob, len, &BytesReturned, NULL ) )
        {
            if( (nfrib.FileReferenceNumber.QuadPart&0x0000FFFFFFFFFFFF) == nfrob->FileReferenceNumber.QuadPart ) // a 48-bit index and a 16-bit sequence number
            {
                PFILE_RECORD_HEADER frh = (PFILE_RECORD_HEADER)nfrob->FileRecordBuffer;
                for( PATTRIBUTE attr=(PATTRIBUTE)((LPBYTE)frh+frh->AttributesOffset); attr->AttributeType!=-1; attr=(PATTRIBUTE)((LPBYTE)attr+attr->Length) )
                {
                    if( attr->AttributeType == AttributeFileName )
                    {
                        PFILENAME_ATTRIBUTE name = (PFILENAME_ATTRIBUTE)( (LPBYTE)attr + PRESIDENT_ATTRIBUTE(attr)->ValueOffset );
                        if( (name->NameType&1) == 1 ) // long name
                        {
                            if( GetFullPathByFileReferenceNumber( hVol, name->DirectoryFileReferenceNumber ) )
                            {
                                printf( "\%.*S", name->NameLength, name->Name );
                                ret = true;
                            }
                        }
                    }
                }
            }
        }
        operator delete( nfrob );
    }
    return ret;
}//[喝小酒的网摘]http://blog.const.net.cn/a/16851.htm

*/