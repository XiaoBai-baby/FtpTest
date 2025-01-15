#include "FileCheck.h"

FileCheck::FileCheck()
{
	isClient = false;
	isWin = true;
	isWindows = false;
	m_type = 0;
}

FileCheck::FileCheck(OS_TcpSocket& sock, string homeDir, bool is_win, bool is_client)
{
	isWin = is_win;
	isClient = is_client;

	m_homeDir = homeDir;
	Sock = sock;
}

void FileCheck::operator()(bool is_windows)
{
	isWindows = is_windows;
}

void FileCheck::operator()(const char* homeDir)
{
	int home_len = strlen(homeDir);
	if (home_len <= 0)
	{
		// homeDir Ϊ "" ʱ, �����ڴ濽��;
		m_homeDir = "";
		return;
	}

	char* cmdline = new char[(home_len + 1)];

#ifdef _WIN32
	// strcpy_s(cmdline, cmd_len, data);
	memcpy_s(cmdline, home_len, homeDir, home_len);
#else
	memcpy(cmdline, homeDir, strlen(homeDir));
#endif

	m_homeDir = cmdline;
}

void FileCheck::operator()(string homeDir)
{
	m_homeDir = homeDir;
}

void FileCheck::operator()(OS_TcpSocket& sock, string homeDir, bool is_win, bool is_client)
{
	isWin = is_win;
	isClient = is_client;

	m_homeDir = homeDir;
	Sock = sock;
}


// notDir Ϊ�����ļ��Ĵ��� (��Ŀ¼�ļ�);
string FileCheck::on_ls(Json::Value& jresult, string& path, bool notDir)
{
	string result;

	// ��������
	// printf("ls %s%s ...\n", m_homeDir.c_str(), m_path.c_str());
	jresult.clear();		// ��ֹ�ظ���ʾ

#ifdef _WIN32
	// �ж���Ϣ
	// msg_list2 Ϊfalse, ��˺������ڴ��� on_ls��Ϣ����; Ϊtrue���� on_ll��Ϣ����;
	bool msg_list2 = (m_type == MSG_LIST2) ? true : false;

	// �����ļ���Ϣ
	FileEntryList files = FileUtils::List(m_homeDir + path, msg_list2, notDir);
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
#else
	int type = m_type;
	m_type = MSG_LIST2;

	result = ls_linux(m_type, jresult, path);
	m_type = type;
#endif

	return result;
}

// notDir Ϊ�����ļ��Ĵ��� (��Ŀ¼�ļ�);
string FileCheck::on_ll(Json::Value& jresult, string& path, bool notDir)
{
	string result;
	jresult.clear();			// ��ֹ�ظ���ʾ

#ifdef _WIN32
	on_ls(jresult, path, notDir);
	result = FileUtils::List2_Result(jresult, result);
#else
	int type = m_type;
	m_type = MSG_LIST2;

	result = ls_linux(m_type, jresult, path);
	m_type = type;
#endif

	return result;
}

#ifndef _WIN32
string FileCheck::ls_linux(unsigned int type, Json::Value& jresult, string& path)
{
	string result;

	// ��ȡ�ͻ��˵�IP��ַ;
	OS_SockAddr SockAddr;
	Sock.GetPeerAddr(SockAddr);

	// ����һ����ʱ�ļ�, ���������û��Ĳ������;
	unsigned short pid = SockAddr.GetPort();
	string file = "outcmd" + std::to_string(pid) + ".txt";

	// ��ʱ�ļ��ı���, ��ֹ�ļ�������;
	do
	{
		int exist = access(file.c_str(), F_OK);
		if (exist != 0) break;

		file = "outcmd" + std::to_string(rand() % pid) + ".txt";
	} while (true);

	// �������潫Ҫִ�е�����;
	string cmdline;

	// ִ������;
	if (type == MSG_LIST2)
		cmdline = "ls -l \'" + m_homeDir + path + "\' >> " + file;					// '>>'�Զ������ļ�;
	else
		cmdline = "ls \'" + m_homeDir + path + "\' >> " + file;
	system(cmdline.c_str());

	// ���ļ�;
	std::ifstream ifs;
	ifs.open(file.c_str());					// ֻ�ܴ��ļ�, �޷������ļ�;

	// ������������������Ľ��;
	string line;
	int count = 0;							// �����ļ��Ĵ���;

	// ��ȡ�����еĽ��;
	while (getline(ifs, line))
	{
		if (count == 0)
		{
			// �ж�'ls'�����Ƿ���Ч;
			int ls_fail = line.find("ls:");
			if (ls_fail > 0)
			{
				result.append(line + "\n");
				break;
			}
		}

		// 'll'����Ĵ���;
		if (type == MSG_LIST2)
		{
			result.append(line + "\n");

			if (count == 0 && isWindows)
				result.append("\n");

			// ���������еĽ��;
			if (count > 0)
			{
				char* file_info[128];
				FileUtils::Split((char*)line.c_str(), file_info);

				// Json::Value�ļ���;
				int number = count - 1;
				jresult[number]["fileName"] = string(file_info[8]);
				jresult[number]["isDir"] = (file_info[0][0] == 'd') ? true : false;
				jresult[number]["fileSize"] = (double)atoll(file_info[4]);							// ��Linux��, jsoncpp���ܴ���long��������;

				// �����ļ�ʱ��;
				string time = file_info[5];
				time += " ";
				time += file_info[6];
				time += " ";
				time += file_info[7];

				jresult[number]["fileTime"] = time;
				jresult[number]["fileMode"] = string(file_info[0]);					// ���ַ�������ʽ���б���;
			}
		}
		else						// 'ls'����Ĵ���;
		{
			// ÿ����5���ļ�����һ�λ���;
			if (count % 5 == 0 && count != 0)
				result.append("\n");

			result.append(line + "   ");
			jresult[count]["fileName"] = line;
		}

		count++;
	}

	// ɾ����ʱ�ļ�;
	cmdline = "rm -f " + file;
	system(cmdline.c_str());

	return result;
}
#endif

void FileCheck::CheckCdCommand(string& result, string& path, string& part)
{
	// �������� ".."������;
	int cd_count = 0;
	// �������� ".."��λ��;
	long long cd_pos = 0;					// long long ���� [] ����Ӽ�, ���� ������� C26451;

	do
	{
		cd_pos = part.find("..", cd_pos);
		if (cd_pos < 0) break;


		// ��ֹ����Ƿ�����, ��: cd ../++../..;
		// ʹ������if �������ֹ�ڴ�й©;
		if (cd_pos != 0)
		{
			if (part[cd_pos - 1] != '/')
			{
				result = "input bad command ! \n";
				throw result;
			}
		}

		// ��ֹ����Ƿ�����, ��: cd ../..+-/..;
		// ʹ������if �������ֹ�ڴ�й©;
		if (cd_pos + 2 != part.length())
		{
			if (part[cd_pos + 2] != '/')
			{
				result = "input bad command ! \n";
				throw result;
			}
		}

		cd_pos += 2;
		cd_count++;
	} while (true);


	// ��ֹ�û�����Խ�����;
	int path_count = std::count(path.begin(), path.end(), '/') - 1;
	if (cd_count > path_count && m_type == MSG_CD)
	{
		result = "The access violation ! \n";
		throw result;
	}
}

void FileCheck::CheckCommand(string& result, string& path, string& part)
{
	int count = 0;						// part �������ļ���;

	// �ļ������ܰ������ַ�: \ / : * ? " < > |
	for (string::iterator it = part.begin(); it != part.end(); it++)
	{
		if (count == 1 && *it == ':' && m_type > MSG_CD)						// get, put
		{
			// on_get �� �������-o, ��Ҫ�ܿ�;
		}
		else if (*it == '\"' && count == 0 || count == part.size() - 1)
		{
			// ����""���ļ���Ҫ�ܿ�;
			// part = part.erase(count, 1);
		}
		else if (*it == '\\' || *it == ':' || *it == '*' || *it == '?'
			|| *it == '"' || *it == '<' || *it == '>' || *it == '|')
		{
			result = "No File or directory ! \n";
			throw result;
		}

		count++;
	}

	//  ���� "//" �ַ���λ��;
	int find_pos = 0;
	if (part.size() >= 4 && m_type > MSG_CD)								// get, put
		find_pos = 3;


	// �����в�֧�� "//" �ַ�;
	long long bad_commend = part.find("//", find_pos);					// long long ���� ������� C26451;
	if (bad_commend >= 0)
	{
		result = "input bad command, cannot input // or \\\\ . \n";
		throw result;
	}

	// ��� "." �йصĴ�������, ��: ls file./file2
	bad_commend = part.find('.', 0);
	if (bad_commend > 0)				// ʹ������if �������ֹ�ڴ�й©;
	{
		if (part[bad_commend - 1] != '/' && part[bad_commend + 1] == '/' || bad_commend == part.length() - 2)
		{
			result = "input bad command ! \n";
			throw result;
		}
	}

	if (m_type >= MSG_CD && path != "/")								// cd, get, put
	{

		CheckCdCommand(result, path, part);
	}
	else						// ��ֹ�û���Խ��Ŀ¼����Խ�����;
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
}

string FileCheck::CheckResult(Json::Value& jresult, string path, string part)
{
	string result;			// ���ؼ��Ľ��;

	bool is_Dir = IsDirectory(path, part);
	// ���ļ���Ŀ¼�ֿ���ʾ;
	//ifstream ifs(m_homeDir + path + part);
	if (!is_Dir)			// �Ƿ�Ϊ�ļ�
	{
		if (m_type != MSG_LIST2)
		{
			result = part;
			result.append("\t");
		}
		else
		{
			path += part;

			result = on_ll(jresult, path, true);
			// path = "/";
		}
	}
	else				// �Ƿ�ΪĿ¼
	{
		string directory;			// ����Ŀ¼��Ϣ;

		// ����Ŀ¼���ļ���Ϣ
		path += part += "/";
		if (m_type != MSG_LIST)
		{
			result += on_ll(jresult, path).append("\n");
		}
		else
		{
			result = on_ls(jresult, path);
			// result = FileUtils::List_Result(jresult, directory);
		}
		// path = "/";
	}

	return result;
}

string FileCheck::IsExistFile(string path, string part)
{
	string result;

#ifdef _WIN32
	int exist = _access_s((m_homeDir + path + part).c_str(), 0);

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
#else

	int exist = 0;
	if (isClient)
		exist = access(part.c_str(), F_OK);
	else
		exist = access((m_homeDir + path + part).c_str(), F_OK);

	if (exist < 0)
	{
		result = "No File or directory ! \n";
		throw result;
	}
#endif

	return result;
}

bool FileCheck::IsDirectory(string path, string part)
{
	bool is_Dir = false;
#ifdef _WIN32
	struct _stat infos;
	_stat((m_homeDir + path + part).c_str(), &infos);

	if (infos.st_mode & _S_IFDIR)
	{
		is_Dir = true;    			//Ŀ¼
	}
	else if (infos.st_mode & _S_IFREG)
	{
		is_Dir = false;				//�ļ�
	}
#else
	struct stat infos;
	stat((m_homeDir + path + part).c_str(), &infos);

	if (infos.st_mode & S_IFDIR)
	{
		is_Dir = true;    			//Ŀ¼
	}
	else if (infos.st_mode & S_IFREG)
	{
		is_Dir = false;				//�ļ�
	}
#endif

	return is_Dir;
}


// ����ļ���Ŀ¼ �Ƿ����, �������򷵻���ص��ļ�����;
string FileCheck::CheckFile(string& path, char* data, unsigned int type)
{
	char* argv[64];					// �����е�������;
	string result;					// ����ļ��Ľ��;
	Json::Value jsonResult;			// �ļ�����ϸ��Ϣ;
	m_type = type;

	int cmd_len = strlen(data) + 1;
	char* cmdline = new char[cmd_len];

#ifdef _WIN32
	// strcpy_s(cmdline, cmd_len, data);
	memcpy_s(cmdline, cmd_len, data, cmd_len);
#else
	// ע��, ��Linuxϵͳ��, ʹ�� memcpy ���޷���ԭ�е��ڴ���ȫ���ǵ���;
	// memcpy �� memcpy_s �����ڴ濽���������ڿ��������м������� \0��Ҳ���������;
	// memcpy(cmdline, data, strlen(data));					// �ڴ����;
	strcpy(cmdline, data);
#endif


	// ��ȡ�����е�������;
	int argc = FileUtils::Split(cmdline, argv);
	if (argc > 0)
	{
		if (argc > 8)
		{
			result = "Access denied, Because command too long ! \n";
			throw result;
		}

		// ����ļ���Ŀ¼
		for (int i = 1; i < argc; i++)
		{
			// ��ȡ������;
			string part;

			// ��ͬϵͳ֮��ı���ת��;
			if (isWindows != isWin && !isClient)
			{
			#ifdef _WIN32
				part = Utf8ToGbk(argv[i]);
			#else
				char cmdline2[128] = { 0 };
				GbkToUtf8(argv[i], strlen(argv[i]), cmdline2, 128);
				part = cmdline2;
			#endif
			}
			else
			{
				part = argv[i];
			}

			CheckCommand(result, path, part);

			// �ж��ļ���Ŀ¼�Ƿ����;
			IsExistFile(path, part);

			// ��ֹ���� ��Ŀ¼�ļ�;
			if (m_type == MSG_CD || m_type == MSG_GET && isClient)
			{
				/* ifstream �ж�Ŀ¼, ֻ������Windowsϵͳ;
				ifstream ifs(m_homeDir + path + part);
				if (ifs.is_open())
				{
					result = "The access violation ! \n";
					throw result;
				}*/
				bool is_Dir = IsDirectory(path, part);
				if (!is_Dir)
				{
					result = "The access violation ! \n";
					throw result;
				}
			}

			// �����ļ���Ŀ¼�Ľ��;
			if (m_type == MSG_LIST || m_type == MSG_LIST2)
				result += CheckResult(jsonResult, path, part);
		}
	}

#ifdef _WIN32
	delete[] cmdline;		// ʹ�� memcpy_s ʱ, ����ɾ��;
#endif

	return result;			// �����ļ�����;
}