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

class RandomHeap
{
private:
	std::vector<HeapEntry*> heap;
public:
	RandomHeap() {}

	RandomHeap(std::vector<HeapEntry*>& origVec);

	bool empty() const;

	HeapEntry& getAtIndex(int pos);

	HeapEntry& getMax();

	HeapEntry extractMax();

	int insert(HeapEntry* item);

	int heapifyUp(int pos);

	void heapifyDown(int pos);

	void deleteRandom(int pos);

	HeapEntry extractRandom(int pos);

	void cleanup();

	~RandomHeap();
};

class RandomHeapTest
{
private:
	void runTest(int n);
public:
	RandomHeapTest(int numElements);
};

#endif