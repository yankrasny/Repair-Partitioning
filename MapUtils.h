#pragma once

#ifndef MAP_UTILS_H
#define MAP_UTILS_H

#include <map>

template<typename T, typename S>
bool mapExists(typename std::map<T, S> theMap, T key)
{
	typename std::map<T, S>::iterator it = theMap.find(key);
	return it != theMap.end();
}

#endif