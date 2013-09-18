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

	// RepairTreeNode* leftNeighbor;
	// RepairTreeNode* rightNeighbor;

	// RepairTreeNode* parent;

	unsigned versionNum;

public:
	RepairTreeNode(unsigned symbol = 0) : symbol(symbol), offsetInFile(0), leftChild(NULL), rightChild(NULL), 
		versionNum(0), size(0) {}

	// RepairTreeNode() : symbol(0), offsetInFile(0), leftChild(NULL), rightChild(NULL), 
	// 	leftNeighbor(NULL), rightNeighbor(NULL), parent(NULL), versionNum(0), size(0) {}

	// RepairTreeNode(unsigned symbol) : symbol(symbol), offsetInFile(0), leftChild(NULL), 
	// 	rightChild(NULL), leftNeighbor(NULL), rightNeighbor(NULL), parent(NULL), versionNum(0), size(0)
	// {
	// 	// TODO offset in file, versionNum, parent
	// }

	// RepairTreeNode(unsigned symbol, unsigned offsetInFile, RepairTreeNode* leftChild, 
	// 	RepairTreeNode* rightChild, RepairTreeNode* leftNeighbor, RepairTreeNode* rightNeighbor, unsigned versionNum = 0) :
	// 		symbol(symbol), offsetInFile(offsetInFile), leftChild(leftChild), rightChild(rightChild), 
	// 		leftNeighbor(leftNeighbor), rightNeighbor(rightNeighbor), parent(NULL), versionNum(versionNum), size(0)
	// {
	// 	if (leftNeighbor)
	// 	{
	// 		leftNeighbor->setRightNeighbor(this);
	// 	}

	// 	if (rightNeighbor)
	// 	{
	// 		rightNeighbor->setLeftNeighbor(this);
	// 	}

	// 	if (rightChild)
	// 	{
	// 		rightChild->setParent(this);
	// 	}

	// 	if (leftChild)
	// 	{
	// 		leftChild->setParent(this);

	// 		// Inherit the version number from children, so we can know which version a node belongs to
	// 		versionNum = leftChild->versionNum;
	// 	}
	// }

	unsigned getVersionNum() const
	{
		return versionNum;
	}

	// RepairTreeNode* getParent() const
	// {
	// 	return this->parent;
	// }

	// bool hasParent() const
	// {
	// 	return parent != NULL;
	// }

	// void setParent(RepairTreeNode* parent)
	// {
	// 	this->parent = parent;
	// }

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

	// RepairTreeNode* getLeftNeighbor() const
	// {
	// 	if (!parent) return NULL;
	// 	return parent->getLeftChild();
	// }
	
	// RepairTreeNode* getRightNeighbor() const
	// {
	// 	return rightNeighbor;
	// }

	// void setRightNeighbor(RepairTreeNode* newRightNeighbor)
	// {
	// 	this->rightNeighbor = newRightNeighbor;
	// }

	// void setLeftNeighbor(RepairTreeNode* newLeftNeighbor)
	// {
	// 	this->leftNeighbor = newLeftNeighbor;
	// }

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

	// bool isLeftChild() const
	// {
	// 	if (!parent) return false;

	// 	// if my parent's left child is me, then I am a left child
	// 	if (parent->getLeftChild() == this)
	// 	{
	// 		return true;
	// 	}
	// 	return false;
	// }
};

#endif