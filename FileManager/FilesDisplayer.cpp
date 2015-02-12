// stdafx.cpp : source file that includes just the standard includes
// FilesDisplayer.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file


#include "FilesDisplayer.h"
#include <algorithm>

FilesDisplayer::FilesDisplayer(std::string name, PartitionGroup* pg, CallBackToOwner notifyFunc)
:
m_name(name),
m_notifyFunc(notifyFunc),
m_newSelectCondition(""),
m_pg(pg)
{
	reSet();
	m_pgUpdatedId = 0;
	m_allFiles.clear();
}

FilesDisplayer::~FilesDisplayer()
{
	reSet();
	m_allFiles.clear();
}

void FilesDisplayer::reSet()
{
	m_showedItemCount = 0;
	m_selectCondition = SelectedConditionDefault;
	m_selectedFiles.clear();
}

void FilesDisplayer::show()
{
	prepareData();
	selectFiles();
	printf("now select condition is:%s\n", m_selectCondition.c_str());
	showDetail();
}

void FilesDisplayer::showDetail()
{
	char buf[1024] = {0};
	if (0 == m_showedItemCount)
	{
		sprintf(buf, "%s=%s%s", PN_ShowName.c_str(), m_name.c_str(), MessageSplitSign.c_str());
		m_notifyFunc(cmdFileListItemClear.c_str(), buf);
	}
	int i = 0, j = 0, nSize = m_selectedFiles.size();
	for (i = m_showedItemCount, j = 0; i < nSize && j < SHOW_FILE_ITEM_NUM; ++i, ++j)
	{
		FileInfo& f = *m_selectedFiles[i];
		SYSTEMTIME cDate;
		FileTimeToSystemTime(&f.createDate, &cDate);
		SYSTEMTIME mDate;
		FileTimeToSystemTime(&f.modifyDate, &mDate);
		SYSTEMTIME lwDate;
		FileTimeToSystemTime(&f.lastAccessTime, &lwDate);
		memset(buf, 0, 1024);
		sprintf(buf,
			"%s=%s,.., RowRefIndex=%I64u,.., ParentRefNo=%I64u,.., name=%s,.., FileAttributes=%d,.., size=%s,.., createDate=%d-%02d-%02d %02d:%02d:%02d,.., modifyDate=%d-%02d-%02d %02d:%02d:%02d,.., lastAccessDate=%d-%02d-%02d %02d:%02d:%02d,.., path=%s,..,",
			PN_ShowName.c_str(),
			m_name.c_str(),
			f.FileRefNo, f.ParentRefNo, f.Name.c_str(), f.FileAttributes, f.fileSizeAsString(),
			cDate.wYear, cDate.wMonth, cDate.wDay, cDate.wHour, cDate.wMinute, cDate.wSecond,
			mDate.wYear, mDate.wMonth, mDate.wDay, mDate.wHour, mDate.wMinute, mDate.wSecond,
			lwDate.wYear, lwDate.wMonth, lwDate.wDay, lwDate.wHour, lwDate.wMinute, lwDate.wSecond,
			f.path.c_str()); 
		m_notifyFunc(cmdFileListItemAdd.c_str(), buf);
	}
	m_showedItemCount = i;
	memset(buf, 0, 1024);
	sprintf(buf, "%s=%s,.., Value=%d / %d", PN_ShowName.c_str(), m_name.c_str(), m_showedItemCount, nSize);
	m_notifyFunc(cmdUpdateShowedNums.c_str(), buf);
}

void FilesDisplayer::selectFiles()
{
	std::string new_ = m_newSelectCondition;
	transform(new_.begin(), new_.end(), new_.begin(), toupper);
	if (new_ == m_selectCondition)
	{
		return ;
	}

	m_selectedFiles.reserve(m_allFiles.size());
	m_selectedFiles.clear();

	std::map<DWORDLONG, FileInfo*>::const_iterator it = m_allFiles.begin();
	FileInfo* p = 0;
	while (m_allFiles.end() != it)
	{
		p = it->second;
		if ("" == new_ || std::string::npos != p->NameUppered.find(new_))
		{
			m_selectedFiles.push_back(p);
		}
		++it;
	}

	m_selectCondition = new_;

	m_showedItemCount = 0;
}


bool operator < (FILETIME a, FILETIME b)
{
	return *(ULONGLONG*)(&a) < *(ULONGLONG*)(&b);
}

bool operator > (FILETIME a, FILETIME b)
{
	return *(ULONGLONG*)(&a) > *(ULONGLONG*)(&b);
}

void FilesDisplayer::sortFileList(std::string s)
{
	std::string sortKeys = s;
	std::transform(sortKeys.begin(), sortKeys.end(), sortKeys.begin(), ::toupper);

	struct order_name_asce
    {
        bool operator()( const FileInfo* a, const FileInfo* b ) const
        {
            return a->Name < b->Name;
        }
    };
	struct order_name_desc
    {
        bool operator()( const FileInfo* a, const FileInfo* b ) const
        {
            return a->Name > b->Name;
        }
    };
	struct order_size_asce
    {
        bool operator()( const FileInfo* a, const FileInfo* b ) const
        {
            if( a->fileSize != b->fileSize )
                return a->fileSize < b->fileSize;

            return a->Name < b->Name;
        }
    };
	struct order_size_desc
    {
        bool operator()( const FileInfo* a, const FileInfo* b ) const
        {
            if( a->fileSize != b->fileSize )
                return a->fileSize > b->fileSize;

            //if( (a->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) != (b->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
            //    return (a->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) > (b->FileAttributes & FILE_ATTRIBUTE_DIRECTORY);

            return a->Name < b->Name;
        }
    };
	struct order_path_asce
    {
        bool operator()( const FileInfo* a, const FileInfo* b ) const
        {
            return a->path < b->path;
        }
    };
	struct order_path_desc
    {
        bool operator()( const FileInfo* a, const FileInfo* b ) const
        {
            return a->path > b->path;
        }
    };
	struct order_createDate_asce
    {
        bool operator()( const FileInfo* a, const FileInfo* b ) const
        {
            return a->createDate < b->createDate;
        }
    };
	struct order_createDate_desc
    {
        bool operator()( const FileInfo* a, const FileInfo* b ) const
        {
            return a->createDate > b->createDate;
        }
    };
	struct order_modifyDate_asce
    {
        bool operator()( const FileInfo* a, const FileInfo* b ) const
        {
            return a->modifyDate < b->modifyDate;
        }
    };
	struct order_modifyDate_desc
    {
        bool operator()( const FileInfo* a, const FileInfo* b ) const
        {
            return a->modifyDate > b->modifyDate;
        }
    };
	struct order_lastAccessDate_asce
    {
        bool operator()( const FileInfo* a, const FileInfo* b ) const
        {
            return a->lastAccessTime < b->lastAccessTime;
        }
    };
	struct order_lastAccessDate_desc
    {
        bool operator()( const FileInfo* a, const FileInfo* b ) const
        {
            return a->lastAccessTime > b->lastAccessTime;
        }
    };

	if (std::string::npos != sortKeys.find(ORDERFILE_NAME))
	{
		if(std::string::npos != sortKeys.find(ORDERFILE_DESC))
			std::sort( m_selectedFiles.begin(), m_selectedFiles.end(), order_name_desc());
		else
			std::sort( m_selectedFiles.begin(), m_selectedFiles.end(), order_name_asce());
	}
	else if(std::string::npos != sortKeys.find(ORDERFILE_SIZE))
	{
		if(std::string::npos != sortKeys.find(ORDERFILE_DESC))
			std::sort( m_selectedFiles.begin(), m_selectedFiles.end(), order_size_desc());
		else
			std::sort( m_selectedFiles.begin(), m_selectedFiles.end(), order_size_asce());
	}
	else if(std::string::npos != sortKeys.find(ORDERFILE_PATH))
	{
		if(std::string::npos != sortKeys.find(ORDERFILE_DESC))
			std::sort( m_selectedFiles.begin(), m_selectedFiles.end(), order_path_desc());
		else
			std::sort( m_selectedFiles.begin(), m_selectedFiles.end(), order_path_asce());
	}
	else if(std::string::npos != sortKeys.find(ORDERFILE_CREATEDATE))
	{
		if(std::string::npos != sortKeys.find(ORDERFILE_DESC))
			std::sort( m_selectedFiles.begin(), m_selectedFiles.end(), order_createDate_desc());
		else
			std::sort( m_selectedFiles.begin(), m_selectedFiles.end(), order_createDate_asce());
	}
	else if(std::string::npos != sortKeys.find(ORDERFILE_MODIFYDATE))
	{
		if(std::string::npos != sortKeys.find(ORDERFILE_DESC))
			std::sort( m_selectedFiles.begin(), m_selectedFiles.end(), order_modifyDate_desc());
		else
			std::sort( m_selectedFiles.begin(), m_selectedFiles.end(), order_modifyDate_asce());
	}
	else if(std::string::npos != sortKeys.find(ORDERFILE_LASTACCESSDATE))
	{
		if(std::string::npos != sortKeys.find(ORDERFILE_DESC))
			std::sort( m_selectedFiles.begin(), m_selectedFiles.end(), order_lastAccessDate_desc());
		else
			std::sort( m_selectedFiles.begin(), m_selectedFiles.end(), order_lastAccessDate_asce());
	}
	m_showedItemCount = 0;
}

//===========================
AllFilesDisplayer::AllFilesDisplayer(std::string name, PartitionGroup* pg, CallBackToOwner notifyFunc)
:
FilesDisplayer(name, pg, notifyFunc)
{
	reSet();
}

AllFilesDisplayer::~AllFilesDisplayer()
{
	reSet();
}

void AllFilesDisplayer::prepareData()
{
	int preId = m_pgUpdatedId;
	m_pgUpdatedId = m_pg->getAllFiles(m_allFiles, preId);
	
	if (preId == m_pgUpdatedId)
	{
		return;
	}
	reSet();
}

//===========================
SameFilesDisplayer::SameFilesDisplayer(std::string name, PartitionGroup* pg, CallBackToOwner notifyFunc)
:
FilesDisplayer(name, pg, notifyFunc)
{
	reSet();
}

SameFilesDisplayer::~SameFilesDisplayer()
{
	reSet();
}

void SameFilesDisplayer::reSet()
{
	FilesDisplayer::reSet();
	SortNode* nd = 0;
	NodeItem* it = 0;
	NodeItem* itn = 0;
	for (int i = m_findSameFiles.size() - 1; i >= 0; --i)
	{
		nd = m_findSameFiles[i];
		it = nd->head;
		while (it)
		{
			itn = it->next;
			delete it;
			it = itn;
		}
		delete nd;
	}
	m_findSameFiles.clear();
}
void SameFilesDisplayer::prepareData()
{
	int preId = m_pgUpdatedId;
	m_pgUpdatedId = m_pg->getAllFiles(m_allFiles, preId);
	
	if (preId == m_pgUpdatedId)
	{
		return;
	}
	reSet();
	
	std::vector<FileInfo*> vec;
	vec.clear();

	std::map<DWORDLONG, FileInfo*>::const_iterator it = m_allFiles.begin();
	FileInfo* p = 0;
	while (m_allFiles.end() != it)
	{
		p = it->second;
		if ((p->FileAttributes & FILE_ATTRIBUTE_ARCHIVE) && !(p->FileAttributes & FILE_ATTRIBUTE_DIRECTORY))// just for file.
		{
			vec.push_back(p);
		}
		++it;
	}

	struct order_size
    {
        bool operator()( const FileInfo* a, const FileInfo* b ) const
        {
            if( a->fileSize != b->fileSize )
                return a->fileSize > b->fileSize;
            return a->Name < b->Name;
        }
    };
	std::sort( vec.begin(), vec.end(), order_size());

	
	int nSize = vec.size();
	m_findSameFiles.clear();
	m_findSameFiles.reserve(nSize);

	int i = 0, pushed = 0;
	while (i < nSize)
	{
		FileInfo* fi = vec[i++];
		SortNode* tn = new SortNode();
		tn->count = 1;
		tn->head = new NodeItem();
		tn->head->value = fi;
		tn->rear = tn->head;
		m_findSameFiles.push_back(tn);
		while (i < nSize)
		{
			if (fi->fileSize == vec[i]->fileSize && fi->Name == vec[i]->Name)
			{
				NodeItem* ni = new NodeItem();
				ni->value = vec[i++];
				tn->rear->next = ni;
				tn->rear = ni;
				tn->count = tn->count + 1;
			}
			else
			{
				break;
			}
		}
	}

	struct order_count
    {
        bool operator()( const SortNode* a, const SortNode* b ) const
        {
            if( a->count != b->count )
                return a->count > b->count;
            return  a->head->value->fileSize > b->head->value->fileSize ; // can not using >=, debug assert error.
        }
    };
	std::sort(m_findSameFiles.begin(), m_findSameFiles.end(), order_count());

}

void SameFilesDisplayer::selectFiles()
{
	std::string new_ = m_newSelectCondition;
	transform(new_.begin(), new_.end(), new_.begin(), toupper);
	if (new_ == m_selectCondition)
	{
		return ;
	}

	int nSize = m_findSameFiles.size();
	m_sameFilesSelected.clear();
	m_sameFilesSelected.reserve(nSize);

	SortNode* p = 0;
	for (int i = 0; i < nSize; ++i)
	{
		p = m_findSameFiles[i];
		if ("" == new_ || std::string::npos != p->head->value->NameUppered.find(new_))
		{
			m_sameFilesSelected.push_back(p);
		}
	}

	m_selectCondition = new_;

	m_showedItemCount = 0;
}

void SameFilesDisplayer::showDetail()
{
	char buf[1024] = {0};
	if (0 == m_showedItemCount)
	{
		sprintf(buf, "%s=%s%s", PN_ShowName.c_str(), m_name.c_str(), MessageSplitSign.c_str());
		m_notifyFunc(cmdFileListItemClear.c_str(), buf);
	}
	int i = 0, j = 0, nSize = m_sameFilesSelected.size();
	for (i = m_showedItemCount, j = 0; i < nSize && j < SHOW_FILE_ITEM_NUM; ++i, ++j)
	{
		SortNode* sn = m_sameFilesSelected[i];
		memset(buf, 0, 1024);
		sprintf(buf,
			"%s=%s,.., RowRefIndex=%d,.., name=%s,.., times=%d,.., size=%s,.., path=%s,..,",
			PN_ShowName.c_str(),
			m_name.c_str(),
			i,
			sn->head->value->Name.c_str(), sn->count, sn->head->value->fileSizeAsString(), sn->head->value->path.c_str()); 
		m_notifyFunc(cmdFileListItemAdd.c_str(), buf);
	}
	m_showedItemCount = i;
	memset(buf, 0, 1024);
	sprintf(buf, "%s=%s,.., Value=%d / %d", PN_ShowName.c_str(), m_name.c_str(), m_showedItemCount, nSize);
	m_notifyFunc(cmdUpdateShowedNums.c_str(), buf);
}

void SameFilesDisplayer::getSameFileAllPaths(std::string msg)
{
	int index = atoi(msg.c_str());
	if (index < m_sameFilesSelected.size())
	{
		std::string paths = "";
		NodeItem* p = m_sameFilesSelected[index]->head;
		std::string name = p->value->Name;
		while (p)
		{
			if (paths.length() > 0)
				paths = paths + ";";
			paths = paths + p->value->path;
			p = p->next;
		}
		std::string msg = PN_ShowName + "=" + m_name + MessageSplitSign;
		msg = msg + PN_MessageValue + "=" + name;
		msg = msg + MessageSplitSign + PN_MessageValue2 + "=" + paths;
		m_notifyFunc(cmdShowSameFileAllPaths.c_str(), msg.c_str());
	}
}

//===========================
MoreFilesDisplayer::MoreFilesDisplayer(std::string name, PartitionGroup* pg, PartitionGroup* cmpToPG, CallBackToOwner notifyFunc)
:
FilesDisplayer(name, pg, notifyFunc),
m_cmpToPG(cmpToPG)
{
	reSet();
	m_cmpToAllFiles.clear();
}

MoreFilesDisplayer::~MoreFilesDisplayer()
{
	reSet();
	m_cmpToAllFiles.clear();
}

void MoreFilesDisplayer::reSet()
{
	FilesDisplayer::reSet();
}

void MoreFilesDisplayer::prepareData()
{
	int preId = m_pgUpdatedId;
	m_pgUpdatedId = m_pg->getAllFiles(m_oriAllFiles, preId);
	
	int preCmpId = m_cmpPGUpdatedId;
	m_cmpPGUpdatedId = m_cmpToPG->getAllFiles(m_cmpToAllFiles, preCmpId);
	
	if (preId == m_pgUpdatedId && preCmpId == m_cmpPGUpdatedId)
	{
		return;
	}
	reSet();

	std::vector<FileInfo*> vec;
	vec.clear();

	std::map<DWORDLONG, FileInfo*>::const_iterator it = m_oriAllFiles.begin();
	FileInfo* p = 0;
	while (m_oriAllFiles.end() != it)
	{
		p = it->second;
		if (p->FileAttributes & FILE_ATTRIBUTE_ARCHIVE)
		{
			vec.push_back(p);
		}
		++it;
	}
	printf("ori pg find FILE_ATTRIBUTE_ARCHIVE Count:%d\n", vec.size());

	std::vector<FileInfo*> cmpVec;
	cmpVec.clear();

	it = m_cmpToAllFiles.begin();
	p = 0;
	while (m_cmpToAllFiles.end() != it)
	{
		p = it->second;
		if (p->FileAttributes & FILE_ATTRIBUTE_ARCHIVE)
		{
			cmpVec.push_back(p);
		}
		++it;
	}

	struct order_size
    {
        bool operator()( const FileInfo* a, const FileInfo* b ) const
        {
            if( a->fileSize != b->fileSize )
                return a->fileSize < b->fileSize;
            return a->Name < b->Name;
        }
    };
	std::sort( vec.begin(), vec.end(), order_size());
	std::sort( cmpVec.begin(), cmpVec.end(), order_size());
	
	m_allFiles.clear();
	FileInfo* pf = 0;
	FileInfo* pCmpF = 0;
	int nSize = vec.size();
	int cmpSize = cmpVec.size();
	int i = 0, j = 0, nCount = 0;
	while (i < nSize)
	{
		pf = vec[i];
		pCmpF = 0;
		while (j < cmpSize)
		{
			bool b = true;
			pCmpF = cmpVec[j];
			if (pf->fileSize > pCmpF->fileSize)
			{
				b = false;
			}
			else if(pf->fileSize == pCmpF->fileSize && pf->Name > pCmpF->Name)
			{
				b = false;
			}
			if (b)
			{
				break;
			}
			++j;
		}
		bool bSame = false;
		if (j < cmpSize)
		{
			if (pf->fileSize == pCmpF->fileSize && pf->Name == pCmpF->Name)
			{
				bSame = true;
				++nCount;
			}
		}
		if (!bSame)
		{
			m_allFiles[pf->FileRefNo] = pf;
		}
		++i;
	}
	printf("MoreFilesDisplayer same Count:%d\n", nCount);
}
