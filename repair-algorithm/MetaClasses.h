#ifndef META_CLASSES_H
#define META_CLASSES_H

#include <ostream>

class VersionDataItem
{
private:
	RepairTreeNode* firstNode;
	RepairTreeNode* rootNode;
	unsigned versionNum; // aka versionID
	unsigned versionSize; // aka number of words
public:
	VersionDataItem(RepairTreeNode* firstNode, unsigned versionNum, unsigned versionSize)
		: firstNode(firstNode), versionNum(versionNum), versionSize(versionSize), rootNode(NULL) {}

	RepairTreeNode* getRootNode() const
	{
		if (rootNode)
			return rootNode;

		RepairTreeNode* current = firstNode;
		if (current)
		{
			while (true)
			{
				if (current->getParent()) // current has a parent, so current is not our root
				{
					current = current->getParent();
				}
				else // current does not have a parent, it must be our root, set it and don't loop through again
				{
					rootNode = current;
					return rootNode;
				}					
			}
		}
	}

	unsigned getVersionSize() const
	{
		return versionSize;
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