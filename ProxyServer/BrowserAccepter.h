#pragma once 
#include <winsock2.h>
#include "Common.h"
#include <ctime> 
#include "LRUCache.h"
#include "TimeoutSocket.h"



struct BrowserAccepter : TimeoutSocket
{
public:
	BrowserAccepter(SOCKET socket);
	virtual ~BrowserAccepter();
};