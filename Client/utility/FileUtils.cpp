#include "FileUtils.h"

/*
* Chinese:
	���ļ��Ƚ�����, Ϊ���ܹ��� Windows �ͻ���������ʹ��, ֻ��֧�� ANSI �� GBK2312����;

* English:
	This file is relatively special, in order to be able to normal use on the Windows client, can only support ANSI or GBK2312 encoding;
*/

#ifdef _WIN32
// ����ļ�������
int FileUtils::CheckMode(const string& dir, FileEntry& entry, DWORD FileAttributes)
{
	// FILE_ATTRIBUTE_DIRECTORY ��ʶĿ¼�ľ��
	if (FileAttributes & FILE_ATTRIBUTE_DIRECTORY)			// ��λ�� ����
		entry.isDirectory = true;

	string name = entry.fileName;

	for (int i = 6; i > 0; i -= 2)
	{
		int exist = _access_s((dir + name).c_str(), i);
		if (!exist)
		{
			entry.fileMode = i + 1;
			break;
		}
	}

	return 0;
}

long long FileUtils::List_size(const string& dir)
{
	/* ��/ ����\\ , ĩβ��\\  */
	string d = dir;
	for (int i = 0; i < d.length(); i++)
		if (d[i] == '/') d[i] = '\\';
	if (d[d.length() - 1] != '\\')
		d.append("\\");


	// printf("dir: %s \n", d.c_str());
	
	char filter[256] = { 0 };
	sprintf_s(filter, "%s*.*", d.c_str());
	
	// ϵͳ�ļ���Ϣ;
	WIN32_FIND_DATAA info;
	HANDLE hFind = FindFirstFileA((LPCSTR)filter, &info);
	if (hFind == INVALID_HANDLE_VALUE)
		printf("FileUtils::List2 FindFirstFileA/W function failed! Error: %d \n", GetLastError());
	
	// �ļ���С
	long long fileSize = 0;

	while (hFind != INVALID_HANDLE_VALUE)
	{
		// ����ļ���Ϣ�Ľṹ��;
		FileEntry entry;
		entry.fileName = info.cFileName;
		// printf("%s \n", (char*)info2.cFileName);

		if (entry.fileName != "." && entry.fileName != "..")			// ����2������Ŀ¼
		{
			// FILE_ATTRIBUTE_DIRECTORY ��ʶĿ¼�ľ��
			if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)			// ��λ�� ����
			{
				fileSize += List_size(d + entry.fileName + "\\");
			}
			else
			{
				entry.fileSize = info.nFileSizeHigh;										// �߽��ļ���С
				entry.fileSize = (entry.fileSize << 32) + info.nFileSizeLow;				// �ͽ��ļ���С

				fileSize += entry.fileSize;
			}
		}

		// ������һ���ļ�
		if (!FindNextFileA(hFind, &info)) break;
	}

	if (hFind != 0)
		FindClose(hFind);			// �ر��ļ����

	return fileSize;
}

FileEntryList FileUtils::List(const string& dir, bool msg_list2, bool notDir)
{
	/* ��/ ����\\ , ĩβ��\\  */
	string d = dir;
	for (int i = 0; i < d.length(); i++)
		if (d[i] == '/') d[i] = '\\';
	if (d[d.length() - 1] != '\\' && !notDir)
		d.append("\\");

	// printf("dir: %s \n", d.c_str());

	// �����ļ�������
	FileEntryList result;

	char filter[256] = { 0 };
	if(!notDir)
		sprintf_s(filter, "%s*.*", d.c_str());
	else
		sprintf_s(filter, "%s", d.c_str());

	// ʹ�ÿ��ֽڴ���ļ����Ľṹ��, FindFirstFile()������ INVALID_HANDLE_VALUE
	// WIN32_FIND_DATA info;											// VCȥ��UNICODEѡ��ſ���
	// HANDLE hFind = FindFirstFile((LPCWSTR)&filter, &info);			// ���ֽڲ����ļ���
	WIN32_FIND_DATAA info;
	HANDLE hFind = FindFirstFileA((LPCSTR)filter, &info);
	if (hFind == INVALID_HANDLE_VALUE) 
		printf("FileUtils::List FindFirstFileA/W function failed! Error: %d \n", GetLastError());

	while (hFind != INVALID_HANDLE_VALUE)
	{
		// ����ļ���Ϣ�Ľṹ��;
		FileEntry entry;
		if(!notDir)
			entry.fileName = info.cFileName;
		// printf("%s \n", (char*)info2.cFileName);
		
		if (entry.fileName != "." && entry.fileName != "..")			// ����2������Ŀ¼
		{
			CheckMode(d, entry, info.dwFileAttributes);									// ����ļ�����
			entry.filetime = info.ftLastWriteTime.dwHighDateTime;						// ��ȡ�ļ�ʱ��
			entry.filetime = (entry.filetime << 32) + info.ftLastWriteTime.dwLowDateTime;

			if (entry.isDirectory && msg_list2)
			{
				entry.fileSize = List_size(d + entry.fileName + "\\");
			}
			else
			{
				entry.fileSize = info.nFileSizeHigh;										// �߽��ļ���С
				entry.fileSize = (entry.fileSize << 32) + info.nFileSizeLow;				// �ͽ��ļ���С
			}
			
			result.push_back(entry);
		}

		// ������һ���ļ�
		if (!FindNextFileA(hFind, &info)) break;
	}

	if(hFind != 0)
		FindClose(hFind);			// �ر��ļ����

	return result;
}

#endif

// �ַ����ķָ�
int FileUtils::Split(char text[], char* parts[])
{
	int count = 0;			// �ֶεĸ���
	int start = 0;			// ÿһ�ֶε��׵�ַ
	int flag = 0;			// �����ܶ�������ʶ��ǰ�Ƿ�����Ч�ַ�

	int stop = 0;			// �ж�ѭ���Ƿ����
	for (int i = 0; !stop; i++)
	{
		char ch = text[i];
		if (ch == 0)
			stop = 1;			// ִ����������������ѭ��

		if (ch == ',' || ch == '\0' || ch == ' ' || ch == '\t')
		{
			if (flag)				// �����ָ���
			{
				flag = 0;			// ��flag ��Ϊ0, ����佫����ִ��, ֱ���´�������Ч�ַ�

				text[i] = 0;		// �޸�Ϊ������, ��ɷֶ�
				parts[count] = text + start;			// д���׵�ַ
				count++;
			}
		}
		else
		{
			if (!flag)				// ������Ч�ַ�
			{
				flag = 1;			// ��flag ��Ϊ1, ����佫����ִ��, ֱ���´������ָ���
				start = i;			// ��¼�׵�ַ
			}
		}
	}

	return count;			// ���طֶθ���
}

int FileUtils::MaxFileUnit(Json::Value list)
{
	int count = 0;
	for (int i = 0; i < list.size(); i++)
	{
		long long fileSize = list[i]["fileSize"].asInt64();

		int n = log10(abs(fileSize)) + 1;		// ��������λ��
		if (count < n)
			count = n;
	}

	return count;
}


string FileUtils::Login_Result(Json::Value list)
{
	string result;
	result.append("* * * * * * * * * * * * * * * * * * * * * * * * \n");
	result.append("*                                             * \n");
	result.append("*          Welcome To The Ftp Server .        * \n");
	result.append("*                                             * \n");
	result.append("*  You can use 'help' to consult commands .   * \n");
	result.append("*                                             * \n");
	result.append("* * * * * * * * * * * * * * * * * * * * * * * * \n");
	result.append("--------------------------OK------------------------ \n");
	
#ifdef _WIN32
	List_Result(list, result);
#endif

	return result;
}

string FileUtils::List_Result(Json::Value list, string& result)
{
	for (int i = 0; i < list.size(); i++)
	{
		if (i % 4 == 0 && i != 0) result.append("\n");

		bool isDir = list[i]["isDir"].asBool();

		result.append(list[i]["fileName"].asString());

		if (isDir == true)
			result.append("[+]         ");
		else
			result.append("      ");
	}

	return result;
}

#ifdef _WIN32
string FileUtils::List2_Result(Json::Value list, string& result)
{
	result.append("������: \n");
	int fileUnit = MaxFileUnit(list);			// ��ȡ���ֵ��λ��;

	for (int i = 0; i < list.size(); i++)
	{
		// ��ӡĿ¼��Ȩ��;
		bool isDir = list[i]["isDir"].asBool();
		if (isDir)
			result.append("d");
		else
			result.append("-");

		// ��ӡ�ļ���Ȩ��;
		int fileMode = list[i]["fileMode"].asInt();
		if (fileMode >= 6)
			result.append("rwx-.    ");
		else if(fileMode >= 4)
			result.append("r-x-.    ");
		else if (fileMode >= 2)
			result.append("-wx-.    ");
		else
			result.append("--x-.    ");

		// ��ȡ�ļ��Ĵ�С;
		long long fileSize = list[i]["fileSize"].asInt64();
		
		// int count = MaxFileUnit(list);		// ��ȡ���ֵ��λ��;

		int n = log10(abs(fileSize)) + 1;		// ��������λ��;
		if (n < 0)	n = 1;
		int balance = fileUnit - n;				// �����ֵ֮���λ����;

		// �Ҷ����ļ���С;
		for (int i = 0; i < balance; i++)
		{
			result.append(" ");
		}
		
		// ���ļ���С ת���� �ַ���;
		string string = to_string(fileSize);
		result.append(string);

		// ��ȡ�ļ���ʱ��;
		long long fileTime = list[i]["fileTime"].asInt64();

		// ���ļ�ʱ�� ת���� Windows�ļ���ʽ;
		FILETIME filetime;
		filetime.dwLowDateTime = fileTime;
		filetime.dwHighDateTime = fileTime >> 32;

		// �� Windows�ļ�ʱ�� װ���� ϵͳ����ʾʱ��;
		SYSTEMTIME systime;
		FileTimeToSystemTime(&filetime, &systime);
		
		int year = systime.wYear;
		int month = systime.wMonth;
		int day = systime.wDay;
		
		int hour = systime.wHour;
		int minute = systime.wMinute;

		// ��ӡ����;
		if (month >= 10)
			result.append("\t " + to_string(month) + "��");
		else
			result.append("\t  " + to_string(month) + "��");

		if(day >= 10)
			result.append("\t " + to_string(day));
		else
			result.append("\t  " + to_string(day));


		// ��ȡ��ǰ�������ʱ��;
		time_t computer_time = NULL;
		time(&computer_time);
		tm tp;
		localtime_s(&tp, &computer_time);

		// �������ʱ�� ת���� ����ʾʱ��;
		int computer_year = 1900 + tp.tm_year;
		if (year < computer_year)					// ��ӡ�ļ������
		{
			result.append("    " + to_string(year));
		}
		else										// ��ȷ���ļ���ʱ����
		{
			if (hour >= 10)
				result.append("   " + to_string(hour) + ":");
			else
				result.append("   0" + to_string(hour) + ":");

			if (minute >= 10)
				result.append(to_string(minute));
			else
				result.append("0" + to_string(minute));
		}


		// ��ӡ�ļ���;
		const char* fileName = list[i]["fileName"].asCString();
		result.append("   ");
		result.append(fileName);

		result.append("\n");
	}

	return result;
}

#endif

string FileUtils::Help_Result()
{
	string result;

	result.append("HELP: \n\n");
	result.append("\tls [FILE] - list directory contents . \n");
	result.append("\tll [FILE] - list detailed directory contents . \n");
	result.append("\tcd [FILE] - Change  the  current  directory . \n");
	result.append("\tget <FILE> - Requests the resource identified by ftp server . \n");
	result.append("\tput <FILE> - Requests the server to store or modify a resource . \n");
	result.append("\tdelete <FILE> - Requests the server to delete the identified resource . \n");
	result.append("\texit - Exit ftp server . \n");

	return result;
}