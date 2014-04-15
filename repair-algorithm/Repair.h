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
#include "HashTableEntry.h"
#include "../util/md5.h"
#include "../indexed-heap/HeapEntry.h"
#include "../indexed-heap/IndexedHeap.h"
#include "../partitioning/Partitioning.h"
#include "../util/FileUtils.h"
#include "RepairTreeNode.h"
#include "Util.h"
#include <assert.h>
#include <stdlib.h>

typedef std::unordered_map<unsigned long long, HashTableEntry*> RepairHashTable;

class RepairAlgorithm
{
private:

	// Each inner vector: The wordIDs for that version, outer vector: all the versions
	std::vector<std::vector<unsigned> > versions;

	// One of the main structures used in repair, allows us to choose entries by priority, in our case numOccurrences
	IndexedHeap myHeap;
	
	// The other main structure used in repair, allows us to access entries by key, which is in our case the pair of symbols
	RepairHashTable hashTable;
	
	// The result of repair, a list of associations in the form (roughly) symbol -> (left, right)
	std::unordered_map<unsigned, Association> associations;

	std::vector<unsigned> versionRoots;
	

	/***** Repair Core Algorithm *****/
	void addOccurrence(unsigned long long key, unsigned version, int idx);

	bool removeOccurrence(unsigned long long key, unsigned v, int idx);

	void extractPairs();

	int scanLeft(unsigned v, int idx);

	int scanRight(unsigned v, int idx);

	void checkVersionAndIdx(unsigned v, int idx);

	unsigned long long getKeyAtIdx(unsigned v, int idx);

	void doReplacements(unsigned repairStoppingPoint);

	void printVector(unsigned v);

	void printSection(unsigned v, unsigned idx, unsigned range);

	
	/***** Tree building, offsets, and partitioning ******/
	RepairTreeNode* buildTree(unsigned symbol, unsigned versionNum);

	void deleteTree(RepairTreeNode* node);

	int getNextRootSymbol(unsigned symbol);	

	unsigned calcOffsets(RepairTreeNode* node);

public:

	RepairAlgorithm()
	{
		versions = std::vector<std::vector<unsigned> >();
	}
	
	RepairAlgorithm(const std::vector<std::vector<unsigned> >& versions)
		:
		versions(versions)
	{
		myHeap = IndexedHeap();
		hashTable = RepairHashTable();
		associations = std::unordered_map<unsigned, Association>();
		versionRoots = std::vector<unsigned>(versions.size());
	}

	void doRepair(unsigned repairStoppingPoint = 0)
	{
		// Run through the string and grab all the initial pairs
		// Add them to all the structures
		extractPairs();

		// Replace pairs with symbols until done
		// (either some early stop condition or one symbol left)
		doReplacements(repairStoppingPoint);
	}

	void clearRepairStructures();

	void clearAssociationsAndReset();

	void getBaseFragments(BaseFragmentsAllVersions& baseFragsAllVersions, unsigned numLevelsDown);
};

#endif