#include "TcpServiceTask.h"
#include "../ftp/FtpService.h"
#include "../utility/ByteBuffer.h"

/*
	������һ��ҵ�������;
*/

int TcpServiceTask::BusinessService()
{	
	FtpService ftp(m_clientSock, (char*)m_buf, 0, m_bufsize);
	ftp.startReceiveData();

	return 0;
}