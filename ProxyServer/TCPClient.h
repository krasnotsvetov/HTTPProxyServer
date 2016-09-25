#pragma once
#include <string>
#include "TCPServer.h"
#include "TCPSocket.h" 


struct TCPClient : TCPSocket {
public: 
	TCPServer* serverSide;
	TCPClient(SOCKET socket, LRUCache* cache);
	virtual EventState onRead(EventCase ec) override;
	virtual EventState onWrite(EventCase ec) override;
	virtual EventState onClose(EventCase ec) override;
	virtual EventState onConnect(EventCase ec) override;

	virtual ~TCPClient();
	friend struct TCPServer;
protected:
	std::string request;
	RequestCommon requestCommon;
};

