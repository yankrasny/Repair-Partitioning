#include "HeapEntry.h"

unsigned long long HeapEntry::getKey()
{
	return key;
}

unsigned HeapEntry::getLeft()
{
	return key >> 32;
}

unsigned HeapEntry::getRight()
{
	return (key << 32) >> 32;
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
	key = 0;
}