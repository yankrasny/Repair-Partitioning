#pragma once

#ifndef INDEXED_HEAP_H
#define INDEXED_HEAP_H

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

	int getSize() {
		return heap.size();
	}

	IndexedHeap(const std::vector<unsigned long long>& origVec);

	bool empty() const;

	HeapEntry& getAtIndex(int pos) const;

	HeapEntry& getMax() const;

	HeapEntry extractMax();

	HeapEntry* insert(unsigned long long key);

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