
#include "stdafx.h"
#include "IOCPServer.h"
#include "MiniDump.h"

CIOCPServer::CIOCPServer(const std::string& name) : Object(name)
{
	CMiniDump::Start();
}

CIOCPServer::~CIOCPServer()
{
	CMiniDump::End();
}

bool CIOCPServer::Start(void* data)
{
	IOCP_Start *iocp_start = static_cast<IOCP_Start*>(data);

	CIOCPServer::SetLanguage();
	CIOCPServer::Initialize_User();
	CIOCPServer::Initialize_NPC();
	
	// Winsock initalize
	WSADATA	wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);
	m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, NULL, 0);
	m_ServerSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(SOCKADDR_IN));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(SERVER_PORT);
	ServerAddr.sin_addr.s_addr = INADDR_ANY;

	// bind
	::bind(m_ServerSocket, reinterpret_cast<sockaddr*>(&ServerAddr), sizeof(ServerAddr));

	// listen
	int res = listen(m_ServerSocket, listen_count);
	
	// error
	if (0 != res)
	{
		error_display("Init Listen Error : ", WSAGetLastError());
		return false;
	}

	m_vThread.push_back(new std::thread{ [&]() { AcceptThread(); } });

	for(auto i = 0; i < iocp_start->nWorkder_Thread_Count; ++i)
		m_vThread.push_back(new std::thread{ [&]() { WorkerThread(); } });


	return true;
}

bool CIOCPServer::End()
{
	for (auto& thread : m_vThread)
	{
		thread->join();
		if (thread) delete thread;
		thread = nullptr;
	}
	m_vThread.clear();


	return true;
}

void CIOCPServer::AcceptThread()
{
	SOCKADDR_IN clientAddr;
	ZeroMemory(&clientAddr, sizeof(SOCKADDR_IN));
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_port = htons(SERVER_PORT);
	clientAddr.sin_addr.s_addr = INADDR_ANY;
	int addr_len = sizeof(clientAddr);
	
	while (true) {
		SOCKET sClient = WSAAccept(m_ServerSocket, reinterpret_cast<sockaddr *>(&clientAddr), &addr_len, NULL, NULL);

		if (INVALID_SOCKET == sClient) error_display("Accept Thread Accept Error :", WSAGetLastError());
		std::cout << "New Client Arrived! :";
		int new_client_id = -1;
		// 빈 소켓 찾기! - 실전에선 사용 X
		for (int i = 0; i < MAX_USER; ++i)
			if (false == m_mSocket[i].player_info.bConnected){	
				new_client_id = i;
				break;
			}
		if (-1 == new_client_id) {
			closesocket(sClient);
			std::cout << "Max User Overflow!!!\n";
			continue;
		}
		std::cout << "ID = " << new_client_id << std::endl;

		//ZeroMemory(&gclients[new_client_id], sizeof(gclients[new_client_id]));
		m_mSocket[new_client_id].curr_packet_size = 0;
		m_mSocket[new_client_id].prev_recv_size = 0;
		m_mSocket[new_client_id].my_mutex.lock();
		m_mSocket[new_client_id].player_info.view_list.clear();
		m_mSocket[new_client_id].my_mutex.unlock();
		m_mSocket[new_client_id].socket = sClient;
		m_mSocket[new_client_id].player_info.bConnected = true;
		m_mSocket[new_client_id].player_info.bIsAlive = true;
		m_mSocket[new_client_id].player_info.x = 100;
		m_mSocket[new_client_id].player_info.y = 100;
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(sClient), m_hIOCP, new_client_id, 0);
		ZeroMemory(reinterpret_cast<char *>(&(m_mSocket[new_client_id].recv_over.overlapped)), sizeof(m_mSocket[new_client_id].recv_over.overlapped));
		m_mSocket[new_client_id].recv_over.event_type = EventType::E_RECV;
		m_mSocket[new_client_id].recv_over.wsabuf.buf = reinterpret_cast<CHAR *>(m_mSocket[new_client_id].recv_over.recv_buf);
		m_mSocket[new_client_id].recv_over.wsabuf.len = sizeof(m_mSocket[new_client_id].recv_over.recv_buf);

		DWORD recvFlag = 0;

		int ret = WSARecv
		(
			sClient
			, &m_mSocket[new_client_id].recv_over.wsabuf
			, 1
			, NULL
			, &recvFlag
			, &m_mSocket[new_client_id].recv_over.overlapped
			, NULL
		);
		if (0 != ret) {
			int err_no = WSAGetLastError();
			if (WSA_IO_PENDING != err_no)
				error_display("Recv Error in Accept Thread ", err_no);
		}
	}
}

void CIOCPServer::DisconnectClient(const int& cl)
{
	closesocket(m_mSocket[cl].socket);
	m_mSocket[cl].player_info.bConnected = false;
	m_mSocket[cl].player_info.bIsAlive = false;

	for (int i = 0; i < MAX_USER; ++i)
	{
		if (true == m_mSocket[i].player_info.bConnected)
		{
			m_mSocket[i].my_mutex.lock();
			if (0 != m_mSocket[i].player_info.view_list.count(cl))
			{
				m_mSocket[i].player_info.view_list.erase(cl);
				m_mSocket[i].my_mutex.unlock();
				//SendRemovePlayerPacket(i, cl);
			}
			else
			{
				m_mSocket[i].my_mutex.unlock();
			}
		}
	}
}

void CIOCPServer::SendPacket(const int & cl, unsigned char * packet)
{
	std::cout << "Send Packet[" << static_cast<int>(packet[1]) << "].player_info to Client : " << cl << std::endl;
	WSAOVERLAPPED_EX *send_over = new WSAOVERLAPPED_EX;
	ZeroMemory(send_over, sizeof(*send_over));
	send_over->event_type = EventType::E_SEND;
	memcpy(send_over->recv_buf, packet, packet[0]);
	send_over->wsabuf.buf = reinterpret_cast<CHAR *>(send_over->recv_buf);
	send_over->wsabuf.len = packet[0];
	DWORD send_flag = 0;
	WSASend(m_mSocket[cl].socket, &send_over->wsabuf, 1, NULL, send_flag, &send_over->overlapped, NULL);
}

void CIOCPServer::WorkerThread()
{
	while (true)
	{
		DWORD io_size;
		unsigned long long cl;
		WSAOVERLAPPED_EX *pOver;
		BOOL is_ok = GetQueuedCompletionStatus(m_hIOCP, &io_size, &cl, reinterpret_cast<LPWSAOVERLAPPED *>(&pOver), INFINITE);

		// std::cout << "GQCS : Event";
		if (false == is_ok)
		{
			int err_no = WSAGetLastError();
			if (64 == err_no) DisconnectClient(cl);
			else error_display("GQCS Error : ", WSAGetLastError());
		}

		if (0 == io_size) {
			DisconnectClient(cl);
			continue;
		}

		if (EventType::E_RECV == pOver->event_type) {
			std::cout << "  data from Client :" << cl;
			int to_process = io_size;
			unsigned char *buf_ptr = m_mSocket[cl].recv_over.recv_buf;
			unsigned char packet_buf[MAX_PACKET_SIZE];
			int psize = m_mSocket[cl].curr_packet_size;
			int pr_size = m_mSocket[cl].prev_recv_size;
			while (0 != to_process) {
				if (0 == psize) psize = buf_ptr[0];
				if (psize <= to_process + pr_size) {
					memcpy(packet_buf, m_mSocket[cl].packet_buf, pr_size);
					memcpy(packet_buf + pr_size, buf_ptr, psize - pr_size);
					ProcessPacket(static_cast<int>(cl), packet_buf);
					to_process -= psize - pr_size; buf_ptr += psize - pr_size;
					psize = 0; pr_size = 0;
				}
				else {
					memcpy(m_mSocket[cl].packet_buf + pr_size, buf_ptr, to_process);
					pr_size += to_process;
					buf_ptr += to_process;
					to_process = 0;
				}
			}
			m_mSocket[cl].curr_packet_size = psize;
			m_mSocket[cl].prev_recv_size = pr_size;
			DWORD recvFlag = 0;
			int ret = WSARecv(m_mSocket[cl].socket, &m_mSocket[cl].recv_over.wsabuf,
				1, NULL, &recvFlag, &m_mSocket[cl].recv_over.overlapped,
				NULL);
			if (0 != ret) {
				int err_no = WSAGetLastError();
				if (WSA_IO_PENDING != err_no)
					error_display("Recv Error in worker thread", err_no);
			}
		}
		else if (EventType::E_SEND == pOver->event_type) {
			std::cout << "Send Complete to Client : " << cl << std::endl;
			if (io_size != pOver->recv_buf[0]) {
				std::cout << "Incomplete Packet Send Error!\n";
				exit(-1);
			}
			delete pOver;
		}
	
		else {
			std::cout << "Unknown GQCS Event Type!\n";
			exit(-1);
		}
	}
}

void CIOCPServer::ProcessPacket(const int & client_index, unsigned char * packet)
{
	std::cout << "Packet [" << packet[1] << "] from Client :" << client_index <<"-_-";
	switch (packet[1])
	{
	case CS_UP:
		std::cout << "CS_UP" << std::endl;
		m_mSocket[client_index].player_info.y -= 10;
		break;
	case CS_DOWN:
		std::cout << "CS_DOWN" << std::endl;
		m_mSocket[client_index].player_info.y += 10;
		break;
	case CS_LEFT:
		std::cout << "CS_LEFT" << std::endl;
		m_mSocket[client_index].player_info.x -= 10;
		break;
	case CS_RIGHT:
		std::cout << "CS_RIGHT" << std::endl;
		m_mSocket[client_index].player_info.x += 10;
		break;
	default:
		std::cout << "Unknown Packet Type from Client[" << client_index << "]\n";
		exit(-1);
		break;
	}


	sc_packet_pos packet_pos;
	packet_pos.size = sizeof(packet_pos);
	packet_pos.type = SC_POS;
	packet_pos.x = m_mSocket[client_index].player_info.x;
	packet_pos.y = m_mSocket[client_index].player_info.y;;

	SendPacket(client_index, reinterpret_cast<unsigned char *>(&packet_pos));
}
