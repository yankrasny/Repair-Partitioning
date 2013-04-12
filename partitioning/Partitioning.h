#ifndef PARTITIONING_H
#define PARTITIONING_H

#include <vector>
#include <string>
#include <ostream>
#include <iostream>
#include <unordered_map>
#include <set>
#include "md5/md5.h"
#include "../random-heap/RandomHeap.h"
#include "../repair-algorithm/MetaClasses.h"
#include "../repair-algorithm/RepairTree.h"

class RandomHeap;
struct VersionDataItem;
struct FragInfo;

// TODO research if they'll come out sorted in a begin, end loop (that's what we want in getPartitioning...)
class SortedNodeSetComparator
{
public:
	bool operator() (const RepairTreeNode* const lhs, const RepairTreeNode* const rhs) const
	{
		return lhs->getLeftBound() < rhs->getLeftBound();
	}
};

typedef std::multiset<RepairTreeNode*, SortedNodeSetComparator> SortedNodeSet;

class RepairDocumentPartition
{
	// Contains information about each version
	// More importantly, holds a pointer to the root node of each version (See class RepairTree and class RepairTreeNode)
	vector<VersionDataItem>& versionData;

	// The entry at i is the number of fragments for version i
	unsigned* versionSizes;
	
	// The offsets that define fragments, for all versions [v0:f0 v0:f1 v0:f2 v1:f0 v1:f1 v2:f0 v2:f1 ...]
	unsigned* offsets;

	// The tree to partition (see class RepairTree for details)
	RepairTree repairTree;

	// The outer vector represents all versions
	// The vector at position i contains fragment objects for version i
	vector<vector<FragInfo > > fragments;

	// Unique Fragments in all the versions
	unordered_map<string, FragInfo>& uniqueFrags;

	// One implementation of get partitioning for one version
	// All implementations can return a list of nodes
	SortedNodeSet getPartitioningOneVersionRecursive(RepairTreeNode* root, unsigned numLevelsDown, SortedNodeSet& nodes);

	// Cuts one version
	unsigned getPartitioningOneVersion(RepairTreeNode* root, unsigned numLevelsDown, unsigned* bounds, unsigned minFragSize, unsigned versionSize);

	// Calls getPartitioningOneVersion and stores the results in offsets
	void setPartitioningsAllVersions(unsigned numLevelsDown, unsigned minFragSize);

public:
	// For extensibility, RepairTree should implement an interface like PartitioningAlgorithm or something
	// In the future, others would also implement that interface, and this code would work with them right away
	// So it would be const PartitioningAlgorithm& alg
	RepairDocumentPartition(const RepairTree& repairTree, vector<VersionDataItem>& versionData, numLevelsDown = 5, minFragSize = 2)
		: repairTree(repairTree), versionData(versionData), offsets(NULL), numLevelsDown(numLevelsDown), minFragSize(minFragSize)
	{
		fragments = vector<vector<FragInfo > >();

	 	unsigned maxArraySize = versionData.size() * MAX_NUM_FRAGMENTS_PER_VERSION;

		this->fragmentList = new unsigned[maxArraySize];
	}

	// Populates this->fragments
	// TODO where will these params come from?
	void setFragments(const vector<vector<unsigned> >& versions, ostream& os, bool print);

	// Populates unique frags using the boundaries found by the partitioning algorithm
	void updateUniqueFragmentHashMap();
	
	// The payload of this class
	unsigned* getOffsets() const
	{
		if (!offsets)
			setPartitioningsAllVersions(numLevelsDown, minFragSize);

		return offsets;
	}
};

#endif