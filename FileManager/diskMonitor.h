// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#include "dllInterface.h"
#include "fileInfo.h"
#include "ThreadWorker.h"

class FileInfo;

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
    AttributeLoggedUtilityStream = 0x100,
	AttributeEnd = 0xFFFFFFFF
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

typedef struct
{
    int BytesPerFileRecordBuf;
    int BytesPerCluster;
} VolNTFSInfoNode;

enum FileOperation
{
	File_Create,
	File_Delete,
	File_Rename,	//may change parent folder.
};

typedef struct
{
    FileOperation operation;
    FileInfo* fi;
}FileOperationRecord;

class IDiskMonitorEvent
{
public:
	virtual ~IDiskMonitorEvent(){};
	virtual bool updateLoadingRate(int rate, const char* vol = 0) = 0;
	virtual bool notifyFilesChange(FileOperation, std::string volume, FileInfo*) = 0;
};

class DiskMonitor : public ThreadWorker
{
public:
	DiskMonitor(std::string volume, IDiskMonitorEvent* eventHandler = 0);
	~DiskMonitor();

	bool loadAllFiles();
	int	 getAllFilesCount(){return m_allFiles.size();};
	void getAllFiles(FilesMapType& allFiles);
	bool EnumUsnRecord( const char* drvname, DuLinkList & fileList);

	static void loadAllVolumeIDs(std::vector<std::string>& ids); // item like: (C:) OS

	bool GetVolumeDetail( HANDLE hVol, int& usnCount);
	//bool GetFileDetail(HANDLE hVol, DWORDLONG FileReferenceNumber, FileInfo& finfo);

private:
	int svc();
	void constructFileFullPath(DuLinkList& files);

	USN			m_startUsn;
	DWORDLONG	m_UsnJournalID;
	std::string m_diskName;
	VolNTFSInfoNode m_volInfo;

	DuLinkList	m_allFiles;
	FilesMapType	m_allFilesMap;		//key is FileRefNo;
	FilesMapType	m_deletedFilesMap;	//key is uuid;
	IDiskMonitorEvent* m_eventHandler;
};
