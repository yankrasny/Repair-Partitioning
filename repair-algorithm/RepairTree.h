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
		// Wait, is it? If this is called sometime in the middle of the algorithm, it might not be.
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
		RepairTreeNode* rightChild, RepairTreeNode* leftNeighbor, RepairTreeNode* rightNeighbor = NULL)
	{
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
		RepairTreeNode* newNode = new RepairTreeNode(symbol, leftBound, leftChild, rightChild, leftNeighbor, rightNeighbor);

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
	RepairTreeNode* addNode(unsigned symbol, Occurrence* oc, RepairTreeNode* leftNeighbor, 
		unsigned leftBound = 0)
	{
		if (done)
			return NULL;

		// TODO will this always be set in the code below?
		// It has to be. If it's not that's a bug.
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

			std::cerr << "Begin: " << *(currentLevel.begin()) << ", End: " << *(currentLevel.end()) << std::endl;

			// Search the current level for nodes whose symbol is right
			// Maybe try RepairTreeNodeComparator as the fourth parameter
			std::pair<RepairTreeSet::iterator, RepairTreeSet::iterator> rightMatches = equal_range(currentLevel.begin(), currentLevel.end(), testNode);

			std::cerr << "First: " << *(rightMatches.first) << ", Second: " << *(rightMatches.second) << std::endl;
			system("pause");

			// Delete the test node used for the call to equal_range()
			delete testNode;
			// To make sure we don't accidentally refer to it again
			testNode = NULL; 

			// Iterate through all of those nodes
			// If the left neighbor has the correct value, we can add the node
			for (RepairTreeSet::iterator it = rightMatches.first; it != rightMatches.second; it++)
			{
				std::cerr << "inside right matches loop" << std::endl;
				RepairTreeNode* rightChild = *it;

				// If the current node's left neighbor has symbol == left, then these two should have the new node as a parent
				if (rightChild->getLeftNeighbor()->getSymbol() == left)
				{
					leftChild = rightChild->getLeftNeighbor();
					
					// Consider passing NULL instead of leftNeighbor (what's semantically better?)
					newNode = createAndInsertNode(symbol, leftBound, leftChild, rightChild, leftNeighbor);
					std::cerr << "New node symbol: " << newNode->getSymbol() << std::endl;
					std::cerr << "New node left child symbol: " << newNode->getLeftChild()->getSymbol() << std::endl;
					std::cerr << "New node right child symbol: " << newNode->getRightChild()->getSymbol() << std::endl;
					system("pause");
				}
			}

			// std::cerr << "Current level size right before erase loop: " << currentLevel.size() << std::endl;
			// system("pause");

			// To maintain currentLevel, delete the ones that now have parents
			for (RepairTreeSet::iterator it = currentLevel.begin(); it != currentLevel.end(); )
			{
				// std::cerr << "Symbol: " << (*it)->getSymbol() << std::endl;
				if ( ( (*it)->hasParent() ) )
				{
					// std::cerr << "Has parent:" << (*it)->getParent() << std::endl;
					currentLevel.erase(it++);

				}
				else
				{
					// std::cerr << "DOES NOT have parent." << std::endl;
					++it;
				}
				system("pause");
			}
		}
		else
		{
			newNode = createAndInsertNode(symbol, leftBound, leftChild, rightChild, leftNeighbor);
		}

		// std::cerr << "currentLevel.size(): " << currentLevel.size() << std::endl;
		// std::cerr << "newNode: " << newNode << std::endl << std::endl;
		// system("pause");

		// assert(newNode != NULL);

		// The size is 1 twice, after adding one node, and in the end
		// We want it to be the latter, so check for existence of children (the first node won't have any)
		if (currentLevel.size() == 1 && (newNode->getLeftChild() || newNode->getRightChild()))
		{
			std::cout << "Done!" << std::endl;
			system("pause");
			// We are done with repair, the only node left in the current level is the root
			RepairTreeSet::iterator it = currentLevel.begin();
			this->head = *it;
			done = true;
		}

		return newNode;
	}
};

#endif