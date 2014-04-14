#include <sstream>
#include "Partitioning.h"
using namespace std;

void RepairDocumentPartition::getNodesTopNLevels(RepairTreeNode* node, unsigned numLevelsDown, SortedByOffsetNodeSet& nodes)
{
	if (!node || numLevelsDown < 0)
	{
		return;
	}
	nodes.insert(node);
	if (numLevelsDown == 1)
	{
		return;
	}
	if (node->getLeftChild() && node->getRightChild())
	{
		getNodesTopNLevels(node->getLeftChild(), numLevelsDown - 1, nodes);
		getNodesTopNLevels(node->getRightChild(), numLevelsDown - 1, nodes);
	}
}

void RepairDocumentPartition::getBaseFragmentsOneVersion(
	const SortedByOffsetNodeSet& nodes,
	BaseFragmentList& baseFragmentsOneVersion)
{
	// TODO remove the unnecessary duplication (compiler was upset about us trying to do it2 = it1 + 1)
	vector<unsigned> offsets = vector<unsigned>();
	for (auto it = nodes.begin(); it != nodes.end(); ++it) {
		unsigned currOffset = (*it)->getOffset();
		offsets.push_back(currOffset);
	}

	BaseFragment frag;
	for (size_t i = 0; i < offsets.size(); ++i) {
		unsigned start = offsets[i];
		for (size_t j = i + 1; j < offsets.size(); ++j) {
			frag.start = offsets[i];
			frag.end = offsets[j];
			baseFragmentsOneVersion.push(frag);
			// cerr << "Frag(" << start << ", " << end << ")" << endl;
		}
	}
}

void RepairDocumentPartition::getPartitioningOneVersion(RepairTreeNode* root,
	BaseFragmentList& baseFragmentsOneVersion, unsigned versionSize)
{
	// bool tooManyCalls = false;
	// SortedByOffsetNodeSet nodes = getBestHorizontalCut(root, 0, tooManyCalls);
	// while (tooManyCalls) {
	// 	tooManyCalls = false;
	// 	this->numLevelsDown--;
	// 	nodes = getBestHorizontalCut(root, 0, tooManyCalls);
	// }

	SortedByOffsetNodeSet nodes = SortedByOffsetNodeSet();
	getNodesTopNLevels(root, this->numLevelsDown, nodes);

	// for (auto it = nodes.begin(); it != nodes.end(); ++it) {
	// 	RepairTreeNode* currNode = (*it);
	// 	cerr << "Symbol: " << currNode->getSymbol() <<  ", Offset: " << currNode->getOffset() << endl;
	// }

	this->getBaseFragmentsOneVersion(nodes, baseFragmentsOneVersion);
}


/*** OLD STUFF BELOW ***/
/*
	We used to try to optimize 
	the cut ourselves, we now provide a ton of candidate fragments 
	for optimization by someone else
*/
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
SortedByOffsetNodeSet RepairDocumentPartition::getBestHorizontalCut(RepairTreeNode* node, int numLevels, bool& tooManyCalls)
{
	++numLevels;
	SortedByOffsetNodeSet nodes = SortedByOffsetNodeSet();
	if (!node)
		return nodes;

	if (tooManyCalls) {
		return nodes;
	}

	// cerr << "Num Calls: " << numCallsSoFar << endl;
	if (++numCallsSoFar > this->maxNumCalls) {
		numCallsSoFar = 0;
		cerr << "Too many calls to getBestHorizontalCut, trying again with numLevelsDown = " << (numLevelsDown - 1) << endl;
		tooManyCalls = true;
	    return nodes;
	}
	
	double myScore = 1.0;
	if (associations.count(node->getSymbol()) > 0)
	{
		// double x = 0.5;
		// double y = 0.5;
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

	SortedByOffsetNodeSet leftSubset = getBestHorizontalCut(leftChild, numLevels + 1, tooManyCalls);
	SortedByOffsetNodeSet rightSubset = getBestHorizontalCut(rightChild, numLevels + 1, tooManyCalls);
	
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