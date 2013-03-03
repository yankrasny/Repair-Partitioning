#ifndef REPAIR_TREE_NODE_H
#define REPAIR_TREE_NODE_H

#include "Util.h"

class RepairTreeNode
{
	unsigned symbol;

	unsigned leftBound;

	RepairTreeNode* leftChild;
	RepairTreeNode* rightChild;

	RepairTreeNode* leftNeighbor;
	RepairTreeNode* rightNeighbor;

public:
	RepairTreeNode() : symbol(0), leftBound(0), leftChild(NULL), rightChild(NULL), leftNeighbor(NULL), rightNeighbor(NULL) {}
	RepairTreeNode(unsigned symbol, RepairTreeNode* leftChild, RepairTreeNode* rightChild, RepairTreeNode* leftNeighbor) 
		: symbol(symbol), leftChild(leftChild), rightChild(rightChild), leftNeighbor(leftNeighbor)
	{
		if (leftChild)
		{
			leftBound = leftChild->getLeftBound();	
		}
		if (leftNeighbor)
		{
			leftNeighbor->setRightNeighbor(this);	
		}
	}

	unsigned getSymbol()
	{
		return symbol;
	}

	unsigned getLeftBound()
	{
		return leftBound;
	}

	RepairTreeNode* getLeftNeighbor()
	{
		return leftNeighbor;
	}

	void setRightNeighbor(RepairTreeNode* newRightNeighbor)
	{
		this->rightNeighbor = newRightNeighbor;
	}
};

#endif