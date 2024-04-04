#include "TcpServer.h"
#include "TcpServiceTask.h"
#include "TcpTaskMonitor.h"

TcpServer::TcpServer()
{
	// startService(8888);
}

TcpServer::TcpServer(int port)
{
	startService(port);
}

TcpServer::~TcpServer()
{
	stopService();
}

int TcpServer::startService(int port)
{
	if (m_WorkSock.Open(OS_SockAddr(port), true) < 0)
	{
		// �˶˿ڲ�����
		return -11;
	}

	if (m_WorkSock.Listen() < 0)
	{
		return -22;
	}

	// ������������
	TcpTaskMonitor::i()->startService();
	printf("Server listening on : %d \n", port);
	
	// �����߳�
	m_quitflag = false;
	Run();
	return 0;
}

void TcpServer::stopService()
{
	m_quitflag = true;
	m_WorkSock.Close();
	Join(this);
}

int TcpServer::Routine()
{
	printf("Server working . \n");

	while (!m_quitflag)
	{
		// ���ڴ���Working��Socket, ÿ��client��Ӧһ��Socket;
		OS_TcpSocket work_sock;
		if (m_WorkSock.Accept(work_sock) < 0)
		{
			break;
		}

		printf("----------------------------- \n\n");

		// ����ҵ������߳�
		TcpServiceTask* task = new TcpServiceTask(work_sock);
		TcpTaskMonitor::i()->monitor(task);		// ��taskҵ���̼߳�����չ�����
	}

	printf("Server exit . \n");
	return 0;
}