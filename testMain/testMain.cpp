// testMain.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../FileManager/dllInterface.h"

int __stdcall CallBackFun(const char* msgType, const char* msgContent);

void test_SetPartitionMulSel();
void test_SetCurrentShow();
void test_UpdateSelectCondition();
void test_SortShowItems();
void test_GetSameFiles();
void test_GetMoreFiles();

void test_InterlockedIncrement64();

void getFiles();
void printStack();
int printStackTrace();

int _tmain(int argc, _TCHAR* argv[])
{
	CallBackFun("ttt", "xxx");

	initService(CallBackFun);

	std::string pg = "pgroup";

	//test_SetPartitionMulSel();
	//test_SetCurrentShow();
	test_UpdateSelectCondition();
	//test_SortShowItems();
	//test_GetSameFiles();
	//test_GetMoreFiles();

	//getFiles();

	//test_InterlockedIncrement64();
	
	int y = getchar();
	return 0;
}

int __stdcall CallBackFun(const char* msgType, const char* msgContent)
{
	timeb timestamp;
	ftime( &timestamp );
	printf("%lld, %s, %s\n", timestamp.time, msgType, msgContent);
	return 0;
}

void test_SetPartitionMulSel()
{
	std::string pg = "pgroup";
	std::string msg = "";

	msg = "ParttGroup=" + pg;
	msg = msg + MessageSplitSign;
	msg = msg + PN_ShowName + "=" + pg + PV_ShowType_AllFiles;
	msg = msg + MessageSplitSign;
	msg = msg + PN_ShowType + "=" + PV_ShowType_AllFiles;
	postMessageToService("SetCurrentShow", msg.c_str());
	Sleep(2000);

	msg = "ParttGroup=" + pg;
	msg = msg + MessageSplitSign;
	msg = msg + "Value=xxx(Q:)";
	postMessageToService("SetPartitionMulSel", msg.c_str());

	Sleep(30000); //waiting for load.

	msg = "ParttGroup=" + pg;
	msg = msg + MessageSplitSign;
	msg = msg + "Value=";
	postMessageToService("SetPartitionMulSel", msg.c_str());

	Sleep(1000); //waiting for load.

	msg = "ParttGroup=" + pg;
	msg = msg + MessageSplitSign;
	msg = msg + "Value=xxx(Q:)";
	postMessageToService("SetPartitionMulSel", msg.c_str());

	Sleep(30000); //waiting for load.
}
void test_SetCurrentShow()
{
	std::string pg = "pgroup";

	std::string msg = "ParttGroup=" + pg;
	msg = msg + MessageSplitSign;
	msg = msg + PN_ShowName + "=" + pg + UiName_AllFiles.c_str();
	msg = msg + MessageSplitSign;
	msg = msg + PN_ShowType + "=" + PV_ShowType_AllFiles;
	postMessageToService("SetCurrentShow", msg.c_str());
	
	msg = "ParttGroup=" + pg;
	msg = msg + MessageSplitSign;
	msg = msg + "Value=xxx(Q:)";
	postMessageToService("SetPartitionMulSel", msg.c_str());

	Sleep(30000); //waiting for load.

	msg = "ParttGroup=" + pg;
	msg = msg + MessageSplitSign;
	msg = msg + PN_ShowName + "=" + pg + UiName_SameFiles.c_str();
	msg = msg + MessageSplitSign;
	msg = msg + PN_ShowType + "=" + PV_ShowType_SameFiles;
	postMessageToService("SetCurrentShow", msg.c_str());
	Sleep(2000);
}

void test_UpdateSelectCondition()
{
	std::string pg = "pgroup";

	std::string msg = "ParttGroup=" + pg;
	msg = msg + MessageSplitSign;
	msg = msg + PN_ShowName + "=" + pg + UiName_AllFiles.c_str();
	msg = msg + MessageSplitSign;
	msg = msg + PN_ShowType + "=" + PV_ShowType_AllFiles;
	postMessageToService("SetCurrentShow", msg.c_str());
	
	msg = "ParttGroup=" + pg;
	msg = msg + MessageSplitSign;
	msg = msg + "Value=xxx(E:)";
	postMessageToService("SetPartitionMulSel", msg.c_str());

	Sleep(30000); //waiting for load.

	msg = PN_ShowName + "=" + pg + UiName_AllFiles.c_str();
	msg = msg + MessageSplitSign;
	msg = msg + PN_SelectCondition + "=xxxfff";
	postMessageToService(cmdUpdateSelectCondition.c_str(), msg.c_str());
	Sleep(2000);
}
void test_SortShowItems()
{
	std::string pg = "pgroup";

	std::string msg = "ParttGroup=" + pg;
	msg = msg + MessageSplitSign;
	msg = msg + PN_ShowName + "=" + pg + UiName_AllFiles.c_str();
	msg = msg + MessageSplitSign;
	msg = msg + PN_ShowType + "=" + PV_ShowType_AllFiles;
	postMessageToService("SetCurrentShow", msg.c_str());
	
	msg = "ParttGroup=" + pg;
	msg = msg + MessageSplitSign;
	msg = msg + "Value=xxx(Q:)";
	postMessageToService("SetPartitionMulSel", msg.c_str());

	Sleep(30000); //waiting for load.

	msg = PN_ShowName + "=" + pg + UiName_AllFiles.c_str();
	msg = msg + MessageSplitSign;
	msg = msg + "Value=SIZE";
	postMessageToService("SortShowItems", msg.c_str());
	Sleep(2000);
}
void test_GetSameFiles()
{
	std::string pg = "pgroup";

	std::string msg = "ParttGroup=" + pg;
	msg = msg + MessageSplitSign;
	msg = msg + PN_ShowName + "=" + pg + UiName_AllFiles.c_str();
	msg = msg + MessageSplitSign;
	msg = msg + PN_ShowType + "=" + PV_ShowType_AllFiles;
	postMessageToService("SetCurrentShow", msg.c_str());
	
	msg = "ParttGroup=" + pg;
	msg = msg + MessageSplitSign;
	msg = msg + "Value=xxx(C:);fdsf(E:);sfsdf;;";
	postMessageToService("SetPartitionMulSel", msg.c_str());

	Sleep(60000); //waiting for load.

	msg = "ParttGroup=" + pg;
	msg = msg + MessageSplitSign;
	msg = msg + PN_ShowName + "=" + pg + UiName_SameFiles.c_str();
	msg = msg + MessageSplitSign;
	msg = msg + PN_ShowType + "=" + PV_ShowType_SameFiles;
	postMessageToService("SetCurrentShow", msg.c_str());
	Sleep(2000);
}
void test_GetMoreFiles()
{
	std::string pg = "pgroup";
	std::string cmpPg = "CmpPG";
	std::string msg = "";
	
	msg = "ParttGroup=" + pg;
	msg = msg + MessageSplitSign;
	msg = msg + "Value=xxx(E:)";
	postMessageToService("SetPartitionMulSel", msg.c_str());

	Sleep(30000); //waiting for load.

	msg = "ParttGroup=" + cmpPg;
	msg = msg + MessageSplitSign;
	msg = msg + "Value=yyy(Q:)";
	postMessageToService("SetPartitionMulSel", msg.c_str());

	Sleep(30000); //waiting for load.

	msg = "ParttGroup=" + pg;
	msg = msg + MessageSplitSign;
	msg = msg + PN_ShowName + "=" + pg + PV_ShowType_MoreFiles.c_str();
	msg = msg + MessageSplitSign;
	msg = msg + PN_ShowType + "=" + PV_ShowType_MoreFiles;
	msg = msg + MessageSplitSign;
	msg = msg + PN_CmpToParttGroup + "=" + cmpPg;
	postMessageToService("SetCurrentShow", msg.c_str());
	Sleep(15000);

	msg = PN_ShowName + "=" + pg + PV_ShowType_MoreFiles;
	msg = msg + MessageSplitSign;
	msg = msg + PN_SelectCondition + "=.mp3";
	postMessageToService(cmdUpdateSelectCondition.c_str(), msg.c_str());
	Sleep(2000);

	msg = PN_ShowName + "=" + pg + PV_ShowType_MoreFiles.c_str();
	msg = msg + MessageSplitSign;
	msg = msg + "Value=SIZE";
	postMessageToService("SortShowItems", msg.c_str());
	Sleep(2000);

	msg = "ParttGroup=" + cmpPg;
	msg = msg + MessageSplitSign;
	msg = msg + "Value=yyy(E:)";
	postMessageToService("SetPartitionMulSel", msg.c_str());
	Sleep(20000);
}

#include <winioctl.h>

#define BUF_LEN 4096

void getFiles()
{
	//EnumUsnRecord("E");
	HANDLE hVol;
	CHAR Buffer[BUF_LEN];

	USN_JOURNAL_DATA JournalData;
	READ_USN_JOURNAL_DATA ReadData = {0, 0xFFFFFFFF, 1, 0, 0};
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

	ReadData.StartUsn = JournalData.NextUsn;
	ReadData.UsnJournalID = JournalData.UsnJournalID;

	DWORD     ReasonMask = 0;
	//ReadData.ReasonMask = USN_REASON_FILE_CREATE | USN_REASON_FILE_DELETE | USN_REASON_RENAME_OLD_NAME | USN_REASON_RENAME_NEW_NAME;
	ReadData.Timeout = 5;
	ReadData.BytesToWaitFor = BUF_LEN;

	printf( "Journal ID: %I64x\n", JournalData.UsnJournalID );
	printf( "FirstUsn: %I64x\n\n", JournalData.FirstUsn );

	for(I=0; I<=1000; I++)
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
			static int nfiles = 0;
			++nfiles;
			printf( "nfiles %d\n", nfiles);
			printf( "USN: %I64x\n", UsnRecord->Usn );
			printf( "FileReferenceNumber: %I64x\n", UsnRecord->FileReferenceNumber);
			printf( "ParentFileReferenceNumber: %I64x\n", UsnRecord->ParentFileReferenceNumber);
			printf("File name: %.*S\n", 
				UsnRecord->FileNameLength/2, 
				UsnRecord->FileName );
			printf( "Reason: %x\n", UsnRecord->Reason );
			printf( "TimeStamp: %lld\n", UsnRecord->TimeStamp.QuadPart / 10 / 1000 / 1000 ); // 100 ns
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

void test_InterlockedIncrement64()
{
	LONGLONG value = 0xffffffffffffffff - 1;
	ULONGLONG value3 = value;
	LONGLONG value2 = value3;
	LONGLONG* pvalue3 = (LONGLONG*)&value3;
	
	printf("%I64u, %I64x, %lld\n", value, value, value);
	printf("%I64u, %I64x, %lld\n", value2, value2, value2);
	printf("%I64u, %I64x, %lld\n", value3, value3, value3);
	printf("\n");
	value = InterlockedIncrement64(&value);
	value2 = InterlockedIncrement64(&value2);
	*pvalue3 = InterlockedIncrement64(pvalue3);

	printf("%I64u, %I64x, %lld\n", value, value, value);
	printf("%I64u, %I64x, %lld\n", value2, value2, value2);
	printf("%I64u, %I64x, %lld\n", value3, value3, value3);
	
}

#include <process.h>
#include <iostream>
#include <Windows.h>
#include "dbghelp.h"

void printStack( void )
{
     unsigned int   i;
     void         * stack[ 100 ];
     unsigned short frames;
     SYMBOL_INFO  * symbol;
     HANDLE         process;

     process = GetCurrentProcess();
     SymInitialize( process, NULL, TRUE );
     frames               = CaptureStackBackTrace( 0, 100, stack, NULL );
     symbol               = ( SYMBOL_INFO * )calloc( sizeof( SYMBOL_INFO ) + 256 * sizeof( char ), 1 );
     symbol->MaxNameLen   = 255;
     symbol->SizeOfStruct = sizeof( SYMBOL_INFO );

     for( i = 0; i < frames; i++ )
     {
         SymFromAddr( process, ( DWORD64 )( stack[ i ] ), 0, symbol );

         printf( "%i: %s - 0x%0X\n", frames - i - 1, symbol->Name, symbol->Address );
     }

     free( symbol );
}
