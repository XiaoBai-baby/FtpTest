#ifndef _TCP_TASK_MONITOR_H
#define _TCP_TASK_MONITOR_H

#include "../osapi/osapi.h"
#include "TcpServiceTask.h"
#include <list>
using namespace std;

/*
   ����TcpServiceTaskҵ���������
   ��Ŀ���߳���ֹʱ�����ո��̵߳���Դ
*/

class TcpTaskMonitor : public OS_Thread
{
public:
	// ����ģʽ, ���ڼ��������ʱʹ��;
	static TcpTaskMonitor* i();
	static TcpTaskMonitor* i(TcpServiceTask* task);

public:
	// ����ģʽ, ����������;
	TcpTaskMonitor();
	TcpTaskMonitor(TcpServiceTask* task);

	~TcpTaskMonitor();

public:
	// ���������߳�;
	int startService();

	// �رռ����߳�;
	void stopService();

	// ���ҵ����
	void monitor(TcpServiceTask* task);

private:
	// ���ڴ����̵߳���Ҫ����;
	virtual int Routine();

private:
	list<TcpServiceTask*> m_Tasks;		// ���ҵ�����˫������
	bool m_quitflag;					// �����̵߳��˳�
	OS_Mutex m_Mutex;					// ��������Ļ�����
	OS_Semaphore m_Sem;					// ����������ź���
};

#endif