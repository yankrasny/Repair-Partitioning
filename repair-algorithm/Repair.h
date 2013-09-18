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
		// New way: call this getAssociations
		doRepair(repairStoppingPoint);

		// Use the output of repair to build a set of repair trees (one per version)
		/* TODO new way: use the same type of loop as we currently have in getTrees()
			Get the current tree, calculate the offsets, and get the partitioning (this requires some refactoring at the class level)
			Instead of getTrees(), it should be: 

			RepairTreeNode* currRoot = NULL;
			versionOffset = 0;
			Partitioning p = Partitioning(); // probably need to change class Partitioning to support all of this
			while (currRoot = getTree()) {
				calcOffsets(currRoot);
				resetOffset();
				numFrags = p.getPartitioningOneVersion(currRoot, &(p->offsets[versionOffset]), numLevelsDown, ...);
				versionOffset += numFrags;
				p->versionSizes[i] = numFragments;
				deleteTree(currRoot);
			}

			// And now offsets is ready to pass along
		*/

		getTrees();

		// Set the file offsets for all the nodes in each tree
		for (unsigned i = 0; i < versionData.size(); i++)
		{
			// Set file offsets for all the nodes in this version
			calcOffsets(versionData[i].getRootNode());

			// Reset the offset counter for the next version
			resetOffset();
		}

		// cleanup();
	}

	void cleanup();
};

#endif