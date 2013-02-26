#include "RepairTreeNode.h"
#include "Occurrence.h"
#include <list>

class RepairTree
{
	RepairTreeNode* head(NULL);

	// TODO haven't decided on a data structure. std::list is a placeholder
	// If repair terminates early, then currentLevel represents the nodes of the document that have resulted from repair (I know that sounds obvious)  
	std::list<RepairTreeNode*> currentLevel;
public:
	RepairTree() {}

	// Again, std::list is a placeholder
	// Also, getCover can be getCut or something, just wanted to show that the result of this must not leave out any parts of the document
	std::list<RepairTreeNode*> getCover()
	{
		// TODO this is where the magic happens
		// The next step is to produce what Jinru calls a base partition
	}

	// TODO this is still pseudocode
	void addNode(Occurrence* oc)
	{
		if (done || !oc)
		{
			return;
		}

		unsigned left = oc->getLeft();
		unsigned right = oc->getRight();

		// Search the current level for left, right, get leftChild and rightChild

		// Once found, remove them from the current level
		currentLevel.remove(leftChild);
		currentLevel.remove(rightChild);

		// Create the new node as a parent of the two children
		RepairTreeNode newNode = new RepairTreeNode(leftChild, rightChild);		
		
		// And now add newNode to the current level
		currentLevel.add(newNode);

		if (currentLevel.size() == 1)
		{
			// We are done with repair, the only node left in the current level is the root
			head = currentLevel[0];
			done = true;
		}
	}
}