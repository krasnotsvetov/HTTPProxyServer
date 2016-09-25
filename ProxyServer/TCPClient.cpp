#include "TCPClient.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include "LRUCache.h"
TCPClient::TCPClient(SOCKET socket, LRUCache* cache) : TCPSocket(socket, cache)
{
	isClientSide = true; 
}



TCPClient::~TCPClient()
{
	 
}


EventState TCPClient::onRead(EventCase ec)
{
	//_ym_uid=1469910941292435466; 
	if (bufferLength <= 0) {
		bufferLength = recv(socketHandler, buffer, MAXBUFFERLEN, 0);
		if (bufferLength == -1) {
			if (RequestState::NonRequest == requestCommon.requestState) {
				return EventState::Failed;
			}
			else {
				return EventState::Success;
			}
		} 
	}

	if (requestCommon.requestState == RequestState::NonRequest || requestCommon.requestState == RequestState::AcceptRequest) {
		if (requestCommon.requestState == RequestState::AcceptRequest) {
			request = "";
		}
		requestCommon.requestState = RequestState::NonRequest;
		request += std::string(buffer, bufferLength);
		requestCommon.parseRequest(request);
		if (requestCommon.requestState == RequestState::BadRequest) {
    		needRemove = serverSide->needRemove = true;
			return EventState::Failed;
		}
		if (requestCommon.requestState == RequestState::NonRequest) {
			return EventState::Failed;
		}




		if (serverSide->isDirty) {
			serverSide->Restart(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
		}
	

		if (requestCommon.method == "GET") {
			std::hash<std::string> hash;
			 
			if (cache->Contains(requestCommon.serverAdress)) {

				auto t = cache->Get(requestCommon.serverAdress);
				serverSide->Etag = t.first;
				serverSide->cacheString = t.second;

				request = requestCommon.headers[0] +"\r\n";
				request += ("If-None-Match: \"" + t.first + "\"" + "\r\n");
				for (int i = 1; i < requestCommon.headers.size(); i++) {
					request += requestCommon.headers[i] + "\r\n";
				}
				request += "\r\n";
				//change buffer
			}
		}



		struct addrinfo *resultInfo = NULL,
			hints;

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		int result = getaddrinfo(requestCommon.url.c_str(), requestCommon.port.c_str(), &hints, &resultInfo);
		if (result != 0) {
			std::cout << "--- TCP Client INFO ---" << std::endl;
			std::cout << "Can't response url address" << std::endl << std::endl;
			needRemove = serverSide->needRemove = true;
			return EventState::Failed;
		};
		//Mozila Firefox reuse open socket to another connection, so recreate server if it's dirty. 
		//For optimization, the check for same servers should be add.
		
		serverSide->isDirty = true;
		result = connect(serverSide->socketHandler, resultInfo->ai_addr, (int)resultInfo->ai_addrlen); 
		
		freeaddrinfo(resultInfo);
	} 

	

	return onWrite(ec);

}

EventState TCPClient::onWrite(EventCase ec)
{
	if (serverSide->wantWrite && bufferLength > 0) {
		int sendLentgh = send(serverSide->socketHandler, request.c_str(), request.length(), 0);
		if (sendLentgh == -1 || WSAEWOULDBLOCK == WSAGetLastError()) {
			serverSide->wantWrite = false; 
			return EventState::Failed;
		}
		else {
			if (sendLentgh == bufferLength) { 
				bufferLength = 0;
				if (wantRead) { //Same situtation. Read about this in TCPServer::OnWrite(EventCase)
					wantRead = false;
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

EventState TCPClient::onConnect(EventCase ec)
{
	return EventState::Success;
}


EventState TCPClient::onClose(EventCase ec)
{
	needRemove = true;
	return EventState::Success;
}

