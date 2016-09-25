#pragma once
#ifndef COMMON_H
#define COMMON_H
#include <map>
#include <memory>
#include <vector>


template<typename T>
bool operator==(std::unique_ptr<T> const& smart_a, T* a)
{
	return smart_a.get() == a;
}

template<typename T>
bool operator==(T* a, std::unique_ptr<T> const& smart_a)
{
	return a == smart_a.get();
}

enum class RequestState {
	BadRequest, AcceptRequest, NonRequest
};

enum class EventState {
	Success, Failed, Retry
};

enum class EventCase {
	Write, WriteAgain, Read, ReadAgain, Connect, ConnectAgain, Accept, AcceptAgain, Close
};

enum class ExitState {
	Non, Soft, Hard
};


enum class CacheState {
	Non, NeedReturn, NeedCache, Failed
};

struct RequestCommon {

public:
	RequestState requestState = RequestState::NonRequest;
	std::string url;
	std::string method;
	std::string version;
	std::string port;
	std::string serverAdress; // used for cache. Need implement better cache key. TO DO
	std::map<std::string, std::string> parametrs;
	std::string request;

	virtual RequestState parseRequest(std::string _request);

	virtual ~RequestCommon();

	std::vector<std::string> headers;
private:
	std::string getMethod(std::string fLine, std::string & method);
	std::string getURL(std::string fLine, std::string & url, std::string & port);

	virtual std::string getVersion(std::string fLine, std::string & version);
};

struct RequestCommonServer : RequestCommon {
public:
	std::string status;
	std::string eTag;
	std::string vary;
	std::string getStatus(std::string fLine, std::string & status);

	int headerSize;
	int contentLength = -1;

	virtual RequestState parseRequest(std::string _request) override;
	virtual std::string getVersion(std::string fLine, std::string & version) override;
};


#endif