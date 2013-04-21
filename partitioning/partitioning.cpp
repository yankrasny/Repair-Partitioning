#include <sstream>
#include "Partitioning.h"
using namespace std;

double RepairDocumentPartition::getScore(ostream& os)
{
	double term;
	double sum(0);
	for (unordered_map<string, FragInfo>::iterator it = uniqueFrags.begin(); it != uniqueFrags.end(); it++)
	{
		term = it->second.count * it->second.fragSize;
		sum += term;
	}
	return sum / (uniqueFrags.size() * versionData.size());
}

SortedByOffsetNodeSet RepairDocumentPartition::getNodesNthLevelDown(RepairTreeNode* root, unsigned numLevelsDown, SortedByOffsetNodeSet& nodes)
{
	if (numLevelsDown < 1) return nodes;
	if (numLevelsDown == 1)
	{
		if (root->getLeftChild() && root->getRightChild())
		{
			nodes.insert(root->getLeftChild());
			nodes.insert(root->getRightChild());			
		}
		return nodes;
	}
	if (root->getLeftChild() && root->getRightChild())
	{
		this->getNodesNthLevelDown(root->getLeftChild(), numLevelsDown - 1, nodes);
		this->getNodesNthLevelDown(root->getRightChild(), numLevelsDown - 1, nodes);
	}
	return nodes;
}

unsigned RepairDocumentPartition::getPartitioningOneVersion(RepairTreeNode* root, unsigned numLevelsDown, unsigned* bounds, unsigned minFragSize, unsigned versionSize)
{
	SortedByOffsetNodeSet nodes = SortedByOffsetNodeSet();
	nodes = getNodesNthLevelDown(root, numLevelsDown, nodes);

	// cerr << endl;
	unsigned numFrags = 0;
	for (auto it = nodes.begin(); it != nodes.end(); ++it)
	{
		RepairTreeNode* current = *it;

		// cerr << current << endl;

		// These offsets are already sorted (see the comparator at the top)
		bounds[numFrags] = current->getOffset();
		numFrags++;
	}
	// We're working with left bounds, so we always need to add the last one on the right
	bounds[numFrags] = versionSize;
	numFrags++;

	// TODO Handle this
	if (numFrags > MAX_NUM_FRAGMENTS_PER_VERSION)
	{
		// Fail or try a partitioning that won't fragment so much
	}
	
	return numFrags;
}

void RepairDocumentPartition::setPartitioningsAllVersions(unsigned numLevelsDown, unsigned minFragSize)
{
	unsigned versionOffset(0); // after the loop, is set to the total number of fragments (also the size of the starts array)
	unsigned numFragments(0); // will be reused in the loop for the number of fragments in each version

	RepairTreeNode* rootForThisVersion = NULL;

	// Iterate over all versions
	for (unsigned i = 0; i < this->versionData.size(); i++)
	{
		rootForThisVersion = this->versionData[i].getRootNode();

		// Do the partitioning for one version, and store the number of fragments
		numFragments = getPartitioningOneVersion(rootForThisVersion, numLevelsDown, &(this->offsets[versionOffset]), minFragSize, versionData[i].getVersionSize());
		versionOffset += numFragments;
		this->versionSizes[i] = numFragments;
	}
}

void RepairDocumentPartition::setFragmentInfo(const vector<vector<unsigned> >& versions, ostream& os, bool print)
{
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
		this->fragments.push_back(vector<FragInfo >());
		if (print)
			os << "Version " << v << endl;

		// One version: iterate over the words in that version
		for (unsigned i = 0; i < this->versionSizes[v] - 1; i++)
		{			
			start = this->offsets[totalCountFragments + i];
			end = this->offsets[totalCountFragments + i + 1];
			fragSize = end - start;
			if (print)
				os << "Fragment " << i << ": ";
			stringstream ss;
			for (unsigned j = start; j < end; j++)
			{
				theID = wordIDs[j];

				// Need a delimiter to ensure uniqueness (1,2,3 is different from 12,3)
				ss << theID << ",";
			}

			// Store the concatenation of the IDs for this fragment
			concatOfWordIDs = new char[MAX_FRAG_LENGTH];
			strcpy(concatOfWordIDs, ss.str().c_str());
			
			// Calculate the hash of the fragment
			string hash; // = new char[128]; // md5 produces 128 bit output
			hash = md5.digestString(concatOfWordIDs);
			this->fragments[v].push_back(FragInfo(0, 0, fragSize, hash));
			ss.str("");
			
			if (print)
				os << "hash (" << hash << ")" << endl;
			delete [] concatOfWordIDs;
			concatOfWordIDs = NULL;
		}
		totalCountFragments += this->versionSizes[v];
		if (print)
			os << endl;
	}
}

void RepairDocumentPartition::updateUniqueFragmentHashMap()
{
	unordered_map<string, FragInfo>::iterator it;
	for (size_t i = 0; i < this->fragments.size(); i++)
	{
		for (size_t j = 0; j < this->fragments[i].size(); j++)
		{
			FragInfo frag = this->fragments[i][j];
			string myHash = frag.hash;

			unsigned theID;
			if (this->uniqueFrags.count(myHash))
			{
				// Found it, so use its ID
				theID = this->uniqueFrags[myHash].id;
				this->uniqueFrags[myHash].count++;
			}
			else
			{
				// Didn't find it, give it an ID
				theID = nextFragID();
				frag.id = theID;
				frag.count = 1;
				this->uniqueFrags[myHash] = frag;
			}
		}
	}
}

void RepairDocumentPartition::writeResults(const vector<vector<unsigned> >& versions, unordered_map<unsigned, string>& IDsToWords, 
	const string& outFilename, bool printFragments, bool printAssociations)
{
	ofstream os(outFilename.c_str());

	os << "Results of re-pair partitioning..." << endl << endl;
	os << "*** Fragment boundaries ***" << endl;
	
	unsigned totalCountFragments(0);
	unsigned diff(0);
	unsigned numVersions = versions.size();
	for (unsigned v = 0; v < numVersions; v++)
	{
		unsigned numFragsInVersion = versionSizes[v];

		if (numFragsInVersion < 1)
		{
			continue;
		}
		os << "Version " << v << endl;
		for (unsigned i = 0; i < numFragsInVersion - 1; i++)
		{
			if (i < numFragsInVersion - 1)
			{
				unsigned currOffset = offsets[totalCountFragments + i];
				unsigned nextOffset = offsets[totalCountFragments + i + 1];
				diff = nextOffset - currOffset;
			}
			else
			{
				diff = 0;
			}

			os << "Fragment " << i << ": " << offsets[totalCountFragments + i] << "-" << 
				offsets[totalCountFragments + i + 1] << " (frag size: " << diff << ")" << endl;
		}
		totalCountFragments += numFragsInVersion;
		os << endl;
	}

	// os << "Number of fragment boundaries: " << starts.size() << endl;
	// os << "Number of fragments: " << (starts.size() - 1) << endl << endl;

	os << "*** Fragments ***" << endl;
	this->setFragmentInfo(versions, os, printFragments);

	// Assign fragment IDs and stick them in a hashmap
	unordered_map<string, FragInfo> uniqueFrags;
	this->updateUniqueFragmentHashMap();

	// Now decide on the score for this partitioning
	double score = this->getScore(os);
	os << "Score: " << score << endl;
}