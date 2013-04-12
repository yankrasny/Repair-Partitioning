#ifndef REPAIR_TREE_H
#define REPAIR_TREE_H

#include "RepairTreeNode.h"
#include "Occurrence.h"
#include <set>

/*
	In class RepairTree, we maintain a multiset of RepairTreeNode pointers. 
	Here, we implement the comparator for that multiset. 
	We want nodes with the same symbol to be next to each other
	so that we can use the function equal_range()
*/
class RepairTreeNodeComparator
{
public:
	bool operator() (const RepairTreeNode* const lhs, const RepairTreeNode* const rhs) const
	{
		return lhs->getSymbol() < rhs->getSymbol();
	}
};

typedef std::multiset<RepairTreeNode*, RepairTreeNodeComparator> RepairTreeSet;

class RepairTree
{
	/*
		If repair terminates early, then currentLevel represents the nodes of the document 
		that have resulted from repair (I know that sounds obvious)  
	*/
	RepairTreeSet currentLevel;

	/*
		This flag is checked before adding a node. If it is set, then we don't add that node.
	*/
	bool done;

public:
	RepairTree()
	{
		currentLevel = RepairTreeSet();
		done = false;
	}

	RepairTreeSet getCurrentLevel()
	{
		return currentLevel;
	}

	/*
		Summary: creates a RepairTreeNode and adds is to currentLevel

		Description:
			This function is called in one of two ways:
				1) The children are NULL and leftNeighbor is set
				2) The children are set and leftNeighbor is NULL
	*/
	RepairTreeNode* createAndInsertNode(unsigned symbol, unsigned leftBound, RepairTreeNode* leftChild, 
		RepairTreeNode* rightChild, RepairTreeNode* leftNeighbor, unsigned versionNum = 0)
	{
		RepairTreeNode* rightNeighbor = NULL;

		// In the case where we scan for children, we don't pass a left neighbor
		// We inherit our child's left neighbor
		if (leftChild && !leftNeighbor)
		{
			leftNeighbor = leftChild->getLeftNeighbor();
		}

		if (rightChild && !rightNeighbor)
		{
			rightNeighbor = rightChild->getRightNeighbor();
		}

		// Create the new node as a parent of the two children
		RepairTreeNode* newNode = new RepairTreeNode(symbol, leftBound, leftChild, rightChild, leftNeighbor, rightNeighbor, versionNum);

		// And now add newNode to the current level
		currentLevel.insert(newNode);

		return newNode;
	}

	/*
		Summary: Creates and adds all the nodes for a symbol to the repair tree

		Description:
			This is called in two ways: during the first run through the string, and during repair itself.
				1) During the first run:
					- we know our left neighbor (see prevTreeNode in repair.cpp:extractPairs())
					- we don't have any children
					- So pass symbol, NULL, leftNeighbor
				2) During repair
					- we don't know our left neighbor (this is figured out by the children)
					- we do have children, and we need to find them
					- So pass symbol, oc, NULL

	*/
	RepairTreeNode* addNodes(unsigned symbol, Occurrence* oc, RepairTreeNode* leftNeighbor, unsigned versionNum = 0, unsigned leftBound = 0)
	{
		// TODO set this somewhere, or just get rid of it
		if (done)
			return NULL;

		// This always has to be set in the code below (if not, we have a bug)
		RepairTreeNode* newNode = NULL;

		RepairTreeNode* leftChild(NULL);
		RepairTreeNode* rightChild(NULL);

		// This is not a leaf: oc contains a left and a right, so search for them
		// If we find them next to each other, connect them as children
		if (oc)
		{
			leftBound = oc->getLeftPositionInSequence();
			
			// An occurrence has a left and right character. symbol is the parent of those two characters in this tree.
			unsigned left = oc->getLeft();
			unsigned right = oc->getRight();

			// Just need this for the equal_range function
			RepairTreeNode* testNode = new RepairTreeNode(right);

			// Search the current level for nodes whose symbol is right
			std::pair<RepairTreeSet::iterator, RepairTreeSet::iterator> rightMatches = currentLevel.equal_range(testNode);

			// Release mem and clear ref for the test node used for the call to equal_range()
			delete testNode;
			testNode = NULL;

			// Iterate through all of those nodes
			// If the left neighbor has the correct value, we can add the node
			for (RepairTreeSet::iterator it = rightMatches.first; it != rightMatches.second; it++)
			{
				RepairTreeNode* rightChild = *it;

				// If the current node's left neighbor has symbol == left, then these two should have the new node as a parent
				if (rightChild->getLeftNeighbor() != NULL) // Edge case
				{
					if (rightChild->getLeftNeighbor()->getSymbol() == left)
					{
						leftChild = rightChild->getLeftNeighbor();
						
						// Consider passing NULL instead of leftNeighbor (what's semantically better?)
						newNode = createAndInsertNode(symbol, leftBound, leftChild, rightChild, leftNeighbor);
					}
				}
			}

			// To maintain currentLevel, delete the ones that now have parents
			for (RepairTreeSet::iterator it = currentLevel.begin(); it != currentLevel.end(); )
			{
				if ( ( (*it)->hasParent() ) )
				{
					currentLevel.erase(it++);
				}
				else
				{
					++it;
				}
			}
		}
		else
		{
			newNode = createAndInsertNode(symbol, leftBound, leftChild, rightChild, leftNeighbor, versionNum);
		}
		
		return newNode;
	}
};

#endif