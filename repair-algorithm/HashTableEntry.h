#ifndef HASH_TABLE_ENTRY_H
#define HASH_TABLE_ENTRY_H

#include "../indexed-heap/HeapEntry.h"
#include <iostream>
#include <set>
#include <unordered_map>
#include <vector>
class HashTableEntry
{
	HeapEntry* heapEntryPointer;

	// The sets of locations of this pair for each version
	// Ex: locationsInDoc[0] = {1,4,7}, locationsInDoc[1] = {1,4,12}
	std::unordered_map<unsigned, std::set<int> > locationsInDoc;

public:
	HashTableEntry(HeapEntry* hp, unsigned version, int firstIdx) : heapEntryPointer(hp)
	{
		locationsInDoc = std::unordered_map<unsigned, std::set<int> >();
		this->addOccurrence(version, firstIdx);
	}

	bool hasLocationsAtVersion(unsigned version);

	std::set<int> getLocationsAtVersion(unsigned version);

	void increment();

	void decrement();

	void removeOccurrence(unsigned version, int idx);

	void addOccurrence(unsigned version, int idx);

	size_t getSize() const;

	HeapEntry* getHeapEntryPointer() const;

	void setHeapEntryPointer(HeapEntry* newHeapEntryPointer);

	// HeapEntryPtr getHeapEntryPointer() const;

	// Copy ctor
	HashTableEntry(const HashTableEntry& rhs)
	{
//		std::cerr << "Copy ctor for HashTableEntry" << std::endl;
	}
	
	// Assignment operator
	HashTableEntry& operator=(const HashTableEntry& rhs)
	{
//		std::cerr << "Assignment operator for HashTableEntry" << std::endl;
	}
	
	~HashTableEntry()
	{
//		std::cerr << "Destructor for HashTableEntry" << std::endl;
		heapEntryPointer = NULL;
	}
};

#endif
