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
	int index; // the object must know where it is, so it can be found in O(1) inside the heap
	size_t priority; // defines the heap ordering
	unsigned long long key; // the identifier
	// std::shared_ptr<IndexedHeap> myHeap; // reference to the heap, shared among all heap entries
	IndexedHeap* myHeap;
public:
	HeapEntry(unsigned long long key, size_t priority, IndexedHeap* myHeap, int index)
		: key(key), priority(priority), index(index), myHeap(myHeap) {}

	/************* BIG 3 *************/
	// This is weird, we're not really managing any memory here, so do we need big three at all?
	// What happens when we use std::swap?

	// Copy Constructor, nothing is initialized
	// HeapEntry(const HeapEntry& rhs)
	// {
	// 	index = rhs.index;
	// 	priority = rhs.priority;
	// 	key = rhs.key;
	// 	myHeap = rhs.myHeap;

	// }

	// Assignment Operator, data is all initialized
	// Do we have anything that needs to be cleaned up before copying values from rhs?
	// HeapEntry& operator=(const HeapEntry& rhs)
	// {
	// 	if (this != &rhs) {
	// 		key = rhs.key;
	// 		priority = rhs.priority;
	// 	}
	// 	return *this;
	// }
	
	// Destructor: do we have anything to release?
	// What happens to myHeap, which is shared among many heap entries?
	// Do we define this explicitly as empty or do we not define it?

	// ~HeapEntry()
	// {
	// 	// myHeap.reset();
	// }
	/************* End BIG 3 *************/

	HeapEntry() : key(0), priority(0), index(-1), myHeap(NULL) {}

	unsigned long long getKey();

	unsigned getLeft();

	unsigned getRight();

	size_t getPriority() const;
	
	void increment();

	void decrement();

	int getIndex();

	void setIndex(int index);

	IndexedHeap* const getMyHeap();
};

// Wrap the pointer to HeapEntry to better understand the memory corruption
class HeapEntryPtr
{
private:
	HeapEntry* realPointer;
public:
	HeapEntryPtr()
	{
		std::cerr << "Constructing Default HeapEntryPtr = NULL " << std::endl;
		std::cerr << std::endl;
		realPointer = NULL;
	}

	HeapEntryPtr(HeapEntry* hp)
	{
		std::cerr << "Constructing HeapEntryPtr with index = " << hp->getIndex() << std::endl;
		std::cerr << std::endl;
		realPointer = hp;
	}

	HeapEntryPtr (const HeapEntryPtr& rhs)
	{
		std::cerr << "Copy Constructor for HeapEntryPtr " << std::endl;
		std::cerr << "this->index = " << this->realPointer->getIndex() << std::endl;
		std::cerr << "rhs->index = " << rhs.realPointer->getIndex() << std::endl;
		std::cerr << std::endl;

		this->realPointer = new HeapEntry(rhs.getPtr()->getKey(), rhs.getPtr()->getPriority(), rhs.getPtr()->getMyHeap(), rhs.getPtr()->getIndex());
	}

	HeapEntryPtr& operator=(const HeapEntryPtr& rhs)
	{
		std::cerr << "operator= for HeapEntryPtr " << std::endl;
		std::cerr << "this->index = " << this->realPointer->getIndex() << std::endl;
		std::cerr << "rhs->index = " << rhs.realPointer->getIndex() << std::endl;
		std::cerr << std::endl;
		if (this != &rhs)
		{
			delete this->realPointer;
			this->realPointer = new HeapEntry(rhs.getPtr()->getKey(), rhs.getPtr()->getPriority(), rhs.getPtr()->getMyHeap(), rhs.getPtr()->getIndex());
		}
		return *this;
	}

	// Give access to the pointer to minimize code changes
	HeapEntry* getPtr() const
	{
		return realPointer;
	}

	// Explicitly release the memory
	void kill()
	{
		std::cerr << "Killing HeapEntryPtr with index = " << realPointer->getIndex() << std::endl;
		std::cerr << std::endl;
		delete realPointer;
	}
	
	// Take note of when this happens, perhaps we go out of scope and don't notice
	~HeapEntryPtr()
	{
		std::cerr << "Destructor for HeapEntryPtr with index = " << realPointer->getIndex() << std::endl;
		std::cerr << std::endl;
		delete realPointer;
	}
};

#endif