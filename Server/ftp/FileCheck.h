#ifndef _FILE_CHECK_H
#define _FILE_CHECK_H

#include "MsgDefine.h"
#include "../osapi/osapi.h"
#include "../utility/FileUtils.h"
#include "../jsoncpp/include/json.h"
#include "../utility/CharacterEncoding.h"

// STL ģ��
#include <string>
#include <fstream>
#include <algorithm>
using std::string;

#ifdef _WIN32

#include <io.h>
#include <direct.h>

#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#endif

/* FileCheck
	�ļ������, ���ڸ��� FileHandler�� �����ļ�;
*/
class FileCheck
{
public:
	FileCheck();

	FileCheck(OS_TcpSocket& Sock, string homeDir, bool isWin, bool isClient = false);

public:
	// ��ģ�庯�� ������Ķ��γ�ʼ��, ��ȷ�Ͽͻ��˵Ĳ���ϵͳ;
	void operator()(bool is_windows);

	// ��ģ�庯�� ������Ķ��γ�ʼ��, ��ָ�������ļ������Ŀ¼;
	void operator()(const char* homeDir);

	// ��ģ�庯�� ������Ķ��γ�ʼ��, ��ָ�������ļ������Ŀ¼;
	void operator()(string homeDir);

	// ��ģ�庯�� ������ĳ�ʼ��;
	void operator()(OS_TcpSocket& Sock, string homeDir, bool is_win, bool isClient);

public:
	// ����MSG_LIST ��Ϣ����, ���� CheckResultʹ��;
	string on_ls(Json::Value& jresult, string& path, bool notDir = false);

	// ����MSG_LIST2 ��Ϣ����, ���� CheckResultʹ��;
	string on_ll(Json::Value& jresult, string& path, bool notDir = false);

	// ��Linux��, ����MSG_LIST, MSG_LIST2 ��Ϣ����, ���� on_lsʹ��;
	string ls_linux(unsigned int type, Json::Value& jresult, string& path);

private:
	// ��� cd ����, ���� CheckCommandʹ��;
	void CheckCdCommand(string& result, string& path, string& part);

	// ��������е�����, ���� CheckFileʹ��;
	void CheckCommand(string& result, string& path, string& part);

	// ��ʾ����ļ���Ŀ¼�Ľ��, ���� CheckFileʹ��;
	string CheckResult(Json::Value& jresult, string path, string part);

	// �ж��ļ���Ŀ¼�Ƿ����, ���� CheckFileʹ��;
	string IsExistFile(string path, string part);

public:
	// �ж��Ƿ�ΪĿ¼, ���� CheckFile UploadFileʹ��;
	bool IsDirectory(string path, string part);

public:
	// ����ļ���Ŀ¼ �Ƿ����;
	string CheckFile(string& path, char* data, unsigned int type);

public:
	bool isWin;							// �жϷ���˵�ϵͳ
	bool isWindows;						// �жϿͻ��˵�ϵͳ
	bool isClient;						// �ж��Ƿ�Ϊ�ͻ���

public:
	string m_homeDir;					// ��Ŀ¼���ڵ�λ��
	unsigned int m_type;				// ������Ϣ������

private:
	OS_TcpSocket Sock;					// ���ӿͻ��˵�socket��ַ
};

#endif