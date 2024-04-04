#ifndef _TCP_SERVICE_TASK_H
#define _TCP_SERVICE_TASK_H

#include "../osapi/osapi.h"
#include "../utility/utility.h"

/* �����õķ����߳�
  �����ҵ����
*/

class TcpServiceTask : public OS_Thread
{
public:
	TcpServiceTask(OS_TcpSocket client_sock);
	~TcpServiceTask();

public:
	// �����߳�
	int Start();

	// �ж��߳��Ƿ����
	bool Alive() const;

	// ��ȡ�ͻ���IP��ַ
	string clientAddress() const;

private:
	// ����ҵ�������;
	int BusinessService();

private:
	// ���ڴ����̵߳���Ҫ����;
	int Routine();


private:
	unsigned char* m_buf;		// ���ݻ�����
	int m_bufsize;				// ��������С

private:
	OS_TcpSocket m_clientSock;		// ���ӿͻ��˵�socket��ַ
	OS_SockAddr m_clientAddr;		// �ͻ��˵�IP��ַ
	bool m_alive;					// ��ʶ�߳��Ƿ����
};

#endif