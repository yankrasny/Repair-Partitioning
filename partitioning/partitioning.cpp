#include <sstream>
#include "Partitioning.h"
using namespace std;

double RepairDocumentPartition::getScore(ostream& os)
{
	double term;
	double sum(0);
	double totalFragSize(0);
	for (unordered_map<string, FragInfo>::iterator it = uniqueFrags.begin(); it != uniqueFrags.end(); it++)
	{
		term = it->second.count * it->second.fragSize;
		sum += term;
		totalFragSize += it->second.fragSize;
	}
	if (uniqueFrags.size() == 0) return 0.0;

	double avgFragSize = totalFragSize / uniqueFrags.size();
	return sum / (uniqueFrags.size() * avgFragSize);
}

SortedByOffsetNodeSet RepairDocumentPartition::getNodesNthLevelDown(RepairTreeNode* root, unsigned numLevelsDown, SortedByOffsetNodeSet& nodes)
{
	if (!root) return nodes;
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

int RepairDocumentPartition::getAssociationLocation(unsigned symbol)
{
	if (memoizedAssociationLocations.count(symbol))
		return memoizedAssociationLocations[symbol];

	// It wasn't saved, so search for it, and save it for next time
	int loc = binarySearch(symbol, associations, 0, associations.size());
	memoizedAssociationLocations[symbol] = loc;

	return loc;
}

double RepairDocumentPartition::getSubsetScore(SortedByOffsetNodeSet subset)
{
	/* AVG */
	// unsigned sum(0);
	// for (SortedByOffsetNodeSet::iterator it = subset.begin(); it != subset.end(); it++)
	// {
	// 	RepairTreeNode* currNode = *it;
	// 	unsigned currScore = 1;
	// 	int loc = getAssociationLocation(currNode->getSymbol());
	// 	if (loc != -1)
	// 	{
	// 		Association a = associations[loc];
	// 		currScore = currNode->getSize() * a.getFreq();
	// 	}
	// 	sum += currScore;
	// }
	// if (subset.size() == 0 || sum == 0) return 0.0;
	// return ((double) sum) / ((double)subset.size());


	/* MAX */
	double currMax(0);
	for (SortedByOffsetNodeSet::iterator it = subset.begin(); it != subset.end(); it++)
	{
		RepairTreeNode* currNode = *it;
		double currScore = 1.0;
		int loc = getAssociationLocation(currNode->getSymbol());
		if (loc != -1)
		{
			Association a = associations[loc];
			currScore = currNode->getSize() * a.getFreq();
		}
		if (currScore > currMax)
		{
			currMax = currScore;
		}
	}
	if (subset.size() == 0 || currMax == 0) return 0.0;
	return currMax;
}

SortedByOffsetNodeSet RepairDocumentPartition::getBestSubset(RepairTreeNode* node)
{
	SortedByOffsetNodeSet nodes = SortedByOffsetNodeSet();
	if (!node)
		return nodes;
	
	double myScore = 1.0;
	int loc = getAssociationLocation(node->getSymbol());
	if (loc != -1)
	{
		Association a = associations[loc];
		myScore = node->getSize() * a.getFreq();
	}

	// node is a terminal 
	if (!node->getLeftChild())
	{
		nodes.insert(node);
		return nodes;
	}

	RepairTreeNode* leftChild = node->getLeftChild();
	RepairTreeNode* rightChild = node->getRightChild();

	SortedByOffsetNodeSet leftSubset = getBestSubset(leftChild);
	SortedByOffsetNodeSet rightSubset = getBestSubset(rightChild);
	
	double leftScore = getSubsetScore(leftSubset);
	double rightScore = getSubsetScore(rightSubset);

	// TODO try changing to max(leftScore, rightScore)
	double childrenScore = fragmentationCoefficient * (leftScore + rightScore); // Coefficient for fragmenting

	if (myScore >= childrenScore)
	{
		nodes.insert(node);
		return nodes;
	}

	nodes.insert(leftSubset.begin(), leftSubset.end());
	nodes.insert(rightSubset.begin(), rightSubset.end());
	return nodes;
}

unsigned RepairDocumentPartition::getPartitioningOneVersion(RepairTreeNode* root, unsigned numLevelsDown, unsigned* bounds, unsigned minFragSize, unsigned versionSize)
{
	SortedByOffsetNodeSet nodes = SortedByOffsetNodeSet();
	switch (this->method)
	{
		case RepairDocumentPartition::NAIVE: nodes = getNodesNthLevelDown(root, numLevelsDown, nodes);
		    break;
		case RepairDocumentPartition::GREEDY: nodes = getBestSubset(root);
		    break;
		// default is greedy
		default: nodes = getBestSubset(root);
		    break;
	}

	unsigned prevVal(0); // the previous node's index in the file 
	unsigned currVal(0); // the current node's index in the file	
	unsigned diff(0); // the difference between consecutive indexes (a large value signifies a good fragment)
	unsigned numFrags(0); // the number of fragments (gets incremented in the following loop)
	RepairTreeNode* previous(NULL);

	// We know the first fragment is always at the beginning of the file, and we'll skip the first node in the loop below
	bounds[0] = 0;
	++numFrags;

	// cerr << "Version Start" << endl;
	for (auto it = nodes.begin(); it != nodes.end(); ++it)
	{

		if (numFrags > MAX_NUM_FRAGMENTS_PER_VERSION - 1) {
			break;
		}

		RepairTreeNode* current = *it;

		// cerr << "Symbol: " << current->getSymbol() << endl;
		// cerr << current->getOffset() << ",";

		if (!previous) {
			previous = current;
			continue;
		}

		prevVal = previous->getOffset();
		currVal = current->getOffset();

		diff = currVal - prevVal;
		if (diff >= minFragSize)
		{
			// These offsets are already sorted (see the comparator at the top)
			bounds[numFrags] = current->getOffset();
			numFrags++;
		}

		// Update previous to point to this node now that we're done with it
		previous = current;
	}
	// cerr << endl << endl;
	// system("pause");

	// Calculate the last diff so that the last fragment obeys the minFragSize rule
	if (nodes.size() >= 1)
	{
		unsigned lastDiff = versionSize - bounds[numFrags - 1];
		if (lastDiff >= minFragSize)
		{
			// Our last fragment is ok, just add the position at the end of the file
			bounds[numFrags] = versionSize;
			numFrags++;	
		}
		else
		{
			// Our last fragment is too small, replace the last offset we had with the position at the end of the file
			bounds[numFrags - 1] = versionSize;
		}
	}


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

		if (i == 202) {
			cout << "cacaw!" << endl;
		}
		// Do the partitioning for one version, and store the number of fragments
		numFragments = getPartitioningOneVersion(rootForThisVersion, numLevelsDown, 
			&(this->offsets[versionOffset]), minFragSize, versionData[i].getVersionSize());
		versionOffset += numFragments;
		this->versionSizes[i] = numFragments;
	}
}

void RepairDocumentPartition::setFragmentInfo(const vector<vector<unsigned> >& versions, ostream& os, bool print)
{
	os << "*** Fragments ***" << endl;

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

	// this->setFragmentInfo(versions, os, printFragments);

	// Assign fragment IDs and stick them in a hashmap
	unordered_map<string, FragInfo> uniqueFrags;
	this->updateUniqueFragmentHashMap();

	// Now decide on the score for this partitioning
	double score = this->getScore(os);
	os << "Score: " << score << endl;
	os.close();
}