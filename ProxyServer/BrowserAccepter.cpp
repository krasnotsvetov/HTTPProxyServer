#include "BrowserAccepter.h"

BrowserAccepter::BrowserAccepter(SOCKET socket) : TimeoutSocket(socket)
{
	int r = WSAEventSelect(socket, WSAEvent, FD_ACCEPT);
	if (r != 0) {
		throw std::exception("Can't select event");
	}
}

BrowserAccepter::~BrowserAccepter()
{
}
