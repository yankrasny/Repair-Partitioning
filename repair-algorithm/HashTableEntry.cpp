#include "HashTableEntry.h"
using namespace std;

void HashTableEntry::increment()
{
	assert(this->heapEntryPointer != NULL);
	this->heapEntryPointer->increment();
}

void HashTableEntry::decrement()
{
	assert(this->heapEntryPointer != NULL);
	this->heapEntryPointer->decrement();
}

bool HashTableEntry::hasLocationsAtVersion(unsigned version)
{
	if (locationsInDoc.find(version) != locationsInDoc.end())
		return locationsInDoc[version].size() > 0;
	return false;
}

unordered_set<int> HashTableEntry::getLocationsAtVersion(unsigned version)
{
	if (locationsInDoc.find(version) != locationsInDoc.end())
		return locationsInDoc[version];
	return unordered_set<int>();
}

void HashTableEntry::addOccurrence(unsigned version, int idx)
{
	if (locationsInDoc.find(version) == locationsInDoc.end()) {
		locationsInDoc[version] = unordered_set<int>();
	}
	locationsInDoc[version].insert(idx);
	increment();
}

void HashTableEntry::removeOccurrence(unsigned version, int idx)
{
	assert(locationsInDoc.find(version) != locationsInDoc.end());
	locationsInDoc[version].erase(idx);
	decrement();
}

// The heap's priority is defined as the number of occurrences of the pair (the pair is that entry's key)
size_t HashTableEntry::getSize() const
{
	return this->heapEntryPointer->getPriority();
}

// TODO remove these 2 (getter and setter)
HeapEntry* HashTableEntry::getHeapEntryPointer() const
{
	return heapEntryPointer;
}
void HashTableEntry::setHeapEntryPointer(HeapEntry* newHeapEntryPointer)
{
	heapEntryPointer = newHeapEntryPointer;
}
