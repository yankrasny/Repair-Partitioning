#ifndef HASH_TABLE_ENTRY_H
#define HASH_TABLE_ENTRY_H

#include "../random-heap/HeapEntry.h"
#include "Occurrence.h"
#include <iostream>

class HashTableEntry
{
	HeapEntry* heapPointer;
	Occurrence* occurrences;
	size_t size;
public:
	HashTableEntry(HeapEntry* hp, Occurrence* prec, Occurrence* succ, unsigned version);

	HashTableEntry(HeapEntry* hp, Occurrence* oc);

	void increment();

	void decrement();

	void removeOccurrence(Occurrence* target);

	void addOccurrence(Occurrence* oc);

	Occurrence* getHeadOccurrence() const;

	size_t getSize() const;

	HeapEntry* getHeapPointer() const;

	~HashTableEntry();
};

#endif