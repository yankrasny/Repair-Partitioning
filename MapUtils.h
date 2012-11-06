#pragma once

#ifndef MAP_UTILS_H
#define MAP_UTILS_H

#include <unordered_map>

template<typename T, typename S>
bool mapExists(typename std::unordered_map<T, S> theMap, T key)
{
	//typename std::unordered_map<T, S>::iterator it = theMap.find(key);
	//return it != theMap.end();
	return 1; //theMap[key]!=(S)0;
}

#endif