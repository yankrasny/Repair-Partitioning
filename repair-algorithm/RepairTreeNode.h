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

	int getDepth() {
		int leftDepth = 0;
		if (this->getLeftChild()) {
			RepairTreeNode* leftChild = this->getLeftChild();
			leftDepth = leftChild->getDepth();	
		}

		int rightDepth = 0;
		if (this->getRightChild()) {
			RepairTreeNode* rightChild = this->getRightChild();
			rightDepth = rightChild->getDepth();	
		}

		return std::max(leftDepth, rightDepth) + 1;
	}

	/*
		Includes the current node
	*/
	int getCountNodes() {
		if (!this->getLeftChild()) {
			return 1;
		}
		RepairTreeNode* leftChild = this->getLeftChild();
		RepairTreeNode* rightChild = this->getRightChild();

		return leftChild->getCountNodes() + rightChild->getCountNodes() + 1;
	}
};

#endif