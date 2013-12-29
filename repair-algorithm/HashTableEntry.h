#ifndef HASH_TABLE_ENTRY_H
#define HASH_TABLE_ENTRY_H

#include "../indexed-heap/HeapEntry.h"
#include "Occurrence.h"
#include <iostream>

class HashTableEntry
{
	HeapEntry* heapEntryPointer;
	// HeapEntryPtr heapEntryPointer;
	Occurrence* occurrences;
	size_t size;
public:
	HashTableEntry(HeapEntry* hp, unsigned version);
	// HashTableEntry(HeapEntryPtr hp, unsigned version);

	void increment();

	void decrement();

	void removeOccurrence(Occurrence* target);

	void addOccurrence(Occurrence* oc);

	Occurrence* getHeadOccurrence() const;

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
		occurrences = NULL;
		heapEntryPointer = NULL;
	}
};

#endif