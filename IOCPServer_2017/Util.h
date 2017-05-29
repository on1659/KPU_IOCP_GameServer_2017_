#pragma once

#include "stdafx.h"

static void error_display(char *msg, int err_no)
{
	WCHAR *lpMsgBuf;
	FormatMessage
	(
		  FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
		, NULL
		, err_no
		, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
		, (LPTSTR)&lpMsgBuf
		, 0
		, NULL
	);
	std::cout << msg;
	std::wcout << TEXT("¿¡·¯") << lpMsgBuf << std::endl;
	LocalFree(lpMsgBuf);
	while (true);
}

static void network_error(const std::string& msg = "Something Wrong")
{
	std::cout << msg << std::endl;
}