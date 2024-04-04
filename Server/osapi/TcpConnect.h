#ifndef _TCP_CONNECT_H
#define _TCP_CONNECT_H

#include "./osapi.h"

/* TcpConnect:
	��һ���߳���ά��Client - WorkingSocket֮���ͨ������
*/

class TcpConnect : public OS_Thread
{
public:
	TcpConnect(OS_TcpSocket& work_sock);
	~TcpConnect();

public:
	// ���ڽ���ָ�����ȵ�����
	int Recv_SpecifyBytes(void* buf, int count, int timeout = 0);

private:
	// ���ڴ���client�������Ҫ����;
	virtual int Routine();

private:
	OS_TcpSocket m_WorkSock;
};

#endif