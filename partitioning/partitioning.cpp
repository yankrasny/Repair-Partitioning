#include <sstream>
#include "Partitioning.h"
using namespace std;

SortedNodeSet RepairDocumentPartition::getPartitioningOneVersionRecursive(RepairTreeNode* root, unsigned numLevelsDown, SortedNodeSet& nodes)
{
	if (numLevelsDown < 1) return nodes;
	if (numLevelsDown == 1)
	{
		nodes.insert(root->getLeftChild());
		nodes.insert(root->getRightChild());
		return nodes;
	}
	nodes.insert(this->getPartitioningOneVersionRecursive(root->getLeftChild(), numLevelsDown - 1, nodes));
	nodes.insert(this->getPartitioningOneVersionRecursive(root->getRightChild(), numLevelsDown - 1, nodes));
	return nodes;
}

unsigned RepairDocumentPartition::getPartitioningOneVersion(RepairTreeNode* root, unsigned numLevelsDown, unsigned* bounds, unsigned minFragSize, unsigned versionSize)
{
	SortedNodeSet nodes = SortedNodeSet();
	nodes = getPartitioningOneVersionRecursive(root, numLevelsDown, nodes);

	unsigned numFrags = 0;
	for (auto it = nodes.begin(); it != nodes.end(); ++it)
	{
		// These left bounds should be sorted (see the comparator at the top)
		bounds[numFrags] = current->getLeftBound();
		numFrags++;
	}
	bounds[numFrags] = versionSize;

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

	// Iterate over all versions
	for (unsigned i = 0; i < this->versionData.size(); i++)
	{
		rootForThisVersion = this->versionData[i].getRootNode();

		// Do the partitioning for one version, and store the number of fragments
		numFragments = getPartitioningOneVersion(rootForThisVersion, numLevelsDown, &(this->fragmentList[versionOffset]), minFragSize, versionData[i].getVersionSize());
		versionOffset += numFragments;
		this->versionSizes[i] = numFragments;
	}
}

void RepairDocumentPartition::setFragments(const vector<vector<unsigned> >& versions, ostream& os, bool print)
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
				//found it, so use its ID
				theID = this->uniqueFrags[myHash].id;
				this->uniqueFrags[myHash].count++;
			}
			else
			{
				//Didn't find it, give it an ID
				theID = nextFragID();
				frag.id = theID;
				frag.count = 1;
				this->uniqueFrags[myHash] = frag;
			}
		}
	}
}