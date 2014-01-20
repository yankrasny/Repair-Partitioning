#pragma once

#ifndef HEAP_ENTRY_H
#define HEAP_ENTRY_H

#include <algorithm>
#include <vector>
#include <iostream>
// #include <memory>
#include "IndexedHeap.h"

class HeapEntry
{
private:
	unsigned long long key; // the identifier
	size_t priority; // defines the heap ordering
	IndexedHeap* myHeap;
	int index; // the object must know where it is, so it can be found in O(1) inside the heap
public:
	HeapEntry(unsigned long long key, size_t priority, IndexedHeap* myHeap, int index)
		: key(key), priority(priority), myHeap(myHeap), index(index) {
			// std::cerr << "Constructor for HeapEntry[key = " << key << "]" << std::endl;
		}

	/************* BIG 3 *************/
	// Copy Constructor, nothing is initialized
	HeapEntry(const HeapEntry& rhs)
	{
		index = rhs.index;
		priority = rhs.priority;
		key = rhs.key;
		myHeap = rhs.myHeap;
	}

	// Assignment Operator, data is all initialized
	// Define assignment as copying key and priority
	// The heap is the same for everyone, just leave it
	// Don't take the index from rhs either, just take its values
	HeapEntry& operator=(const HeapEntry& rhs)
	{
		if (this != &rhs) {
			this->key = rhs.key;
			this->priority = rhs.priority;
		}
		return *this;
	}
	
	// Don't release the mem for myHeap because others are pointing to it
	// Just make sure this object no longer points to it
	~HeapEntry()
	{
		myHeap = NULL;
		// std::cerr << "Destructor for HeapEntry[key = " << key << "]" << std::endl;
	}
	/************* End BIG 3 *************/

	HeapEntry() : key(0), priority(0), myHeap(NULL), index(-1) {}

	unsigned long long getKey()
	{
		return key;
	}

	unsigned getLeft()
	{
		return key >> 32;
	}

	unsigned getRight()
	{
		return (key << 32) >> 32;
	}

	size_t getPriority() const
	{
		return priority;
	}
	
	void increment()
	{
		priority++;
		myHeap->heapifyUp(index);
	}

	void decrement()
	{
		priority--;
		myHeap->heapifyDown(index);
	}

	int getIndex()
	{
		return index;
	}

	void setIndex(int index)
	{
		this->index = index;
	}

	IndexedHeap* const getMyHeap()
	{
		return myHeap;
	}

};
#endif
