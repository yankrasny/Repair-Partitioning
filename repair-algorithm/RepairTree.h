#ifndef REPAIR_TREE_H
#define REPAIR_TREE_H

#include "RepairTreeNode.h"
#include "Occurrence.h"
#include <set>

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
	std::multiset<RepairTreeNode*> currentLevel;

	/*
		
	*/
	bool done;

public:
	RepairTree()
	{
		currentLevel = std::multiset<RepairTreeNode*>();
		done = false;
		head = NULL;
	}

	RepairTreeNode* getHead()
	{
		return head;
	}

	/*
		getCover can be getCut or something, just wanted to show that the result of this must 
		not leave out any parts of the document
	*/
	std::multiset<RepairTreeNode*> getCover()
	{
		// TODO this is where the magic happens
		// The next step is to produce what Jinru calls a base partition

		// Actually a really good first step, it's definitely a cover!
		return currentLevel;
	}

	/*
		Summary: Adds a node to the repair tree, connecting to the children and neighbors

		Description:
			This is called in two ways: during the first run through the string, and during repair itself.
				1) During the first run:
					- we know our left neighbor (see prevTreeNode in extractPairs())
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

		RepairTreeNode* leftChild(NULL);
		RepairTreeNode* rightChild(NULL);


		// This is not a leaf: oc contains a left and a right, so search for them
		// If we find them next to each other, make a new node and connect them as children
		if (oc)
		{
			unsigned leftPos = oc->getLeftPositionInSequence();
			
			unsigned left = oc->getLeft();
			unsigned right = oc->getRight();

			// Search the current level for right
			// Finding all of the rights is O(n) -> bad but we'll optimize
			std::multiset<RepairTreeNode*>::iterator it;
			for (it = currentLevel.begin(); it != currentLevel.end(); it++)
			{
				RepairTreeNode* currNode = *it;   
				if (currNode->getSymbol() == right)
				{
					// OK, we found a node that can be the right child of our new node
					rightChild = currNode;
					
					// Now see if that node's left neighbor can be the left child of our new node
					if (rightChild->getLeftNeighbor()->getSymbol() == left)
					{
						leftChild = rightChild->getLeftNeighbor();
						
						// Once found, remove them from the current level
						currentLevel.erase(leftChild);
						currentLevel.erase(rightChild);
					}
				}
			}
		}

		// This part is fucking brilliant
		if (leftChild && !leftNeighbor)
		{
			leftNeighbor = leftChild->getLeftNeighbor();
		}

		// Create the new node as a parent of the two children
		RepairTreeNode* newNode = new RepairTreeNode(symbol, leftChild, rightChild, leftNeighbor);

		// And now add newNode to the current level
		currentLevel.insert(newNode);

		// The size is 1 twice, after adding one node, and in the end
		// We want it to be the latter, so check for existence of children (the first node won't have any)
		if (currentLevel.size() == 1 && (leftChild || rightChild))
		{
			// We are done with repair, the only node left in the current level is the root
			std::multiset<RepairTreeNode*>::iterator it = currentLevel.begin();
			this->head = *it;
			done = true;
		}

		return newNode;
	}
};

#endif