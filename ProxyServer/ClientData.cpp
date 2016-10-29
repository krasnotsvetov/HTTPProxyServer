#include <memory>

#include "ClientData.h"
#include "Common.h"




void ClientData::AddClient(std::unique_ptr<TimeoutSocket> socket)
{
	
	this->eventData.push_back(socket->WSAEvent);
	
	try {
		this->socketData.push_back(std::move(socket));
	}
	catch (...) {
		this->eventData.pop_back();
		throw;
	}
}



ClientData::ClientData()
{

}

void ClientData::MakeLast(TimeoutSocket* fSocket)
{
	std::unique_ptr<TimeoutSocket> fSocket_ptr;
	auto WSAevent = this->eventData.erase(std::find(eventData.cbegin(), eventData.cend(), fSocket->WSAEvent));
	auto temp = std::find(socketData.begin(), socketData.end(), fSocket);
	fSocket_ptr = std::move(*temp);
	socketData.erase(temp);
	this->AddClient(std::move(fSocket_ptr)); 
}

void ClientData::Remove(TimeoutSocket * fSocket)
{
	auto WSAevent = this->eventData.erase(std::find(eventData.cbegin(), eventData.cend(), fSocket->WSAEvent));
    auto socket = this->socketData.erase(std::find(socketData.cbegin(), socketData.cend(), fSocket));
}
