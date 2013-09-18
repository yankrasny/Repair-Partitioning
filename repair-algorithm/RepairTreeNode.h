#ifndef REPAIR_TREE_NODE_H
#define REPAIR_TREE_NODE_H

#include "Util.h"

class RepairTreeNode
{
	unsigned symbol;

	unsigned offsetInFile;
	unsigned size;

	RepairTreeNode* leftChild;
	RepairTreeNode* rightChild;

	unsigned versionNum;

public:
	RepairTreeNode(unsigned symbol = 0) : symbol(symbol), offsetInFile(0), leftChild(NULL), rightChild(NULL), 
		versionNum(0), size(0) {}

	unsigned getVersionNum() const
	{
		return versionNum;
	}

	unsigned getSymbol() const
	{
		return symbol;
	}

	unsigned getOffset() const
	{
		return offsetInFile;
	}

	void setOffset(unsigned offset)
	{
		offsetInFile = offset;
	}

	unsigned getSize() const
	{
		return size;
	}

	void setSize(unsigned size)
	{
		this->size = size;
	}

	RepairTreeNode* getLeftChild() const
	{
		return leftChild;
	}
	
	RepairTreeNode* getRightChild() const
	{
		return rightChild;
	}

	void setRightChild(RepairTreeNode* newRightChild)
	{
		// newRightChild->setParent(this);
		this->rightChild = newRightChild;
	}

	void setLeftChild(RepairTreeNode* newLeftChild)
	{
		// newLeftChild->setParent(this);
		this->leftChild = newLeftChild;
	}
};

#endif