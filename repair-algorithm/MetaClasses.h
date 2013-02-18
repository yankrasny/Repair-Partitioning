#ifndef META_CLASSES_H
#define META_CLASSES_H

#include <ostream>

class Occurrence;

struct VersionDataItem
{
	Occurrence* leftMostOcc;
	unsigned index; // index in the original file (the indexes of consecutive occurrences define the interval spanned by those symbols)
	unsigned versionSize; // in words
	VersionDataItem(Occurrence* leftMostOcc, unsigned index, unsigned versionSize) : leftMostOcc(leftMostOcc), index(index), versionSize(versionSize) {}
};


struct FragInfo
{
	friend std::ostream& operator<<(std::ostream& os, const FragInfo& f)
	{
		return os << f.id << " -> " << "(count = " << f.count << ", fragSize = " << f.fragSize << ")" << std::endl;
	}

	unsigned id;
	unsigned count;
	unsigned fragSize;
	std::string hash;

	FragInfo() : id(-1), count(0), fragSize(0), hash("") {}
	FragInfo(unsigned id, unsigned count, unsigned fragSize, const std::string& hash) : id(id), count(count), fragSize(fragSize), hash(hash) {}
};


//Used to store associations from one symbol to two others
struct Association
{
	friend std::ostream& operator<<(std::ostream& os, const Association& a)
	{
		return os << a.symbol << " -> " << "(" << a.left << ", " << a.right << "), " << "freq: " << a.freq << std::endl;
	}

	unsigned symbol;
	unsigned left;
	unsigned right;
	unsigned freq;

	Association(unsigned symbol, unsigned left, unsigned right, unsigned freq) : symbol(symbol), left(left), right(right), freq(freq) {}
};

#endif