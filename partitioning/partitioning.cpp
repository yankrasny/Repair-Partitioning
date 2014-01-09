#include <sstream>
#include "Partitioning.h"
using namespace std;


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

int i = 0;
SortedByOffsetNodeSet RepairDocumentPartition::getBestSubset(RepairTreeNode* node, int numLevels)
{
	numLevels++;
	SortedByOffsetNodeSet nodes = SortedByOffsetNodeSet();
	if (!node)
		return nodes;
	
	double myScore = 1.0;
	int loc = getAssociationLocation(node->getSymbol());
	if (loc != -1)
	{
		Association a = this->associations[loc];
		myScore = node->getSize() * a.getFreq();
	}

	// node is a terminal 
	if (!node->getLeftChild())
	{
		nodes.insert(node);
		return nodes;
	}

	// int depth = node->getDepth();
	// cerr << "Depth: " << depth << endl;

	// Limit the number of recursive calls
	if (numLevels > this->numLevelsDown) {
		nodes.insert(node);
		return nodes;
	}
	// cerr << ++i << endl;
	// system("pause");

	RepairTreeNode* leftChild = node->getLeftChild();
	RepairTreeNode* rightChild = node->getRightChild();

	SortedByOffsetNodeSet leftSubset = getBestSubset(leftChild, numLevels + 1);
	SortedByOffsetNodeSet rightSubset = getBestSubset(rightChild, numLevels + 1);
	
	double leftScore = getSubsetScore(leftSubset);
	double rightScore = getSubsetScore(rightSubset);

	// TODO try changing to max(leftScore, rightScore)
	double childrenScore = fragmentationCoefficient * (leftScore + rightScore); // Raise coefficient to favor fragmenting more

	if (myScore >= childrenScore)
	{
		nodes.insert(node);
		return nodes;
	}

	nodes.insert(leftSubset.begin(), leftSubset.end());
	nodes.insert(rightSubset.begin(), rightSubset.end());
	return nodes;
}

unsigned RepairDocumentPartition::getPartitioningOneVersion(RepairTreeNode* root, unsigned numLevelsDown,
	vector<unsigned>& bounds, unsigned minFragSize, unsigned versionSize)
{
	// cerr << "Version Number: " << root->getVersionNum() << endl;
	SortedByOffsetNodeSet nodes = SortedByOffsetNodeSet();
	switch (this->method)
	{
		case RepairDocumentPartition::NAIVE: nodes = getNodesNthLevelDown(root, numLevelsDown, nodes);
		    break;
		case RepairDocumentPartition::GREEDY: nodes = getBestSubset(root);
		    break;
		default: nodes = getBestSubset(root);
		    break;
	}

	unsigned prevVal(0); // the previous node's index in the file
	unsigned currVal(0); // the current node's index in the file
	unsigned diff(0); // the difference between consecutive indexes (a large value signifies a good fragment)
//	unsigned numFrags(0); // the number of fragments (gets incremented in the following loop)
	RepairTreeNode* previous(NULL);

	// We know the first fragment is always at the beginning of the file, and we'll skip the first node in the loop below
	bounds.push_back(0);

	// cerr << "Version Start" << endl;
	for (auto it = nodes.begin(); it != nodes.end(); ++it)
	{
		// We've decided that this is too many, the last one is gonna be huge and stupid 
		// Leave room for the last one
		if (bounds.size() > MAX_NUM_FRAGMENTS_PER_VERSION - 1) {
			break;
		}

		RepairTreeNode* current = *it;

		if (!previous) {
			previous = current;
			continue;
		}

		prevVal = previous->getOffset();
		currVal = current->getOffset();

		// assert prevVal < currVal
		if (prevVal >= currVal) {
			throw 6;
		}

		diff = currVal - prevVal;
		if (diff >= minFragSize)
		{
			// These offsets are already sorted (see the comparator at the top)
			// bounds[++numFrags] = currVal;
			bounds.push_back(currVal);
		}

		// Update previous to point to this node now that we're done with it
		previous = current;
	}

	// Calculate the last diff so that the last fragment obeys the minFragSize rule
	if (nodes.size() >= 1)
	{
		unsigned lastDiff = versionSize - bounds.back();
		if (lastDiff >= minFragSize)
		{
			// Our last fragment is ok, just add the position at the end of the file
			// bounds[++numFrags] = versionSize;
			bounds.push_back(versionSize);
		}
		else
		{
			// Our last fragment is too small, replace the last offset we had with the position at the end of the file
			// bounds[numFrags - 1] = versionSize;
			bounds.pop_back();
			bounds.push_back(versionSize);
		}
	}

	// return numFrags;
	return bounds.size();
}
