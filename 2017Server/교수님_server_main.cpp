
#include <WinSock2.h>
#include <winsock.h>
#include <Windows.h>

#include <iostream>
#include <thread>
#include <vector>
#include <unordered_set>
#include <mutex>
#include <chrono>

#include <queue>

using namespace std;
using namespace chrono;

#include "protocol.h"

HANDLE ghIOCP;
SOCKET gsServer;
#define VIEW_MAX 7

enum EVENTTYPE { E_RECV, E_SEND, E_DOAI, E_MOVE };

struct WSAOVERLAPPED_EX {
	WSAOVERLAPPED over;
	WSABUF wsabuf;
	unsigned char IOCP_buf[MAX_BUFF_SIZE];
	EVENTTYPE event_type;
};

struct ClientInfo {
	int x, y;
	volatile bool bConnected;
	volatile bool bIsAlive;  //NPC 생사여부
	volatile bool bIsActive;  //NPC활동여부

	SOCKET s;
	WSAOVERLAPPED_EX recv_over;
	unsigned char packet_buf[MAX_PACKET_SIZE];
	int prev_recv_size;
	int curr_packet_size;
	unordered_set <int> view_list;
	mutex vl_lock;
};
struct TimerItem
{
	int id;
	unsigned int exec_time;
	EVENTTYPE t_event;
};
class comparision
{
public:
	constexpr bool operator() (const TimerItem& a, const TimerItem& b) const { return (a.exec_time > b.exec_time);  }
};
std::priority_queue<TimerItem, std::vector<TimerItem>, comparision> timer_queue;
std::mutex gTmer_lock;

ClientInfo gclients[NUM_OF_NPC];

void error_display(char *msg, int err_no)
{
	WCHAR *lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	std::cout << msg;
	std::wcout << L"에러" << lpMsgBuf << std::endl;
	LocalFree(lpMsgBuf);
	while (true);
}

bool IsPlayer(int id)
{
	if (id < MAX_USER) return true;
	else return false;
}

constexpr bool IsNear(const int& from,const int& to)
{
	return (VIEW_MAX * VIEW_MAX) >= ((gclients[from].x - gclients[to].x) * (gclients[from].x - gclients[to].x)
		+ (gclients[from].y - gclients[to].y)
		* (gclients[from].y - gclients[to].y));
}
constexpr bool isNPC(const int& id) { return id >= NPC_START ? true : false; }
void WakeUpNPC(const int& id);

void InitializeNPC()
{
	for (int i = NPC_START; i < NUM_OF_NPC; ++i)
	{
		gclients[i].x = rand() % BOARD_WIDTH;
		gclients[i].y = rand() % BOARD_HEIGHT;
		gclients[i].bConnected = false;
		gclients[i].bIsAlive = true;
		gclients[i].bIsActive = false;
		
	}
}

void InitializeServer()
{
	std::wcout.imbue(std::locale("korean"));

	for (int i = 0; i < MAX_USER; ++i) {
		gclients[i].bConnected = false;
		gclients[i].bIsAlive = false;
	}

	InitializeNPC();

	WSADATA	wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	ghIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, NULL, 0);

	gsServer = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(SOCKADDR_IN));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(MY_SERVER_PORT);
	ServerAddr.sin_addr.s_addr = INADDR_ANY;

	::bind(gsServer, reinterpret_cast<sockaddr *>(&ServerAddr), sizeof(ServerAddr));
	int res = listen(gsServer, 5);
	if (0 != res)
		error_display("Init Listen Error : ", WSAGetLastError());
}

void CloseServer()
{
	closesocket(gsServer);
	WSACleanup();
}

void NetworkError()
{
	std::cout << "Something Wrong\n";
}

void SendPacket(const int& cl, unsigned char *packet)
{
	std::cout << "Send Packet[" << static_cast<int>(packet[1]) << "] to Client : " << cl << std::endl;
	WSAOVERLAPPED_EX *send_over = new WSAOVERLAPPED_EX;
	ZeroMemory(send_over, sizeof(*send_over));
	send_over->event_type = E_SEND;
	memcpy(send_over->IOCP_buf, packet, packet[0]);
	send_over->wsabuf.buf = reinterpret_cast<CHAR *>(send_over->IOCP_buf);
	send_over->wsabuf.len = packet[0];
	DWORD send_flag = 0;
	WSASend(gclients[cl].s, &send_over->wsabuf, 1, NULL,
		send_flag, &send_over->over, NULL);
}

void SendPositionPacket(const int& to, const int& object)
{
	sc_packet_pos packet;
	packet.id = object;
	packet.size = sizeof(packet);
	packet.type = SC_POS;
	packet.x = gclients[object].x;
	packet.y = gclients[object].y;

	SendPacket(to, reinterpret_cast<unsigned char *>(&packet));
}

void SendPutPlayerPacket(const int& target_client, const int& new_client)
{
	sc_packet_put_player packet;
	packet.id = new_client;
	packet.size = sizeof(packet);
	packet.type = SC_PUT_PLAYER;
	packet.x = gclients[new_client].x;
	packet.y = gclients[new_client].y;

	SendPacket(target_client, reinterpret_cast<unsigned char *>(&packet));
}

void SendRemovePlayerPacket(const int& target_client, const int& new_client)
{
	sc_packet_remove_player packet;
	packet.id = new_client;
	packet.size = sizeof(packet);
	packet.type = SC_REMOVE_PLAYER;

	SendPacket(target_client, reinterpret_cast<unsigned char *>(&packet));
}

void HEART_BEAT(const int& i)
{
	volatile int dummy;

	for (int j = 0; j < 20000; ++j) dummy += j;
	int direction = rand() % 4;
	switch (direction) {
	case 0: gclients[i].x++;
		if (gclients[i].x >= BOARD_WIDTH) gclients[i].x = BOARD_WIDTH - 1;
		break;
	case 1:gclients[i].y++;
		if (gclients[i].y >= BOARD_HEIGHT) gclients[i].x = BOARD_HEIGHT - 1;
		break;
	case 2:gclients[i].x--;
		if (gclients[i].x < 0) gclients[i].x = 0;
		break;
	case 3:gclients[i].y--;
		if (gclients[i].y < 0) gclients[i].y = 0;
	}

	for (int pi = 0; pi < MAX_USER; ++pi)
	{
		if (false == gclients[pi].bConnected) continue;
		if (true == IsNear(pi, i)) {
			gclients[pi].vl_lock.lock();
			if (0 != gclients[pi].view_list.count(i))
			{
				gclients[pi].vl_lock.unlock();
				SendPositionPacket(pi, i);
			}
			else {
				gclients[pi].view_list.insert(i);
				gclients[pi].vl_lock.unlock();
				SendPutPlayerPacket(pi, i);
			}

		}
		else {
			gclients[pi].vl_lock.lock();
			if (0 != gclients[pi].view_list.count(i))
			{
				gclients[pi].view_list.erase(i);
				gclients[pi].vl_lock.unlock();
				SendRemovePlayerPacket(pi, i);
			}
			else gclients[pi].vl_lock.unlock();
		}
	}

	for (int p = 0; p < MAX_USER; ++p)
	{
		if (gclients[p].bConnected && (IsNear(p, i)))
		{
			gclients[i].bIsAlive = true;

			gTmer_lock.lock();
			timer_queue.push(TimerItem{ i, GetTickCount() + 1000, E_MOVE });
			gTmer_lock.unlock();
			return;
		}
	}
	gclients[i].bIsAlive = false;

}

void ProcessPacket(const int& cl, unsigned char *packet)
{
	std::cout << "Packet [" << packet[1] << "] from Client :" << cl << std::endl;
	switch (packet[1])
	{
	case CS_UP:
		if (0 < gclients[cl].y) gclients[cl].y--;
		break;
	case CS_DOWN:
		if (BOARD_HEIGHT - 1 > gclients[cl].y) gclients[cl].y++;
		break;
	case CS_LEFT:
		if (0 < gclients[cl].x) gclients[cl].x--;
		break;
	case CS_RIGHT:
		if (BOARD_WIDTH - 1 > gclients[cl].x) gclients[cl].x++;
		break;
	default:
		std::cout << "Unknown Packet Type from Client[" << cl << "]\n";
		exit(-1);
	}

	unordered_set<int> new_view_list;
	for (int i = 0; i < NUM_OF_NPC; ++i)
		if ((true == gclients[i].bIsAlive) && (true == IsNear(cl, i)))
			new_view_list.insert(i);

	unordered_set<int> ovl;
	gclients[cl].vl_lock.lock();
	ovl = gclients[cl].view_list;
	gclients[cl].vl_lock.unlock();

	// 보이지 않다가 보이게 된 객체 처리 : 시야리스트에 존재하지 않았던 객체
	for (auto id : new_view_list)
	{
		if (0 == ovl.count(id)) {
			ovl.insert(id);
			SendPutPlayerPacket(cl, id);
			if (true != IsPlayer(id)) continue;
			gclients[id].vl_lock.lock();
			if (0 == gclients[id].view_list.count(cl)) {
				gclients[id].view_list.insert(cl);
				gclients[id].vl_lock.unlock();
				SendPutPlayerPacket(id, cl);
			}
			else {
				gclients[id].vl_lock.unlock();
				SendPositionPacket(id, cl);
			}
		}
		else {
			// 계속 보이고 있는 객체 처리
			if (false == IsPlayer(id)) continue;
			gclients[id].vl_lock.lock();
			if (0 == gclients[id].view_list.count(cl)) {
				gclients[id].view_list.insert(cl);
				gclients[id].vl_lock.unlock();
				SendPutPlayerPacket(id, cl);
			}
			else {
				gclients[id].vl_lock.unlock();
				SendPositionPacket(id, cl);
			}
		}
	}

	// 보이다가 보이지 않게된 객체 처리 : 시야리스트에 존재했던 객체
	unordered_set<int> remove_list;
	for (auto id : ovl) {
		if (0 == new_view_list.count(id)) {
			remove_list.insert(id);
			SendRemovePlayerPacket(cl, id);

			if (isNPC(id)) WakeUpNPC(id);

			if (false == IsPlayer(id)) continue;
			gclients[id].vl_lock.lock();
			if (0 != gclients[id].view_list.count(cl)) {
				gclients[id].view_list.erase(cl);
				gclients[id].vl_lock.unlock();
				SendRemovePlayerPacket(id, cl);
			} else gclients[id].vl_lock.unlock();
		}
	}

	gclients[cl].vl_lock.lock();
	for (auto p : ovl) gclients[cl].view_list.insert(p);
	for (auto d : remove_list) gclients[cl].view_list.erase(d);
	gclients[cl].vl_lock.unlock();
}

void DisconnectClient(const int& cl)
{
	closesocket(gclients[cl].s);
	gclients[cl].bConnected = false;
	gclients[cl].bIsAlive = false;
	for (int i = 0; i < MAX_USER;++i)
		if (true == gclients[i].bConnected) {
			gclients[i].vl_lock.lock();
			if (0 != gclients[i].view_list.count(cl)) {
				gclients[i].view_list.erase(cl);
				gclients[i].vl_lock.unlock();
				SendRemovePlayerPacket(i, cl);
			} else gclients[i].vl_lock.unlock();
		}
}

void WorkerThread()
{
	while (true)
	{
		DWORD io_size;
		unsigned long long cl;
		WSAOVERLAPPED_EX *pOver;
		BOOL is_ok = GetQueuedCompletionStatus(ghIOCP, &io_size, &cl, 
			reinterpret_cast<LPWSAOVERLAPPED *>(&pOver), INFINITE);

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

		if (E_RECV == pOver->event_type) {
			std::cout << "  data from Client :" << cl;
			int to_process = io_size;
			unsigned char *buf_ptr = gclients[cl].recv_over.IOCP_buf;
			unsigned char packet_buf[MAX_PACKET_SIZE];
			int psize = gclients[cl].curr_packet_size;
			int pr_size = gclients[cl].prev_recv_size;
			while (0 != to_process) {
				if (0 == psize) psize = buf_ptr[0];
				if (psize <= to_process + pr_size) {
					memcpy(packet_buf, gclients[cl].packet_buf, pr_size);
					memcpy(packet_buf + pr_size, buf_ptr, psize - pr_size);
					ProcessPacket(static_cast<int>(cl), packet_buf);
					to_process -= psize - pr_size; buf_ptr += psize - pr_size;
					psize = 0; pr_size = 0;
				}
				else {
					memcpy(gclients[cl].packet_buf + pr_size, buf_ptr, to_process);
					pr_size += to_process;
					buf_ptr += to_process;
					to_process = 0;
				}
			}
			gclients[cl].curr_packet_size = psize;
			gclients[cl].prev_recv_size = pr_size;
			DWORD recvFlag = 0;
			int ret = WSARecv(gclients[cl].s, &gclients[cl].recv_over.wsabuf,
				1, NULL, &recvFlag, &gclients[cl].recv_over.over,
				NULL);
			if (0 != ret) {
				int err_no = WSAGetLastError();
				if (WSA_IO_PENDING != err_no)
					error_display("Recv Error in worker thread", err_no);
			}
		}
		else if (E_SEND == pOver->event_type) {
			std::cout << "Send Complete to Client : " << cl << std::endl;
			if (io_size != pOver->IOCP_buf[0]) {
				std::cout << "Incomplete Packet Send Error!\n";
				exit(-1);
			}
			delete pOver;
		}
		else if (E_DOAI == pOver->event_type)
		{
			HEART_BEAT(cl);
			delete pOver;
		}
		else {
			std::cout << "Unknown GQCS Event Type!\n";
			exit(-1);
		}
	}
}

void AcceptThread()
{
	SOCKADDR_IN clientAddr;
	ZeroMemory(&clientAddr, sizeof(SOCKADDR_IN));
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_port = htons(MY_SERVER_PORT);
	clientAddr.sin_addr.s_addr = INADDR_ANY;
	int addr_len = sizeof(clientAddr);
	while (true) {
		SOCKET sClient = WSAAccept(gsServer, reinterpret_cast<sockaddr *>(&clientAddr), &addr_len, NULL, NULL);

		if (INVALID_SOCKET == sClient)
			error_display("Accept Thread Accept Error :", WSAGetLastError());
		std::cout << "New Client Arrived! :";
		int new_client_id = -1;
		for (int i = 0; i < MAX_USER; ++i)
			if (false == gclients[i].bConnected) {
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
		gclients[new_client_id].curr_packet_size = 0;
		gclients[new_client_id].prev_recv_size = 0;
		gclients[new_client_id].vl_lock.lock();
		gclients[new_client_id].view_list.clear();
		gclients[new_client_id].vl_lock.unlock();
		gclients[new_client_id].s = sClient;
		gclients[new_client_id].bConnected = true;
		gclients[new_client_id].bIsAlive = true;
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(sClient), ghIOCP, new_client_id, 0);
		ZeroMemory(reinterpret_cast<char *>(&(gclients[new_client_id].recv_over.over)),
			sizeof(gclients[new_client_id].recv_over.over));
		gclients[new_client_id].recv_over.event_type = E_RECV;
		gclients[new_client_id].recv_over.wsabuf.buf = reinterpret_cast<CHAR *>(gclients[new_client_id].recv_over.IOCP_buf);
		gclients[new_client_id].recv_over.wsabuf.len = sizeof(gclients[new_client_id].recv_over.IOCP_buf);
		DWORD recvFlag = 0;
		int ret = WSARecv(sClient, &gclients[new_client_id].recv_over.wsabuf,
			1, NULL, &recvFlag, &gclients[new_client_id].recv_over.over,
			NULL);
		if (0 != ret) {
			int err_no = WSAGetLastError();
			if (WSA_IO_PENDING != err_no)
				error_display("Recv Error in Accept Thread ", err_no);
		}
		SendPutPlayerPacket(new_client_id, new_client_id);

		unordered_set <int> locallist;
		for (int i = 0; i < NUM_OF_NPC; ++i)
		{
			if (true == gclients[i].bIsAlive)
			{
				if (i != new_client_id)
				{
					if (true == IsNear(i, new_client_id)) 
					{
						if (isNPC(i)) WakeUpNPC(i);

						locallist.insert(i);
						SendPutPlayerPacket(new_client_id, i);
						if (false == gclients[i].bConnected) continue;
						gclients[i].vl_lock.lock();
						if (0 == gclients[i].view_list.count(new_client_id)) 
						{
							gclients[i].view_list.insert(new_client_id);
							gclients[i].vl_lock.unlock();
							SendPutPlayerPacket(i, new_client_id);
						}
						else gclients[i].vl_lock.unlock();	
					}
				}
			}
		}
		gclients[new_client_id].vl_lock.lock();
		for (auto p : locallist) gclients[new_client_id].view_list.insert(p);
		gclients[new_client_id].vl_lock.unlock();
	}
}

void TimerThread()
{
	while (true)
	{
		Sleep(10);
		do {
			
			gTmer_lock.lock();

			if (timer_queue.empty())
			{
				gTmer_lock.unlock();
				break;
			}

			TimerItem t = timer_queue.top();
			if (t.exec_time > GetTickCount())
			{
				gTmer_lock.unlock();
				break;
			}
			timer_queue.pop();
			gTmer_lock.unlock();

			WSAOVERLAPPED_EX *over_ex = new WSAOVERLAPPED_EX;

			if (E_MOVE == t.t_event)
				over_ex->event_type = E_DOAI;
			
			PostQueuedCompletionStatus(ghIOCP, 1, t.id, &over_ex->over);
		} while (true);
	}
}

void npc_control_thread()
{
	while (true) {
		auto start_time = high_resolution_clock::now();
		for (int i = NPC_START; i < NUM_OF_NPC; ++i)
		{
			WSAOVERLAPPED_EX *over_ex = new WSAOVERLAPPED_EX;
			over_ex->event_type = E_DOAI;
			PostQueuedCompletionStatus(ghIOCP, 1, i, &over_ex->over);
		}

		auto du = high_resolution_clock::now() - start_time;
		int duration = 1000 - duration_cast<milliseconds>(du).count();
		cout << "AI Duration :" << duration_cast<milliseconds>(du).count() << "ms\n";
		if (duration > 0) Sleep(duration);
	}
}

//#define _USE_AI_THREAD
int main()
{
	std::vector <std::thread*> worker_threads;
	//
	InitializeServer();

	#ifdef _USE_AI_THREAD
		thread ai_thread { npc_control_thread };
	#else
		thread timer_thread{ TimerThread };
	#endif
	

	for (int i = 0; i < 6; ++i)
		worker_threads.push_back(new std::thread{ WorkerThread });
	//
	std::thread accept_thread{ AcceptThread };
	accept_thread.join();
	for (auto pth : worker_threads) { pth->join(); delete pth; }
	worker_threads.clear();

	#ifdef _USE_AI_THREAD
		ai_thread.join();
	#else 
		timer_thread.join();
	#endif

	CloseServer();
}

void WakeUpNPC(const int& id)
{
	if (true == gclients[id].bIsActive)return;
	HEART_BEAT(id);
}
