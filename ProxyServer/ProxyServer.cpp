#include "ProxyServer.h" 
#include "TimeoutSocket.h"
#include "TCPClient.h"
#include "TCPServer.h"
#include <assert.h>
#include <ctime> 
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

#include <Windows.h> 
ProxyServer::ProxyServer(int timeout, int port) : port(port)
{
	this->timeout = timeout;
	cache = new LRUCache(128, 16384);
	exitEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("ExitEvent"));
	if (exitEvent == 0) {
		throw std::exception("Can't create exit event");
	}

	lastClientCount = 1;
	isDisposed = false;
}







void ProxyServer::Run()
{
	if (isDisposed) {
		throw std::exception("Proxy server is disposed");
	}

	if (isRunning) {
		throw std::exception("Proxy server has been running");
	}

	isRunning = true;
	Initialization();


	thread = std::unique_ptr<std::thread>(new std::thread([this]()
	{
		while (ExitState::Hard != exitState) {

			Update();
		}
	}));
}

void ProxyServer::Initialization()
{
	clientData = ClientData();

	int startUpResult = WSAStartup(MAKEWORD(2, 2), &WSAdata);
	if (startUpResult != 0) {

		std::cout << "Error occured in WSAStartup : " << startUpResult;
		throw std::exception("Initialization error");
	}

	init = true;
	std::unique_ptr<BrowserAccepter> mainAcceptSocket = std::unique_ptr<BrowserAccepter>(new BrowserAccepter(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)));
	sockaddr_in inetAddr;
	inetAddr.sin_family = AF_INET;
	inetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	inetAddr.sin_port = htons(port);
	if (bind(mainAcceptSocket->socketHandler, (SOCKADDR*)&inetAddr, sizeof(inetAddr))) {
		throw std::exception("Initialization error");
	}

	if (listen(mainAcceptSocket->socketHandler, 10)) {
		throw std::exception("Initialization error");
	}
	mainAcceptSocket->lastTime = clock();
	clientData.AddClient(std::move(mainAcceptSocket));

}


void ProxyServer::Stop() {
	exitState = ExitState::Soft;
	SetEvent(exitEvent);
	int result = SetEvent(exitEvent);
	if (result == 0) {
		throw std::exception("Dispose error");
	}
}

void ProxyServer::Dispose() {
	if (isDisposed) {
		return;
	}
	int result = SetEvent(exitEvent);
	if (result == 0) {
		throw std::exception("Dispose error");
	}
	isDisposed = true;
	exitState = ExitState::Hard;
	thread->join();

}

ProxyServer::~ProxyServer()
{
	if (!isDisposed) {
		Dispose();
	}
	CloseHandle(exitEvent);
	if (init) {
		WSACleanup();
	}
}


void ProxyServer::Update()
{


	//delete before call WSAEVENT, because index can change.
	for (size_t i = 0; i < clientData.socketData.size(); i++) {
		TimeoutSocket* tSocket = clientData.socketData[i].get();
		if (dynamic_cast<BrowserAccepter*>(tSocket) != 0) {
			tSocket->lastTime = clock();
			tSocket->needRemove = false;
		}
		else {
			TCPSocket* tcpSocket = dynamic_cast<TCPSocket*>(tSocket);
			TCPSocket* other;
			if (!tcpSocket->isClientSide) {
				other = static_cast<TCPServer*>(tcpSocket)->clientSide;
			}
			else {
				other = static_cast<TCPClient*>(tcpSocket)->serverSide;
			}
			if (tcpSocket->needRemove && other->needRemove) {
				clientData.Remove(tcpSocket);
				clientData.Remove(other);
				i--;
			}
		}
	}

	if (lastClientCount != clientData.socketData.size()) {
		if (lastClientCount > clientData.socketData.size()) {
			std::cout << "---Client disconnect---" << std::endl;
		}
		else {
			std::cout << "---Client connect---" << std::endl;
		}
		std::cout << "Client count:" << (clientData.socketData.size() - 1) / 2 << std::endl << std::endl << std::endl;
		lastClientCount = clientData.socketData.size();
	}

	WSAEVENT* events = clientData.eventData.data();
	clock_t t = (clock() - clientData.socketData[0]->lastTime) / CLOCKS_PER_SEC;
	int _timeout = t * 1000 < timeout ? timeout : 1;
	if (clientData.socketData.size() == 1) {
		_timeout = -1;
	}

	int size = clientData.socketData.size();
	HANDLE tempEvents[2];
	if (clientData.socketData.size() == 1) {
		tempEvents[0] = events[0];
		tempEvents[1] = exitEvent;
		size = 2;
	}
	//before I use WSAWaitForMultipleEvents.
	int result = WaitForMultipleObjects(min(size, WSA_MAXIMUM_WAIT_EVENTS - 1), (clientData.socketData.size() == 1) ? tempEvents : events, false, _timeout);


	if (exitState == ExitState::Hard) {
		return;
	}
	if (exitState == ExitState::Soft && clientData.socketData.size() == 1) {
		exitState = ExitState::Hard;
		return;
	}
	if (result == WSA_WAIT_TIMEOUT) {


		for (size_t i = 0; i < clientData.socketData.size(); i++) {
			clock_t time = (clock() - clientData.socketData[i]->lastTime) / CLOCKS_PER_SEC;
			if (time * 1000 > timeout) {
				clientData.socketData[i]->needRemove = true;
			}
		}

		TimeoutSocket* tSocket = clientData.socketData[0].get();
		if (dynamic_cast<BrowserAccepter*>(tSocket) != 0) {
			tSocket->lastTime = clock();
			tSocket->needRemove = false;
			clientData.MakeLast(tSocket);
		}
		else {
			TCPSocket* tcpSocket = dynamic_cast<TCPSocket*>(tSocket);
			TCPSocket* other;
			if (!tcpSocket->isClientSide) {
				other = static_cast<TCPServer*>(tcpSocket)->clientSide;
			}
			else {
				other = static_cast<TCPClient*>(tcpSocket)->serverSide;
			}
			if (!tcpSocket->needRemove || !other->needRemove) {
				clientData.MakeLast(tcpSocket);
			}
		}
	}



	if (result != WSA_WAIT_FAILED && result != WSA_WAIT_TIMEOUT) {
		result -= WSA_WAIT_EVENT_0;
		WSANETWORKEVENTS WSANetWork;
		WSAEnumNetworkEvents(clientData.socketData[result]->socketHandler, clientData.socketData[result]->WSAEvent, &WSANetWork);
		if ((WSANetWork.lNetworkEvents & FD_CONNECT) && WSANetWork.iErrorCode[FD_CONNECT_BIT] == 0)
		{
			UpdateEvent(EventCase::Connect, clientData.socketData[result].get());

		}
		if ((WSANetWork.lNetworkEvents & FD_WRITE) && WSANetWork.iErrorCode[FD_WRITE_BIT] == 0)
		{
			UpdateEvent(EventCase::Write, clientData.socketData[result].get());
		}
		if ((WSANetWork.lNetworkEvents & FD_READ) && WSANetWork.iErrorCode[FD_READ_BIT] == 0)
		{

			UpdateEvent(EventCase::Read, clientData.socketData[result].get());

		}

		if ((WSANetWork.lNetworkEvents&FD_ACCEPT) && WSANetWork.iErrorCode[FD_ACCEPT_BIT] == 0 && exitState != ExitState::Soft)//3
		{

			UpdateEvent(EventCase::Accept, clientData.socketData[result].get());

		}
		if ((WSANetWork.lNetworkEvents & FD_CLOSE) && WSANetWork.iErrorCode[FD_CLOSE_BIT] == 0)
		{

			UpdateEvent(EventCase::Close, clientData.socketData[result].get());
		}
		clientData.MakeLast(clientData.socketData[result].get());


		if (result == WSA_WAIT_FAILED) {
			std::cout << "INFO WSA WAIT FAILED" << std::endl;
			std::cout << WSAGetLastError() << std::endl << std::endl;
		}
	}
}


void ProxyServer::UpdateEvent(EventCase ec, TimeoutSocket * tSocket)
{
	EventState next; //Response a status.

	TCPSocket* other = nullptr;
	TCPSocket* tcpSocket = nullptr;
	int size = clientData.socketData.size();

	if (dynamic_cast<BrowserAccepter*>(tSocket) == 0) {
		tcpSocket = static_cast<TCPSocket*>(tSocket);
		if (!tcpSocket->isClientSide) {
			other = static_cast<TCPServer*>(tSocket)->clientSide;
		}
		else {
			other = static_cast<TCPClient*>(tSocket)->serverSide;
		}
		tcpSocket->lastTime = other->lastTime = clock();
		tcpSocket->needRemove = other->needRemove = false;
	}
	switch (ec)
	{
	case EventCase::Write:
		tcpSocket->wantWrite = true;
		//std::cout << "WRITE EVENT COME" << "\r\n";
		if (other->bufferLength > 0) {
			next = other->onWrite(ec);
		}
		break;
	case EventCase::Read:
		if (tcpSocket->bufferLength > 0) {
			//Strange situtation. But it happenned sometimes, because Send function crashed and we have unsend information.
			//Before we read, we must send information, so we remember that we want read and call OnRead in OnWrite method again, if we send method will success.
			//std::cout << "BUFFERLENGTHABOVEZERO";
			if (!tcpSocket->isClientSide) {
				int r = WSAEventSelect(tcpSocket->socketHandler, tcpSocket->WSAEvent, FD_CONNECT | FD_CLOSE | FD_WRITE);
				if (r != 0) {
					throw std::exception("Can't select event");
				}
			}
			tcpSocket->wantRead = true;
		}
		else {
			next = tcpSocket->onRead(ec);
		}
		if (tcpSocket->isClientSide) {
			//std::cout << "CLIENT WANTS READ";
		}
		if (!tcpSocket->isClientSide) {
			//std::cout << "SERVER WANTS READ\r\n";
		}
		break;
	case EventCase::Connect:
		assert(!tcpSocket->isClientSide);
		tcpSocket->onConnect(ec);
		break;
	case EventCase::Accept:
		assert(dynamic_cast<BrowserAccepter*>(tSocket) != 0);

		onAccept(tSocket);
		break;
	case EventCase::Close:
		assert(dynamic_cast<BrowserAccepter*>(tSocket) == 0);
		TCPSocket* tSocketOther;
		if (tcpSocket->isClientSide) {
			tSocketOther = static_cast<TCPClient*>(tSocket)->serverSide;
		}
		else {
			tSocketOther = static_cast<TCPServer*>(tSocket)->clientSide;
		}

		tcpSocket->onClose(ec);
		tSocketOther->onClose(ec);

		break;
	}
}

EventState ProxyServer::onAccept(TimeoutSocket* fSocket)
{
	TCPClient* tcpClient = new TCPClient(accept(fSocket->socketHandler, NULL, NULL), cache);
	TCPServer* tcpServer = new TCPServer(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP), cache);
	tcpClient->serverSide = tcpServer;
	tcpServer->clientSide = tcpClient;
	tcpClient->lastTime = tcpServer->lastTime = clock();
	clientData.AddClient(std::unique_ptr<TCPClient>(tcpClient));
	clientData.AddClient(std::unique_ptr<TCPServer>(tcpServer));
	return EventState::Success;
}