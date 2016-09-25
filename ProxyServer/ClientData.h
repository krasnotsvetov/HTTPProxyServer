#ifndef CLIENTDATA_H
#define CLIENTDATA_H
#pragma once
#include <vector>
#include <memory>
#include <WinSock2.h>
#include "TimeoutSocket.h"
struct ClientData {
public:
	ClientData();

	std::vector<WSAEVENT> eventData;
	std::vector<std::unique_ptr<TimeoutSocket>> socketData;

	void MakeLast(TimeoutSocket* socket);
	void Remove(TimeoutSocket* socket);
	void AddClient(std::unique_ptr<TimeoutSocket> socket);

};
#endif