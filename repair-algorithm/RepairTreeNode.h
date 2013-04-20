#ifndef REPAIR_TREE_NODE_H
#define REPAIR_TREE_NODE_H

#include "Util.h"

class RepairTreeNode
{
	unsigned symbol;

	unsigned offsetInFile;

	RepairTreeNode* leftChild;
	RepairTreeNode* rightChild;

	RepairTreeNode* leftNeighbor;
	RepairTreeNode* rightNeighbor;

	RepairTreeNode* parent;

	unsigned versionNum;

public:
	RepairTreeNode() : symbol(0), offsetInFile(0), leftChild(NULL), rightChild(NULL), 
		leftNeighbor(NULL), rightNeighbor(NULL), parent(NULL), versionNum(0) {}

	RepairTreeNode(unsigned symbol) : symbol(symbol), offsetInFile(0), leftChild(NULL), 
		rightChild(NULL), leftNeighbor(NULL), rightNeighbor(NULL), parent(NULL), versionNum(0)
	{
		// TODO offset in file, versionNum, parent
	}

	RepairTreeNode(unsigned symbol, unsigned offsetInFile, RepairTreeNode* leftChild, 
		RepairTreeNode* rightChild, RepairTreeNode* leftNeighbor, RepairTreeNode* rightNeighbor, unsigned versionNum = 0) :
			symbol(symbol), offsetInFile(offsetInFile), leftChild(leftChild), rightChild(rightChild), 
			leftNeighbor(leftNeighbor), rightNeighbor(rightNeighbor), parent(NULL), versionNum(versionNum)
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

			// Inherit the version number from children, so we can know which version a node belongs to
			versionNum = leftChild->versionNum;
		}
	}

	unsigned getVersionNum() const
	{
		return versionNum;
	}

	RepairTreeNode* getParent() const
	{
		return this->parent;
	}

	bool hasParent() const
	{
		return parent != NULL;
	}

	void setParent(RepairTreeNode* parent)
	{
		this->parent = parent;
	}

	unsigned getSymbol() const
	{
		return symbol;
	}

	unsigned getOffsetInFile() const
	{
		return offsetInFile;
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

	void setRightChild(RepairTreeNode* newRightChild)
	{
		this->rightChild = newRightChild;
	}

	void setLeftChild(RepairTreeNode* newLeftChild)
	{
		this->leftChild = newLeftChild;
	}

};

#endif