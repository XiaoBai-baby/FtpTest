#ifndef _OSAPI_SOCKET_H
#define _OSAPI_SOCKET_H

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <string>
using std::string;

#ifdef _WIN32

// Windows �µ�socket����
#include <WinSock2.h>
#include <ws2tcpip.h>

typedef SOCKET socket_t;

#define socket_open socket
#define socket_close closesocket
#define socket_ioctl ioctlsocket

#else

// Linux �µ�socket����
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>

typedef int socket_t;

#define socket_open socket
#define socket_close close
#define socket_ioctl ioctl

// ���¶���Linux�µ�Sleep����;
void Sleep(int ms);

#endif

// Windows �� Linux �µ�socket����
#define socket_accept accept
#define socket_bind bind
#define socket_connect connect
#define socket_listen listen
#define socket_send send
#define socket_recv recv
#define socket_sendto sendto
#define socket_recvfrom recvfrom
#define socket_select select
#define socket_getsockopt getsockopt
#define socket_setsockopt setsockopt

// ���WSA�������;
static int Check_WSAError(int sockopt, const char* function);

/* OS_Socket:
	*) ����һ������, �û�����ֱ�Ӹ�ֵ����;
	*) ����û����������, �û���Ҫ��ʾ�Ĺر�Close();
	*) �û�����ֱ�Ӳ���Socket��hSock;
*/

// �����ŵ�ַ����
// �������ֱ��ǿ��ת��Ϊsockaddr_in �� sockaddr�ṹ
class OS_SockAddr
{
public:
	explicit OS_SockAddr();
	explicit OS_SockAddr(const char* ip, unsigned short port);
	explicit OS_SockAddr(const char* ip);	// Ĭ�϶˿�Ϊ0;
	explicit OS_SockAddr(unsigned short port);	 // Ĭ��IP��ַΪ0.0.0.0
	explicit OS_SockAddr(sockaddr_in addr);

	// ����IP��ַ��˿�;
	void SetIp(const char* ip);
	void SetIp(unsigned int ip);
	void SetIp(unsigned short port);

	string GetIp_str() const;
	unsigned int GetIp_n() const;
	unsigned short GetPort() const;

public:
	sockaddr_in iAddr;		// ���IP��ַ��������Ϣ;
};

class OS_Socket
{
public:
	OS_Socket();

	// ms = 0ʱ��������ʱ, ms = 1 ������Ϊ���������� (1ms�ܿ����)
	int SetOpt_RecvTimeout(int ms);
	int SetOpt_SendTimeout(int ms);

	// ���ؽ����뷢�͵ĳ�ʱʱ��; ��ѡ���Ĭ��ֵΪ�㣬��ʾ�����뷢�Ͳ��������ᳬʱ;
	int GetOpt_RecvTimeout();
	int GetOpt_SendTimeout();

	// ���������׽��ֵ�I/Oģʽ;
	int Ioctl_SetBlockedIo(bool blocked);

	// ���������׽���ѡ��, һ�����������׽��ֽ��պͷ��͵�ϵͳ�����С;
	void Set_SockOption(int optname, const char* bufsize);

	// �����׽��ְ󶨵Ķ˿��Ƿ��ظ�ʹ��; һ������Ϊ"��"����ֹ������򡰾ܾ����񡱹��������ݵ���;
	int SetOpt_ReuseAddr(bool reuse = false);

	// �������׽��ֶԷ��ĵ�ַ;
	int GetPeerAddr(OS_SockAddr& addr) const;

	// �������׽��ֵı��ص�ַ;
	int GetLocalAddr(OS_SockAddr& addr) const;

	// select���ƣ���ѯ��д״̬
	// ����ֵ����0, ��ʾ���Զ���д; ����0��ʾ��ʱ; С��0��ʾSocket�����û����.
	int Select_ForReading(int timeout);
	int Select_ForWriting(int timeout);

protected:
	socket_t hSock;		//�׽���������, ����ֱ�ӷ������socket
};

class OS_TcpSocket : public OS_Socket
{
public:
	// ����Socket�׽���
	int Open(bool reuse = false);
	int Open(const OS_SockAddr& addr, bool reuse = false);

	// �ر�Socket, ���ֶ��ر�;
	void Close();

	// ������
	int Listen(int backlog = 100);
	int Accept(OS_TcpSocket& peer);

	// �ͻ���
	int Connect(const OS_SockAddr& addr);

	// ���ͽ���
	int Send(const void* buf, int len, bool OOB_flag = false);
	int Recv(void* buf, int len, bool OOB_flag = false);
	int Recv_OOB(int OOB_flag = 1);
};

class OS_UdpSocket : public OS_Socket
{
public:
	// ����Socket�׽���
	int Open(bool reuse = false);
	int Open(const OS_SockAddr& addr, bool reuse = false);

	// �ر�Socket, ���ֶ��ر�;
	void Close();

	// ���ͽ���
	int SendTo(const char* buf, int len, const OS_SockAddr& peer);
	int RecvFrom(void* buf, int len, const OS_SockAddr& peer);
};

class OS_McastSock :public OS_Socket
{
public:
	// ����Socket�׽���
	int Open(const char* mcast_ip, int port, const char* local_ip);

	// �ر�Socket, ���ֶ��ر�;
	void Close();

	/* ���Ͷಥʱ, ʹ����ͨUdpSock + �ಥ·�ɼ��� */
	//int SendTo(const char* buf, int len, const OS_SockAddr& peer);
	int RecvFrom(void* buf, int max_len, const OS_SockAddr& peer);

private:
	ip_mreq m_McReq;	//�ಥ��ַ
};

#endif