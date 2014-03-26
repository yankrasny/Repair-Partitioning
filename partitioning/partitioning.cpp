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

double RepairDocumentPartition::getSubsetScore(SortedByOffsetNodeSet subset)
{
	/* MAX */
	double currMax(0);
	for (SortedByOffsetNodeSet::iterator it = subset.begin(); it != subset.end(); it++)
	{
		RepairTreeNode* currNode = *it;
		double currScore = 1.0;

		if (associations.count(currNode->getSymbol()) > 0)
		{
			Association a = associations[currNode->getSymbol()];
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

/*
TODO

global var numCalls
Also see commented code below and finish it

*/
unsigned numCalls = 0;
SortedByOffsetNodeSet RepairDocumentPartition::getBestSubset(RepairTreeNode* node, int numLevels, bool& tooManyCalls)
{
	++numLevels;
	SortedByOffsetNodeSet nodes = SortedByOffsetNodeSet();
	if (!node)
		return nodes;

	if (tooManyCalls) {
		return nodes;
	}

	// cerr << "Num Calls: " << numCalls << endl;
	if (++numCalls > this->maxNumCalls) {
		numCalls = 0;
		cerr << "Too many calls to getBestSubset, trying again with numLevelsDown--" << endl;
		tooManyCalls = true;
	    return nodes;
	}
	
	double myScore = 1.0;
	if (associations.count(node->getSymbol()) > 0)
	{
		double x = 0.5;
		double y = 0.5;
		Association a = this->associations[node->getSymbol()];
		// myScore = x * node->getSize() + y * a.getFreq();
		myScore = node->getSize() * a.getFreq();
	}

	// node is a terminal
	if (!node->getLeftChild())
	{
		nodes.insert(node);
		return nodes;
	}

	// Limit the number of recursive calls
	if (numLevels > this->numLevelsDown) {
		nodes.insert(node);
		return nodes;
	}

	RepairTreeNode* leftChild = node->getLeftChild();
	RepairTreeNode* rightChild = node->getRightChild();

	SortedByOffsetNodeSet leftSubset = getBestSubset(leftChild, numLevels + 1, tooManyCalls);
	SortedByOffsetNodeSet rightSubset = getBestSubset(rightChild, numLevels + 1, tooManyCalls);
	
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

void RepairDocumentPartition::getPartitioningOneVersion(RepairTreeNode* root,
	vector<unsigned>& bounds, unsigned versionSize)
{
	bool tooManyCalls = false;
	SortedByOffsetNodeSet nodes = getBestSubset(root, 0, tooManyCalls);
	while (tooManyCalls) {
		tooManyCalls = false;
		this->numLevelsDown--;
		nodes = getBestSubset(root, 0, tooManyCalls);
	}

	unsigned prevVal(0); // the previous node's index in the file
	unsigned currVal(0); // the current node's index in the file
	unsigned diff(0); // the difference between consecutive indexes (a large value signifies a good fragment)
	RepairTreeNode* previous(NULL);

	// cerr << "Version Start" << endl;

	// We know the first fragment is always at the beginning of the file, and we'll skip the first node in the loop below
	bounds.push_back(0);

	// There is no partitioning, just return [0, size]
	if (nodes.size() < 2) {
		bounds.push_back(versionSize);
	} else {
		for (auto it = nodes.begin(); it != nodes.end(); ++it)
		{
			// We've decided that this is too many, the last one is gonna be huge and stupid
			// Leave room for the last one

			// TODO re-partition with less levels
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

			assert(prevVal < currVal);

			diff = currVal - prevVal;

			// TODO
			// if (diff > MAX_FRAG_LENGTH)
			// {

			// }

			if (diff >= this->minFragSize)
			{
				// These offsets are already sorted (see the comparator at the top)
				bounds.push_back(currVal);
			}

			// Update previous to point to this node now that we're done with it
			previous = current;
		}
		
		// We might only have bounds = [0], in that case just add version size regardless of anything else
		if (bounds.size() == 1)
		{
			assert(bounds[0] == 0);
			bounds.push_back(versionSize);
		} else { // Calculate the last diff so that the last fragment obeys the minFragSize rule
			unsigned lastDiff = versionSize - bounds.back();
			if (lastDiff >= this->minFragSize)
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
	}
}