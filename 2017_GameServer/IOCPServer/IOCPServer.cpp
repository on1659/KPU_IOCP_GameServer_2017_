
#include "../header.h"
#include "IOCPServer.h"


CIOCPServer::CIOCPServer()
{
	CMiniDump::Start();
}


CIOCPServer::~CIOCPServer()
{
	CMiniDump::End();
}
