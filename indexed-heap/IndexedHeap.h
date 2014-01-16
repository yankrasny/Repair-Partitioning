#pragma once

#ifndef INDEXED_HEAP_H
#define INDEXED_HEAP_H

#include <algorithm>
#include <cmath>
#include <vector>
#include <iostream>
#include <assert.h>


/*
Goal: to be able to refer to a specific entry from an outside data structure, in O(1)
That's why we store the index in HeapEntry
*/

class HeapEntry;
// class HeapEntryPtr;

class IndexedHeap
{
	friend std::ostream& operator<<(std::ostream& os, const IndexedHeap& rhs);

private:
	std::vector<HeapEntry*> heap;
	unsigned numDeleted = 0;
public:
	IndexedHeap()
	{
		heap = std::vector<HeapEntry*>();
	}

	int getSize() {
		return heap.size();
	}

	IndexedHeap(const std::vector<unsigned long long>& origVec);

	bool empty() const;

	unsigned getNumDeleted() const
	{
		return numDeleted;
	}

	void incrementDeleted()
	{
		++numDeleted;
	}

	void printHeap() const;

	HeapEntry* getAtIndex(unsigned pos) const;

	HeapEntry* getMax() const;

	HeapEntry* getBack() const;

	HeapEntry* extractMax();

	HeapEntry* insert(unsigned long long key);

	int deleteAtIndex(int pos);

	HeapEntry* extractAtIndex(int pos);

	int heapifyUp(int pos);

	int heapifyDown(int pos);

	void cleanup();

	// Big 3
	// IndexedHeap(const IndexedHeap& rhs);

	// IndexedHeap& operator=(const IndexedHeap& rhs);

	// ~IndexedHeap();

	void checkValid();
};

class IndexedHeapTest
{
private:
	void runTest(unsigned long long n);
public:
	IndexedHeapTest(unsigned long long numElements);
};

#endif
