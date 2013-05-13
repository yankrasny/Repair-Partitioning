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
#include "util/md5.h" // TODO Not cross platform...
#include "Occurrence.h"
#include "HashTableEntry.h"
#include "MetaClasses.h"
#include "../random-heap/HeapEntry.h"
#include "../random-heap/RandomHeap.h"
#include "../util/Profiler.h"
#include "../util/FileUtils.h"
#include "RepairTreeNode.h"
#include "Util.h"

class RepairAlgorithm
{
private:

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

	void cleanup();

	RepairTreeNode* buildTree(int loc, unsigned versionNum);

	int getNextRootLoc(int loc);

	void getTrees();

	unsigned calcOffsets(RepairTreeNode* node);

public:

	RepairAlgorithm(std::vector<std::vector<unsigned> > versions) : versions(versions)
	{
		// Allocate the heap, hash table, array of associations, and list of pointers to neighbor structures	
		myHeap = RandomHeap();
		
		hashTable = std::unordered_map<unsigned long long, HashTableEntry*> ();
		
		associations = std::vector<Association>();
		
		versionData = std::vector<VersionDataItem>();
	}

	std::vector<VersionDataItem> getVersionData() const
	{
		return versionData;
	}

	std::vector<Association> getAssociations() const
	{
		return associations;
	}

	void run(unsigned repairStoppingPoint = 0)
	{
		// Run through the string and grab all the initial pairs
		// Add them to all the structures
		extractPairs();

		// Replace pairs with symbols until done (either some early stop condition or one symbol left)
		doRepair(repairStoppingPoint);

		// Use the output of repair to build a set of repair trees (one per version)
		getTrees();

		// Set the file offsets for all the nodes in each tree
		for (unsigned i = 0; i < versionData.size(); i++)
		{
			// Set file offsets for all the nodes in this version
			calcOffsets(versionData[i].getRootNode());

			// Reset the offset counter for the next version
			resetOffset();
		}
	}
};

#endif