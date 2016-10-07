#include "TCPSocket.h"

TCPSocket::TCPSocket(SOCKET socket, LRUCache * cache) : TimeoutSocket(socket), cache(cache)
{
	if (WSAEventSelect(socket, WSAEvent, FD_CLOSE | FD_READ | FD_WRITE | FD_CONNECT)) {
		throw std::exception("Can't select event");
	}
}

TCPSocket::~TCPSocket()
{
}
