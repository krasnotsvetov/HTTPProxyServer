#include "Common.h"

RequestState RequestCommon::parseRequest(std::string _request)
{
	int numLine = 0;
	int pos = 0;
	std::string temp = _request;
	while ((pos = temp.find("\r\n")) != 0)
	{
		if (pos == std::string::npos) {
			requestState = RequestState::BadRequest;
			return requestState;
		}
		std::string line = temp.substr(0, pos);
		headers.push_back(line);
		temp = temp.substr(pos + 2);
		numLine++;

	}


	this->request = _request;
	std::string firstLine = request.substr(0, request.find("\r\n"));
	firstLine = getMethod(firstLine, method);
	if (method != "GET" && method != "POST" && method != "CONNECT") {
		requestState = RequestState::BadRequest;
		return requestState;
	}
	firstLine = getURL(firstLine, url, port); 
	getVersion(firstLine, version);
	if (version != "HTTP/1.1" && version != "HTTP/1.0") {
		requestState = RequestState::BadRequest;
		return RequestState::BadRequest;
	}
	requestState = RequestState::AcceptRequest;
	return requestState;
}

RequestCommon::~RequestCommon()
{
}

std::string RequestCommon::getMethod(std::string fLine, std::string& method) {
	int i = fLine.find(" ");
	
	method = fLine.substr(0, i);
	return fLine.substr(i + 1);
}

std::string RequestCommon::getURL(std::string fLine, std::string& url, std::string& port) {
	int i = fLine.find(" ");
	
	std::string _serverAdress = fLine.substr(0, i);
	this->serverAdress = _serverAdress;
	fLine = fLine.substr(i + 1);
	std::string pattern = "http://";
	
	_serverAdress = _serverAdress.substr(_serverAdress.find(pattern) == std::string::npos ? 0 : pattern.size());

	i = _serverAdress.find_first_of("/");
	url = "";
	if (i != std::string::npos)
	{ 
		url = _serverAdress.substr(0, i);
		i = _serverAdress.find_last_of(":");
		if (i == std::string::npos)
		{
			port = "80";
		}
		else {
			port = _serverAdress.substr(i + 1);
			if (port.find_first_not_of("0123456789") != std::string::npos)
			{
				port = "80";
			} 
		}
	}
	
	return fLine;
}

std::string RequestCommon::getVersion(std::string fLine, std::string& version) {
	version = fLine;
	return fLine;
}

RequestState RequestCommonServer::parseRequest(std::string _request)
{
	int numLine = 0;
	int pos = 0;

	std::string temp = _request;
	this->request = _request;

	while ((pos = temp.find("\r\n")) != 0)
	{
		if (pos == std::string::npos) {
			requestState = RequestState::BadRequest;
			return requestState;
		}
		std::string line = temp.substr(0, pos);
		headers.push_back(line);

		if (line.find("Vary") != std::string::npos) {
			vary = "Accept-encoding";
		}

		if (line.find("ETag") != std::string::npos) {
			eTag = line.substr(6);
			int endPos = eTag.find_last_of("\"");
			if (endPos != std::string::npos) {
				eTag = eTag.substr(0, endPos);

				int startPos = eTag.find_first_of("\"");
				if (startPos != std::string::npos) {
					eTag = eTag.substr(startPos + 1);
				}
				
				
			}
		}

		if (line.find("Content-Length:") != std::string::npos) {
			std::string num = line.substr(16);
			contentLength = std::atoi(num.c_str());
		}
		temp = temp.substr(pos + 2);
		numLine++;

	}
	headerSize = 0;
	if (temp.find("\r\n") != std::string::npos) {
		headerSize = temp.substr(2).length();
	}


	{
		std::string fLine = headers[0];
		fLine = getVersion(fLine, version);
		if (version != "HTTP/1.1" && version != "HTTP/1.0") {
			requestState = RequestState::BadRequest;
			return RequestState::BadRequest;
		}
		fLine = getStatus(fLine, status);
		if (status == "Failed") {
			requestState = RequestState::BadRequest;
			return RequestState::BadRequest;
		}
	}
	/*
	if (status == "301") {
		std::string temp = request;
		int i = request.find("\r\n");
		std::string fLine = request.substr(0, i);
		request = request.substr(i + 2);
		i = request.find("\r\n");
		std::string sLine = request.substr(0, i);
		request = request.substr(i + 2);
		std::string finalSLine = "";
		for (int i = 0; i < sLine.length(); i++) {
			finalSLine += sLine[i];
			if (finalSLine.find("http://") != std::string::npos || finalSLine.find("https://") != std::string::npos) {
				sLine = sLine.substr(i + 1);
				int pos = sLine.find("http://");
				int pos2 = sLine.find("https://");
				int finalPos = sLine.length();

				if (pos != std::string::npos  && pos < finalPos) {
					finalPos = pos;
				}

				if (pos2 != std::string::npos  && pos2 < finalPos) {
					finalPos = pos2;
				}

				finalSLine += sLine.substr(0, pos);
				break;
			}
		}
		i = sLine.find("http://");


		sLine = sLine.substr(0, i);
		request = fLine + "\r\n" + finalSLine + "\r\n" + request;
		//		std::string fLine = request.substr(0, i);
	}*/

	requestState = RequestState::AcceptRequest;
	return RequestState::AcceptRequest;
}

std::string RequestCommonServer::getVersion(std::string fLine, std::string & version)
{
	int i = fLine.find(" ");
	if (i == std::string::npos) {
		version = "Failed";
		return fLine;
	}
	version = fLine.substr(0, i);
	return fLine.substr(i + 1);
}

std::string RequestCommonServer::getStatus(std::string fLine, std::string & status)
{
	int i = fLine.find_first_of(" ");
	if (i == std::string::npos) {
		i = fLine.length();
	}
	status = fLine.substr(0, i);
	if (status.find_first_not_of("0123456789") != std::string::npos)
	{
		status = "Failed";
	}
	if (i == fLine.length()) {
		return "";
	}
	return fLine.substr(i + 1); 
}

