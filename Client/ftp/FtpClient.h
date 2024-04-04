#ifndef _FTP_CLIENT_H
#define _FTP_CLIENT_H

#include "./MsgDefine.h"
#include "../osapi/osapi.h"
#include "../utility/utility.h"
#include "../jsoncpp/include/json.h"

/*	FtpClient
	ʵ������ FTP�ķ���
*/

class FtpClient
{
public:
	FtpClient(OS_TcpSocket sock, int bufsize);

public:
	// ��������;
	void SendData(unsigned short type);

	void SendData(unsigned short type, const char* data);

	void SendData(unsigned short type, const char* username, const char* password);

public:
	// ��������;
	void Start();

private:
	// ������Ϣ����;
	int SendMessages(unsigned short type, const void* data, unsigned int length);

	// ��ʾJSON�����Ӧ��;
	void ShowResponse(Json::Value& jsonResponse);

	// �������˵Ĳ���;
	int RequestServer(unsigned short type, const char* username, const char* password);

private:
	bool isWindows;					// �жϿͻ��˵�ϵͳ

	string m_path;

private:
	OS_TcpSocket m_SendSock;		// �ͻ��˵�socket��ַ
	ByteBuffer m_buffer;			// �������ݵ��ֽڱ�����
};

#endif