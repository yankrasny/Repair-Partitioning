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
    // We have a data structure like this:
    // Level 0: [0, versionSize]
    // Level 1: [0, o1, versionSize]
    // Level 2: [0, o1, o2, versionSize]
    // ...
    // Level n: [0, o1, o2, ..., on, versionSize] we don't necessarily get n inner offsets here, but we get more as we go down the tree

    vector<vector<unsigned> > offsetsAllVersions = vector<vector<unsigned> >();
    vector<unsigned> offsets;
    SortedByOffsetNodeSet nodesOneVersion;
    for (auto it = nodes.begin(); it != nodes.end(); ++it) {
        nodesOneVersion = (*it);
        offsets = vector<unsigned>();
        for (auto it2 = nodesOneVersion.begin(); it2 != nodesOneVersion.end(); ++it2) {
            unsigned currOffset = (*it2)->getOffset();
            offsets.push_back(currOffset);
        }
        // We get all the offsets except for version size, just add that in
        offsets.push_back(versionSize);
        offsetsAllVersions.push_back(offsets);
    }

    BaseFragment frag;
    for (size_t i = 0; i < offsetsAllVersions.size() - 1; ++i) {
        // cerr << "Level: " << i << endl;
        frag.start = offsets[i];
        frag.end = offsets[i + 1];
        baseFragmentsOneVersion.push(frag);

        // for (size_t j = 0; j < offsetsAllVersions[i].size(); ++j) {
        //     cerr << "CurrOffset: " << offsetsAllVersions[i][j] << endl;
        // }
        // cerr << endl;
    }

    // exit(1);

    // TODO think about what it means when you're missing nodes
    // Let's say that level 1 and 2 are the same, that's because we have some missing nodes
    // Ok, so then... shit lost train of thought. Whatever, I think I'll find it somewhere, in a dark place, years from now.

    // Yea this be TODO also mofo
    // Not sure what to do with kmax exactly, but we had the right idea. let's just make this shit work
    // Ok for now get the adjacent ones, just hierarchical and leave kmax for later

    // unsigned kmax = 3;
    // BaseFragment frag;
    // for (size_t i = 0; i < offsets.size(); ++i) {
    //     for (size_t j = i + 1; j < i + 1 + kmax && j < offsets.size(); ++j) {
    //         frag.start = offsets[i];
    //         frag.end = offsets[j] + 1;
    //         baseFragmentsOneVersion.push(frag);
    //         // cerr << "Frag(" << frag.start << ", " << frag.end << ")" << endl;
    //     }
    // }
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