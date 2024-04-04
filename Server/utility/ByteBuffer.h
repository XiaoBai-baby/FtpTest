#ifndef _BYTE_BUFFER_H
#define _BYTE_BUFFER_H

// #include "utility.h"		// C3646: δ֪��д˵����, ���˳�����

#include <string>
#include <cstring>
using namespace std;

/* ByteBuffer �ֽڱ�������
	���ڻ���ͱ��������� �޹������ݻ��ֵȡ�
*/

class ByteBuffer
{
public:
	ByteBuffer(int bufsize);
	ByteBuffer(char* buffer, int offset, int bufsize);

	~ByteBuffer();

public:
	// ���ػ�������ʼλ�õ�����;
	unsigned char* Position();

	// ����д�����ݸ���;
	int Written() const;

	// ���ض�ȡ���ݸ���;
	int Read() const;

	// ���û�����������;
	// ��������÷��������, ����Ϊ���� ByteBuffer���η��ͺͽ���ʱʹ��;
	// ������ʹ��ʱ�޸� Position{ }���� �ͱ�ͷ��Ϣ��;
	void Clear();

public:
	// �������������������ݱ�ͷ;
	unsigned char* Header();

	// �������ݱ�ͷ�Ĵ�С;
	int Headsize() const;

public:
	// �������ݱ�ͷ;
	void loadHeader(void* buf, int bufsize);

	// ���͵����ݶθ���;
	int sendCount() const;

	// �����Ƿ�Ϊ������������;
	bool* isNumber();

	// �������ݵĴ�С;
	unsigned int* Individual();

	// �����ݵĴ�С;
	unsigned int Total() const;

public:
	// ���뵥����������
	void putUnit8(unsigned char value);
	void putUnit16(unsigned short value);
	void putUnit32(unsigned int value);
	void putUnit64(unsigned long long value);
	
	// �������ֽ�����
	void putString(const string& value);
	void putBytes(const char* data, int length);

public:
	// �Ƴ�������������
	unsigned char getUnit8();
	unsigned short getUnit16();
	unsigned int getUnit32();
	unsigned long long getUnit64();

	// �Ƴ�����ֽ�����
	string getString();
	int getBytes(char* buf);

private:
	char* m_buffer;		// ������ݵĻ�����
	int m_bufsize;		// �������Ĵ�С
	int m_offset;	// �ֶ����� m_buffer��ƫ����

	unsigned char* m_start;		// ��������ʼλ�õ�����;
	int m_written;	// �Ѿ�д������ݸ���;
	int m_read;		// �Ѿ���ȡ�����ݸ���;
	

private:
	// ���ڱ��뻺�������ݵ����ݱ�ͷ;
	struct m_header
	{
		// ע��, �ڽṹ����û����ȿ�������, ���Բ�Ҫ���κ��������͵�ָ��;
		unsigned int m_sendcount;		// ���ʹ���, Ҳ�� is_number �� individual_datasize ���±��С;
		unsigned int m_total;			// �����ݵĴ�С;
		bool is_number[1024];			// �ж�ÿ�������Ƿ�Ϊ����;
		unsigned int individual_datasize[1024];		// ÿ�����ݶεĴ�С;
	}m_header;

	// m_header �ṹ�������ݵļ�д;
	#define M_sendcount m_header.m_sendcount
	#define M_total m_header.m_total
	#define Is_number m_header.is_number
	#define Individual_datasize m_header.individual_datasize
};

#endif