#ifndef _FTP_SERVICE_H
#define _FTP_SERVICE_H

#include "./MsgDefine.h"
#include "../osapi/osapi.h"
#include "../utility/utility.h"
#include "../jsoncpp/include/json.h"

#include <fstream>
#include <algorithm>
using namespace std;

#ifdef _WIN32
#include <io.h>

#else
#include <unistd.h>
#include <fcntl.h>

#endif

/*	FtpService
	ʵ������ FTP�ķ���
*/

class FtpService
{
public:
	FtpService(OS_TcpSocket& recv_sock, int bufsize = 8192);

	FtpService(OS_TcpSocket& recv_sock, char* buffer, int offset, int bufsize);

	~FtpService();

public:
	// ��ʼ��������;
	void startReceiveData();

	// �������;
	void Clear();

private:
	// ������Ϣ����;
	int ReceiveMessages();

	// ����MSG_LOGIN ��Ϣ����;
	int on_Login(const string& jsonreq);

	// ����MSG_LOGIN2 ��Ϣ����, �������Ĵ���;
	int on_Login2();

	// ����MSG_LIST ��Ϣ����;
	string on_ls(Json::Value& jresult, bool notDir = false);

	// ����MSG_LIST2 ��Ϣ����;
	string on_ll(Json::Value& jresult, bool notDir = false);

	// ����MSG_CD ��Ϣ����;
	string on_cd(Json::Value& jresult);

private:
	// ��ʾ����ļ���Ŀ¼�Ľ��, ���� CheckFileʹ��;
	string CheckResult(Json::Value& jresult, string& path);

	// ����ļ���Ŀ¼ �Ƿ����;
	string CheckFile();

	string LinuxHandler();

	// ������Ϣ���͵Ĵ���;
	int MessageHandler(string& result, string& reason, Json::Value file_block);

	// ��Ӧ�ͻ��˲���;
	int ResponseClient();

private:
	// ��������
	int ReceiveN(void* buf, int count, int timeout = 0);

private:
	string m_homeDir;				// ��Ŀ¼���ڵ�λ��
	string m_path;					// �û������Ŀ¼

	string username;				// �ͻ��˵��û���
	string password;				// �ͻ��˵�����

private:
	bool login_OK;					// ��¼�Ƿ�ɹ�
	bool exit_OK;					// �˳��Ƿ�ɹ�
	
	bool isWindows;					// �жϿͻ��˵�ϵͳ

private:
	char* m_data;					// ������Ϣ������
	unsigned int m_type;			// ������Ϣ������
	unsigned int m_length;			// ������Ϣ�����ݳ���

private:
	int m_bufsize;					// ���ջ�������С
	ByteBuffer m_buffer;			// �������ݵ��ֽڱ�����

private:
	OS_Mutex m_Mutex;				// ���ƽ������ݵĻ�����
	OS_TcpSocket m_RecvSock;		// ���ӿͻ��˵�socket��ַ
	OS_SockAddr m_SockAddr;			// �ͻ��˵�IP��ַ
};

#endif