// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#include "ace/Thread_Semaphore.h"
#include "ace/Thread_Mutex.h"
#include "ace/Task.h"

class MutexGuard
{
public:
	MutexGuard(ACE_Thread_Mutex& mutex):m_mutex(mutex){m_mutex.acquire();};
	~MutexGuard(){m_mutex.release();};
private:
	ACE_Thread_Mutex& m_mutex;
};

class WorkerTask
{
public:
	virtual ~WorkerTask(){};
	virtual void execute() = 0;
};

class ThreadWorker : public ACE_Task_Base
{
public:
	ThreadWorker(WorkerTask* item = 0):
		m_semaphore(0), 
		m_bRunning(false)
	{
		m_task = item;
	};
	virtual ~ThreadWorker(){};
	
	virtual int svc()
	{
		while (m_bRunning)
		{
			m_semaphore.acquire();
			if(false == m_bRunning)
			{
				break;
			}
			if (m_task)
			{
				m_task->execute();
			}
		}
		return 0;
	};
	void startProcess()
	{
		m_bRunning = true;
		this->activate();
	};
	void stopProcess()
	{
		m_bRunning = false;
		m_semaphore.release();
		this->wait();
	};

protected:
	WorkerTask*				m_task;
	volatile bool			m_bRunning;
	ACE_Thread_Semaphore	m_semaphore;
};
