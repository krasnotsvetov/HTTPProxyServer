#pragma once
#include "TCPSocket.h" 

struct TCPClient;
struct TCPServer : TCPSocket { 
	TCPClient* clientSide;
	
	TCPServer(SOCKET socket, LRUCache* cache);

	virtual EventState onRead(EventCase ec) override;
	virtual EventState onConnect(EventCase ec) override;
	virtual EventState onWrite(EventCase ec) override;
	virtual EventState onClose(EventCase ec) override;

	virtual ~TCPServer();

	friend struct TCPClient;
private:
	void cachePage();
	void Restart(SOCKET socket);
	
	RequestCommonServer requestCommonServer;
	bool isDirty;
	std::string Etag;
	std::string answer;
	std::string cacheString = "";
	size_t cachePos = 0;
	CacheState cacheState;
};

 