#include "LRUCache.h"
#include <iostream>

bool LRUCache::Add(std::string serverAdress, std::string eTag, std::string value)
{
	std::unique_lock<std::mutex> lock(mutex);
	if (map.find(serverAdress) != map.end()) {
		auto element = (*map[serverAdress]);
		element.first = eTag;
		element.second = value;
		list.splice(list.begin(), list, map[serverAdress]);
		//list.front() = std::make_pair(hashCode, value);
		
	}
	else {
		list.insert(list.begin(), std::make_pair(eTag, value));
	}

	map[serverAdress] = list.begin();
		
	if (list.size() > cacheSize) {
		
		map.erase(list.back().first);
		list.pop_back();
	}
	return true;
}

bool LRUCache::Contains(std::string serverAdress)
{
	std::unique_lock<std::mutex> lock(mutex);
	auto pos = map.find(serverAdress);
	if (pos == map.end()) {
		return false;
	}
	return true;
}

std::pair<std::string, std::string> LRUCache::Get(std::string serverAdress)
{
	std::unique_lock<std::mutex> lock(mutex);
	auto pos = map.find(serverAdress);
	if (pos != map.end()) {
		return (*map[serverAdress]);
	}
	else {
		throw std::exception("");
	}

}
LRUCache::LRUCache(size_t cacheSize, size_t objectMaxSize)
{
	this->cacheSize = cacheSize;
	this->objectMaxSize = objectMaxSize;
}
 
