#include "Repair.h"
using namespace std;

int x(0);
void RepairAlgorithm::addOrUpdatePair(unsigned long long key, unsigned version)
{
	x++;
	cerr << x << endl;
	if (hashTable.count(key))
	{
		hashTable[key]->addOccurrence(new Occurrence(key, version));
	}
	else // First time we've seen this pair
	{
		int sizeBefore = myHeap.getSize();

		// Create a heap entry with this key
		HeapEntry* hp = myHeap.insert(key);

		int sizeAfter = myHeap.getSize();

		// Assertion
		if (sizeAfter != sizeBefore + 1) {
			throw 7;
		}		

		// Create a hash table entry, and initialize it with its heap entry pointer
		hashTable[key] = new HashTableEntry(hp, version); // This creates the first occurrence (see the constructor)
	}
}

void RepairAlgorithm::extractPairs()
{
	vector<unsigned> wordIDs;
	for (size_t v = 0; v < versions.size(); v++)
	{
		unsigned long long currPair;

		wordIDs = versions[v];

		// The previous entry in the HT (used to set preceding and succeeding pointers)
		Occurrence* prevOccurrence(NULL);

		// Go through the string and get all overlapping pairs, and process them
		for (size_t i = 0; i < wordIDs.size() - 1; i++)
		{
			// Squeeze the pair of two unsigned numbers together for storage
			currPair = combineToUInt64((unsigned long long)wordIDs[i], (unsigned long long)wordIDs[i+1]);

			// Add the pair to our structures
			addOrUpdatePair(currPair, v);

			// The first occurrence was the last one added because we add to the head
			Occurrence* lastAddedOccurrence = hashTable[currPair]->getHeadOccurrence();

			// Checks for existence of prev, and links them to each other
			doubleLinkNeighbors(prevOccurrence, lastAddedOccurrence);

			// Update the previous occurrence variable
			prevOccurrence = lastAddedOccurrence;
		}
	}
}

void RepairAlgorithm::removeFromHeap(HeapEntry* hp)
{
	if (hp && !myHeap.empty())
	{
		int sizeBefore = myHeap.getSize();
		myHeap.deleteAtIndex(hp->getIndex());
		int sizeAfter = myHeap.getSize();
		if (sizeAfter != sizeBefore - 1) {
			throw 8;
		}
	}
}

void RepairAlgorithm::removeOccurrence(Occurrence* oc)
{
	if (!oc)
	{
		return;
	}
	unsigned long long key = oc->getPair();
	if (hashTable.count(key))
	{
		HeapEntry* hp = hashTable[key]->getHeapEntryPointer();
		// Handles relinking the nodes around this one in the linked list
		hashTable[key]->removeOccurrence(oc);
		if (hashTable[key]->getSize() < 1)
		{
			removeFromHeap(hp);
			delete hashTable[key];
			hashTable.erase(key);
		}
	}
}

unsigned long long RepairAlgorithm::getNewRightKey(unsigned symbol, Occurrence* succ)
{
	if (!succ) return 0;
	unsigned symbolToTheRight = succ->getRight();
	return combineToUInt64(symbol, symbolToTheRight);
}

unsigned long long RepairAlgorithm::getNewLeftKey(unsigned symbol, Occurrence* prec)
{
	if (!prec) return 0;
	unsigned symbolToTheLeft = prec->getLeft();
	return combineToUInt64(symbolToTheLeft, symbol);
} 

/*
	While the heap is not empty, get the max and process it (that is, replace all occurrences and modify all prec and succ pointers)
	The max will keep getting removed, as well as the occurrences it touches
	Two new occurrences will be added (resulting from the replacement)
	So 3 occurrences are removed and 2 are added during each iteration
		Don't forget to link the new occurrences together

	Example of one iteration: abcd -> axd (replacing bc with symbol x)
		New occurrences to add:		ax, xd
		Old occurrences to remove:	ab, bc, cd

	We used to rely on the linked lists in the end to do the partitioning
	Now we're going to use associations
	-> Now we're going to build some trees inside here -yk, 2/24/13

*/
void RepairAlgorithm::doRepair(unsigned repairStoppingPoint)
{
	while (!myHeap.empty())
	{
		unsigned symbol;
		unsigned symbolToTheLeft;
		unsigned symbolToTheRight;
		
		// Get the max from the heap
		HeapEntry hp = myHeap.getMax();

		// The pair of ints represented as one 64 bit int
		unsigned long long key = hp.getKey();

		// Get the hash table entry (so all occurrences and so on)
		HashTableEntry* max = hashTable[key];
		size_t numOccurrences = max->getSize();

		// TODO think about this number
		// Thought about it: it should be well below the number of versions
		// Imagine a fragment that occurs in numVersions - 2 of the versions. That's a good fragment, let's keep it. Maybe repairStoppingPoint := numVersions / 2
		if (numOccurrences < repairStoppingPoint)
			return;

		Occurrence* curr;
		Occurrence* prec;
		Occurrence* succ;

		Occurrence* newLeftOcc(NULL);
		Occurrence* newRightOcc(NULL);

		// Will use this as the new symbol (say we're replacing abcd with axd, this is x)
		symbol = nextWordID();

		curr = max->getHeadOccurrence();

		// For all occurrences of this entry, do the replacement and modify the corresponding entries
		for (size_t i = 0; i < numOccurrences; i++)
		{
			newLeftOcc = NULL;
			newRightOcc = NULL;
			
			// Get the occurrence and its neighbors
			curr = max->getHeadOccurrence();

			// If curr is null, we have a problem. This should never happen.
			if (!curr)
				break;

			prec = curr->getPrec();
			succ = curr->getSucc();

			// Store the association and which version it occurs in
			if (i == 0)
				associations.push_back(Association(symbol, curr->getLeft(), curr->getRight(), numOccurrences, curr->getVersion()));
			else
				associations.back().addVersion(curr->getVersion());

			
			// Now go through all the edge cases (because of the links we have to make, there are a lot)
			bool onLeftEdge(false);
			bool onRightEdge(false);
			bool nearLeftEdge(false);
			bool nearRightEdge(false);

			unsigned long long newLeftKey;
			unsigned long long newRightKey;

			// Use these bools instead of following the pointers repeatedly
			if (!prec)
				onLeftEdge = true;
			else
				if (!prec->getPrec())
					nearLeftEdge = true;
			
			if (!succ)
				onRightEdge = true;
			else
				if (!succ->getSucc())
					nearRightEdge = true;

			newLeftKey = getNewLeftKey(symbol, prec);
			newRightKey = getNewRightKey(symbol, succ);

			// Just creates the occurrence in the hash table and heap, doesn't link it to its neighbors
			if (!onLeftEdge)
				addOrUpdatePair(newLeftKey, curr->getVersion());
			
			if (!onRightEdge)
				addOrUpdatePair(newRightKey, curr->getVersion());

			if (!nearLeftEdge && !onLeftEdge)
			{
				// Have 2 neighbors to the left
				newLeftOcc = hashTable[newLeftKey]->getHeadOccurrence();
				doubleLinkNeighbors(prec->getPrec(), newLeftOcc);
			}
			if (!nearRightEdge && !onRightEdge)
			{
				// Have 2 neighbors to the right
				newRightOcc = hashTable[newRightKey]->getHeadOccurrence();
				doubleLinkNeighbors(newRightOcc, succ->getSucc());
			}
			if (!onRightEdge && !onLeftEdge)
			{
				// A neighbor on each side, link them
				newLeftOcc = hashTable[newLeftKey]->getHeadOccurrence();
				newRightOcc = hashTable[newRightKey]->getHeadOccurrence();
				doubleLinkNeighbors(newLeftOcc, newRightOcc);
			}

			//cerr << "Removing curr: " << curr->getLeft() << "," << curr->getRight() << endl;
			removeOccurrence(curr);
			
			if (!onRightEdge)
			{
				//cerr << "Removing succ: " << succ->getLeft() << "," << succ->getRight() << endl;
				removeOccurrence(succ);
			}
			if (!onLeftEdge)
			{
				//cerr << "Removing prec: " << prec->getLeft() << "," << prec->getRight() << endl;
				removeOccurrence(prec);
			}
		}
	}
}

// New way: realize that this was bullshit to begin with. What about all the nodes in the repair trees, how about clearing those?
// So still need this, but obviously we didn't clean up enough
void RepairAlgorithm::cleanup()
{
	for (unordered_map<unsigned long long, HashTableEntry*>::iterator it = hashTable.begin(); it != hashTable.end(); it++)
	{
		delete it->second;
		it->second = NULL;
	}
	this->associations.clear();
	resetcurrentWordID();
	resetFragID();
}

/*************************************************************************************************/
/*
New Tree Building Code: takes the resulting vector<Association> from repair 
and builds a tree for each version
*/
/*************************************************************************************************/

RepairTreeNode* RepairAlgorithm::buildTree(int loc, unsigned versionNum)
{
	// Allocate the current node and set its symbol
	RepairTreeNode* root = new RepairTreeNode(associations[loc].getSymbol());

	// Keep track of which versions we've processed in order to choose a root (see getNextRootLoc)
	associations[loc].removeFromVersions(versionNum);
	
	unsigned left = associations[loc].getLeft();
	unsigned right = associations[loc].getRight();

	int lLoc = binarySearch(left, associations, 0, loc);
	int rLoc = binarySearch(right, associations, 0, loc);

	if (lLoc == -1) root->setLeftChild(new RepairTreeNode(left));
	else root->setLeftChild(buildTree(lLoc, versionNum));

	if (rLoc == -1) root->setRightChild(new RepairTreeNode(right));
	else root->setRightChild(buildTree(rLoc, versionNum));

	return root;
}

int RepairAlgorithm::getNextRootLoc(int loc)
{
	multiset<unsigned> whichVersions = associations[loc].getVersions();
	while (whichVersions.size() <= 0)
	{
		--loc;
		if (loc < 0)
		{
			return -1;
		}
		whichVersions = associations[loc].getVersions();
	}
	return loc;
}

unsigned* RepairAlgorithm::getVersionPartitionSizes() {
	return this->versionPartitionSizes;
}

unsigned* RepairAlgorithm::getOffsetsAllVersions()
{
	int loc = associations.size() - 1;
	RepairTreeNode* currRoot = NULL;
	int versionNum = 0;

	SortedPartitionsByVersionNum offsetMap = SortedPartitionsByVersionNum();
	PartitionList theList;

	RepairDocumentPartition partitionAlg = RepairDocumentPartition(this->associations, this->versions.size(),
		this->numLevelsDown, this->minFragSize, this->fragmentationCoefficient);

	vector<unsigned> bounds;
	while (true)
	{
		loc = getNextRootLoc(loc);
		if (loc == -1) break;

		while (true)
		{
			// TODO verify that this is the lowest version number in the list
			// Wait, why? I don't remember the logic...
			// Ok, versionNum is one of the versions in which this association occurred
			// We don't go in order of versionNum, which is worrying because we use it as an array index below
			// It is the lowest because we return begin() and the set is sorted by symbol
			// BULLSHIT: the lowest for that location, but not the lowest overall, IDIOT!
			versionNum = associations[loc].getVersionAtBegin();
			if (versionNum == -1) break;

			// Assert that versionNum is valid
			if (versionNum > versions.size() - 1) {
				throw 5;
			}
	
			currRoot = buildTree(loc, versionNum);

			// Let's see if this tree is reasonable
			// int countNodes = currRoot->getCountNodes();

			this->calcOffsets(currRoot);
			resetOffset();

			theList = PartitionList(versionNum);
			bounds = vector<unsigned>();

			partitionAlg.getPartitioningOneVersion(currRoot, this->numLevelsDown, bounds, this->minFragSize, versions[versionNum].size());

			for (size_t i = 0; i < bounds.size(); i++) {
				theList.push(bounds[i]);
			}

			// Now theList should be populated, just insert it into the sorted set
			offsetMap.insert(theList);

			// TODO Do we need to do anything with theList?

			// Deallocate all those tree nodes
			this->deleteTree(currRoot);
		}
		--loc;
	}

	unsigned totalOffsetInArray = 0;
	unsigned numVersions = 0;

	// Post processing to get offsetMap into offsets and versionPartitionSizes
	for (SortedPartitionsByVersionNum::iterator it = offsetMap.begin(); it != offsetMap.end(); it++)
	{
		// Reusing the same var from above, should be ok
		theList = *it;
		for (size_t j = 0; j < theList.size(); j++)
		{
			this->offsets[totalOffsetInArray + j] = theList.get(j);
		}
		this->versionPartitionSizes[numVersions++] = theList.size();
		totalOffsetInArray += theList.size();
	}

	// Clean up offsetMap
	offsetMap.clear();

	return this->offsets;
}

// Not tested, well sorta tested
void RepairAlgorithm::deleteTree(RepairTreeNode* node) {
	RepairTreeNode* leftChild = node->getLeftChild();
	RepairTreeNode* rightChild = node->getRightChild();
	if (leftChild) {
		deleteTree(leftChild);
	}
	if (rightChild) {
		deleteTree(rightChild);
	}
	delete node;
	node = NULL;
}

unsigned RepairAlgorithm::calcOffsets(RepairTreeNode* node)
{
	if (!node) return 0;

	// node is not a terminal
	if (node->getLeftChild())
	{
		RepairTreeNode* leftChild = node->getLeftChild();
		RepairTreeNode* rightChild = node->getRightChild();
		
		// The left ones must be set first, because the right ones depend on them
		unsigned leftOffset = calcOffsets(leftChild);
		calcOffsets(rightChild);
		
		node->setOffset(leftOffset);
		
		// By calling the recursive function first, we guarantee that these sizes are set
		unsigned leftSize = leftChild->getSize();
		unsigned rightSize = rightChild->getSize();
		node->setSize(leftSize + rightSize);
		
		return leftOffset;
	}

	// node is a terminal, so its size is 1 by definition
	node->setSize(1);

	// We're going in pre-order (right, that's what it's called?)
	unsigned offset = nextOffset();
	node->setOffset(offset);
	return offset;
}