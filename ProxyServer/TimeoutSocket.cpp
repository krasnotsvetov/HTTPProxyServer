#include "TimeoutSocket.h"
#include <iostream>
TimeoutSocket::TimeoutSocket(SOCKET socket) : socketHandler(socket)
{
	
	//int t =setsockopt(socket, SOL_SOCKET, SO_RCVBUF, new char[1024], 1024);
	WSAEvent = WSACreateEvent(); 
	if (WSAEvent == 0) {
		throw std::exception("Can't create WSA event");
	}
}
 

TimeoutSocket::~TimeoutSocket()
{
	if (socketHandler != INVALID_SOCKET) { 
		closesocket(socketHandler);
		socketHandler = INVALID_SOCKET;
		WSACloseEvent(WSAEvent);
	}
}
