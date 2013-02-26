class RepairTreeNode
{
	int* expansion(NULL);
	int leftBound;
	int rightBound;

	RepairTreeNode* leftChild(NULL);
	RepairTreeNode* rightChild(NULL);

public:
	RepairTreeNode(RepairTreeNode* leftChild, RepairTreeNode* rightChild) : leftChild(leftChild), rightChild(rightChild)
	{
		// TODO this is fake
		expansion = leftChild->getExpansion() + rightChild->getExpansion();
	}

	int* getExpansion()
	{
		// TODO
	}
}