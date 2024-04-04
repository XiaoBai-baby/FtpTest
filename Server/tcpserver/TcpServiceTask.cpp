#include "TcpServiceTask.h"

TcpServiceTask::TcpServiceTask(OS_TcpSocket client_sock)
	: m_clientSock(client_sock), m_alive(true)
{
	// ����һ���󻺳���, ��Ž��ռ����͵�����
	m_bufsize = 64 * 1024;
	m_buf = new unsigned char[m_bufsize];

	Start();	// �����߳�
}

TcpServiceTask::~TcpServiceTask()
{
	m_clientSock.Close();
	m_alive = false;
	printf("The Service thread has exited (client = %s) ...\n", clientAddress().c_str());
}

// �����߳�
int TcpServiceTask::Start()
{
	// �ͻ�����Ϣ
	m_clientSock.GetPeerAddr(m_clientAddr);
	printf("Got a connect: %s: %d \n", clientAddress().c_str(), m_clientAddr.GetPort());

	m_alive = true;
	Run();
	return 0;
}

// �ж��߳��Ƿ����
bool TcpServiceTask::Alive() const
{
	return m_alive;
}

// ���ؿͻ��˵�IP��ַ
string TcpServiceTask::clientAddress() const
{
	return m_clientAddr.GetIp_str();
}

// ��Ҫ�����������ò�ͬ��ҵ�������;
int TcpServiceTask::Routine()
{
	return BusinessService();
}