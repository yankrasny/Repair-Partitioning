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

// class HeapEntry;
class HeapEntryPtr;

class IndexedHeap
{
private:
	std::vector<HeapEntryPtr> heap;
public:
	IndexedHeap()
	{
		heap = std::vector<HeapEntryPtr>();
	}

	int getSize() {
		return heap.size();
	}

	IndexedHeap(const std::vector<unsigned long long>& origVec);

	bool empty() const;

	HeapEntryPtr getAtIndex(int pos) const;

	HeapEntryPtr getMax() const;

	HeapEntryPtr extractMax();

	HeapEntryPtr insert(unsigned long long key);

	void deleteAtIndex(int pos);

	HeapEntryPtr extractAtIndex(int pos);

	int heapifyUp(int pos);

	void heapifyDown(int pos);

	void cleanup();

	IndexedHeap(const IndexedHeap& rhs);

	IndexedHeap& operator=(const IndexedHeap& rhs);

	~IndexedHeap();

	bool checkValid();
};

class IndexedHeapTest
{
private:
	void runTest(unsigned long long n);
public:
	IndexedHeapTest(unsigned long long numElements);
};

#endif