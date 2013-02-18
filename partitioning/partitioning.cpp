#include <sstream>
#include "../repair-algorithm/Occurrence.h"
#include "Partitioning.h"
using namespace std;


/*

TODO make this a class, and use some inheritance to try different partitioning methods

Well write the other version first, and see if it fits.
	
	No, wtf? It must fit. That's the whole point of a class bro...

	Just realized how different the implementations are, the params and data are different

*/

unsigned getPartitioningOneVersion(Occurrence* current, vector<VersionDataItem>& versionData, unsigned* offsets, unsigned versionNum, unsigned minFragSize)
{
	unsigned currVal(0); // the current occurrence's starting index
	unsigned nextVal(0); // the next occurrence's starting index 
	unsigned diff(0); // the difference between consecutive indexes (a large value signifies a good fragment)
	unsigned fragmentNum(0); //the number of fragments so far in the current version

	if (current)
	{
		offsets[0] = 0;
	}
	while (current)
	{
		currVal = current->getLeftPositionInSequence();		
		if (current->getSucc()) // Move to the right, calculate difference and add the index if we like it
		{
			current = current->getSucc();
			nextVal = current->getLeftPositionInSequence();
			diff = nextVal - currVal;
			if (diff >= minFragSize)
			{
				fragmentNum++;
				offsets[fragmentNum] = nextVal;			
			}
		}
		else // Store the last fragment, and break because current has no right neighbor
		{			
			fragmentNum++;
			offsets[fragmentNum] = currVal;
			break;
		}
	}
	// We have to cover the whole document, and we may have done replacement on the end of it
	if (offsets[fragmentNum] != versionData[versionNum].versionSize)
		offsets[fragmentNum] = versionData[versionNum].versionSize - 1;

	return fragmentNum;
}

unsigned* getPartitioningsAllVersions(RandomHeap& myHeap, unsigned minFragSize, vector<VersionDataItem>& versionData, unsigned* versionPartitionSizes)
{
	unsigned maxArraySize = versionData.size() * MAX_NUM_FRAGMENTS_PER_VERSION;
	unsigned* fragmentList = new unsigned[maxArraySize];
	
	if (myHeap.empty())
		return NULL;

	unsigned versionOffset(0); // after the loop, is set to the total number of fragments (also the size of the starts array)
	unsigned numFragments(0); // will be reused in the loop for the number of fragments in each version

	//Iterate over all versions
	for (unsigned i = 0; i < versionData.size(); i++)
	{
		Occurrence* startingOccurrence = versionData[i].leftMostOcc; // We've stored the leftmost occurrences for each version throughout repair
		numFragments = getPartitioningOneVersion(startingOccurrence, versionData, &fragmentList[versionOffset], i, minFragSize); // Do the partitioning for one version, and store the number of fragments
		versionOffset += numFragments;
		versionPartitionSizes[i] = numFragments;
	}
	return fragmentList;
}

vector<vector<FragInfo > > getFragmentHashes(const vector<vector<unsigned> >& versions, unsigned* offsetsAllVersions, 
	unsigned* versionPartitionSizes, unordered_map<unsigned, string>& IDsToWords, ostream& os, bool print)
{
	vector<vector<FragInfo > > fragmentHashes = vector<vector<FragInfo > >();
	unsigned start, end, theID, fragSize;
	string word;
	vector<unsigned> wordIDs;
	
	MD5 md5;
	char* concatOfWordIDs;
	
	unsigned totalCountFragments(0);

	// Iterate over versions
	for (unsigned v = 0; v < versions.size(); v++)
	{
		wordIDs = versions[v]; // all the word IDs for version v
		fragmentHashes.push_back(vector<FragInfo >());
		if (print)
			os << "Version " << v << endl;

		// One version: iterate over the words in that version
		for (unsigned i = 0; i < versionPartitionSizes[v] - 1; i++)
		{			
			start = offsetsAllVersions[totalCountFragments + i];
			end = offsetsAllVersions[totalCountFragments + i + 1];
			fragSize = end - start;
			if (print)
				os << "Fragment " << i << ": ";
			stringstream ss;
			for (unsigned j = start; j < end; j++)
			{
				theID = wordIDs[j];
				ss << theID << ",";

				// word = IDsToWords[theID];
				// os << word << " ";
			}
			// Store the concatenation of the IDs for this fragment
			concatOfWordIDs = new char[MAX_FRAG_LENGTH];
			strcpy(concatOfWordIDs, ss.str().c_str());
			
			// Calculate the hash of the fragment
			string hash; // = new char[128]; // md5 produces 128 bit output
			hash = md5.digestString(concatOfWordIDs);
			fragmentHashes[v].push_back(FragInfo(0, 0, fragSize, hash));
			ss.str("");
			
			if (print)
				os << "hash (" << hash << ")" << endl;
			delete [] concatOfWordIDs;
			concatOfWordIDs = NULL;
		}
		totalCountFragments += versionPartitionSizes[v];
		if (print)
			os << endl;
	}
	return fragmentHashes;
}

void updateFragmentHashMap(vector<vector<FragInfo> >& fragmentHashes, unordered_map<string, FragInfo>& uniqueFrags)
{
	unordered_map<string, FragInfo>::iterator it;
	for (size_t i = 0; i < fragmentHashes.size(); i++)
	{
		for (size_t j = 0; j < fragmentHashes[i].size(); j++)
		{
			FragInfo frag = fragmentHashes[i][j];
			string myHash = frag.hash;

			unsigned theID;
			if (uniqueFrags.count(myHash))
			{
				//found it, so use its ID
				theID = uniqueFrags[myHash].id;
				uniqueFrags[myHash].count++;
			}
			else
			{
				//Didn't find it, give it an ID
				theID = nextFragID();
				frag.id = theID;
				frag.count = 1;
				uniqueFrags[myHash] = frag;
			}
		}
	}
}

/*
New Method: use the expansions from repair

*/

//We can call this with associations, some position (for example the last one like below to expand the whole thing), and a map for memoization
//expand(associations, associations.size(), knownExpansions);


