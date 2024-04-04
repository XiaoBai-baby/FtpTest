#include "osapi/osapi.h"
#include "ftp/FtpClient.h"
#include "utility/utility.h"

int main()
{
	// ����socket
	OS_TcpSocket client_sock;
	client_sock.Open();

	// ��������IP��ַ
	const char* server_ip = "127.0.0.1";
	int port = 8888;

	// ���ӷ�����
	if (client_sock.Connect(OS_SockAddr(server_ip, port)) < 0)
	{
		printf("Cannot connect to the server (%s: %d) !", server_ip, port);
		return -1;
	}
	
	// �÷�װ�õ����ݰ���������
	FtpClient ftp(client_sock, 1024);
	// ftp.SendData(MSG_LOGIN, "root", "123456");

	string user;
	user.append("��ss1_ ");
	user.append("123456");
	ftp.SendData(MSG_LOGIN2, user.c_str());

	ftp.Start();

	// �ر�socket
	client_sock.Close();

	return 0;
}
