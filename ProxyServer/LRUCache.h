#pragma once
#ifndef LRUCACHE_H
#define LRUCACHE_H
#include <string>
#include <map>
#include <list>
#include <mutex>
#include <list>
#include <vector>

struct LRUCache {
public:
	size_t cacheSize = 1024;
	size_t objectMaxSize = 16384;

	LRUCache(size_t cacheSize, size_t objectMaxSize);

	bool Add(std::string serverAdress, std::string eTag, std::string value);
	bool Contains(std::string serverAdress);

	std::pair<std::string, std::string> LRUCache::Get(std::string serverAdress);
private:
	mutable std::mutex mutex;
	std::list<std::pair<std::string, std::string>> list;
	std::map<std::string, std::list<std::pair<std::string, std::string>>::iterator> map;
	//std::map<int, decltype(list.begin())> map;


};
#endif
