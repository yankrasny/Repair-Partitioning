#pragma once

#ifndef RANDOM_HEAP_H
#define RANDOM_HEAP_H

#include <algorithm>
#include <cmath>
#include <vector>

class HeapEntry
{
private:
	int index; //the object must know where it is, so that some other reference can find it in O(1)
	size_t priority;
	std::vector<unsigned> key;
public:
	HeapEntry(std::vector<unsigned> key, size_t priority, int index) : key(key), priority(priority), index(index) {}
	HeapEntry(std::vector<unsigned> key, size_t priority) : key(key), priority(priority), index(-1) {}
	HeapEntry() : key(std::vector<unsigned>()), priority(0), index(-1) {}

	std::vector<unsigned> getKey() {return key;}

	size_t getPriority() const
	{
		return priority;
	}
	
	void increment()
	{
		priority++;
	}

	void decrement()
	{
		priority--;
	}

	~HeapEntry()
	{
		priority = 0;
		index = -1;
	}
	
	int getIndex()
	{
		return index;
	}

	void setIndex(int index)
	{
		this->index = index;
	}
};

/*
Goal: to be able to refer to a specific entry from an outside data structure, in O(1)
That's why we store the index in HeapEntry
*/
class RandomHeap
{
private:
	std::vector<HeapEntry*> heap;
public:
	RandomHeap() {}

	RandomHeap(std::vector<HeapEntry*>& origVec)
	{
		for (size_t i = 0; i < origVec.size(); i++)
		{
			this->insert(origVec[i]);
		}
	}

	bool empty() const
	{
		return heap.size() <= 0;
		//return heap.empty();
	}

	HeapEntry& getAtIndex(int pos)
	{
		if (pos >= 0 && pos < heap.size())
		{
			return *heap[pos];
		}
	}

	HeapEntry& getMax()
	{
		if (heap.size() > 0)
			return *heap[0];
	}

	HeapEntry extractMax()
	{
		return extractRandom(0);
	}

	int insert(HeapEntry* item)
	{
		heap.push_back(item);
		int index = heapifyUp(heap.size() - 1);
		heap[index]->setIndex(index);
		return index;
	}

	int heapifyUp(int pos)
	{
		bool done = false;
		while (!done)
		{
			// if at any point during this alg we go out of bounds, we're done
			if ( !( pos >= 1 && pos < heap.size() ) )
			{
				return pos;
			}

			// some nonsense casting to make c++ happy
			int parent = (float) floor( ((float)pos-1) / 2.0 );

			// the current element is greater than its parent, swap it up and continue with the parent's position
			if ( heap[pos]->getPriority() > heap[parent]->getPriority() )
			{
				//Keeping the indexes correct while swapping
				heap[pos]->setIndex(parent);
				heap[parent]->setIndex(pos);

				//Swap
				std::swap( heap[pos], heap[parent] );
				
				//Next position we're looking at is the parent
				pos = parent;
				continue;
			}
			done = true;
		}
		return pos;
	}

	void heapifyDown(int pos)
	{
		bool done = false;
		while (!done)
		{
			int leftChildIndex = 2*pos + 1;
			int rightChildIndex = 2*pos + 2;
			if (leftChildIndex >= heap.size())
			{
				// there is no left child, so we're at a leaf, done
				done = true;
				continue;
			}
			if (heap[pos]->getPriority() < heap[leftChildIndex]->getPriority())
			{
				// keep indexes updated
				heap[pos]->setIndex(leftChildIndex);
				heap[leftChildIndex]->setIndex(pos);

				// swap
				std::swap(heap[pos], heap[leftChildIndex]);
				
				// the current element is smaller than its left child, swap and check that position
				pos = leftChildIndex;
				continue;
			}

			if (rightChildIndex >= heap.size())
			{
				// there is no right child, and we already checked the left, done
				done = true;
				continue;
			}

			if (heap[pos]->getPriority() < heap[rightChildIndex]->getPriority())
			{
				// keep indexes updated
				heap[pos]->setIndex(rightChildIndex);
				heap[rightChildIndex]->setIndex(pos);
				
				// swap
				std::swap(heap[pos], heap[rightChildIndex]);
				
				// the current element is smaller than its right child, swap and check that position
				pos = rightChildIndex;
				continue;
			}

			// if we got here, then none of those ifs ran, so there is nothing left to do
			done = true;
		}
	}

	void deleteRandom(int pos)
	{
		if (pos >= 0 && pos < heap.size())
		{
			//This messes up the index field
			heap[pos] = heap.back();

			//So reset it
			heap[pos]->setIndex(pos);
			
			//Remove the last element
			heap.pop_back();

			//Heapify from the position we just messed with
			heapifyDown(pos);
		}
	}

	//Can't we store and update an association (key -> index) ??
	HeapEntry extractRandom(int pos)
	{
		if (pos >= 0 && pos < heap.size())
		{
			HeapEntry item = *heap[pos];
			deleteRandom(pos);
			return item;
		}
	}

	void cleanup()
	{
		while (!empty())
		{
			deleteRandom(0);
		}
	}

	~RandomHeap()
	{
		cleanup();
	}
};

class RandomHeapTest
{
private:
	void runTest(int n)
	{
		std::vector<std::vector<unsigned> > keys = std::vector<std::vector<unsigned> >();
		for (int i = 0; i < n; i++)
		{
			std::vector<unsigned> key = std::vector<unsigned>();
			key.push_back(i);
			key.push_back(i+1);
			keys.push_back(key);
		}
		std::vector<HeapEntry*> vec = std::vector<HeapEntry*>();
		for (int i = 0; i < n; i++)
		{
			HeapEntry* hp = new HeapEntry(keys[i], i);
			vec.push_back(hp);
		}
	
		RandomHeap rHeap(vec);

		rHeap.extractRandom(3);
		rHeap.extractRandom(2);

		HeapEntry max;
		while (!rHeap.empty())
		{
			max = rHeap.extractMax();
		}
	}
public:
	RandomHeapTest(int numElements)
	{
		runTest(numElements);
	}
};

#endif