#pragma once

#ifndef MAP_UTILS_H
#define MAP_UTILS_H

#include <tr1/unordered_map>

template<typename T, typename S>
bool mapExists(typename std::tr1::unordered_map<T, S> theMap, T key)
{
	typename std::tr1::unordered_map<T, S>::iterator it = theMap.find(key);
	return it != theMap.end();
}

#endif