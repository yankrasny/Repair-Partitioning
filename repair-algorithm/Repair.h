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
#include "Occurrence.h"
#include "HashTableEntry.h"
#include "MetaClasses.h"
#include "../util/md5.h"
#include "../random-heap/HeapEntry.h"
#include "../random-heap/RandomHeap.h"
#include "../partitioning/Partitioning.h"
#include "../util/Profiler.h"
#include "../util/FileUtils.h"
#include "RepairTreeNode.h"
#include "Util.h"

class RepairAlgorithm
{
private:

	// The entry at i is the number of fragments for version i
	unsigned* versionPartitionSizes;
	
	// The offsets that define fragments, for all versions [v0:f0 v0:f1 v0:f2 v1:f0 v1:f1 v2:f0 v2:f1 ...]
	unsigned* offsets;

	unsigned numLevelsDown;
	
	unsigned minFragSize;

	std::vector<std::vector<unsigned> > versions;

	RandomHeap myHeap;
	
	std::unordered_map<unsigned long long, HashTableEntry*> hashTable;
	
	std::vector<Association> associations;
	
	std::vector<VersionDataItem> versionData;

	void addOrUpdatePair(unsigned long long key, unsigned leftPosition,
		unsigned version, Occurrence* prec = NULL, Occurrence* succ = NULL);

	void extractPairs();

	void removeFromHeap(HeapEntry* hp);

	void removeOccurrence(Occurrence* oc);

	unsigned long long getNewRightKey(unsigned symbol, Occurrence* succ);

	unsigned long long getNewLeftKey(unsigned symbol, Occurrence* prec);

	bool updateLeftmostOccurrence(Occurrence* oldOcc, Occurrence* newOcc);

	void doRepair(unsigned repairStoppingPoint);

	// Tree building, offsets, and partitioning
	void deleteTree(RepairTreeNode* node);

	RepairTreeNode* buildTree(int loc, unsigned versionNum);

	int getNextRootLoc(int loc);

	unsigned calcOffsets(RepairTreeNode* node);

public:

	RepairAlgorithm(std::vector<std::vector<unsigned> > versions,
		unsigned numLevelsDown = 1, unsigned minFragSize = 2) : 
	versions(versions), numLevelsDown(numLevelsDown), minFragSize(minFragSize)
	{
		// Allocate the heap, hash table, array of associations, and list of pointers to neighbor structures	
		myHeap = RandomHeap();
		
		hashTable = std::unordered_map<unsigned long long, HashTableEntry*> ();
		
		associations = std::vector<Association>();
		
		versionData = std::vector<VersionDataItem>();

	 	unsigned maxArraySize = versionData.size() * MAX_NUM_FRAGMENTS_PER_VERSION;

		this->offsets = new unsigned[maxArraySize];

		this->versionPartitionSizes = new unsigned[versionData.size()];
	}

	std::vector<VersionDataItem> getVersionData() const
	{
		return versionData;
	}

	unsigned* getVersionPartitionSizes();

	unsigned* getOffsetsAllVersions();

	std::vector<Association> getAssociations(unsigned repairStoppingPoint = 0)
	{
		// Run through the string and grab all the initial pairs
		// Add them to all the structures
		extractPairs();

		// Replace pairs with symbols until done (either some early stop condition or one symbol left)
		doRepair(repairStoppingPoint);

		return this->associations;		
	}

	void cleanup();
};

#endif