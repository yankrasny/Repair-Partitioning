#ifndef PARTITIONING_H
#define PARTITIONING_H

#include <vector>
#include <string>
#include <ostream>
#include <iostream>
#include <unordered_map>
#include "md5/md5.h"
// #include "Partitioning.h"
#include "../repair-algorithm/Occurrence.h"
#include "../random-heap/RandomHeap.h"
#include "../repair-algorithm/MetaClasses.h"


class Occurrence;
class RandomHeap;
struct VersionDataItem;
struct FragInfo;

unsigned getPartitioningOneVersion(Occurrence* current, std::vector<VersionDataItem>& versionData, unsigned* offsets, unsigned versionNum, unsigned minFragSize);

unsigned* getPartitioningsAllVersions(RandomHeap& myHeap, unsigned minFragSize, std::vector<VersionDataItem>& versionData, unsigned* versionPartitionSizes);

std::vector<std::vector<FragInfo > > getFragmentHashes(const std::vector<std::vector<unsigned> >& versions, unsigned* offsetsAllVersions, 
	unsigned* versionPartitionSizes, std::unordered_map<unsigned, std::string>& IDsToWords, std::ostream& os = std::cerr, bool print = false);

void updateFragmentHashMap(std::vector<std::vector<FragInfo> >& fragmentHashes, std::unordered_map<std::string, FragInfo>& uniqueFrags);

#endif