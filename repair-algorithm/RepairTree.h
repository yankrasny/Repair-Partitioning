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
struct RepairTreeNodeComparator
{
	bool operator() (const RepairTreeNode* lhs, const RepairTreeNode* rhs) const
	{
		return lhs->getSymbol() < rhs->getSymbol();
	}
};

typedef std::multiset<RepairTreeNode*, RepairTreeNodeComparator> RepairTreeSet;

class RepairTree
{
	/*
		This is only used if we go all the way with repair
	*/
	RepairTreeNode* head;

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
		head = NULL;
	}

	RepairTreeNode* getHead() const
	{
		return head;
	}

	/*
		getCover can be getCut or something, just wanted to show that the result of this must 
		not leave out any parts of the document
	*/
	RepairTreeSet getCover()
	{
		// TODO this is where the magic happens
		// The next step is to produce what Jinru calls a base partition

		// Actually a really good first step, it's definitely a cover!
		return currentLevel;
	}

	/*
		Summary: creates a RepairTreeNode and adds is to currentLevel

		Description:
			This function is called in one of two ways:
				1) The children are NULL and leftNeighbor is set
				2) The children are set and leftNeighbor is NULL
	*/
	RepairTreeNode* createAndInsertNode(unsigned symbol, RepairTreeNode* leftChild, RepairTreeNode* rightChild, RepairTreeNode* leftNeighbor)
	{
		// This part is fucking brilliant
		// In the case where we scan for children, we don't pass a left neighbor
		// We inherit our child's left neighbor
		if (leftChild && !leftNeighbor)
		{
			leftNeighbor = leftChild->getLeftNeighbor();
		}

		// Create the new node as a parent of the two children
		RepairTreeNode* newNode = new RepairTreeNode(symbol, leftChild, rightChild, leftNeighbor);

		// And now add newNode to the current level
		currentLevel.insert(newNode);

		return newNode;
	}

	/*
		Summary: Adds all the nodes for a symbol to the repair tree

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
	RepairTreeNode* addNode(unsigned symbol, Occurrence* oc, RepairTreeNode* leftNeighbor)
	{
		if (done)
			return NULL;

		// TODO will this always be set in the code below?
		RepairTreeNode* newNode = NULL;

		RepairTreeNode* leftChild(NULL);
		RepairTreeNode* rightChild(NULL);

		// This is not a leaf: oc contains a left and a right, so search for them
		// If we find them next to each other, connect them as children
		if (oc)
		{
			// TODO add this to the RepairTreeNode constructor
			unsigned leftPos = oc->getLeftPositionInSequence();
			
			// An occurrence has a left and right character. symbol is the parent of those two characters in this tree.
			unsigned left = oc->getLeft();
			unsigned right = oc->getRight();

			// Just need this for the equal_range function
			RepairTreeNode* testNode = new RepairTreeNode(right);

			// Search the current level for nodes whose symbol is right
			std::pair<RepairTreeSet::iterator, RepairTreeSet::iterator> rightMatches = equal_range(currentLevel.begin(), currentLevel.end(), testNode);

			// Delete the test node used for the call to equal_range()
			delete testNode;
			testNode = NULL;

			// Iterate through all of those nodes
			// If the left neighbor has the correct value, we can add the node
			for (RepairTreeSet::iterator it = rightMatches.first; it != rightMatches.second; it++)
			{
				RepairTreeNode* rightChild = *it;

				// If the current node's left neighbor has symbol == left, then these two should have the new node as a parent
				if (rightChild->getLeftNeighbor()->getSymbol() == left)
				{
					leftChild = rightChild->getLeftNeighbor();
					
					// Consider passing NULL instead of leftNeighbor (what's semantically better?)
					newNode = createAndInsertNode(symbol, leftChild, rightChild, leftNeighbor);
				}
			}

			// To maintain currentLevel, delete the ones that now have parents
			for (RepairTreeSet::iterator it = currentLevel.begin(); it != currentLevel.end(); )
			{
				if ( !((*it)->hasParent()) )
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
			newNode = createAndInsertNode(symbol, leftChild, rightChild, leftNeighbor);
		}

		// The size is 1 twice, after adding one node, and in the end
		// We want it to be the latter, so check for existence of children (the first node won't have any)
		std::cout << newNode << std::endl;
		if (currentLevel.size() == 1 && newNode && (newNode->getLeftChild() || newNode->getRightChild()))
		{
			std::cout << "Here" << std::endl;
			system("pause");
			// We are done with repair, the only node left in the current level is the root
			RepairTreeSet::iterator it = currentLevel.begin();
			this->head = *it;
			done = true;
		}

		return newNode;
	}
};

/* Alternate implementation: sparse array

1 2 3 1 2 4 [n = 6]

numLevels = ceil(log2(n)) + 1
numNodes = sum(2^numLevels) for range(0, n-1)
array = allocate(numNodes)

(5 -> 1,2)

[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0]

[1 2 0 0 0 0 0 0 0 0 0 0 0 0 0 0 5 3 1 2 0 0 0 0 6 5 0 0 7 4 8]

[lvl(n) lvl(n-1) ... lvl(0)]
[Leaves .............. Root]

5 3 5 4

(6 -> 5,3)

6 5 4

(7 -> 6,5)

7 4

(8 -> 7,4)

8

					8
				   / \
				  7   4
				 /\  
				6  5
			   /\  /\
			  5  3 1 2
			 /\
			1  2

*/

#endif