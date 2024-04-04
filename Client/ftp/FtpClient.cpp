#include "./FtpClient.h"

FtpClient::FtpClient(OS_TcpSocket sock, int bufsize) : m_SendSock(sock), m_buffer(bufsize), isWindows(false)
{
	// �ж��Ƿ�Ϊ Windowsϵͳ;
#ifdef _WIN32
	isWindows = true;
#endif

	// ע��, m_buffer ֻ����CustomTcpSend�й���; 
	// ������� delete_scalar.cpp �г����ڴ���ʴ���������ϵ� ��Խ��д�����, ���¶��ڴ汻�ƻ�;
}

void FtpClient::SendData(unsigned short type)
{
	SendData(type, "", "");
}

void FtpClient::SendData(unsigned short type, const char* data)
{
	SendData(type, data, "");
}

void FtpClient::SendData(unsigned short type, const char* username, const char* password)
{
	try {
		RequestServer(type, username, password);
	}
	catch (string e)
	{
		printf("%s \n", e.c_str());
	}
}


void FtpClient::Start()
{
	char cmdline[256];
	char cmdline2[256];

	const char* root = "[root@localhost";
	const char* root2 = "]#  ";

	int count = 0;
	while (true)
	{
	#ifndef _WIN32
		if (count == 0)	system("pause");
	#endif

		cout << root << m_path << root2;

	#ifdef _WIN32
		gets_s(cmdline);
	#else
		fgets(cmdline, strlen(cmdline), NULL);
	#endif
		count++;

		if (strlen(cmdline) == 0) continue;

		strcpy(cmdline2, cmdline);
		char* argv[64];
		int argc = FileUtils::Split(cmdline, argv);
		
		string cmd = argv[0];
		if (cmd == "ls")
		{
			if (argc > 1)
				SendData(MSG_LIST, cmdline2);
			else
				SendData(MSG_LIST);

			continue;
		}
		else if (cmd == "ll")
		{
			if (argc > 1)
				SendData(MSG_LIST2, cmdline2);
			else
				SendData(MSG_LIST2);

			continue;

		}
		else if (cmd == "cd")
		{
			if (argc > 1)
				SendData(MSG_CD, cmdline2);
			else
				SendData(MSG_CD, cmdline);

			continue;
		}
		else if (cmd == "help")
		{
			SendData(MSG_HELP);
			continue;
		}
		else if (cmd == "exit")
		{
			SendData(MSG_EXIT);
			break;
		}
		else
		{
			printf("input cmdline error ! \n");
			continue;
		}

		OS_Thread::Msleep(300);
	}

#ifndef _WIN32
	if (count == 0)	system("pause");
#endif

	printf("Exit successfully . \n");
}

// ����ֵ: <=0 ����ʧ��(socket�����Ѿ��Ͽ�), >0 ���ͳɹ�;
// length: �����ֽڵĳ��ȣ�����Ϊ0;
int FtpClient::SendMessages(unsigned short type, const void* data, unsigned int length)
{
	// ������Ϣ����
	// m_SendSock.Send(&type, 8);
	// m_SendSock.Send(&length, 8);

	// ������Ϣ����
	m_buffer.putUnit16(type);
	m_buffer.putUnit16(isWindows);

	m_buffer.putUnit32(length);			// ��˴���
	m_SendSock.Send(m_buffer.Position(), 8, false);		// �ȷ���8���ֽڵ�����, ����ָ�������Ϣ�ı�ͷ
	m_buffer.Clear();

	// ���ݲ�����0���ֽ����˳�
	if (length <= 0) return 0;

	// ���������Ϣ
	return m_SendSock.Send(data, length, false);
}

void FtpClient::ShowResponse(Json::Value& jsonResponse)
{
	int errorCode = jsonResponse["errorCode"].asInt();
	string reason = jsonResponse["reason"].asString();
	bool isWin = jsonResponse["isWindows"].asBool();
	string result = jsonResponse["result"].asString();
	
	char buf[1024 * 6];					// ����Ӧ��Ļ���;
	int response_length = 0;			// ����Ӧ��ĳ���;
	string character;					// ����ת���Ľ��;

	// ��ʾӦ����;
	switch (errorCode)
	{
	case 11:
		// ��ͬϵͳ֮��ʹ�ö��ν�������;
		response_length = m_SendSock.Recv(buf, 1024 * 6, false);
		buf[response_length] = 0;			// ����ַ�������ֹ��

		// ��ͬϵͳ֮��ı���ת��;
		if (isWindows != isWin)
		{
		#ifdef _WIN32
			character = Utf8ToGbk(buf);
		#else
			GbkToUtf8((char*)buf, response_length, (char*)character.c_str(), character.length());
		#endif
			throw string(character);
		}
		else
		{
			throw string(buf);
		}
		break;

	case 12:
		if (reason == "OK")
		{
			if (result.size() > 0)
				m_path = " " + result;
			else
				m_path = result;
		}
		else
		{
			throw string(result);
		}
		break;

	case 0:
		throw string(result);
		break;

	default:
		throw string("reason: " + reason);
		throw string("result:" + result);
		break;
	}

}

int FtpClient::RequestServer(unsigned short type, const char* username, const char* password)
{
	// ����JSON����
	string jsonreq;
	if (strcmp(password, "") != 0)
	{
		Json::Value jsonRequest;
		Json::FastWriter writer;
		jsonRequest["username"] = username;
		jsonRequest["password"] = password;
		jsonreq = writer.write(jsonRequest);
	}
	else
	{
		jsonreq = username;
	}
	
	// ���������֤;
	SendMessages(type, jsonreq.c_str(), jsonreq.length());

	

	// ����JSON�����Ӧ��
	char buf[1024];					// ��������Ӧ��Ļ���;
	int response_length = 0;

	// �Ƚ���4���ֽڵ�����, ����ָ������;
	m_SendSock.Recv(&response_length, 4, false);
	response_length = m_SendSock.Recv(buf, response_length, false);			//��������
	if (response_length <= 0)
	{
		throw string("failed to recv response! login failed! \n");
	}

	buf[response_length] = 0;			// ����ַ�������ֹ��



	// ����JSON�����Ӧ��
	string response(buf, response_length);
	Json::Reader jsonReader;
	Json::Value jsonResponse;
	if (!jsonReader.parse(response, jsonResponse, false))
		throw string("bad json format! \n");

	ShowResponse(jsonResponse);				// ��ʾӦ��

	return 0;
}
