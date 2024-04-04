#include "FtpService.h"

// �ж��Ƿ�Ϊ Windowsϵͳ;
#ifdef _WIN32
bool isWin = true;
#else
bool isWin = false;
#endif

FtpService::FtpService(OS_TcpSocket& recv_sock, int bufsize)
	: m_RecvSock(recv_sock), m_buffer(bufsize), m_bufsize(bufsize)
{
	m_data = 0;
	m_type = 0;
	m_length = 0;

	exit_OK = false;
	login_OK = false;

	m_homeDir = "E:/CProjects";			// ftp��Ŀ¼
	m_path = "/";						// ��ʼĿ¼
	
	/// m_buffer ������CustomTcpRecv�й���; 
	// Ϊ�˷�ֹ�� delete_scalar.cpp ���ڳ��ִ���ʴ���������ϵ� ��Խ��д�����, ���¶��ڴ汻�ƻ�;
}

FtpService::FtpService(OS_TcpSocket& recv_sock, char* buffer, int offset, int bufsize)
	: m_RecvSock(recv_sock), m_buffer(buffer, offset, bufsize), m_bufsize(bufsize)
{
	m_data = 0;
	m_type = 0;
	m_length = 0;
	
	exit_OK = false;
	login_OK = false;

	m_homeDir = "E:/CProjects";			// ftp��Ŀ¼
	m_path = "/";						// ��ʼĿ¼
}

FtpService::~FtpService()
{
	Clear();
}


void FtpService::startReceiveData()
{
	ResponseClient();
}

void FtpService::Clear()
{
	m_type = 0;
	m_length = 0;
}


// ����ֵ: >=0ʱ����ʾ���յ������ݳ��� (����Ϊ0��
//	, <0ʱ����ʾ���ճ���
int FtpService::ReceiveMessages()
{
	// ����ͷ����Ϣ
	if (8 != ReceiveN(m_buffer.Position(), 8))
	{
		return -11;		// ������Ϣ����;
	}

	// ָ��ͷ����Ϣ
	this->m_type = m_buffer.getUnit16();
	this->isWindows = m_buffer.getUnit16();

	this->m_length = m_buffer.getUnit32();			// JSON��ʽ�Ĵ�С;
	m_buffer.Clear();

	// ���ݲ�����0���ֽ����˳�
	if (m_length <= 0)
		return 0;

	m_data = new char[m_length];

	// �������ݲ���
	int n = ReceiveN(m_data, m_length);
	if (n != m_length)
	{
		return -13;		// �������ݲ��ֳ���;
	}

	m_data[m_length] = 0;		// ����ַ�������ֹ��

	return n;
}

int FtpService::on_Login(const string& jsonreq)
{
	// ����JSON����
	Json::Reader reader;
	Json::Value req;
	if (!reader.parse(jsonreq, req, false))
		throw string("bad json format! \n");

	this->username = req["username"].asString();
	this->password = req["password"].asString();
	if (username != "root")
		throw string("bad username!");
	if (password != "123456")
		throw string("bad password!");

	return 0;
}

int FtpService::on_Login2()
{
	char* argv[64];
	string user(m_data);
	int argc = FileUtils::Split((char*)user.c_str(), argv);
	
	if (argc < 2)
		return -1;

	this->username = argv[0];
	this->password = argv[1];

	// ��ͬϵͳ֮��ı���ת��;
	if (isWindows != ::isWin)
	{
		#ifdef _WIN32
			this->username = Utf8ToGbk(username.c_str());
		#else
			string name;
			GbkToUtf8((char*)username.c_str(), username.length(), (char*)name.c_str(), name.length());
			this->username = name;
		#endif
	}

	if (username != "��ss1_")
		throw string("bad username!");
	if (password != "123456")
		throw string("bad password!");

	return 0;
}


// notDir Ϊ�����ļ��Ĵ��� (��Ŀ¼�ļ�);
// msg_list2 Ϊfalse, ��˺������ڴ��� on_ls��Ϣ����; Ϊtrue���� on_ll��Ϣ����;
#ifdef _WIN32
string FtpService::on_ls(Json::Value& jresult, bool notDir)
{	
	// ��������
	// printf("ls %s%s ...\n", m_homeDir.c_str(), m_path.c_str());
	jresult.clear();		// ��ֹ�ظ���ʾ

	// �ж���Ϣ
	bool msg_list2 = (m_type == MSG_LIST2) ? true : false;

	// �����ļ���Ϣ
	string result;
	FileEntryList files = FileUtils::List(m_homeDir + m_path, msg_list2, notDir);
	for (FileEntryList::iterator iter = files.begin();
		iter != files.end(); iter++)
	{
		FileEntry& entry = *iter;
		Json::Value jobj;
		jobj["fileName"] = entry.fileName;
		jobj["isDir"] = entry.isDirectory;
		jobj["fileSize"] = (double)entry.fileSize;				// ��Linux��, jsoncpp���ܴ���long��������;
		jobj["fileTime"] = (double)entry.filetime;
		jobj["fileMode"] = entry.fileMode;
		jresult.append(jobj);
	}
	
	result = FileUtils::List_Result(jresult, result);

	return result;
}

// notDir Ϊ�����ļ��Ĵ��� (��Ŀ¼�ļ�);
string FtpService::on_ll(Json::Value& jresult, bool notDir)
{
	string result;
	on_ls(jresult, notDir);
	result = FileUtils::List2_Result(jresult, result);

	return result;
}
#endif

string FtpService::on_cd(Json::Value& jresult)
{
	string result;
	string cmdline(m_data);

	char* argv[64];
	int argc = FileUtils::Split((char*) cmdline.c_str(), argv);
	if (argc > 1)
	{
		result = CheckFile();
		if (result.length() <= 0)
		{
			m_path += argv[1];				// ֻ�õ�һ��������, ����������Ч;

			int find_pos = 0;				// ���� "."�� ".."��λ��; 
			int count = 0;					// ���ҵ��� ".." �ַ�������;
			int argv[128] = {0};			// ������� ".." �ַ������±�;
			bool isII = false;				// ����ȷ���������Ƿ���� ".."��ֵ;


			// ��m_path �е� "." �ַ���ɾ��;
			do
			{
				find_pos = m_path.find("/./", find_pos);
				
				if(find_pos >= 0)
					m_path.erase(find_pos, 2);
			} while (find_pos > 0);

			// �ٽ�ĩβ�� "." �ַ���ɾ��;
			find_pos = m_path.length();
			if(m_path[find_pos - 2] == '/' && m_path[find_pos - 1] == '.')
				m_path.erase(find_pos - 2, 2);

			// ���� ".." �ַ�����λ��;
			do
			{
				// find_last_of �ǴӺ���ǰ���������ַ������ַ�, �ɹ��ҵ��򷵻��ַ��������һ���ַ�;
				find_pos = m_path.find_last_of("..", find_pos);
				if (find_pos < 0) break;

				isII = true;
				argv[count] = find_pos - 2;
				find_pos -= 3;
				count++;

			} while (true);
			
			// �ж϶�������ʱ, �Ƿ���� ".." �ַ���;
			int dir = m_path.find_last_of('/', m_path.length() - 2);
			if (dir == argv[0])
				dir = -1;

			// ��ȡ���������е�Ŀ¼��;
			string dir2;				// ���������Ŀ¼��;
			string dir3;				// ������������һ��Ŀ¼��;
			if (count > 0)
			{
				if (dir > 0)
				{
					dir2 = m_path.substr(argv[0] + 3);				// ȡ�����һ�� ".."֮����ַ���;

					// ���dir2Ŀ¼�е����һ��Ŀ¼;
					int dir33 = dir2.find_last_of('/', dir2.length() - 2);
					dir3 = dir2.substr(dir33, dir2.length());
				}

				m_path = m_path.substr(0, argv[count - 1]);			// m_path ����".."֮ǰ���ַ���;
			}
			
			// �� m_path ������һ����Ŀ¼;
			while (count > 0)
			{
				int cd_pos = m_path.find_last_of('/', m_path.length());
				m_path.erase(cd_pos);

				count -= 1;
			}

			// ɾ��ĩβ��'/', �����������;
			int length = m_path.length();
			if (length > 0)
			{
				if(m_path[length - 1] == '/')
					m_path.erase(length - 1, 1);
			}

			// ��Ŀ¼���ӵ� m_path����, ������Ŀ¼��;  
			if (dir > 0 && isII)
			{
				result = dir3;

				// ���Ŀ¼��;
				m_path += dir2;

				// ɾ��resultĩβ��'/';
				length = result.length();
				if (result[length - 1] == '/')
					result.erase(length - 1, 1);
			}
			else				// ������û��".." �ַ����Ĵ���;
			{
				result = m_path;

				// �������һ��Ŀ¼��;
				int cd_pos = m_path.find_last_of('/', result.length());
				if(cd_pos >= 0)
					result = result.substr(cd_pos, result.length());
			}


			// ɾ��result���׵�'/';
			if(result.size() > 0)
				result.erase(0, 1);

			m_path += '/';
		}
	}
	else
	{
		m_path = '/';
	}

	return result;
}


#ifdef _WIN32
string FtpService::CheckResult(Json::Value& jresult, string& path)
{
	string result;			// ���ؼ��Ľ��;

	// ���ļ���Ŀ¼�ֿ���ʾ;
	ifstream ifs(m_homeDir + m_path + path);
	if (ifs.is_open())		// �Ƿ�Ϊ�ļ�
	{
		if (m_type != MSG_LIST2)
		{
			result = path;
			result.append("\t");
		}
		else
		{
			m_path += path;

			result = on_ll(jresult, true);
			m_path = "/";
		}
	}
	else				// �Ƿ�ΪĿ¼
	{
		string directory;			// ����Ŀ¼��Ϣ;

		// ����Ŀ¼���ļ���Ϣ
		m_path += path += "/";
		if (m_type != MSG_LIST)
		{
			result += on_ll(jresult).append("\n");
		}
		else
		{
			on_ls(jresult);
			result = FileUtils::List_Result(jresult, directory);
		}

		m_path = "/";
	}

	return result;
}
#endif

// ����ļ���Ŀ¼ �Ƿ����, �������򷵻���ص��ļ�����;
string FtpService::CheckFile()
{
	char* argv[64];					// �����е�������;
	string result;					// ����ļ��Ľ��;
	Json::Value jsonResult;			// �ļ�����ϸ��Ϣ;

	// ��ͬϵͳ֮��ı���ת��;
	string cmdline = m_data;
	if (isWindows != ::isWin)
	{
		#ifdef _WIN32
			cmdline = Utf8ToGbk(cmdline.c_str());
		#else
			string cmdline2;
			GbkToUtf8((char*)cmdline.c_str(), cmdline.length(), (char*)cmdline2.c_str(), cmdline2.length());
			cmdline = cmdline2;
		#endif
	}

	// ��ȡ�����е�������;
	int argc = FileUtils::Split((char*)cmdline.c_str(), argv);
	if (argc > 0)
	{
		// ��ֹ�û���Խ��Ŀ¼����Խ�����;
		string part = argv[1];			// ��ȡ������;
		part += '\0';

		if (m_type == MSG_CD && m_path != "/")
		{
			int cd_pos = 0;
			int cd_count = 0;

			do
			{
				cd_pos = part.find("..", cd_pos);
				if (cd_pos < 0) break;

				cd_pos += 2;
				cd_count++;
			} while (true);
			
			int path_count = std::count(m_path.begin(), m_path.end(), '/') - 1;
			
			if (cd_count > path_count)
			{
				result = "The access violation ! \n";
				throw result;
			}
		}
		else
		{

			int size = part.size();
			if (size <= 2)
				char CanAccess;				// �޲���ָ��;
			else if (size > 2 && part[0] == part[1] && part[0] == '.'
				|| size > 2 && part[2] == part[3] && part[3] == '.' && part[0] == '.')
			{
				result = "The access violation ! \n";
				throw result;
			}
		}

		string path;		// ����������;

		// ����ļ���Ŀ¼
		for (int i = 1; i < argc; i++)
		{
			path = argv[i];

			// �ж��ļ���Ŀ¼�Ƿ����;
		#ifdef _WIN32
			int exist = _access_s((m_homeDir + m_path + path).c_str(), 0);
		#else
			int exist = access((m_homeDir + m_path + path).c_str(), F_OK);
		#endif

			// δ���ҵ��쳣����;
			if (exist == ENOENT)
			{
				result = "No File or directory ! \n";
				throw result;
			}
			if (exist == EACCES)
			{
				result = "Access denied ! \n";
				throw result;
			}

			ifstream ifs(m_homeDir + m_path + path);
			if(m_type == MSG_CD && ifs.is_open())
			{
				result = "The access violation ! \n";
				throw result;
			}
			
			// �����ļ���Ŀ¼�Ľ��;
		#ifdef _WIN32
			if(m_type == MSG_LIST || m_type == MSG_LIST2)
				result += CheckResult(jsonResult, path);
		#endif
		}
	}
	
	return result;			// �����ļ�����;
}

#ifndef _WIN32
string FtpService::LinuxHandler()
{
	string result;

	unsigned short pid = m_SockAddr.GetPort();
	string file = "outcmd" + to_string(pid) + ".txt";

	do
	{
		int exist = access(file.c_str(), F_OK);
		if (exist != 0) break;

		pid = rand() % pid;
		file = "outcmd" + to_string(pid) + ".txt";
	} while (true);
	

	string cmdline;
	switch (m_type)
	{
	case (MSG_LOGIN || MSG_LOGIN2):
		cmdline = "ls -F >> ";
		break;

	case MSG_LIST:
		if (m_length > 0)
		{
			cmdline = m_data;
			cmdline += " -F >> ";
		}
		else
		{
			cmdline = "ls -F >> ";
		}
		break;

	case MSG_LIST2:
		if (m_length > 0)
		{
			cmdline = m_data;
			cmdline += " >> ";
		}
		else
		{
			cmdline = "ls -l >> ";
		}
		break;

	case MSG_CD:
		if (m_length > 0)
		{
			cmdline = m_data;
			cmdline += " -L >> ";
		}
		else
		{
			cmdline = "cd -L >> ";
		}
		break;

	default:
		break;
	}

	cmdline += file;

	ifstream ifs;
	system(cmdline.c_str());				// �Զ������ļ�
	ifs.open(file.c_str());					// ֻ�ܴ��ļ�, �޷������ļ�;
	

	if (m_type == MSG_LOGIN || m_type == MSG_LOGIN2)
	{
		Json::Value fileblock;				// ֻ���ڴ��ݲ���, ��ʵ����;;
		result = FileUtils::Login_Result(fileblock);
	}

	string s;
	int count = 0;
	if (m_type == MSG_CD)
	{
		getline(ifs, s);

		// �������һ��Ŀ¼��;
		int cd_pos = s.find_last_of('/', s.length() - 2);
		if (cd_pos >= 0)
			result = s.substr(cd_pos + 1, s.length());
	}
	else if (m_type == MSG_LIST2)
	{
		while (getline(ifs, s))
		{
			result.append(s + "\n");
		}
		result.append("\n");
	}
	else
	{
		while (getline(ifs, s))
		{
			if (count % 5 == 0 && count != 0)
				result.append("\n");

			result.append(s + "   ");
			count++;
		}
	}

	result.append("\n");
	ifs.close();

	cmdline = "rm -f " + file;
	system(cmdline.c_str());

	return result;
}
#endif

int FtpService::MessageHandler(string& result, string& reason, Json::Value fileblock)
{
	int errorCode = 0;
	string jsonreq(m_data, m_length);

	switch (m_type)
	{
	case MSG_LOGIN:
		try {
			on_Login(jsonreq);
			login_OK = true;

			m_RecvSock.GetPeerAddr(m_SockAddr);
			printf("\nlogin seccussfully (PID: %d). \n", m_SockAddr.GetPort());
			
		#ifdef _WIN32
			on_ls(fileblock);
			result = FileUtils::Login_Result(fileblock).append("\n");
		#else
			result = LinuxHandler();
		#endif
			
			if(isWindows != ::isWin || ::isWin)
				errorCode = 11;
		}
		catch (string e)
		{
			errorCode = -1;
			reason = e;
			m_RecvSock.GetPeerAddr(m_SockAddr);
			printf("\nLogin failed: %s (PID: %d) \n", e.c_str(), m_SockAddr.GetPort());
		}
		break;

	case MSG_LOGIN2:
		try {
			on_Login2();
			login_OK = true;

			m_RecvSock.GetPeerAddr(m_SockAddr);
			printf("\nlogin seccussfully (PID: %d). \n", m_SockAddr.GetPort());

		#ifdef _WIN32
			on_ls(fileblock);
			result = FileUtils::Login_Result(fileblock).append("\n");
		#else
			result = LinuxHandler();
		#endif

			if (isWindows != ::isWin || ::isWin)
				errorCode = 11;
		}
		catch (string e)
		{
			errorCode = -1;
			reason = e;
			m_RecvSock.GetPeerAddr(m_SockAddr);
			printf("\nLogin2 failed: %s (PID: %d) \n", e.c_str(), m_SockAddr.GetPort());
		}
		break;

	case MSG_LIST:
		if (!this->login_OK) break;

	#ifdef _WIN32
		try {
			if (m_length > 0)
			{
				result = CheckFile();
			}
			else
			{
				result = on_ls(fileblock);
			}

			if (isWindows != ::isWin || ::isWin)
				errorCode = 11;
		}
		catch (string e)
		{
			errorCode = -1;
			reason = e;
			printf("\ncmd - list failed: %s (PID: %d) \n", e.c_str(), m_SockAddr.GetPort());
		}
	#else
		result = LinuxHandler();
	#endif
		break;

	case MSG_LIST2:
		if (!this->login_OK) break;

	#ifdef _WIN32
		try {
			if (m_length > 0)
			{
				result = CheckFile();
			}
			else
			{
				result = on_ll(fileblock);
			}

			if (isWindows != ::isWin || ::isWin)
				errorCode = 11;
		}
		catch (string e)
		{
			errorCode = -1;
			reason = e;
			printf("\ncmd - list2 failed: %s (PID: %d) \n", e.c_str(), m_SockAddr.GetPort());
		}
	#else
		result = LinuxHandler();
	#endif
		break;

	case MSG_CD:
		if (!this->login_OK) break;

	#ifdef _WIN32
		try {
			result = on_cd(fileblock);
			errorCode = 12;
		}
		catch (string e)
		{
			errorCode = -1;
			reason = e;
			// printf("\ncmd - cd failed: %s (PID: %d) \n", e.c_str(), m_SockAddr.GetPort());
		}
	#else
		result = LinuxHandler();
	#endif
		break;

	case MSG_HELP:
		if (!this->login_OK) break;

		try {
			result = FileUtils::Help_Result();
		}
		catch (string e)
		{
			errorCode = -1;
			reason = e;
			printf("\ncmd - help failed: %s (PID: %d) \n", e.c_str(), m_SockAddr.GetPort());
		}
		break;

	case MSG_EXIT:
		if (!this->login_OK) break;
		exit_OK = true;

		break;

	default:
		errorCode = -1;
		reason = "----Unknown Request !----";
		printf("\nError: Unknown Request ! (PID: %d) \n", m_SockAddr.GetPort());
		break;
	}

	return errorCode;
}

int FtpService::ResponseClient()
{
	// �������ʽ����
	while (true && !this->exit_OK)
	{
		// ��������
		int msg_len = ReceiveMessages();
		if (msg_len < 0)
		{
			break;
		}


		// ��������
		string result;
		string reason = "OK";
		Json::Value fileblock;
		int errorCode = MessageHandler(result, reason, fileblock);


		// �ظ�JSON����
		Json::Value response;
		Json::FastWriter writer;
		response["errorCode"] = errorCode;
		response["reason"] = reason;
		response["isWindows"] = isWin;
		
		// ��ͬϵͳ֮��ʹ�ö��η�������;
		if(errorCode != 11)
			response["result"] = result;
		else
			response["result"] = 0;
		std::string jsonresp = writer.write(response);

		// ��������ʱ����ָ������;
		int length = jsonresp.length();
		m_RecvSock.Send(&length, 4, false);			// �ȷ���4���ֽڵ�����, ����ָ������;
		m_RecvSock.Send(jsonresp.c_str(), jsonresp.length(), false);
		
		// ���·���һ��result, ��Ϊ jsoncpp�޷���ȫ��������;
		if (errorCode == 11)
			m_RecvSock.Send(result.c_str(), result.length(), false);
	}

	Clear();

	return 0;
}


int FtpService::ReceiveN(void* buf, int count, int timeout)
{
	// ���ó�ʱ
	if (timeout > 0)
	{
		m_RecvSock.SetOpt_RecvTimeout(timeout);
	}

	// �������ݹ������쳣����
	if (count > m_bufsize)
	{
		printf("ReceiveData2 function error: send data is too long ! (PID: %d) \n", m_SockAddr.GetPort());
		return -12;
	}

	// ������������, ֱ������ָ�����ֽ���;
	int bytes_got = 0;
	while (bytes_got < count)
	{
		m_Mutex.Lock();
		int n = m_RecvSock.Recv((char*)buf + bytes_got, count - bytes_got, false);
		m_Mutex.Unlock();
		if (n <= 0)
		{
			continue;
		}

		bytes_got += n;
	}

	return bytes_got;	// ���ؽ������ݵĴ�С;
}
