
#include "stdafx.h"
#include "IOCPServer.h"

int main()
{
	CIOCPServer server;


	IOCP_Start iocp_start;
	iocp_start.nWorkder_Thread_Count = 6;
	server.Start(&iocp_start);
	server.End();
}

