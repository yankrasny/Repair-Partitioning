#ifndef META_CLASSES_H
#define META_CLASSES_H

#include "RepairTreeNode.h"
#include <ostream>
#include <vector>
#include <set>

class VersionDataItem
{
private:
	RepairTreeNode* rootNode;
	unsigned versionNum; // aka versionID
	unsigned versionSize; // aka number of words
public:
	VersionDataItem(unsigned versionNum, unsigned versionSize)
		: versionNum(versionNum), versionSize(versionSize) {}

	RepairTreeNode* getRootNode()
	{
		return rootNode;
	}

	unsigned getVersionSize() const
	{
		return versionSize;
	}

	void setRootNode(RepairTreeNode* rootNode)
	{
		this->rootNode = rootNode; 
	}
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
class Association
{
	friend std::ostream& operator<<(std::ostream& os, const Association& a)
	{
		os << a.symbol << " -> " << "(" << a.left << ", " << a.right << "), " << "freq: " << a.freq << std::endl;
		os << "Versions: {";
		for (std::multiset<unsigned>::iterator it = a.versions.begin(); it != a.versions.end(); it++)
		{
			os << (*it) << ", ";
		}
		os << "}" << std::endl << std::endl;
		return os;
	}

private:
	unsigned symbol;
	unsigned left;
	unsigned right;
	unsigned freq;

	std::multiset<unsigned> versions;
public:
	Association(unsigned symbol, unsigned left, unsigned right, unsigned freq, unsigned firstVersion) :
		symbol(symbol), left(left), right(right), freq(freq)
	{
		versions = std::multiset<unsigned>();
		versions.insert(firstVersion);
	}

	unsigned getSymbol() const
	{
		return symbol;
	}
	unsigned getLeft() const
	{
		return left;
	}
	unsigned getRight() const
	{
		return right;
	}

	unsigned getFreq() const
	{
		return freq;
	}

	void addVersion(unsigned v)
	{
		versions.insert(v);
	}

	std::multiset<unsigned> getVersions() const
	{
		return versions;
	}

	void removeFromVersions(unsigned versionNum)
	{
		versions.erase(versionNum);
	}

	// return begin or false
	int getVersionAtBegin()
	{
		if (versions.begin() == versions.end())
		{
			return -1;
		}
		else
		{
			std::multiset<unsigned>::iterator it = versions.begin();
			return (int)(*it);
		}
	}
};

/*
Decided to put binarySearch here because this implementation relies on the Association class
*/
inline int binarySearch(unsigned target, const std::vector<Association>& associations, int leftPos, int rightPos)
{
	// indexes that don't make sense, means we haven't found the target
	if (leftPos > rightPos)
		return -1;

	if (leftPos == rightPos)
	{
		if (associations[leftPos].getSymbol() == target)
		{
			return leftPos;
		}
		return -1;
	}

	int mid = floor(((float)leftPos + rightPos) / 2);
	unsigned midVal = associations[mid].getSymbol();

	// found it
	if (target == midVal)
		return mid;

	// target is on the left
	if (target < midVal)
		return binarySearch(target, associations, leftPos, mid);
	
	// target is on the right
	if (target > midVal)
		return binarySearch(target, associations, mid + 1, rightPos);
}

#endif