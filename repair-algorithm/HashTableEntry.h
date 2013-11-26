#ifndef HASH_TABLE_ENTRY_H
#define HASH_TABLE_ENTRY_H

#include "../indexed-heap/HeapEntry.h"
#include "Occurrence.h"
#include <iostream>

class HashTableEntry
{
	HeapEntryPtr heapEntryPointer;
	Occurrence* occurrences;
	size_t size;
public:
	HashTableEntry(HeapEntryPtr hp, unsigned version);

	void increment();

	void decrement();

	void removeOccurrence(Occurrence* target);

	void addOccurrence(Occurrence* oc);

	Occurrence* getHeadOccurrence() const;

	size_t getSize() const;

	HeapEntryPtr getHeapEntryPointer() const;
};

#endif