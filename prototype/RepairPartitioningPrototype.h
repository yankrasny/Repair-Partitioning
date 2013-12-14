#ifndef REPAIR_PARTITIONING_PROTOYPE_H
#define REPAIR_PARTITIONING_PROTOYPE_H

#include "../repair-algorithm/Repair.h"
#include "../partitioning/Partitioning.h"
#include "../repair-algorithm/Util.h"
#include <ostream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <assert.h>


class RepairPartitioningPrototype
{
private:

	// The outer vector represents all versions
	// The vector at position i contains fragment objects for version i
	std::vector<std::vector<FragInfo > > fragments;

	// Unique Fragments in all the versions
	std::unordered_map<std::string, FragInfo> uniqueFrags;

	unsigned* offsetsAllVersions;

	unsigned* versionPartitionSizes;

	

public:
	RepairPartitioningPrototype() 
	{
		fragments = std::vector<std::vector<FragInfo > >();

		uniqueFrags = std::unordered_map<std::string, FragInfo>();
	}

	double getScore(std::ostream& os);

	void setFragmentInfo(
		const std::vector<std::vector<unsigned> >& versions, 
		std::ostream& os, 
		bool print);

	void updateUniqueFragmentHashMap();

	void writeResults(
		const std::vector<std::vector<unsigned> >& versions, 
		std::unordered_map<unsigned, std::string>& IDsToWords, 
		const std::string& outFilename);

	double getScore(std::unordered_map<std::string, FragInfo>& uniqueFrags, 
		unsigned numVersions, 
		std::ostream& os = std::cerr);

	void writeAssociations(const std::vector<Association>& associations, 
		std::ostream& os = std::cerr);

	void writeResults(
		const std::vector<std::vector<unsigned> >& versions, 
		unsigned* offsetsAllVersions, 
		unsigned* versionPartitionSizes, 
		const std::vector<Association>& associations, 
		std::unordered_map<unsigned, std::string>& IDsToWords, 
		const std::string& outFilename, 
		bool printFragments = true, 
		bool printAssociations = false);

	void printIDtoWordMapping(std::unordered_map<unsigned, std::string>& IDsToWords, 
		std::ostream& os = std::cerr);

	// Use this version with our main
	double runRepairPartitioning(
		std::vector<std::vector<unsigned> > versions,
		std::unordered_map<unsigned, std::string>& IDsToWords, 
		unsigned*& offsetsAllVersions, 
		unsigned*& versionPartitionSizes, 
		unsigned minFragSize, 
		float fragmentationCoefficient, 
		unsigned method);

	// Use this version with Jinru's code
	double runRepairPartitioning(
		std::vector<std::vector<unsigned> > versions,
		unsigned*& offsetsAllVersions, 
		unsigned*& versionPartitionSizes, 
		unsigned minFragSize, 
		float fragmentationCoefficient, 
		unsigned method);

	int run(int argc, char* argv[]);
};

#endif