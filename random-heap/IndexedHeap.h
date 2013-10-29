#pragma once

#ifndef RANDOM_HEAP_H
#define RANDOM_HEAP_H

#include <algorithm>
#include <cmath>
#include <vector>

/*
Goal: to be able to refer to a specific entry from an outside data structure, in O(1)
That's why we store the index in HeapEntry
*/

class HeapEntry;

class IndexedHeap
{
private:
	std::vector<HeapEntry*> heap;
public:
	IndexedHeap()
	{
		heap = std::vector<HeapEntry*>();
	}

	IndexedHeap(const std::vector<HeapEntry*>& origVec);

	bool empty() const;

	HeapEntry& getAtIndex(int pos) const;

	HeapEntry& getMax() const;

	HeapEntry extractMax();

	int insert(HeapEntry* item);

	int heapifyUp(int pos);

	void heapifyDown(int pos);

	void deleteAtIndex(int pos);

	HeapEntry extractAtIndex(int pos);

	void cleanup();

	IndexedHeap(const IndexedHeap& rhs);

	IndexedHeap& operator=(const IndexedHeap& rhs);

	~IndexedHeap();
};

class IndexedHeapTest
{
private:
	void runTest(int n);
public:
	IndexedHeapTest(int numElements);
};

#endif