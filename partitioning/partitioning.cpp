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

void RepairDocumentPartition::getPartitioningOneVersion(
	RepairTreeNode* root,
	BaseFragmentList& baseFragmentsOneVersion,
	unsigned versionSize,
	unsigned numLevelsDown)
{
	// bool tooManyCalls = false;
	// SortedByOffsetNodeSet nodes = getBestHorizontalCut(root, 0, tooManyCalls);
	// while (tooManyCalls) {
	// 	tooManyCalls = false;
	// 	numLevelsDown--;
	// 	nodes = getBestHorizontalCut(root, 0, tooManyCalls);
	// }

	// TODO don't control numLevelsDown in this class, just take it as a regular param here
	SortedByOffsetNodeSet nodes = SortedByOffsetNodeSet();
	getNodesTopNLevels(root, numLevelsDown, nodes);

	// for (auto it = nodes.begin(); it != nodes.end(); ++it) {
	// 	RepairTreeNode* currNode = (*it);
	// 	cerr << "Symbol: " << currNode->getSymbol() <<  ", Offset: " << currNode->getOffset() << endl;
	// }

	this->getBaseFragmentsOneVersion(nodes, baseFragmentsOneVersion);
}