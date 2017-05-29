#pragma once

#include "protocol.h"

class CIOCPServer : public Object
{
public:
	CIOCPServer(const std::string& name = "IOCPServer");

	~CIOCPServer();

	bool Start(void* data)override;

	bool End()override;

private:

	// Connect
	void AcceptThread();

	void DisconnectClient(const int& cl);

	// User
	void Initialize_User() {
		for (auto i = 0; i < MAX_USER; ++i)
			Initialize_User(i);
	}
	void Initialize_User(const int& id) {
		m_mSocket[id].player_info.bConnected = false;
		m_mSocket[id].player_info.bIsAlive = false;
	}

	// NPC
	void Initialize_NPC()
	{
		for (auto i = 0; i < MAX_NPC; ++i)
			Initialize_NPC(i);
	}
	void Initialize_NPC(const int& id)
	{
		//m_mSocket[id].player_info.x = rand() % MAP_WIDTH;
		//m_mSocket[id].player_info.y = rand() % MAP_HEIGHT;
		//m_mSocket[id].player_info.bConnected = false;
		//m_mSocket[id].player_info.bIsAlive = true;
		//m_mSocket[id].player_info.bIsActive = false;
	}

	// Set
	void SetLanguage(const std::string& language = "korean")
	{
		std::wcout.imbue(std::locale(language));
	}

	// Sned
	void SendPacket(const int& cl, unsigned char *packet);

	// Worker Thread
	void WorkerThread();

	// ProcessPacket
	void ProcessPacket(const int& cl, unsigned char *packet);


private:
	HANDLE						m_hIOCP{ NULL };
	SOCKET						m_ServerSocket{ NULL };
	const unsigned int			listen_count = 5;
	std::map<UINT, SOCKET_INFO> m_mSocket;
	std::vector<std::thread*>	m_vThread;

	bool IsNear(const int& from, const int& to)
	{
		return (VIEW_MAX * VIEW_MAX) >= ((m_mSocket[from].player_info.x - m_mSocket[to].player_info.x) * (m_mSocket[from].player_info.x - m_mSocket[to].player_info.x)
			+ (m_mSocket[from].player_info.y - m_mSocket[to].player_info.y)
			* (m_mSocket[from].player_info.y - m_mSocket[to].player_info.y));
	}

};

