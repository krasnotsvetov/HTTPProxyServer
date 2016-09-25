#pragma once 
#include <winsock2.h>
#include "Common.h"
#include <ctime> 
#include "LRUCache.h"
#include "TimeoutSocket.h"

#define MAXBUFFERLEN 4096


struct TCPSocket : TimeoutSocket
{

public:

	LRUCache* cache;

	char buffer[MAXBUFFERLEN];
	int bufferLength = 0;

	bool wantWrite = false;
	bool wantRead = false;

	bool isClientSide = true;


	TCPSocket(SOCKET socket, LRUCache* cache);

	virtual EventState onRead(EventCase ec) = 0;
	virtual EventState onWrite(EventCase ec) = 0;
	virtual EventState onConnect(EventCase ec) = 0;
	virtual EventState onClose(EventCase ec) = 0;

	virtual ~TCPSocket();

};