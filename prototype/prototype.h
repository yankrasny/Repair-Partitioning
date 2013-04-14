// #ifndef PROTOYPE_H
// #define PROTOYPE_H

// #include "../repair-algorithm/Repair.h"
// #include "../repair-algorithm/UndoRepair.h"
// #include "../partitioning/Partitioning.h"
// #include "../repair-algorithm/Util.h"
// #include <ostream>
// #include <vector>
// #include <unordered_map>
// #include <string>

// class Prototype
// {
// public:
// 	Prototype() {}

// 	bool checkOutput(std::vector<Association> associations, std::vector<unsigned> wordIDs);

// 	double getScore(std::unordered_map<std::string, FragInfo>& uniqueFrags, unsigned numVersions, std::ostream& os = std::cerr);

// 	void writeAssociations(const std::vector<Association>& associations, std::ostream& os = std::cerr);

// 	void writeResults(const std::vector<std::vector<unsigned> >& versions, unsigned* offsetsAllVersions, unsigned* versionPartitionSizes, 
// 		const std::vector<Association>& associations, std::unordered_map<unsigned, std::string>& IDsToWords, const std::string& outFilename, 
// 		bool printFragments = false, bool printAssociations = false);

// 	void printIDtoWordMapping(std::unordered_map<unsigned, std::string>& IDsToWords, std::ostream& os = std::cerr);

// 	double runRepairPartitioning(std::vector<std::vector<unsigned> > versions, std::unordered_map<unsigned, std::string>& IDsToWords, 
// 		unsigned*& offsetsAllVersions, unsigned*& versionPartitionSizes, std::vector<Association>& associations,
// 		unsigned minFragSize, unsigned repairStoppingPoint, bool printFragments = false);

// 	int run(int argc, char* argv[]);
// };

// #endif