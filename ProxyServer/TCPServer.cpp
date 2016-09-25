#include "TCPServer.h"
#include <iostream>
#include "TCPClient.h"
#include "LRUCache.h"
#include <assert.h>

TCPServer::TCPServer(SOCKET socket, LRUCache* cache) : TCPSocket(socket, cache), isDirty(false), cacheState(CacheState::Non)
{
	isClientSide = false;
}



TCPServer::~TCPServer()
{
}
 
 
EventState TCPServer::onRead(EventCase ec)
{
	
	//std::cout << "!\n";
	if (bufferLength <= 0) {
		bufferLength = recv(socketHandler, buffer, MAXBUFFERLEN, 0);
		if (bufferLength == -1) {
			if (RequestState::NonRequest == requestCommonServer.requestState) {
				return EventState::Failed;
			}
		}
	}
	//0x0034be6c "HTTP/1.0 301 Moved Permanently\r\nLocation: https://slashdot.orghttp://slashdot.org/\r\nConnection: close\r\nContent-Length: 0\r\n\r\n様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様様...

	std::string chunk = std::string(buffer, bufferLength);
	RequestState lastRequestState = requestCommonServer.requestState;
	bool isFirstRequest = false;

	if (lastRequestState == RequestState::NonRequest) {
		isFirstRequest = true;
	}
	else {
		requestCommonServer.headerSize += bufferLength;
	}

	

	if (lastRequestState == RequestState::NonRequest && requestCommonServer.parseRequest(chunk) == RequestState::BadRequest) {
		needRemove = clientSide->needRemove = true;
		return EventState::Failed;
	} 

	if (requestCommonServer.eTag != "" && requestCommonServer.status == "200" && requestCommonServer.vary == "" && cacheState != CacheState::Failed) {
		this->Etag = requestCommonServer.eTag;
		cacheState = CacheState::NeedCache;
	}

	//chunk = requestCommonServer.request;

	if (cacheState == CacheState::NeedCache) {
		answer += chunk;
		if (answer.size() > (cache->objectMaxSize)) {
			cacheState = CacheState::Failed;
			answer = "";
		}
	}
	
	if (isFirstRequest && requestCommonServer.status != "304") {
		bufferLength = requestCommonServer.request.length();
		for (int i = 0; i < bufferLength; i++) {
			buffer[i] = requestCommonServer.request[i];
		}
	}
	else {
		requestCommonServer.request = std::string(buffer, bufferLength);
	}
		
	if (requestCommonServer.status == "304") {
		cacheState = CacheState::NeedReturn;
	}


	if (requestCommonServer.contentLength == requestCommonServer.headerSize) {
		cachePage();
	}

	return onWrite(ec);

	
}

EventState TCPServer::onConnect(EventCase ec)
{
	isDirty = true;
	return EventState::Success;
}

EventState TCPServer::onWrite(EventCase ec)
{
	if (cacheState == CacheState::NeedReturn) {
		bufferLength = 1; //Need to create OnWrite event in ProxyServer.cpp.
		size_t cacheLength = min((cacheString.size() - cachePos), MAXBUFFERLEN);
		if (clientSide->wantWrite && cacheLength > 0) {
			while (true) {
				int sendLength = send(clientSide->socketHandler, cacheString.c_str() + cachePos, cacheLength, 0);
				cachePos += cacheLength;
				cacheLength = min((cacheString.size() - cachePos), MAXBUFFERLEN);
				if (sendLength == -1 || WSAGetLastError() == WSAEWOULDBLOCK) {
					clientSide->wantWrite = false;
					return EventState::Failed;
				}
				if (cacheLength == 0) {
					bufferLength = 0; // Nothing to send, we have already send all information that we have.
					return EventState::Success;
				}
			}
		}
		return EventState::Failed;
	}
	else {
		if (clientSide->wantWrite && bufferLength > 0) {

			int sendLength = send(clientSide->socketHandler, buffer, bufferLength, 0);
			if (sendLength == -1 || WSAGetLastError() == WSAEWOULDBLOCK) {
				clientSide->wantWrite = false; 
				return EventState::Failed;
			}
			else {
				if (sendLength == bufferLength) { 
					bufferLength = 0;
					if (wantRead) { //It happend, when SEND(clientSide->socketInfo, ...) crashed, but event FD_READ to TCPServer come. So we shouldn't call OnRead,we will
									//remember this fact. And only when we send buffer, we should read again/

						wantRead = false;
						int r = WSAEventSelect(socketHandler, WSAEvent, FD_CLOSE | FD_READ | FD_WRITE | FD_CONNECT);
						if (r != 0) {
							throw std::exception("Can't select event");
						}
						onRead(ec);
					}
					return EventState::Success;
				}
				else {
					return EventState::Failed;
				}
			}
			return EventState::Success;
		}
		else {
			if (ec == EventCase::Read) {
				return EventState::Success;
			}
			return EventState::Failed;
		}
	}
}


EventState TCPServer::onClose(EventCase ec)
{
	needRemove = true;
	return EventState::Success;
}

void TCPServer::cachePage()
{
	if (cacheState == CacheState::NeedCache) {
			cache->Add(clientSide->requestCommon.serverAdress, Etag, answer);
	}
}

void TCPServer::Restart(SOCKET tSocket)
{
	//reini
	bufferLength = 0;
	answer = "";
	cacheState = CacheState::Non;
	cacheString = "";
	cachePos = 0;

	wantWrite = false;
	needRemove = false;
	wantRead = false;
	isClientSide = false;
	isDirty = false;
	
	//recreate socket
	closesocket(socketHandler);

	socketHandler = tSocket;
	WSACloseEvent(WSAEvent);
	WSAEvent = WSACreateEvent();
	WSAEventSelect(socketHandler, WSAEvent, FD_ACCEPT | FD_CLOSE | FD_READ | FD_WRITE | FD_CONNECT);

	requestCommonServer = RequestCommonServer();
	lastTime = clock();

}

