#pragma once 
#include <winsock2.h>
#include "Common.h"
#include <ctime> 
#include "LRUCache.h"

#define MAXBUFFERLEN 4096


struct TimeoutSocket
{

public:
	WSAEVENT WSAEvent;
	SOCKET socketHandler;

	clock_t lastTime;
	bool needRemove = false;

	
	TimeoutSocket(SOCKET socket); 
	virtual ~TimeoutSocket();
 
}; 