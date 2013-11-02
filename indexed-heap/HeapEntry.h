#pragma once

#ifndef HEAP_ENTRY_H
#define HEAP_ENTRY_H

#include <algorithm>
#include <vector>
#include <memory>
#include "IndexedHeap.h"


class HeapEntry
{
private:
	int index; //the object must know where it is, so that some other reference can find it in O(1) inside the heap
	size_t priority;
	unsigned long long key;
	std::shared_ptr<IndexedHeap> myHeap;
	// IndexedHeap* myHeap;
public:
	HeapEntry(unsigned long long key, size_t priority, int index, IndexedHeap* myHeap)
		: key(key), priority(priority), index(index), myHeap(myHeap) {}
	
	HeapEntry(unsigned long long key, size_t priority, IndexedHeap* myHeap)
		: key(key), priority(priority), index(-1), myHeap(myHeap) {}

	HeapEntry(const HeapEntry& rhs)
	{
		index = rhs.index;
		priority = rhs.priority;
		key = rhs.key;
		myHeap = rhs.myHeap;
	}

	HeapEntry& operator=(const HeapEntry& rhs)
	{
		key = rhs.key;
		priority = rhs.priority;
		// The following line doesn't work, but we don't need it
		// Never copy a heap entry to a different heap
		// myHeap.reset(rhs.myHeap.get());
		return *this;
	}
	
	HeapEntry() : key(0), priority(0), index(-1), myHeap(NULL) {}

	unsigned long long getKey();

	unsigned getLeft();

	unsigned getRight();

	size_t getPriority() const;
	
	void increment();

	void decrement();

	~HeapEntry()
	{
		priority = 0;
		index = -1;
		key = 0;
		// myHeap deleter is called
	}

	int getIndex();

	void setIndex(int index);
};

#endif