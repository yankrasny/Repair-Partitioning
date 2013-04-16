#ifndef PARTITIONING_H
#define PARTITIONING_H

#include <vector>
#include <string>
#include <ostream>
#include <iostream>
#include <unordered_map>
#include <set>
#include "md5/md5.h"
#include "../repair-algorithm/MetaClasses.h"
#include "../repair-algorithm/RepairTree.h"

struct VersionDataItem;
struct FragInfo;

// TODO research if they'll come out sorted in a begin, end loop (that's what we want in getPartitioning...)
class SortNodesByOffsetComparator
{
public:
	bool operator() (const RepairTreeNode* const lhs, const RepairTreeNode* const rhs) const
	{
		return lhs->getOffsetInFile() < rhs->getOffsetInFile();
	}
};

typedef std::multiset<RepairTreeNode*, SortNodesByOffsetComparator> SortedByOffsetNodeSet;

class RepairDocumentPartition
{
	unsigned numLevelsDown;

	unsigned minFragSize;

	// Contains information about each version
	// More importantly, holds a pointer to the root node of each version (See class RepairTree and class RepairTreeNode)
	std::vector<VersionDataItem>& versionData;

	// The entry at i is the number of fragments for version i
	unsigned* versionSizes;
	
	// The offsets that define fragments, for all versions [v0:f0 v0:f1 v0:f2 v1:f0 v1:f1 v2:f0 v2:f1 ...]
	unsigned* offsets;

	// The tree to partition (see class RepairTree for details)
	RepairTree repairTree;

	// The outer vector represents all versions
	// The vector at position i contains fragment objects for version i
	std::vector<std::vector<FragInfo > > fragments;

	// Unique Fragments in all the versions
	std::unordered_map<std::string, FragInfo> uniqueFrags;

	// One implementation of get partitioning for one version
	// All implementations can return a list of nodes
	SortedByOffsetNodeSet getNodesNthLevelDown(RepairTreeNode* root, unsigned numLevelsDown, SortedByOffsetNodeSet& nodes);

	// Cuts one version
	unsigned getPartitioningOneVersion(RepairTreeNode* root, unsigned numLevelsDown, unsigned* bounds, unsigned minFragSize, unsigned versionSize);

	// Calls getPartitioningOneVersion and stores the results in offsets
	void setPartitioningsAllVersions(unsigned numLevelsDown, unsigned minFragSize);

	// Populates unique frags using the boundaries found by the partitioning algorithm
	void updateUniqueFragmentHashMap();

	// Populates this->fragments
	void setFragmentInfo(const std::vector<std::vector<unsigned> >& versions, std::ostream& os, bool print);
public:
	// For extensibility, RepairTree should implement an interface like PartitioningAlgorithm or something
	// In the future, others would also implement that interface, and these param types could stay the same
	// So it would be const PartitioningAlgorithm& alg
	RepairDocumentPartition(const RepairTree& repairTree, std::vector<VersionDataItem>& versionData, unsigned numLevelsDown = 1, unsigned minFragSize = 2)
		: repairTree(repairTree), versionData(versionData), offsets(NULL), numLevelsDown(numLevelsDown), minFragSize(minFragSize)
	{
		fragments = std::vector<std::vector<FragInfo > >();

	 	unsigned maxArraySize = versionData.size() * MAX_NUM_FRAGMENTS_PER_VERSION;

		this->offsets = new unsigned[maxArraySize];

		this->versionSizes = new unsigned[versionData.size()];

		uniqueFrags = std::unordered_map<std::string, FragInfo>();

		setPartitioningsAllVersions(numLevelsDown, minFragSize);
	}

	// The payload of this class
	unsigned* getOffsets()
	{
		return offsets;
	}

	unsigned* getVersionSizes()
	{
		return versionSizes;
	}

	void writeResults(const std::vector<std::vector<unsigned> >& versions, std::unordered_map<unsigned, std::string>& IDsToWords, 
	const std::string& outFilename, bool printFragments, bool printAssociations);

	double getScore(std::ostream& os = std::cout);
};

#endif