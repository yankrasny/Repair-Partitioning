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

const unsigned MAX_NUM_FRAGMENTS_PER_VERSION(1000000);
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

inline void resetCurrentWordID()
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

// Used to store associations from one symbol to two others
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

struct BaseFragment
{
    int start;
    int end;
};

// Use this class with the comparator below to sort the output by version number
class BaseFragmentList
{
private:
	std::vector<BaseFragment> baseFragments;
	unsigned versionNum;
public:
	BaseFragmentList(unsigned versionNum) : versionNum(versionNum) {}
	BaseFragmentList() {}

	void push(BaseFragment f)
	{
		baseFragments.push_back(f);
	}

	int size() const
	{
		return baseFragments.size();
	}

	BaseFragment get(size_t index) const
	{
		return baseFragments[index];
	}

	unsigned getVersionNum() const
	{
		return versionNum;
	}

	~BaseFragmentList()
	{
		baseFragments.clear();
	}
};

class BaseFragmentsListCompare
{
public:
	bool operator() (const BaseFragmentList lhs, const BaseFragmentList rhs) const
	{
		return lhs.getVersionNum() < rhs.getVersionNum();
	}
};

typedef std::set<BaseFragmentList, BaseFragmentsListCompare> BaseFragmentsAllVersions;

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

#endif