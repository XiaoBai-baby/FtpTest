#include "./TcpConnect.h"

TcpConnect::TcpConnect(OS_TcpSocket& work_sock)
{
	m_WorkSock = work_sock;
}

TcpConnect::~TcpConnect()
{

}

// count: ׼�����ն����ֽ�
// timeout: ���Ϊ0����ʾ�����ȴ������򣬱�ʾ���ý��ճ�ʱ
int TcpConnect::Recv_SpecifyBytes(void* buf, int count, int timeout)
{
	// ���ó�ʱ
	if (timeout > 0)
	{
		m_WorkSock.SetOpt_RecvTimeout(timeout);
	}

	// ������������, ֱ������ָ�����ֽ���;
	int bytes_got = 0;
	while (bytes_got<count)
	{
		int n = m_WorkSock.Recv((char*)buf + bytes_got, count - bytes_got, false);
		if (n <= 0)
		{
			return bytes_got;
		}

		bytes_got += n;
	}

	return bytes_got;
}

int TcpConnect::Routine()
{
	// Ϊclient�ṩ����
	unsigned char buf[1024];

	m_WorkSock.Recv(buf, 1024, false);

	OS_Thread::Msleep(3000);

	// Ӧ��ͻ�
	const char* ret = "send seccusssfully ...";
	m_WorkSock.Send(ret, strlen(ret), false);

	// �ر�socket
	m_WorkSock.Close();

	return 0;
}