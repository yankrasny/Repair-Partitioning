#ifndef UTIL_H
#define UTIL_H

#include "../util/Tokenizer.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <algorithm>

const unsigned MAX_NUM_FRAGMENTS_PER_VERSION(1000);
const unsigned MAX_FRAG_LENGTH(10000);

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

extern unsigned currentID;
inline unsigned nextID()
{
	return ++currentID;
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
			theID = nextID();
			uniqueWordIDs[myHash] = theID;
			IDsToWords[theID] = tokens[i];
			// cerr << tokens[i] << endl;
			//it->second = nextID();
		}
		ret.push_back(theID);
	}
	return ret;
}

#endif