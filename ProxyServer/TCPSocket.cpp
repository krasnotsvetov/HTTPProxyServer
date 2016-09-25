#include "TCPSocket.h"

TCPSocket::TCPSocket(SOCKET socket, LRUCache * cache) : TimeoutSocket(socket), cache(cache)
{
	int r = WSAEventSelect(socket, WSAEvent, FD_CLOSE | FD_READ | FD_WRITE | FD_CONNECT);
	if (r != 0) {
		throw std::exception("Can't select event");
	}
}

TCPSocket::~TCPSocket()
{
}
