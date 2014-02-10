#ifndef UTIL_H
#define UTIL_H

#include "../util/Tokenizer.h"
#include "RepairTreeNode.h"
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <algorithm>
#include <set>

const unsigned MAX_NUM_FRAGMENTS_PER_VERSION(1000);
const unsigned MAX_FRAG_LENGTH(10000);
class RepairTreeNode;

extern unsigned currentOffset;
inline unsigned nextOffset()
{
	return currentOffset++;
}

inline void resetOffset()
{
	currentOffset = 0;	
}

extern unsigned currentFragID;
inline unsigned nextFragID()
{
	return ++currentFragID;
}

inline void resetFragID()
{
	currentFragID = 0;	
}

extern unsigned currentWordID;
inline unsigned nextWordID()
{
	return ++currentWordID;
}

inline unsigned currWordID()
{
	return currentWordID;
}

inline void resetcurrentWordID()
{
	currentWordID = 0;	
}

inline unsigned long long combineToUInt64(unsigned long long left, unsigned long long right)
{
	return (left << 32) | right;
}

inline unsigned getLeft(unsigned long long key)
{
	return key >> 32;
}

inline unsigned getRight(unsigned long long key)
{
	return (key << 32) >> 32;
}

inline std::string getKeyAsString(unsigned long long key)
{
	std::stringstream ss;
	ss << "(" << getLeft(key) << ", " << getRight(key) << ")";
	return ss.str();
}

inline std::vector<unsigned> stringToWordIDs(const std::string& text, std::unordered_map<unsigned, 
	std::string>& IDsToWords, std::unordered_map<unsigned, unsigned>& uniqueWordIDs)
{
	std::vector<unsigned> ret = std::vector<unsigned>();

    std::istringstream iss(text);
	
	std::vector<std::string> tokens;
	//copy(istream_iterator<string>(iss),
	//		 istream_iterator<string>(),
	//		 back_inserter<std::vector<string> >(tokens));
	
	//remove punctuation and produce std::vector of strings (called tokens)
	std::string delimiters = ",.:;\"/!?() []{}\n";
	bool trimEmpty = false;
	tokenize(text, tokens, delimiters, trimEmpty);

	//Would it be faster to download the code for a hash function? TODO
	std::locale loc;
	const std::collate<char>& coll = std::use_facet<std::collate<char> >(loc);
	unsigned myHash;

	std::unordered_map<unsigned, unsigned>::iterator it;
	for (size_t i = 0; i < tokens.size(); i++)
	{
		myHash = coll.hash(tokens[i].data(), tokens[i].data() + tokens[i].length());
		it = uniqueWordIDs.find(myHash);
		unsigned theID;
		if (it != uniqueWordIDs.end())
		{
			//found it, so use its ID
			theID = it->second;
		}
		else
		{
			//Didn't find it, give it an ID
			theID = nextWordID();
			uniqueWordIDs[myHash] = theID;
			IDsToWords[theID] = tokens[i];
			// cerr << tokens[i] << endl;
			//it->second = nextWordID();
		}
		ret.push_back(theID);
	}
	return ret;
}


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
	Association() : symbol(0), left(0), right(0), freq(0) {}

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
		// Erases all elements with value versionNum
		versions.erase(versionNum);
	}

	const std::string getVersionString() const {
		std::stringstream ss;
		for (std::multiset<unsigned>::iterator it = versions.begin(); it != versions.end(); it++) {
			ss << (*it) << ",";
		}
		return ss.str();
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

	return -1;
}


#endif
