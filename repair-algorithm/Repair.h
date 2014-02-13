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

	// Pass this to the partitioning alg, it prevents us from going too far down a repair tree
	unsigned numLevelsDown;
	
	// The minimum allowed size for a fragment, used during partitioning
	unsigned minFragSize;

	// How much should be favor fragmenting into small fragments. Should we keep them bigger, go for more occurrences, etc.
	double fragmentationCoefficient;

	// Each inner vector: The wordIDs for that version, outer vector: all the versions
	std::vector<std::vector<unsigned> > versions;

	// One of the main structures used in repair, allows us to choose entries by priority, in our case numOccurrences
	IndexedHeap myHeap;
	
	// The other main structure used in repair, allows us to access entries by key, which is in our case the pair of symbols
	RepairHashTable hashTable;
	
	// The result of repair, a list of associations in the form (roughly) symbol -> (left, right)
	std::unordered_map<unsigned, Association> associations;
	

	/***** Repair Core Algorithm *****/
	void addOccurrence(unsigned long long key, unsigned version, int idx);

	bool removeOccurrence(unsigned long long key, unsigned v, int idx);

	void extractPairs();

	int scanLeft(unsigned v, int idx);

	int scanRight(unsigned v, int idx);

	void checkVersionAndIdx(unsigned v, int idx);

	unsigned long long getKeyAtIdx(unsigned v, int idx);

	void doRepair(unsigned repairStoppingPoint);

	void printSection(unsigned v, unsigned idx, unsigned range);

	
	/***** Tree building, offsets, and partitioning ******/
	void deleteTree(RepairTreeNode* node);

	RepairTreeNode* buildTree(unsigned symbol, unsigned versionNum);

	int getNextRootSymbol(unsigned symbol);

	unsigned calcOffsets(RepairTreeNode* node);

public:

	RepairAlgorithm(std::vector<std::vector<unsigned> > versions,
		unsigned numLevelsDown = 1,
		unsigned minFragSize = 2,
		double fragmentationCoefficient = 1.0) :
			versions(versions),
			numLevelsDown(numLevelsDown),
			minFragSize(minFragSize),
			fragmentationCoefficient(fragmentationCoefficient)
	{
		myHeap = IndexedHeap();
		hashTable = RepairHashTable();
		associations = std::unordered_map<unsigned, Association>();
	}

	void getOffsetsAllVersions(unsigned* offsetsAllVersions, unsigned* versionPartitionSizes);

	std::unordered_map<unsigned, Association> getAssociations(unsigned repairStoppingPoint = 0)
	{
		// Run through the string and grab all the initial pairs
		// Add them to all the structures
		extractPairs();

		// Replace pairs with symbols until done
		// (either some early stop condition or one symbol left)
		doRepair(repairStoppingPoint);

		assert(hashTable.empty());
		assert(myHeap.empty());

		// Return the payload
		return this->associations;
	}

	void cleanup();
};


// Use this class with the comparator below to sort the output by version number
class PartitionList
{
private:
	std::vector<unsigned> offsets;
	unsigned versionNum;
public:
	PartitionList(unsigned versionNum) : versionNum(versionNum) {}
	PartitionList() {}

	void push(unsigned offset)
	{
		offsets.push_back(offset);
	}

	int size() const
	{
		return offsets.size();
	}

	unsigned get(unsigned index) const
	{
		return offsets[index];
	}

	unsigned getVersionNum() const
	{
		return versionNum;
	}

	~PartitionList()
	{
		offsets.clear();
	}
};

class SortPartitionListsByVersion
{
public:
	bool operator() (const PartitionList lhs, const PartitionList rhs) const
	{
		return lhs.getVersionNum() < rhs.getVersionNum();
	}
};

typedef std::set<PartitionList, SortPartitionListsByVersion> SortedPartitionsByVersion;
#endif
