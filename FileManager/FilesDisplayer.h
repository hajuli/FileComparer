// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "PartitionGroup.h"
#include <vector>
#include <list>
#include <map>

const std::string ORDERFILE_DESC = "DESC";
const std::string ORDERFILE_NONE = "NONE";
const std::string ORDERFILE_NAME = "NAME";
const std::string ORDERFILE_SIZE = "SIZE";
const std::string ORDERFILE_PATH = "PATH";
const std::string ORDERFILE_CREATEDATE = "CREATEDATE";
const std::string ORDERFILE_MODIFYDATE = "MODIFYDATE";
const std::string ORDERFILE_LASTACCESSDATE = "LASTACCESSDATE";

struct NodeItem
{
	FileInfo* value;
	NodeItem* next;
	NodeItem(){value = 0; next = 0;};
};

struct SortNode
{
	NodeItem* head;
	NodeItem* rear;
	int	count;
	SortNode(){head = 0; rear = 0; count = 0;};
};

class FilesDisplayer
{
public:
	FilesDisplayer(std::string name, PartitionGroup* pg, CallBackToOwner notifyFunc);
	~FilesDisplayer();

	virtual void show();
	virtual void prepareData() = 0;
	virtual void selectFiles();
	virtual void showDetail();

	void updateSelectCondition(std::string condition){m_newSelectCondition = condition;};
	void sortFileList(std::string s);
	virtual void getSameFileAllPaths(std::string msg){};	//using vector index.
	
	virtual void reSet();
protected:
	
	std::string				m_name;
	int						m_showedItemCount;
	int						m_pgUpdatedId;
	std::string				m_selectCondition;
	std::string				m_newSelectCondition;
	CallBackToOwner			m_notifyFunc;

	PartitionGroup*			m_pg;
	FilesMapType	m_allFiles;
	std::vector<FileInfo*>		m_selectedFiles;
	FilesMapType				m_showedFiles;
};

class AllFilesDisplayer : public FilesDisplayer
{
public:
	AllFilesDisplayer(std::string name, PartitionGroup* pg, CallBackToOwner notifyFunc);
	~AllFilesDisplayer();

	virtual void prepareData();
	//virtual void showDetail();

private:
};

class SameFilesDisplayer : public FilesDisplayer
{
public:
	SameFilesDisplayer(std::string name, PartitionGroup* pg, CallBackToOwner notifyFunc);
	~SameFilesDisplayer();
	virtual void reSet();

	virtual void prepareData();
	virtual void selectFiles();
	virtual void showDetail();

	virtual void getSameFileAllPaths(std::string msg);	//using vector index.
private:
	std::vector<SortNode*>	m_findSameFiles;
	std::vector<SortNode*>	m_sameFilesSelected;
};

class MoreFilesDisplayer : public FilesDisplayer
{
public:
	MoreFilesDisplayer(std::string name, PartitionGroup* pg, PartitionGroup* cmpToPG, CallBackToOwner notifyFunc);
	~MoreFilesDisplayer();
	virtual void reSet();

	virtual void prepareData();
private:
	int						m_cmpPGUpdatedId;
	PartitionGroup*			m_cmpToPG;
	FilesMapType	m_oriAllFiles;
	FilesMapType	m_cmpToAllFiles;
};
