#include "RepairTreeNode.h"
#include <list>

class RepairTree
{
	RepairTreeNode* head(NULL);
public:
	RepairTree(RepairTreeNode* leftBranch, RepairTreeNode* rightBranch)
	{
		head = new RepairTreeNode(leftBranch, rightBranch);
	}

	std::list<RepairTreeNode*> getCover()
	{
		// TODO this is where the magic happens
	}
}