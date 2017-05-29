#pragma once

#include <WinSock2.h>
#include <windows.h>   // include important windows stuff

#pragma comment (lib, "ws2_32.lib")

class CIOCPClient : public CSingleTonBase<CIOCPClient>
{
public:
	CIOCPClient(const std::string& name = "IOCP_Connect");

	~CIOCPClient();

	bool Start(void* connect_data) override;

	void SocketProc(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void ProcessPacket(char * ptr);

	bool SendPacket(const KeyStateCheck& keystate);

	CPlayer *m_pPlayer{ nullptr };

	void SetPlayer(CPlayer* player) { m_pPlayer = player; };

private:


	void ReadPacket(SOCKET socket);

	SOCKET	 g_mysocket;
	WSABUF	send_wsabuf;
	char 	send_buffer[BUF_SIZE];
	WSABUF	recv_wsabuf;
	char	recv_buffer[BUF_SIZE];
	char	packet_buffer[BUF_SIZE];
	DWORD	in_packet_size = 0;
	int		saved_packet_size = 0;
	int		g_myid;

};

