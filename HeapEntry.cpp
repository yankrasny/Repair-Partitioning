#include "HeapEntry.h"

std::vector<unsigned> HeapEntry::getKey() 
{
	return key;
}

size_t HeapEntry::getPriority() const
{
	return priority;
}

void HeapEntry::increment()
{
	priority++;
	myHeap->heapifyUp(index);
}

void HeapEntry::decrement()
{
	priority--;
	myHeap->heapifyDown(index);
}

int HeapEntry::getIndex()
{
	return index;
}

void HeapEntry::setIndex(int index)
{
	this->index = index;
}

HeapEntry::~HeapEntry()
{
	priority = 0;
	index = -1;
}