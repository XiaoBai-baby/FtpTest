#ifndef _TCP_SERVER_H
#define _TCP_SERVER_H

#include "../osapi/osapi.h"

/*
	TCP����������
*/

class TcpServer : public OS_Thread
{
public:
	TcpServer();
	TcpServer(int port);

	~TcpServer();

	// ����TCP����
	int startService(int port);

	// �ر�TCP����
	void stopService();

private:
	// ���ڴ���client�������Ҫ����;
	virtual int Routine();

private:
	OS_TcpSocket m_WorkSock;		// �����socket��ַ
	bool m_quitflag;				// �����̵߳��˳�
};

#endif