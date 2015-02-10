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

int _tmain(int argc, _TCHAR* argv[])
{
	CallBackFun("ttt", "xxx");

	initService(CallBackFun);

	std::string pg = "pgroup";

	test_GetMoreFiles();
	
	Sleep(20000);
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
	msg = msg + "Value=xxx(Q:)";
	postMessageToService("SetPartitionMulSel", msg.c_str());

	Sleep(30000); //waiting for load.

	msg = PN_ShowName + "=" + pg + UiName_AllFiles.c_str();
	msg = msg + MessageSplitSign;
	msg = msg + "Value=.ini";
	postMessageToService("UpdateSelectCondition", msg.c_str());
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
	msg = msg + "Value=yyy(C:)";
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
	msg = msg + "Value=.mp3";
	postMessageToService("UpdateSelectCondition", msg.c_str());
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
