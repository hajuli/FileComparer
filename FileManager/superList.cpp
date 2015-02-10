// stdafx.cpp : source file that includes just the standard includes
// FileManager.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file


#include "superList.h"

DuLinkList::DuLinkList()
:
m_listHead(0),
m_listTail(0),
m_size(0)
{
}

DuLinkList::~DuLinkList()
{
	m_listHead = 0;
	m_listTail = 0;
	m_size = 0;
}

void DuLinkList::cleanUp()
{
	IListItem* t = 0;
	while(m_listHead)
	{
		t = m_listHead;
		m_listHead = m_listHead->m_next;
		t->m_last = 0;
		t->m_next = 0;
	}
	m_listHead = 0;
	m_listTail = 0;
	m_size = 0;
}

void DuLinkList::InsertItem(IListItem* item)
{
	if (0 != item)
	{
		if(m_listTail != item && 0 == item->m_next)
		{
			if (m_listHead) // the list is not empty.
			{
				m_listTail->m_next = item;
				item->m_last = m_listTail;
				m_listTail = m_listTail->m_next;
			}
			else
			{
				m_listHead = item;
				m_listTail = item;
			};
			++m_size;
		}
	}
}

void DuLinkList::removeItem(IListItem* item)
{
	if (0 != item)
	{
		IListItem* last = item->m_last;
		IListItem* next = item->m_next;
		if (m_listHead != item && 0 == last && 0 == next)
		{ 
			return;	// item is not in this list.
		}
		if (last)
		{
			last->m_next = next;
		}
		else
		{
			m_listHead = next;
		}
		if (next)
		{
			next->m_last = last;
		}
		else
		{
			m_listTail = last;
		}
		item->m_last = 0;
		item->m_next = 0;
		--m_size;
	}
}

IListItem* DuLinkList::pop()
{
	IListItem* ret = 0;
	if (m_size)
	{
		ret = m_listHead;
		m_listHead = m_listHead->m_next;
		ret->m_next = 0;

		if (m_listHead)
		{
			m_listHead->m_last = 0;
		}
		else
		{
			m_listTail = 0;
		}
		--m_size;
	}
	return ret;
}

///////////////////////////////////
SgLinkList::SgLinkList()
:
m_listHead(0),
m_listTail(0),
m_size(0)
{
}

SgLinkList::~SgLinkList()
{
	m_listHead = 0;
	m_listTail = 0;
	m_size = 0;
}

void SgLinkList::cleanUp()
{
	IListItem* t = 0;
	while(m_listHead)
	{
		t = m_listHead;
		m_listHead = m_listHead->m_next;
		t->m_next = 0;
	}
	m_listHead = 0;
	m_listTail = 0;
	m_size = 0;
}

void SgLinkList::InsertItem(IListItem* item)
{
	if (0 != item)
	{
		if(m_listTail != item && 0 == item->m_next)
		{
			if (m_listHead) // the list is not empty.
			{
				m_listTail->m_next = item;
				m_listTail = m_listTail->m_next;
			}
			else
			{
				m_listHead = item;
				m_listTail = item;
			};
			++m_size;
		}
	}
}

IListItem* SgLinkList::pop()
{
	IListItem* ret = 0;
	if (m_size)
	{
		ret = m_listHead;
		m_listHead = m_listHead->m_next;
		ret->m_next = 0;
		if (0 == m_listHead)
		{
			m_listTail = 0;
		}
		--m_size;
	}
	return ret;
}