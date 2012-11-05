#pragma once

#ifndef HEAP_ENTRY_H
#define HEAP_ENTRY_H

#include <algorithm>
#include <vector>
#include "RandomHeap.h"


class HeapEntry
{
private:
	int index; //the object must know where it is, so that some other reference can find it in O(1) inside the heap
	size_t priority;
	std::vector<unsigned> key;
	RandomHeap* myHeap;
public:
	HeapEntry(std::vector<unsigned> key, size_t priority, int index, RandomHeap* myHeap) : key(key), priority(priority), index(index), myHeap(myHeap) {}
	HeapEntry(std::vector<unsigned> key, size_t priority, RandomHeap* myHeap) : key(key), priority(priority), index(-1), myHeap(myHeap) {}
	HeapEntry() : key(std::vector<unsigned>()), priority(0), index(-1), myHeap(NULL) {}

	std::vector<unsigned> getKey();

	size_t getPriority() const;
	
	void increment();

	void decrement();

	~HeapEntry();
	
	int getIndex();

	void setIndex(int index);
};

#endif