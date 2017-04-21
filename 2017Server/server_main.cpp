
#include <WinSock2.h>	//순서 중요 winwod 헤더 포함
#include <winsock.h>

#include <iostream>
#include <thread>
#include <array>
#include <vector>
#include <set>
#include <unordered_set>
#include "protocol.h"

#include <mutex>
#include <atomic>

#include "Util.h"

HANDLE ghIOCP;
SOCKET gsServer;

enum EVENTTYPE { E_RECV, E_SEND };

class MyMutext
{
public:
	void lock() {}
	void unlock() {}
};
struct WSAOVERLAPPED_EX
{
	WSAOVERLAPPED over;
	WSABUF wsabuf;
	unsigned char IOCP_buf[MAX_BUFF_SIZE];
	EVENTTYPE event_type;
};

struct ClientInfo
{
	std::mutex glock; //접근 할때마다 lock을 걸어야 할 수 있음
	int x, y;
	//std::atomic<bool> bConnected;
	volatile bool bConnected;

	SOCKET s;
	WSAOVERLAPPED_EX recv_over;	//only use worker thread

	//
	unsigned char packet_buf[MAX_PACKET_SIZE];
	int prev_recv_size;
	int curr_packet_size;
	std::unordered_set<int> view_list;
	std::mutex vl_lock;
};
	
#define VIEW_MAX 3

ClientInfo gclients[MAX_USER];

bool IsNear(int from, int to)
{
	return VIEW_MAX * VIEW_MAX > pow((gclients[from].x - gclients[to].x), 2) + pow((gclients[from].y - gclients[to].y), 2);
}

void pack_show(char packet)
{
	switch (packet)
	{

	case CS_UP:
		std::cout << "CS_UP";
		break;

	case CS_DOWN:
		std::cout << "CS_DOWN";
		break;

	case CS_LEFT:
		std::cout << "CS_LEFT";
		break;

	case CS_RIGHT:
		std::cout << "CS_RIGHT";
		break;

	default:
		std::cout << "unknown packet\n";
		break;
	}
	

}

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
	std::wcout << TEXT("에러") << lpMsgBuf << std::endl;
	LocalFree(lpMsgBuf);
}

void InitializeServer()
{

	//'17.04.07 KYT
	/*
	 한글 출력
	*/
	std::wcout.imbue(std::locale("korean"));

	for (int i = 0; i < MAX_USER; ++i)
	{
		gclients[i].bConnected = false;
	}

	WSADATA	wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	ghIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, NULL, 0);

	//이걸 설정해야 된다. 이걸 이렇게 안해 놓으면.. accept는 되는 IOCP 역활전달이 되지 않는다. error메시지가 나온지 않는다.
	gsServer = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED); 

	SOCKADDR_IN ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(SOCKADDR_IN));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(MY_SERVER_PORT);
	ServerAddr.sin_addr.s_addr = (INADDR_ANY);

	::bind(gsServer, reinterpret_cast<sockaddr *>(&ServerAddr), sizeof(ServerAddr));
	int res = listen(gsServer, 5);
	if (res != 0)
	{
		error_display("Error Listen", WSAGetLastError());
	}

}

void SendPacket(const int& client_index, unsigned char* packet)
{
	//'17.04.07 KYT
	/*
		msg 추가
	*/
	std::cout << "Send Packet [";
	pack_show(packet[1]);
	std::cout << "] to client_index : " << client_index << std::endl;


	WSAOVERLAPPED_EX *send_over = new WSAOVERLAPPED_EX;
	ZeroMemory(send_over, sizeof(WSAOVERLAPPED_EX));
	send_over->event_type = E_SEND;
	memcpy(send_over->IOCP_buf,packet, packet[0]);
	send_over->wsabuf.buf = reinterpret_cast<CHAR*>(send_over->IOCP_buf);
	send_over->wsabuf.len = packet[0];
	DWORD send_flag = 0;
	WSASend
	(
		gclients[client_index].s
		, &send_over->wsabuf
		, 1
		, NULL
		, send_flag
		, reinterpret_cast<LPWSAOVERLAPPED>(&send_over->over)
		, NULL
	);

}

void SendPositionPacket(const int& client_index)
{
	sc_packet_pos packet;
	packet.id = static_cast<WORD>(client_index);
	packet.size = sizeof(sc_packet_pos);
	packet.type = SC_POS;
	packet.x = static_cast<BYTE>(gclients[client_index].x);
	packet.y = static_cast<BYTE>(gclients[client_index].y);
	
	SendPacket(client_index, reinterpret_cast<unsigned char*>(&packet));
}

void SendPlayerPacket(const int& target_client, const int& new_client)
{
	sc_packet_put_player packet;
	packet.id = static_cast<WORD>(new_client);
	packet.size = sizeof(packet);
	packet.type = SC_PUT_PLAYER;
	packet.x = static_cast<BYTE>(gclients[new_client].x);
	packet.y = static_cast<BYTE>(gclients[new_client].y);

	SendPacket(target_client, reinterpret_cast<unsigned char*>(&packet));
}

void SendOtherPlayerPacket(const int& to, const int& object)
{
	sc_packet_pos packet;
	packet.id = static_cast<WORD>(object);
	packet.size = sizeof(packet);
	packet.type = SC_PUT_PLAYER;

	packet.x = static_cast<BYTE>(gclients[object].x);
	packet.y = static_cast<BYTE>(gclients[object].y);

	SendPacket(to, reinterpret_cast<unsigned char*>(&packet));
}

void SendRemovePlayerPacket(const int& to, const int& object)
{
	//'17.04.07 KYT
	/*
		sc_packet_remove_player *packet = new sc_packet_remove_player();
		이렇게 만들어야되고, 5천명이면 5천번 보내야 된 후  delete 해야 한다.
	*/
	sc_packet_remove_player packet;
	packet.id = static_cast<WORD>(object);
	packet.size = sizeof(packet);
	packet.type = SC_REMOVE_PLAYER;

	SendPacket(to, reinterpret_cast<unsigned char*>(&packet));
}

void ProcessPacket(const int& client_index, unsigned char *packet)
{
	//'17.04.07 KYT
	/*
		msg 추가
	*/
	std::cout << "pakcet[";
	pack_show(packet[1]);
	std::cout << "] from client : " << client_index << std::endl;

	switch (packet[1])
	{

		case CS_UP: 
			if(gclients[client_index].y > 0) gclients[client_index].y--;
			break;

		case CS_DOWN:
			if(BOARD_HEIGHT - 1 > gclients[client_index].y) gclients[client_index].y++;
			break;

		case CS_LEFT:
			if (gclients[client_index].x > 0) gclients[client_index].x--;
			break;

		case CS_RIGHT:
			if (BOARD_WIDTH - 1 > gclients[client_index].x) gclients[client_index].x++;
			break;

		default:
			std::cout << "unknown packet from client [ " << client_index << " ]\n";
			break;
	}



	std::unordered_set<int> new_view_list;

	for (int i = 0; i < MAX_USER; ++i)
	{
		if (i == client_index)continue;
		
		if (gclients[i].bConnected & (IsNear(client_index, i)) )
		{
			new_view_list.insert(i);
		}

	}
	for (auto& id : new_view_list)
	{

		//gclients[client_index].vl_lock.lock();									-- 1차 lock
		gclients[client_index].vl_lock.lock();										//-- 2차 lock

		//보이지 않다가 보이게 된 객체 처리  : 시야 리스트에 존재하지 않았떤 존재
		if (0 == gclients[client_index].view_list.count(id))
		{
			gclients[client_index].view_list.insert(id);
			gclients[client_index].vl_lock.unlock();								//--2차 unlock

			SendPlayerPacket(client_index, id);


			gclients[id].vl_lock.lock();										//-- 2차 lock
			if (0 == gclients[id].view_list.count(client_index))
			{
				gclients[id].view_list.insert(client_index);
				gclients[id].vl_lock.unlock();								//--2차 unlock

				SendPlayerPacket(id, client_index);
			}
			else
			{
				gclients[client_index].vl_lock.unlock();					
				SendPlayerPacket(id, client_index);
			}


		}	
		//계속 보이고 있떤 객체 처리
		else
		{
			gclients[client_index].vl_lock.unlock();						
			gclients[id].vl_lock.lock();									
			if (0 == gclients[id].view_list.count(client_index))
			{
				gclients[id].view_list.insert(client_index);
				gclients[id].vl_lock.unlock();								

				SendPlayerPacket(id, client_index);
			}
			else
			{
				gclients[id].vl_lock.unlock();									
				SendPlayerPacket(id, client_index);
			}
		}
		//gclients[client_index].vl_lock.unlock();								
	}

	gclients[client_index].vl_lock.lock();										
	std::unordered_set<int> localviewlist = gclients[client_index].view_list;
	gclients[client_index].vl_lock.unlock();									

	//보이다가 보이지 않게 되는 객체 처리 : 시야리스트에서 제거해야 되는 존재
	for (auto& id : localviewlist)
	{
		gclients[client_index].vl_lock.lock();							
		if (0 != gclients[client_index].view_list.count(id))
		{
			gclients[client_index].view_list.erase(id);
			gclients[client_index].vl_lock.unlock();					

			SendRemovePlayerPacket(client_index, id);

			//이미 지워져 있으면 지울 필요가 없다.
			gclients[id].vl_lock.lock();								
			if (0 == gclients[id].view_list.count(client_index))
			{
				gclients[id].vl_lock.unlock();							

				SendRemovePlayerPacket(id, client_index);

				gclients[id].vl_lock.lock();							
				gclients[id].view_list.erase(client_index);
				gclients[id].vl_lock.unlock();							
			}
			else
				gclients[id].vl_lock.unlock();							

		}
		else
			gclients[client_index].vl_lock.unlock();							

	}




	for (int i = 0; i < MAX_USER; ++i)
	{
		if (true == gclients[i].bConnected)
		{
			if (i != client_index)
			{
				SendPlayerPacket(i, client_index);
				SendPlayerPacket(client_index, i);
			}
		}
	}

	SendPositionPacket(client_index);
}

void NetWorkError()
{
	std::cout << "NetWork Error!!!\n";
	exit(-1);

}

void clear_clientt(const int& client_id)
{

}

void AcceptThread()
{
	SOCKADDR_IN clientAddr;
	ZeroMemory(&clientAddr, sizeof(SOCKADDR_IN));
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_port = htons(MY_SERVER_PORT);
	clientAddr.sin_addr.s_addr = (INADDR_ANY);
	int addr_len = sizeof(clientAddr);
	while (true) 
	{
		SOCKET sClient = WSAAccept(gsServer, reinterpret_cast<sockaddr *>(&clientAddr), &addr_len, NULL, NULL);

		if (INVALID_SOCKET == sClient)
		{
			error_display("Accept Thread Accept Error : ", WSAGetLastError());
		}

		std::cout << "New Client Arrived! : ";

		int new_client_id = -1;
		for (int i = 0; i < MAX_USER; ++i)
		{
			if (false == gclients[i].bConnected)
			{
				new_client_id = i;
				break;
			}
		}
		if (-1 == new_client_id)
		{
			closesocket(sClient);
			std::cout << "Max User Overflow!!!\n";
			continue;
		}

		std::cout << " ID = " << new_client_id << " " << std::endl;

	//	ZeroMemory(&gclients[new_client_id], sizeof(gclients[new_client_id]));

		gclients[new_client_id].curr_packet_size = 0;
		gclients[new_client_id].prev_recv_size = 0;
		ZeroMemory(&gclients[new_client_id].recv_over, sizeof(gclients[new_client_id].recv_over));
		gclients[new_client_id].packet_buf[0] = NULL;

		gclients[new_client_id].vl_lock.lock();
		gclients[new_client_id].view_list.clear();
		gclients[new_client_id].vl_lock.unlock();


		gclients[new_client_id].s = sClient;
		gclients[new_client_id].bConnected = true;
		CreateIoCompletionPort(reinterpret_cast<HANDLE>(sClient), ghIOCP, new_client_id, 0);
		gclients[new_client_id].recv_over.event_type = E_RECV;		//Recv니깐
		gclients[new_client_id].recv_over.wsabuf.buf = reinterpret_cast<CHAR *>(gclients[new_client_id].recv_over.IOCP_buf);//WSA니깐 wsabuf

		// len = 0 이 되면 아무것도 하지 않는다.

		gclients[new_client_id].recv_over.wsabuf.len = sizeof(gclients[new_client_id].recv_over.IOCP_buf);//진짜 버퍼의 길이
		
		SendPlayerPacket(new_client_id, new_client_id);

		for (int i = 0; i < MAX_USER; ++i)
		{
			if (true == gclients[i].bConnected)
			{
				if (i != new_client_id)
				{
					if (true == IsNear(new_client_id, i))
					{
						gclients[i].vl_lock.lock();
						gclients[i].view_list.insert(new_client_id);
						gclients[i].vl_lock.unlock();

						SendPlayerPacket(i, new_client_id);
						SendPlayerPacket(new_client_id, i);
					}
				}
			}
		}



		DWORD recvFlag = 0;

		int res =  WSARecv
		(
			  sClient
			, &gclients[new_client_id].recv_over.wsabuf
			, 1			// 1을 넣어야 된다. 여기에 wsabuf.len을 넣으면 ㄹㅇ 산으로간다. 하나밖에 안썼으니 1을 넣어야 한다.
			, NULL
			, &recvFlag	// 꼭 0 으로 넣어줘야 합니다.
			, &gclients[new_client_id].recv_over.over
			, NULL
		);

		if (0 != res)
		{
			int error_no = WSAGetLastError();
			if (WSA_IO_PENDING != error_no)		// 이건 에러가 아님
			{
				error_display("SendPacket:WSASend", error_no);
			}
		}

		std::cout << "Accept " << std::endl;
	}
}

//'17.04.07 KYT
/*
	Disconnect Client & socket init
*/
void DisconnectClient(const int& client_index)
{
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (true == gclients[i].bConnected)
		{
			if (i != client_index)
			{
				if (gclients[client_index].bConnected)
				{
					gclients[i].vl_lock.lock();
					if (0 != gclients[i].view_list.count(client_index))
					{
						gclients[i].vl_lock.unlock();

						SendRemovePlayerPacket(i, client_index);

						//죽어요 싱쓰 자료구조야
						gclients[i].vl_lock.lock();
						gclients[i].view_list.erase(client_index);
						gclients[i].vl_lock.unlock();
					}
					else
						gclients[i].vl_lock.unlock();

				}
			}
		}
	}

	closesocket(gclients[client_index].s);
	gclients[client_index].bConnected = false;
}

void WorkerThread()
{
	while (true)
	{
		DWORD io_size;
		unsigned long long client_index;
		WSAOVERLAPPED_EX *pOver;

		BOOL succ = GetQueuedCompletionStatus		// GQCS
		(
			ghIOCP	//global IOCP
			, &io_size
			, &client_index
			, reinterpret_cast<LPOVERLAPPED*>(&pOver) //Overlap pointer, 우리는 확장 구조체를 쓰는데 윈도우는 아니라서 reinter_cast를 한다
			, INFINITE
		);

		if (!succ)
		{
			error_display("DisconnectClient : ", WSAGetLastError());
			DisconnectClient(static_cast<int>(client_index));
			//NetWorkError();
		}
		std::cout << "GQCS : Event ";

		if (0 == io_size)
		{
			// 접속 종료
			DisconnectClient(static_cast<int>(client_index));
			continue; 
		}


		else if (E_RECV == pOver->event_type)
		{
			std::cout << " data from Client" ;
			int to_process = io_size;
			unsigned char *buf_ptr = gclients[client_index].recv_over.IOCP_buf;	// ?
			unsigned char packet_buf[MAX_PACKET_SIZE]; // 패킷을 저장할 패킷 버퍼를 만든다.
			
			int pSize = gclients[client_index].curr_packet_size;		
			int prevSize = gclients[client_index].prev_recv_size;	//변수명이 길어서 깔끔하게 하려고 변수 하나 더 선언
			
			while (0 != to_process)
			{
				if (pSize == 0) pSize = buf_ptr[0];		//packet_size가 0 이면 buf_ptr[0]을 처리 할 상태이다.

				if (pSize <= to_process + gclients[client_index].prev_recv_size)
				{
					//'17.04.07 KYT
					/*
						momory overhead...!
					*/
					memcpy(packet_buf, gclients[client_index].packet_buf, prevSize); //지난번에 받은 패킷.
					memcpy(packet_buf + prevSize, buf_ptr, pSize - prevSize);
					//지난번에 받은 버퍼 + 지금 받을 버퍼.. (1) 지난번에 받은 패킷 바로 뒤에서부터 받아야하기 대문

					ProcessPacket(static_cast<int>(client_index), packet_buf);
					to_process -= pSize - prevSize;	//남은 사이즈 감소
					   buf_ptr += pSize - prevSize; // buf도 사용했으니깐 전진
					pSize = 0; prevSize = 0; 
					//받은사이즈가 크거나 같으면!, 
					// to_prcess는 이번에 받은거고, prev_recv_size는 지난번에 받은거
				}
				else
				{
					memcpy(gclients[client_index].packet_buf + prevSize, buf_ptr, to_process);	// 지난번에 받은거 이어서 받아야한다.
					prevSize += to_process;
					to_process = 0;
					buf_ptr += to_process;
				}
			}
				// loop 밖에 있어야 한다.
				gclients[client_index].curr_packet_size = pSize;
				gclients[client_index].prev_recv_size = prevSize;
		
				DWORD recvFlag = 0;

			int ret = WSARecv
			(
				  gclients[client_index].s
				, &gclients[client_index].recv_over.wsabuf
				, 1
				, NULL
				, &recvFlag
				, &gclients[client_index].recv_over.over
				, NULL
			);

			if (0 != ret)
			{
				int error_no = WSAGetLastError();
				if (WSA_IO_PENDING != error_no)
				{
					error_display("Recv Error", WSAGetLastError());
				}
			}

		}
		else if (E_SEND == pOver->event_type)
		{
			std::cout << "Send Completed to Client : " << client_index << std::endl;
			if (io_size != pOver->IOCP_buf[0])
			{
				std::cout << "Out\n";
			}
			delete pOver;
			pOver = nullptr;
		}
		else
		{
			std::cout << "unkonw GQDC \n";
		}

	}
}

int main()
{
	InitializeServer();
	std::thread accept_thread{ AcceptThread };

	std::vector<std::thread*> vWorker_Thread;
	for (int i = 0; i < 6; ++i)
	{
		vWorker_Thread.push_back(new std::thread{WorkerThread});
	}

	//CreateAcceptThread();

	accept_thread.join();

	for (auto &thread : vWorker_Thread)
	{
		if (thread)
		{
			thread->join();
			delete thread;
			thread = nullptr;
		}
	}
	vWorker_Thread.clear();
}
