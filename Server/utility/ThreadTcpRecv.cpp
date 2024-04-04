#include "ThreadTcpRecv.h"

// ���ƽ��ա��������ݵĻ�����;
// ������������ȫ�ֱ���, ��Ϊ�������ݺ��������̲߳���, 
namespace ThreadTcpReceive
{
	OS_Mutex m_Mutex;
}

ThreadTcpRecv::ThreadTcpRecv(OS_TcpSocket& recv_sock, int bufsize)
	: m_RecvSock(recv_sock), m_buffer(bufsize), m_Sem(0)
{
	m_bufsize = bufsize;
}

ThreadTcpRecv::ThreadTcpRecv(OS_TcpSocket& recv_sock, char* buffer, int offset, int bufsize)
	: m_RecvSock(recv_sock), m_buffer(buffer, offset, bufsize), m_Sem(0)
{
	m_bufsize = bufsize;
}

ThreadTcpRecv::~ThreadTcpRecv()
{
	Join(this);
}


void ThreadTcpRecv::StartReceiveData(void* head_buf, int head_size)
{
	// �������ݱ�ͷ
	m_buffer.loadHeader(head_buf, head_size);

	Run();
}

// buf: ��ʾ�������ݵĻ�����
// timeout: ���Ϊ0����ʾ�����ȴ������򣬱�ʾ���ý��ճ�ʱ
int ThreadTcpRecv::ReceiveData(void* buf, unsigned int total, int timeout)
{
	// ���ó�ʱ
	if (timeout > 0)
	{
		m_RecvSock.SetOpt_RecvTimeout(timeout);
	}

	// �������ݹ������쳣����
	if (total > m_bufsize)
	{
		printf("ReceiveData2 function error: send data is too long ! \n");
		return -12;
	}
	
	int bytes_got = 0;		// �������������ݺ���ֽ�ƫ����; 
	int sendCount = m_buffer.sendCount();		// ���͵����ݶθ���;


	// ������������, ֱ������ָ�����ֽ���;
	for (int i = 0; i < sendCount; i++)
	{
		ThreadTcpReceive::m_Mutex.Lock();

		// ������������ֱ�� Post();
		if (bytes_got >= m_buffer.Total())
		{
			m_Sem.Post();

			continue;
		}

		// byte_got: �����ݵ��ֽ�ƫ����
		int byte_got = 0;
		int count = *(m_buffer.Individual() + i);

		// �������һ������ʱ, ��������󼸱�������������, �˺������쳣����Ҳ��������;
		if (i == sendCount - 1)
		{
			count = count * 4;		// ��count * 4 �ĳ� count ���������ڴ���ʳ�ͻ;
		}

		while (byte_got < count)
		{
			// ע��, �����Recv()����ϵͳ�Դ���RecvBuf��������������, ���ǿͻ��˱���;
			int n = m_RecvSock.Recv((char*)buf + bytes_got, count - byte_got, false);
			if (n <= 0)
			{
				break;		// ���û�����ݾͽ���ѭ��;
			}
			else if (n >= count && i != sendCount)
			{
				m_Sem.Post();		// ��ֹ�쳣, ÿ�յ�һ�����ݾ�ֱ�� Post();
			}
			else
			{
				m_Sem.Post();	// �������һ������ʱ, ĩβ�ټ�һ��Post();
			}

			byte_got += n;
			bytes_got += byte_got;
		}

		ThreadTcpReceive::m_Mutex.Unlock();
	}

	return bytes_got;
}

int ThreadTcpRecv::ReceiveData2(void* buf, unsigned int total, int timeout)
{
	// ���ó�ʱ
	if (timeout > 0)
	{
		m_RecvSock.SetOpt_RecvTimeout(timeout);
	}

	// �������ݹ������쳣����
	if (total > m_bufsize)
	{
		printf("ReceiveData2 function error: send data is too long ! \n");
		return -12;
	}

	// ������������, ֱ������ָ�����ֽ���;
	int bytes_got = 0;
	while (bytes_got < total)
	{
		ThreadTcpReceive::m_Mutex.Lock();
		int n = m_RecvSock.Recv((char*)buf + bytes_got, total - bytes_got, false);
		ThreadTcpReceive::m_Mutex.Unlock();
		if (n <= 0)
		{
			continue;
		}

		bytes_got += n;
	}

	return bytes_got;	// ���ؽ������ݵĴ�С;
}

void ThreadTcpRecv::DelayReceive(unsigned int numberSize)
{
	if (numberSize > 1024 * 1024 * 1024)
	{
		OS_Thread::Sleep(3);		// �ӳ�3��;
	}
	else if (numberSize > 1024 * 1024)		// ����������֮ǰ���Լ����ӳ�, ����ֹ����ջ��������������������;
	{
		OS_Thread::Sleep(1);		// �ӳ�1��;
	}
	else if (numberSize > 1024)
	{
		OS_Thread::Msleep(200);		// �ӳ�200����;
	}

	m_Sem.Wait();		// �ź�����1
}

void ThreadTcpRecv::ParseData()
{
	// OS_Thread::Msleep(200);		// ���������һ��200�������ϵ��ӳ�, ��Ϊ�̴߳�������ͨ������;

	int sendCount = m_buffer.sendCount();		// ���͵����ݶθ���;

	// ��ʼ��������;
	for (int i = 0; i < sendCount; i++)
	{
		ThreadTcpReceive::m_Mutex.Lock();

		bool isNumber = *(m_buffer.isNumber() + i);

		unsigned int numberSize = *(m_buffer.Individual() + i);

		// DelayReceive(numberSize);
		// m_Sem.Wait();		// �汾1.0ʱʹ��
		
		if (isNumber)	// ��������;
		{
			switch (numberSize)
			{
			case 1:
				printf("get number: %d \n", m_buffer.getUnit8());
				break;
			case 2:
				printf("get number: %d \n", m_buffer.getUnit16());
				break;
			case 4:
				printf("get number: %d \n", m_buffer.getUnit32());
				break;
			case 8:
				printf("get number: %lld \n", m_buffer.getUnit64());	// long long
				break;
			default:
				break;
			}
		}
		else
		{
			printf("get string: %s \n", m_buffer.getString().c_str());
		}

		ThreadTcpReceive::m_Mutex.Unlock();
	}

}

int ThreadTcpRecv::Routine()
{
	return ReceiveData2(m_buffer.Position(), m_buffer.Total());
}