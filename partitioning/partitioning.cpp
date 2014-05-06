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

    cerr << "All offsets: ";
    for (size_t i = 0; i < offsets.size(); ++i) {
        cerr << offsets[i] << ",";
    }
    cerr << endl;

    unsigned kmax = 3;
    BaseFragment frag;
    for (size_t i = 0; i < offsets.size(); ++i) {
        for (size_t j = i + 1; j < i + 1 + kmax && j < offsets.size(); ++j) {
            frag.start = offsets[i];
            frag.end = offsets[j] + 1;
            baseFragmentsOneVersion.push(frag);
            cerr << "Frag(" << frag.start << ", " << frag.end << ")" << endl;
        }
    }
}

void RepairDocumentPartition::getPartitioningOneVersion(
    RepairTreeNode* root,
    BaseFragmentList& baseFragmentsOneVersion,
    unsigned versionSize,
    unsigned numLevelsDown)
{
    SortedByOffsetNodeSet nodes = SortedByOffsetNodeSet();
    getNodesTopNLevels(root, numLevelsDown, nodes);

    // for (auto it = nodes.begin(); it != nodes.end(); ++it) {
    //  RepairTreeNode* currNode = (*it);
    //  cerr << "Symbol: " << currNode->getSymbol() <<  ", Offset: " << currNode->getOffset() << endl;
    // }

    this->getBaseFragmentsOneVersion(nodes, baseFragmentsOneVersion);
}