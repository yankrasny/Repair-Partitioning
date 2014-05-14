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

void RepairDocumentPartition::getNodesNthLevelDown(RepairTreeNode* node, unsigned numLevelsDown, SortedByOffsetNodeSet& nodes)
{
    if (!node || numLevelsDown < 0)
    {
        return;
    }
    if (numLevelsDown == 0)
    {
        nodes.insert(node);
        return;
    }
    if (numLevelsDown == 1)
    {
        if (node->getLeftChild()) {
            nodes.insert(node->getLeftChild());
            nodes.insert(node->getRightChild());
        }
        nodes.insert(node);
        return;
    }
    if (node->getLeftChild() && node->getRightChild())
    {
        getNodesTopNLevels(node->getLeftChild(), numLevelsDown - 1, nodes);
        getNodesTopNLevels(node->getRightChild(), numLevelsDown - 1, nodes);
    }
}

void RepairDocumentPartition::getBaseFragmentsOneVersion(
    const vector<SortedByOffsetNodeSet>& nodes,
    BaseFragmentList& baseFragmentsOneVersion,
    unsigned versionSize)
{
    // We have a data structure like this (just an array of size n):
    // Level 0: [0, versionSize]
    // Level 1: [0, o1, versionSize]
    // Level 2: [0, o1, o2, versionSize]
    // ...
    // Level n: [0, o1, o2, ..., on, versionSize] we don't necessarily get n inner offsetsOneLevel here, but we get more as we go down the tree

    vector<vector<unsigned> > offsetsAllLevels = vector<vector<unsigned> >();
    vector<unsigned> offsetsOneLevel;
    SortedByOffsetNodeSet nodesOneLevel;
    for (auto it = nodes.begin(); it != nodes.end(); ++it) {
        nodesOneLevel = (*it);
        offsetsOneLevel = vector<unsigned>();
        for (auto it2 = nodesOneLevel.begin(); it2 != nodesOneLevel.end(); ++it2) {
            unsigned currOffset = (*it2)->getOffset();
            offsetsOneLevel.push_back(currOffset);
        }
        // We get all the offsetsOneLevel except for version size, just add that in
        offsetsOneLevel.push_back(versionSize);
        offsetsAllLevels.push_back(offsetsOneLevel);
    }

    BaseFragment frag;
    int kmax = 2;
    for (size_t i = 0; i < offsetsAllLevels.size(); ++i) {
        // cerr << "Level: " << i << endl;
        for (size_t j = 0; j < offsetsAllLevels[i].size() - 1; ++j) {

            for (size_t k = 0; k < kmax && j + k < offsetsAllLevels[i].size() - 1; ++k) {
                frag.start = offsetsAllLevels[i][j];
                frag.end = offsetsAllLevels[i][j + k + 1];

                if (frag.start >= frag.end) {
                    cerr << "Fragment is invalid..." << endl;
                    cerr << "(" << frag.start << "," << frag.end << ")" << endl;
                    exit(1);
                }

                baseFragmentsOneVersion.push(frag);
            }
        }

        // for (size_t j = 0; j < offsetsAllLevels[i].size(); ++j) {
        //     cerr << "CurrOffset: " << offsetsAllLevels[i][j] << endl;
        // }
        // cerr << endl;
    }
}

void RepairDocumentPartition::getPartitioningOneVersion(
    RepairTreeNode* root,
    BaseFragmentList& baseFragmentsOneVersion,
    unsigned versionSize,
    unsigned numLevelsDown)
{
    vector<SortedByOffsetNodeSet> nodes = vector<SortedByOffsetNodeSet>();
    
    for (size_t n = 0; n < numLevelsDown; n++) {
        nodes.push_back(SortedByOffsetNodeSet());
        nodes[n] = SortedByOffsetNodeSet();
        getNodesNthLevelDown(root, n, nodes[n]);

        // cerr << "Level: " << n << endl;
        for (auto it = nodes[n].begin(); it != nodes[n].end(); ++it) {
            RepairTreeNode* currNode = (*it);
            // cerr << "Symbol: " << currNode->getSymbol() <<  ", Offset: " << currNode->getOffset() << endl;
        }
    }
    // cerr << endl;
    // cerr << "Version Size: " << versionSize << endl;

    this->getBaseFragmentsOneVersion(nodes, baseFragmentsOneVersion, versionSize);
}