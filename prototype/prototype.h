#ifndef PROTOYPE_H
#define PROTOYPE_H

#include "../repair-algorithm/Repair.h"
#include "../partitioning/Partitioning.h"
#include "../repair-algorithm/Util.h"
#include <ostream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <string>

class Prototype
{
public:
	Prototype() {}

	double getScore(std::unordered_map<std::string, FragInfo>& uniqueFrags, unsigned numVersions, std::ostream& os = std::cerr);

	void writeAssociations(const std::vector<Association>& associations, std::ostream& os = std::cerr);

	void writeResults(const std::vector<std::vector<unsigned> >& versions, unsigned* offsetsAllVersions, 
		unsigned* versionPartitionSizes, const std::vector<Association>& associations, 
		std::unordered_map<unsigned, std::string>& IDsToWords, const std::string& outFilename, bool printFragments = true, bool printAssociations = false);

	void printIDtoWordMapping(std::unordered_map<unsigned, std::string>& IDsToWords, std::ostream& os = std::cerr);

	double runRepairPartitioning(std::vector<std::vector<unsigned> > versions, std::unordered_map<unsigned, std::string>& IDsToWords, 
		unsigned*& offsetsAllVersions, unsigned*& versionPartitionSizes, std::vector<Association>& associations,
		unsigned minFragSize, float fragmentationCoefficient, unsigned repairStoppingPoint, unsigned numLevelsDown, 
		unsigned method, bool printFragments = true, bool printAssociations = false);

	int run(int argc, char* argv[]);
};

#endif