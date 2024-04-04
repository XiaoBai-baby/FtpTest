#ifndef _FILE_UTILS_H
#define _FILE_UTILS_H

#include <list>
#include <string>
#include <cmath>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "../jsoncpp/include/json.h"
using namespace std;

#ifdef _WIN32
#include <Windows.h>
#include <io.h>
#else
typedef unsigned long DWORD;
#endif

/*	FileEntry
	һ���ļ��������Ϣ
*/

// �ļ��Ľṹ��������ⲿ����, ���ܱ��ⲿ��������;
struct FileEntry
{
	FileEntry() :isDirectory(false), fileSize(0) {}
	
	string fileName;		// �ļ���
	bool isDirectory;		// �Ƿ�ΪĿ¼
	long long fileSize;		// �ļ���С
	long long filetime;		// �ļ�ʱ��
	int fileMode;			// �ļ�����
};

typedef list<FileEntry> FileEntryList;			// ����ļ���Ϣ������

/*	FileUtils
	ʵ�õ��ļ�����, ����Windowsϵͳ��ʵ��Linux��������ģʽ;
*/

// ʵ�ù��������ȫ��̬����, ���㱻�ⲿ��������;

class FileUtils
{
public:
	// ����ļ�������;
	static int CheckMode(const string& dir, FileEntry& file, DWORD FileAttributes);

	// �г�Ŀ¼�������ļ��Ĵ�С, ���ڸ��� List ʹ��;
	static long long List_size(const string& dir);

	// �г�ȫ���ļ���Ϣ;
	static FileEntryList List(const string& dir, bool msg_list2 = false, bool notDir = false);

public:
	// �ָ�������, ���ڽ���;
	static int Split(char* cmdline, char* parts[]);

	// ��ȡ���ֵ���ļ�λ��, �����Ҷ��� �ļ���Сʹ��;
	static int MaxFileUnit(Json::Value list);

public:
	// ��¼�ɹ��ķ��ؽ��;
	static string Login_Result(Json::Value jsonlist);
	
	// 'ls'����ķ��ؽ��;
	static string List_Result(Json::Value jsonlist, string& result);

	// 'll'����ķ��ؽ��;
	static string List2_Result(Json::Value jsonlist, string& result);

	// 'help'����ķ��ؽ��;
	static string Help_Result();
};

#endif