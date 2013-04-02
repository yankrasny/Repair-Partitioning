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

	RepairTreeNode* parent;

public:
	RepairTreeNode() : symbol(0), leftBound(0), leftChild(NULL), rightChild(NULL), leftNeighbor(NULL), rightNeighbor(NULL), parent(NULL) {}

	RepairTreeNode(unsigned symbol) : symbol(symbol), leftBound(0), leftChild(NULL), rightChild(NULL), leftNeighbor(NULL), rightNeighbor(NULL), parent(NULL) {}

	RepairTreeNode(unsigned symbol, RepairTreeNode* leftChild, RepairTreeNode* rightChild, RepairTreeNode* leftNeighbor)
		: symbol(symbol), leftChild(leftChild), rightChild(rightChild), leftNeighbor(leftNeighbor)
	{
		// TODO is this correct?
		if (leftChild)
		{
			leftBound = leftChild->getLeftBound();	
		}

		// This part is fine
		if (leftNeighbor)
		{
			leftNeighbor->setRightNeighbor(this);	
		}

		if (rightChild)
		{
			rightChild->setParent(this);
		}

		if (leftChild)
		{
			leftChild->setParent(this);
		}
	}

	RepairTreeNode* getParent() const
	{
		return this->parent;
	}

	bool hasParent() const
	{
		return (bool) parent;
	}

	void setParent(RepairTreeNode* parent)
	{
		this->parent = parent;
	}

	unsigned getSymbol() const
	{
		return symbol;
	}

	unsigned getLeftBound() const
	{
		return leftBound;
	}

	RepairTreeNode* getLeftNeighbor() const
	{
		return leftNeighbor;
	}

	void setRightNeighbor(RepairTreeNode* newRightNeighbor)
	{
		this->rightNeighbor = newRightNeighbor;
	}

	RepairTreeNode* getLeftChild()
	{
		return leftChild;
	}
	
	RepairTreeNode* getRightChild()
	{
		return rightChild;
	}
};

#endif