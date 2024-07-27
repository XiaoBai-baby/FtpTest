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
	一个文件的相关信息
*/

// 文件的结构体必须在外部声明, 才能被外部函数调用;
struct FileEntry
{
	FileEntry() :isDirectory(false), fileSize(0) {}
	
	string fileName;		// 文件名
	bool isDirectory;		// 是否为目录
	long long fileSize;		// 文件大小
	long long filetime;		// 文件时间
	int fileMode;			// 文件属性
};

typedef list<FileEntry> FileEntryList;			// 存放文件信息的链表

/*	FileUtils
	实用的文件工具, 可在Windows系统上实现Linux的命令行模式;
*/

// 实用工具类采用全静态函数, 方便被外部函数调用;

class FileUtils
{
public:
	// 检查文件的属性;
	static int CheckMode(const string& dir, FileEntry& file, DWORD FileAttributes);

	// 列出目录中所有文件的大小, 用于辅助 List 使用;
	static long long List_size(const string& dir);

	// 列出全部文件信息;
	static FileEntryList List(const string& dir, bool msg_list2 = false, bool notDir = false);

public:
	// 分割命令行, 用于解析;
	static int Split(char* cmdline, char* parts[]);

	// 获取最大值的文件位数, 用于右对齐 文件大小使用;
	static int MaxFileUnit(Json::Value list);

public:
	// 登录成功的返回结果;
	static string Login_Result(Json::Value jsonlist);
	
	// 'ls'命令的返回结果;
	static string List_Result(Json::Value jsonlist, string& result);

	// 'll'命令的返回结果;
	static string List2_Result(Json::Value jsonlist, string& result);

	// 'help'命令的返回结果;
	static string Help_Result();
};

#endif