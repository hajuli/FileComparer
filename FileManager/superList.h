// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#include <vector>

class DuLinkList;
class SgLinkList;
class IListItem
{
	friend class DuLinkList;
	friend class SgLinkList;
public:
	IListItem(): m_last(0), m_next(0) {};
	virtual ~IListItem(){};

private:
	IListItem* m_last;
	IListItem* m_next;
};

//Singly-linked list
class SgLinkList
{
public:
	SgLinkList();
	virtual ~SgLinkList();
	void cleanUp();
	void InsertItem(IListItem*);
	//void removeItem(IListItem*);
	IListItem* pop();
	unsigned long size() {return m_size;};


private:
	IListItem* m_listHead;
	IListItem* m_listTail;
	unsigned long	m_size;
};

//doubly linked list
class DuLinkList
{
public:
	DuLinkList();
	virtual ~DuLinkList();
	void cleanUp();
	void InsertItem(IListItem*);
	void removeItem(IListItem*);
	IListItem* pop();
	unsigned long size() {return m_size;};

	IListItem* head() {return m_listHead;};
	IListItem* next(IListItem* cur){return cur->m_next;};

private:
	IListItem* m_listHead;
	IListItem* m_listTail;
	unsigned long	m_size;
};

