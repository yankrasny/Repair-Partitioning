#ifndef REPAIR_H
#define REPAIR_H

#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <iterator>
#include <sstream>
#include <locale>
#include <string>
#include "md5/md5.h"
#include "Occurrence.h"
#include "HashTableEntry.h"
#include "MetaClasses.h"
#include "../random-heap/HeapEntry.h"
#include "../random-heap/RandomHeap.h"
#include "../util/Profiler.h"
#include "../util/FileUtils.h"
#include "RepairTree.h"
// #include "UndoRepair.h"
#include "Util.h"

// TODO make this a class
// members can include the heap, vector<VersionData>

void doubleLinkOccurrences(Occurrence* prev, Occurrence* curr);

void doubleLinkNeighbors(Occurrence* prec, Occurrence* curr);

void addOrUpdatePair(RandomHeap& myHeap, std::unordered_map<unsigned long long, 
	HashTableEntry*>& hashTable, unsigned long long key, unsigned leftPosition,
	unsigned version, Occurrence* prec = NULL, Occurrence* succ = NULL);

void extractPairs(const std::vector<std::vector<unsigned> >& versions, RandomHeap& myHeap, 
	std::unordered_map<unsigned long long, HashTableEntry*>& hashTable, 
	std::vector<VersionDataItem>& versionData, RepairTree& repairTree);

void removeFromHeap(RandomHeap& myHeap, HeapEntry* hp);

void removeOccurrence(RandomHeap& myHeap, 
	std::unordered_map<unsigned long long, HashTableEntry*>& hashTable, Occurrence* oc);

unsigned long long getNewRightKey(unsigned symbol, Occurrence* succ);

unsigned long long getNewLeftKey(unsigned symbol, Occurrence* prec);

bool updateLeftmostOccurrence(std::vector<VersionDataItem>& versionData, Occurrence* oldOcc, Occurrence* newOcc);

void doRepair(RandomHeap& myHeap, std::unordered_map<unsigned long long, HashTableEntry*>& hashTable, 
	std::vector<Association>& associations, unsigned repairStoppingPoint, 
	std::vector<VersionDataItem>& versionData, RepairTree& repairTree);

void cleanup(std::unordered_map<unsigned long long, HashTableEntry*>& hashTable);

#endif