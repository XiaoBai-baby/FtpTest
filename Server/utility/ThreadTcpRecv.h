#ifndef _THREAD_TCP_RECV_H
#define _THREAD_TCP_RECV_H

#include "../osapi/osapi.h"
#include "./ByteBuffer.h"

/* ThreadTcpRecv �߳̽�����
	���ڶ࿪һ���߳��������������, �����ڶ��̡߳��߲����ĳ���;
*/

class ThreadTcpRecv : public OS_Thread
{
public:
	ThreadTcpRecv(OS_TcpSocket& recv_sock, int bufsize);

	ThreadTcpRecv(OS_TcpSocket& recv_sock, char* buffer, int offset, int bufsize);

	~ThreadTcpRecv();

public:
	// �����������ݵ��̷߳���;
	void StartReceiveData(void* head_buf, int head_size);

public:
	// ��������, �汾1.0; ������, ʱ��BUG;
	int ReceiveData(void* buf, unsigned int bufsize, int timeout = 2000);		// ��������һ�����ճ�ʱ����������

	// ��������, �汾2.0; �����ٶȿ�, ����BUG;
	int ReceiveData2(void* buf, unsigned int bufsize, int timeout = 6000);

	// �������ݵĸ����ӳ�, �汾1.0ʱʹ��;
	void DelayReceive(unsigned int numberSize);

	// ��������, ����ҵ������
	void ParseData();

private:
	// �̺߳���;
	virtual int Routine();

protected:
	int m_bufsize;		// ���ջ�������С

protected:
	OS_TcpSocket m_RecvSock;		// ���ӿͻ��˵�socket��ַ
	OS_Semaphore m_Sem;				// ���ƽ��ա��������ݵ��ź���, �汾1.0ʱʹ��;
	ByteBuffer m_buffer;			// �������ݵ��ֽڱ�����
};

#endif