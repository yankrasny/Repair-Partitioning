#include "IndexedHeap.h"
#include "HeapEntry.h"

IndexedHeap::IndexedHeap(std::vector<HeapEntry*>& origVec)
{
	for (size_t i = 0; i < origVec.size(); i++)
	{
		this->insert(origVec[i]);
	}
}

bool IndexedHeap::empty() const
{
	return heap.size() <= 0;
	//return heap.empty();
}

HeapEntry& IndexedHeap::getAtIndex(int pos)
{
	if (pos >= 0 && pos < heap.size())
	{
		return *heap[pos];
	}
}

HeapEntry& IndexedHeap::getMax()
{
	if (heap.size() > 0)
		return *heap[0];
}

HeapEntry IndexedHeap::extractMax()
{
	return extractAtIndex(0);
}

int IndexedHeap::insert(HeapEntry* item)
{
	heap.push_back(item);
	int index = heapifyUp(heap.size() - 1);
	heap[index]->setIndex(index);
	return index;
}

int IndexedHeap::heapifyUp(int pos)
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

void IndexedHeap::heapifyDown(int pos)
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

void IndexedHeap::deleteAtIndex(int pos)
{
	if (pos >= 0 && pos < heap.size())
	{
		//This messes up the index field
		*heap[pos] = *heap.back();

		//So reset it
		heap[pos]->setIndex(pos);
		
		//Remove the last element
		delete heap.back();
		heap.pop_back();

		//Heapify from the position we just messed with
		heapifyDown(pos);
	}
}

HeapEntry IndexedHeap::extractAtIndex(int pos)
{
	if (pos >= 0 && pos < heap.size())
	{
		HeapEntry item = *heap[pos];
		deleteAtIndex(pos);
		return item;
	}
}

IndexedHeap::~IndexedHeap()
{
	for (size_t i = 0; i < heap.size(); i++)
	{
		delete heap[i];
	}
	heap.clear();
}

void IndexedHeapTest::runTest(int n)
{
	std::vector<unsigned long long> keys = std::vector<unsigned long long>();
	for (int i = 0; i < n; i++)
	{
		keys.push_back((i << 32) | (i+1));
	}
	std::vector<HeapEntry*> vec = std::vector<HeapEntry*>();
	for (int i = 0; i < n; i++)
	{
		HeapEntry* hp = new HeapEntry(keys[i], i, NULL);
		vec.push_back(hp);
	}

	IndexedHeap rHeap(vec);

	rHeap.extractAtIndex(3);
	rHeap.extractAtIndex(2);

	HeapEntry max;
	while (!rHeap.empty())
	{
		max = rHeap.extractMax();
	}
}

IndexedHeapTest::IndexedHeapTest(int numElements)
{
	runTest(numElements);
}
