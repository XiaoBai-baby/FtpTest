#include "TcpTaskMonitor.h"

TcpTaskMonitor* TcpTaskMonitor::i()
{
	static TcpTaskMonitor one;
	return &one;
}

TcpTaskMonitor* TcpTaskMonitor::i(TcpServiceTask* task)
{
	static TcpTaskMonitor one;
	one.monitor(task);

	return &one;
}

TcpTaskMonitor::TcpTaskMonitor()
{
	// startService();
}

TcpTaskMonitor::TcpTaskMonitor(TcpServiceTask* task)
{
	monitor(task);
	// startService();
}

TcpTaskMonitor::~TcpTaskMonitor()
{
	stopService();
}


int TcpTaskMonitor::startService()
{
	m_quitflag = false;
	Run();
	return 0;
}

void TcpTaskMonitor::stopService()
{
	m_quitflag = true;
}

void TcpTaskMonitor::monitor(TcpServiceTask* task)
{
	// ����m_tasksʱ��Ҫʹ��������
	m_Mutex.Lock();
	m_Tasks.push_back(task);
	m_Mutex.Unlock();

	m_Sem.Post();	// ���ź�����ֵ��1
}

int TcpTaskMonitor::Routine()
{
	while (!m_quitflag)
	{
		// ʹ���ź�������, ��������ѭ������ѯ����;
		m_Sem.Wait();	//�ź�����1

		// ����m_tasksʱ��Ҫʹ��������
		m_Mutex.Lock();

		// ����m_tasks���ҵ��Ѿ���ֹ���̲߳�����֮
		for (list<TcpServiceTask*>::iterator iter = m_Tasks.begin();
			iter != m_Tasks.end(); )
		{
			TcpServiceTask* task = *iter;
			if (task->Alive())		// ����̶߳����Ƿ��Ѿ��˳� 
			{
				iter++;
			}
			else
			{
				// �߳��Ѿ���ֹ, ����������߳���Դ
				printf("====Retired TcpServiceTask (client = %s) \n", task->clientAddress().c_str());
				iter = m_Tasks.erase(iter);
				Join(task);
				delete task;
			}
		}

		m_Mutex.Unlock();

		OS_Thread::Msleep(10);
	}

	return 0;
}
