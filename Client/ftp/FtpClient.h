#ifndef _FTP_CLIENT_H
#define _FTP_CLIENT_H

#include "./MsgDefine.h"
#include "./FtpFileHandler.h"
#include "../osapi/osapi.h"
#include "../utility/utility.h"
#include "../jsoncpp/include/json.h"

#include <vector>
using std::vector;

/*	FtpClient
	实现类似 FTP的服务
*/
class FtpClient
{
public:
	FtpClient(OS_TcpSocket sock, int bufsize);

public:
	// 发送任务;
	void SendData(unsigned short type);

	void SendData(unsigned short type, const char* data);

	void SendData(unsigned short type, const char* username, const char* password);

public:
	// 启动服务;
	void Start();
	
	// 检查 get 命令;
	int getCheck(string homeDir, char* argv[128], int out_argc);

	// 发送 get 命令;
	void in_get(string cmdline, char* argv[128], int argc);

	// 发送 put 命令;
	void in_put(string cmdline, char* argv[128], int argc);

private:
	// 发送消息类型;
	int SendMessages(unsigned short type, const void* data, unsigned int length);

	// 显示并处理 服务端 请求的应答;
	void ShowResponse(Json::Value& jsonResponse);

	// 所有服务类型的响应;
	int ServiceRequest(Json::Value& jsonResponse);

	// 请求服务端的操作;
	int RequestServer(unsigned short type, const char* username, const char* password);

private:
	string m_homeDir;					// 下载文件的根目录
	string m_path;						// 下载文件的相对目录

	char client_system;					// 判断客户端的系统
	char server_system;					// 判断服务端的系统

private:
	bool is_login;						// 判断登录成功
	string m_cmdline;					// 保存命令行的命令

private:
	OS_TcpSocket m_SendSock;			// 客户端的socket地址
	ByteBuffer m_buffer;				// 缓存数据的字节编码器
	FileHandler m_file;					// 操作文件的文件处理器
};

#endif