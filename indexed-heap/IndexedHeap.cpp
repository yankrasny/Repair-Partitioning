#include "IndexedHeap.h"
#include "HeapEntry.h"

IndexedHeap::IndexedHeap(const std::vector<unsigned long long>& origVec)
{
	for (size_t i = 0; i < origVec.size(); i++)
	{
		this->insert(origVec[i]);
	}
}

bool IndexedHeap::empty() const
{
	return heap.size() <= 0;
}

HeapEntry& IndexedHeap::getAtIndex(int pos) const
{
	if (pos >= 0 && pos < heap.size())
	{
		return *heap[pos];
	}
}

HeapEntry& IndexedHeap::getMax() const
{
	if (heap.size() > 0)
		return *heap[0];
}

HeapEntry IndexedHeap::extractMax()
{
	return extractAtIndex(0);
}

HeapEntry* IndexedHeap::insert(unsigned long long key)
{
	HeapEntry* entry = new HeapEntry(key, 1, this);
	heap.push_back(entry);
	int index = heapifyUp(heap.size() - 1);
	heap[index]->setIndex(index);
	return entry;
}

void IndexedHeap::deleteAtIndex(int pos)
{
	if (pos >= 0 && pos < heap.size())
	{
		if (pos == heap.size() - 1)
		{
			delete heap.back();
			heap.pop_back();
			return;
		}

		// Copy heap.back() into the position of target, thus overwriting it
		*heap[pos] = *heap.back();

		// Fix the index field for the just copied element
		heap[pos]->setIndex(pos);
		
		// We've removed the target by overwriting it with heap.back()
		// Now get rid of the extra copy of heap.back()
		// Release the mem, then pop back to get rid of the pointer
		delete heap.back();
		heap.pop_back();

		// Heapify from the position we just messed with
		// use heapifyDown because back() always has a lower priority than the element we are removing
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
		if (heap[pos]->getPriority() > heap[parent]->getPriority())
		{
			// swap
			std::swap(heap[pos], heap[parent]);

			// keeping the indexes correct after swapping
			heap[pos]->setIndex(pos);
			heap[parent]->setIndex(parent);
			
			// next position we're looking at is the parent
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
			// they are out of order, swap
			std::swap(heap[pos], heap[leftChildIndex]);

			// keep indexes updated
			heap[pos]->setIndex(pos);
			heap[leftChildIndex]->setIndex(leftChildIndex);
			
			// the current element was smaller than its left child
			// now that we've swapped them, we move down
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
			// they are out of order, swap
			std::swap(heap[pos], heap[rightChildIndex]);

			// keep indexes updated
			heap[pos]->setIndex(pos);
			heap[rightChildIndex]->setIndex(rightChildIndex);
			
			// the current element was smaller than its right child
			// now that we've swapped them, we move down
			pos = rightChildIndex;
			continue;
		}

		// if we got here, then none of those ifs ran, meaning everything is in order so there is nothing left to do
		done = true;
	}
}

/****************************** BIG 3 *********************************/
IndexedHeap::IndexedHeap(const IndexedHeap& rhs) 
{
	std::vector<HeapEntry*> otherHeap = rhs.heap;
	for (size_t i = 0; i < otherHeap.size(); i++)
	{
		this->heap.push_back(otherHeap[i]);
	}
}

IndexedHeap& IndexedHeap::operator=(const IndexedHeap& rhs) 
{
	for (size_t i = 0; i < heap.size(); i++)
	{
		delete heap[i];
	}
	heap.clear();

	std::vector<HeapEntry*> otherHeap = rhs.heap;
	for (size_t i = 0; i < otherHeap.size(); i++)
	{
		this->heap.push_back(otherHeap[i]);
	}
	return *this;
}

IndexedHeap::~IndexedHeap()
{
	for (size_t i = 0; i < heap.size(); i++)
	{
		delete heap[i];
	}
	heap.clear();
}
/****************************** END BIG 3 *********************************/


void IndexedHeapTest::runTest(unsigned long long n)
{
	std::vector<unsigned long long> keys = std::vector<unsigned long long>();
	for (unsigned long long i = 0; i < n; i++)
	{
		keys.push_back((i << 32) | (i+1));
	}

	IndexedHeap rHeap(keys);
	keys.clear();

	for (unsigned long long i = 0; i < n / 2; i++) {
		rHeap.extractAtIndex(3);
	}

	rHeap.insert(1284762);
	rHeap.insert(12864588);
	rHeap.insert(12864589);
	rHeap.insert(12864587);

	HeapEntry max;
	while (!rHeap.empty())
	{
		max = rHeap.extractAtIndex(rHeap.getSize()-1);
	}
}

IndexedHeapTest::IndexedHeapTest(unsigned long long numElements)
{
	runTest(numElements);
}
