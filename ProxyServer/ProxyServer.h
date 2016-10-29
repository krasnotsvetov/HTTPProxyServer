#pragma once
#include <iostream>
#include <atomic>
#include <winsock2.h>
#include <thread>
	#include "ClientData.h"
#include "Common.h"
#include "LRUCache.h"
#include "BrowserAccepter.h"
#include "TCPServer.h"
#include "TCPClient.h"

struct ProxyServer {
public:
	

	ProxyServer(int timeout, int port);
	void Run();
	void Dispose();
	void Stop();
	

	~ProxyServer();
private:
	
	//std::thread* serverThread;

	bool init = false;
	int timeout;
	const int port;
	LRUCache* cache;
	ClientData clientData;
	size_t lastClientCount;

	WSADATA WSAdata;

	std::unique_ptr<std::thread> thread;

	std::atomic<bool> isDisposed = false;
	std::atomic<bool> isRunning = false;

	std::atomic<ExitState> exitState = ExitState::Non;
	std::atomic<HANDLE> exitEvent;

	EventState onAccept(TimeoutSocket* fSocket);

	void Initialization();

	void Update();
	void UpdateEvent(EventCase ec, TimeoutSocket * tSocket);
};