#pragma once

#include "stdafx.h"

enum class EventType 
{
	  E_RECV
	, E_SEND
};

struct WSAOVERLAPPED_EX {
	WSAOVERLAPPED overlapped;
	WSABUF wsabuf;
	unsigned char recv_buf[MAX_BUFF_SIZE];
	EventType event_type;
};


struct PlayerInfo
{
	int x, y;
	volatile bool bConnected;
	volatile bool bIsAlive;  //NPC 积荤咯何
	std::unordered_set<int> view_list;

};

struct NPC_Info{
	int x, y;
	volatile bool bConnected;
	volatile bool bIsAlive;  //NPC 积荤咯何

};


// Logic
struct IOCP_Start
{
	int nWorkder_Thread_Count;
};

struct SOCKET_INFO {

	UINT id;

	SOCKET socket;
	WSAOVERLAPPED_EX recv_over;
	unsigned char packet_buf[MAX_PACKET_SIZE];
	int prev_recv_size;
	int curr_packet_size;

	std::mutex my_mutex;

	PlayerInfo player_info;
};