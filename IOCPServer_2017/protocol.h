#pragma once

//Connect
#define SERVER_PORT  4000

//User
#define MAX_USER 1000

//NPC
#define MAX_NPC 10000

//Board
#define MAP_WIDTH  1000
#define MAP_HEIGHT 1000



// Protocol ID
#define CS_UP			 1
#define CS_DOWN			 2
#define CS_LEFT			 3
#define CS_RIGHT		 4
#define CS_CHAT			 5

#define SC_POS           1
#define SC_PUT_PLAYER    2
#define SC_REMOVE_PLAYER 3
#define SC_CHAT			 4

// buf size
#define	BUF_SIZE				1024

#pragma pack (push, 1)

struct cs_packet_move {
	BYTE size;
	BYTE type;
};

struct sc_packet_pos {
	BYTE size;
	BYTE type;
	BYTE x;
	BYTE y;
};

#pragma pop