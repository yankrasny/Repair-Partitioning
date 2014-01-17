#include "IndexedHeap.h"
#include "HeapEntry.h"
#include <iostream>

std::ostream& operator<<(std::ostream& os, const IndexedHeap& rhs)
{
	os << "Printing Heap... " << std::endl;
	for (size_t i = 0; i < rhs.heap.size(); i++)
	{
		os << "i: " << i << ", ";
		os << "Index: " << rhs.heap[i]->getIndex() << ", ";
		os << "Priority: " << rhs.heap[i]->getPriority() << ", ";
		os << "Key: " << rhs.heap[i]->getKey() << std::endl;
	}
	return os;
}

IndexedHeap::IndexedHeap(const std::vector<unsigned long long>& origVec)
{
	for (size_t i = 0; i < origVec.size(); i++)
	{
		this->insert(origVec[i]);
	}
}

bool IndexedHeap::empty() const
{
	// unsigned numValid = heap.size() - numDeleted;
	// return numValid <= 0;
	return heap.size() < 1;
}

void IndexedHeap::printHeap() const
{
	std::cerr << "Printing Heap... " << std::endl;
	for (size_t i = 0; i < heap.size(); i++)
	{
		std::cerr << "i: " << i << ", ";
		std::cerr << "Index: " << heap[i]->getIndex() << ", ";
		std::cerr << "Priority: " << heap[i]->getPriority() << ", ";
		std::cerr << "Key: " << heap[i]->getKey() << std::endl;
	}
}

HeapEntry* IndexedHeap::getAtIndex(unsigned pos) const
{
	if (pos >= 0 && pos < heap.size())
	{
		return heap[pos];
	}
	return NULL;
}

HeapEntry* IndexedHeap::getMax() const
{
	if (heap.size() > 0)
	{
		return heap[0];
	}
	return NULL;
}

HeapEntry* IndexedHeap::getBack() const
{
	return heap.back();
}

HeapEntry* IndexedHeap::extractMax()
{
	return extractAtIndex(0);
}

HeapEntry* IndexedHeap::insert(unsigned long long key)
{
	HeapEntry* entry = new HeapEntry(key, 1, this, heap.size());
	heap.push_back(entry);
	// int index = heapifyUp(heap.size() - 1);
	// heap[index]->setIndex(index);
	// this->printHeap();
	return entry;
}

int IndexedHeap::deleteAtIndex(int pos)
{
	this->checkValid();
	// std::cerr << "Starting deleteAtIndex(" << pos << ")..." << std::endl;
	// this->printHeap();
	if (pos == heap.size() - 1)
	{
		// HeapEntry* last = heap.back();
		// last.kill();
		delete heap.back();
		heap.pop_back();
		// this->printHeap();
		return -1;
	}

	// Copy heap.back() into the position of target, thus overwriting it
	*heap[pos] = *heap.back();

	// Fix the index field for the just copied element
	// heap[pos]->setIndex(pos);
	
	// We've removed the target by overwriting it with heap.back()
	// Now get rid of the extra copy of heap.back()
	// Release the mem, then pop back to get rid of the pointer
	// HeapEntry* last = heap.back();
	// last.kill();
	delete heap.back();
	heap.pop_back();

	// Heapify from the position we just messed with
	// use heapifyDown because back() always has a lower priority than the element we are removing
	int newPos = heapifyDown(pos);
	assert(newPos >= 0 && newPos < heap.size());
	// std::cerr << "Ending deleteAtIndex(" << pos << "): newPos is " << newPos << std::endl;
	// this->printHeap();
	return newPos;
}

HeapEntry* IndexedHeap::extractAtIndex(int pos)
{
	if (pos >= 0 && pos < heap.size())
	{
		HeapEntry* entry = heap[pos];
		deleteAtIndex(pos);
		return entry;
	}
	return NULL;
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

int IndexedHeap::heapifyDown(int pos)
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
	return pos;
}

/****************************** BIG 3 *********************************/
// IndexedHeap::IndexedHeap(const IndexedHeap& rhs) 
// {
// 	std::vector<HeapEntry*> otherHeap = rhs.heap;
// 	for (size_t i = 0; i < otherHeap.size(); i++)
// 	{
// 		this->heap.push_back(otherHeap[i]);
// 	}
// }

// IndexedHeap& IndexedHeap::operator=(const IndexedHeap& rhs) 
// {
// 	// for (size_t i = 0; i < heap.size(); i++)
// 	// {
// 	// 	heap[i].kill();
// 	// }
// 	heap.clear();

// 	std::vector<HeapEntry*> otherHeap = rhs.heap;
// 	for (size_t i = 0; i < otherHeap.size(); i++)
// 	{
// 		this->heap.push_back(otherHeap[i]);
// 	}
// 	return *this;
// }

// IndexedHeap::~IndexedHeap()
// {
// 	// for (size_t i = 0; i < heap.size(); i++)
// 	// {
// 	// 	heap[i].kill();
// 	// }
// 	heap.clear();
// }
/****************************** END BIG 3 *********************************/

void IndexedHeap::checkValid()
{
	int currIdx;
	size_t currPriority;
	size_t prevPriority;
	for (size_t i = 0; i > heap.size(); i++)
	{
		currIdx = heap[i]->getIndex();
		std::cerr << "i: " << i << std::endl;
		std::cerr << "heap[i]->index: " << currIdx << std::endl;
		assert(currIdx == i);

		currPriority = heap[i]->getPriority();
		if (i == 0)
		{
			prevPriority = currPriority;
		}
		std::cerr << "currPriority: " << currPriority << std::endl;
		std::cerr << "prevPriority: " << prevPriority << std::endl;		

		assert(currPriority <= prevPriority);

		prevPriority = currPriority;
	}
}


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

	HeapEntry* max(NULL);
	while (!rHeap.empty())
	{
		max = rHeap.extractAtIndex(rHeap.getSize()-1);
	}
}

IndexedHeapTest::IndexedHeapTest(unsigned long long numElements)
{
	runTest(numElements);
}
