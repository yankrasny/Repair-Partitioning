#ifndef HASH_TABLE_ENTRY_H
#define HASH_TABLE_ENTRY_H

#include "../indexed-heap/HeapEntry.h"
#include "Occurrence.h"
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <vector>
class HashTableEntry
{
	HeapEntry* heapEntryPointer;
	std::unordered_map<unsigned, std::unordered_set<int> > locationsInDoc;

public:
	HashTableEntry(HeapEntry* hp, unsigned version, int firstIdx) : heapEntryPointer(hp)
	{
		this->addOccurrence(version, firstIdx);
	}

	std::unordered_set<int> getLocationsAtVersion(unsigned version);

	void increment();

	void decrement();

	void removeOccurrence(unsigned version, int idx);

	void addOccurrence(unsigned version, int idx);

	size_t getSize() const;

	HeapEntry* getHeapEntryPointer() const;

	void setHeapEntryPointer(HeapEntry* newHeapEntryPointer);

	// HeapEntryPtr getHeapEntryPointer() const;

	// Big 3
	// HashTableEntry(const HashTableEntry& rhs)
	// {

	// }
	
	// HashTableEntry& operator=(const HashTableEntry& rhs)
	// {

	// }
	
	~HashTableEntry()
	{
		// std::cerr << "Destructor for HashTableEntry[key = " << heapEntryPointer->getKey() << "]" << std::endl;
		heapEntryPointer = NULL;
	}
};

#endif
