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

	RepairTreeNode(unsigned symbol, unsigned leftBound, RepairTreeNode* leftChild, RepairTreeNode* rightChild, RepairTreeNode* leftNeighbor, RepairTreeNode* rightNeighbor)
		: symbol(symbol), leftBound(leftBound), leftChild(leftChild), rightChild(rightChild), leftNeighbor(leftNeighbor), rightNeighbor(rightNeighbor)
	{
		if (leftNeighbor)
		{
			leftNeighbor->setRightNeighbor(this);
		}

		if (rightNeighbor)
		{
			rightNeighbor->setLeftNeighbor(this);
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
	
	RepairTreeNode* getRightNeighbor() const
	{
		return rightNeighbor;
	}

	void setRightNeighbor(RepairTreeNode* newRightNeighbor)
	{
		this->rightNeighbor = newRightNeighbor;
	}

	void setLeftNeighbor(RepairTreeNode* newLeftNeighbor)
	{
		this->leftNeighbor = newLeftNeighbor;
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