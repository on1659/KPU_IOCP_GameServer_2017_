#include "stdafx.h"
#include "IOCPClient.h"
#include "Player.h"

CIOCPClient::CIOCPClient(const std::string& name) : CSingleTonBase(name)
{
}


CIOCPClient::~CIOCPClient()
{
	WSACleanup();
}

bool CIOCPClient::Start(void* connect_data)
{
	IOCP_CONNECT_DATA *connect = static_cast<IOCP_CONNECT_DATA*>(connect_data);

	WSADATA	wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);
	
	g_mysocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
	
	SOCKADDR_IN ConnectAddr;
	ZeroMemory(&ConnectAddr, sizeof(SOCKADDR_IN));
	ConnectAddr.sin_family = AF_INET;
	ConnectAddr.sin_port = htons(SERVER_PORT);
	ConnectAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	int Result = WSAConnect(g_mysocket, (sockaddr *)&ConnectAddr, sizeof(ConnectAddr), NULL, NULL, NULL, NULL);

	if (Result == -1)
		return false;

	WSAAsyncSelect(g_mysocket, connect->hwnd, WM_SOCKET, FD_CLOSE | FD_READ);
	
	send_wsabuf.buf = send_buffer;
	send_wsabuf.len = BUF_SIZE;
	recv_wsabuf.buf = recv_buffer;
	recv_wsabuf.len = BUF_SIZE;


	return true;
}

void CIOCPClient::SocketProc(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (WSAGETSELECTERROR(lParam))
	{
		closesocket((SOCKET)wParam);
		return;
	}
	switch (WSAGETSELECTEVENT(lParam))
	{
		case FD_READ:
			std::cout << "read packet" << std::endl;
			ReadPacket((SOCKET)wParam);
			return;
		case FD_CLOSE:
			closesocket((SOCKET)wParam);
			//clienterror();
			return;
	}
}

void CIOCPClient::ProcessPacket(char * ptr)
{
	if (!m_pPlayer)return;

	switch (ptr[1])
	{
	case SC_POS:
		sc_packet_pos *my_packet = reinterpret_cast<sc_packet_pos *>(ptr);
		m_pPlayer->set(my_packet->x, my_packet->y);
		std::cout << "SC_POS " << my_packet->x << "," << my_packet->y << std::endl;
		break;
	}


}

bool CIOCPClient::SendPacket(const KeyStateCheck& keystate)
{
	cs_packet_move *my_packet = reinterpret_cast<cs_packet_move *>(send_buffer);
	my_packet->size = sizeof(my_packet);
	send_wsabuf.len = sizeof(my_packet);
	DWORD iobyte;

	if (keystate.key == YT_KEY::YK_LEFT)
	{
		my_packet->type = CS_LEFT;
	}
	else if (keystate.key == YT_KEY::YK_RIGHT)
	{
		my_packet->type = CS_RIGHT;
	}
	else if (keystate.key == YT_KEY::YK_UP)
	{
		my_packet->type = CS_UP;
	}
	else if (keystate.key == YT_KEY::YK_DOWN)
	{
		my_packet->type = CS_DOWN;
	}

	WSASend(g_mysocket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
	std::cout << "send socket " << keystate.key << std::endl;
	return true;
}

void CIOCPClient::ReadPacket(SOCKET socket)
{
	DWORD iobyte, ioflag = 0;

	int ret = WSARecv(socket, &recv_wsabuf, 1, &iobyte, &ioflag, NULL, NULL);
	if (ret) {
		int err_code = WSAGetLastError();
		printf("Recv Error [%d]\n", err_code);
	}

	BYTE *ptr = reinterpret_cast<BYTE *>(recv_buffer);

	while (0 != iobyte) {
		if (0 == in_packet_size) in_packet_size = ptr[0];
		if (iobyte + saved_packet_size >= in_packet_size) {
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			ProcessPacket(packet_buffer);
			ptr += in_packet_size - saved_packet_size;
			iobyte -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy(packet_buffer + saved_packet_size, ptr, iobyte);
			saved_packet_size += iobyte;
			iobyte = 0;
		}
	}
}