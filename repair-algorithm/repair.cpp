#include "Repair.h"
using namespace std;

void RepairAlgorithm::addOrUpdatePair(unsigned long long key, unsigned version, int idx)
{
	if (hashTable.count(key) > 0)
	{
		hashTable[key]->addOccurrence(version, idx);
	}
	else // First time we've seen this pair
	{
		// Create a heap entry with this key, and assert that we have one more entry than we did before
		int sizeBefore = myHeap.getSize();
		HeapEntry* entry = myHeap.insert(key);
		int sizeAfter = myHeap.getSize();
		assert(sizeAfter == sizeBefore + 1);

		// Create a hash table entry, and initialize it with its heap entry pointer
		// This creates the first occurrence (see the constructor)
		// cerr << "Version: " << version << ", idx: " << idx << endl;
		hashTable[key] = new HashTableEntry(entry, version, idx);
	}
}

void RepairAlgorithm::checkVersionAndIdx(unsigned v, int idx)
{
	assert(v >= 0 && v < this->versions.size());
	assert(idx >= 0 && idx < versions[v].size());
}

int RepairAlgorithm::scanLeft(unsigned v, int idx)
{
	checkVersionAndIdx(v, idx);
	while (idx > 0) {
		if (versions[v][--idx] != 0) return idx;
	}
	return -1;
}

int RepairAlgorithm::scanRight(unsigned v, int idx)
{
	checkVersionAndIdx(v, idx);
	while (idx < versions[v].size() - 1) {
		if (versions[v][++idx] != 0) return idx;
	}
	return -1;
}

unsigned long long RepairAlgorithm::getKeyAtIdx(unsigned v, int idx)
{
	checkVersionAndIdx(v, idx);
	if (versions[v][idx] == 0) {
		return 0;
	}
	unsigned rightIdx = scanRight(v, idx);
	if (rightIdx == -1)
		return 0;

	return combineToUInt64(versions[v][idx], versions[v][rightIdx]);
}

void RepairAlgorithm::extractPairs()
{
	for (size_t v = 0; v < versions.size(); v++)
	{
		unsigned long long currPair;

		// Go through the string and get all overlapping pairs, and process them
		for (size_t i = 0; i < versions[v].size() - 1; i++)
		{
			// Squeeze the pair of two unsigned numbers together for storage
			currPair = combineToUInt64((unsigned long long)versions[v][i], (unsigned long long)versions[v][i+1]);

			// Add the pair to our structures
			addOrUpdatePair(currPair, v, i);
		}
	}
	cerr << "Size: " <<  hashTable.size() << endl;
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
	// for (auto it = hashTable.begin(); it != hashTable.end(); it++)
	// {
	// 	unsigned long long k = it->first;
	// 	HashTableEntry* ht = it->second;
	// 	cerr << k << ": " << &ht << endl;
	// }
	// exit(1);
	// TODO update the condition since we're not actually deleting them any more
	while (!myHeap.empty())
	{
		unsigned symbol;
		
		// Get the max from the heap
		HeapEntry* hp = myHeap.getMax();

		assert(hp != NULL);

		if (hp->isDeleted()) continue;

		// The pair of ints represented as one 64 bit int
		unsigned long long key = hp->getKey();

		// Get the hash table entry (so all occurrences and so on)
		HashTableEntry* max = hashTable[key];
		assert(max != NULL);
		size_t numOccurrences = max->getSize();

		// TODO think about this number
		// Thought about it: it should be well below the number of versions
		// Imagine a fragment that occurs in numVersions - 2 of the versions. That's a good fragment, let's keep it. Maybe repairStoppingPoint := numVersions / 2
		if (numOccurrences < repairStoppingPoint)
			return;

		// Will use this as the new symbol (say we're replacing abcd with axd, this is x)
		symbol = nextWordID();

		unsigned totalCount = 0;

		// For all versions
		for (size_t v = 0; v < versions.size(); v++)
		{
			// For all locations of the pair in the current version
			if (!max->hasLocationsAtVersion(v)) {
				continue;
			}
			auto indexes = max->getLocationsAtVersion(v);
			for (auto it = indexes.begin(); it != indexes.end(); ++it)
			{
				int idx = *it;

				// EXAMPLE: idx = 3, so currKey = (5,6)
				// 0 1 2 3 4 5 6 7 8
				// 1 3 0 5 0 0 6 2 2

				// Find the key to the left of this one and remove that occurrence of it from our structures
				// 1 3 0 5 0 0 6 2 2
				// We want leftKey = (3,5)
				// leftIdx = scanLeft(v, idx) // should be 1
				// leftKey = getKeyAtIdx(leftIdx)
				// hashTable[leftKey]->removeOccurrence(v, leftIdx)

				int leftIdx = scanLeft(v, idx);
				if (leftIdx != -1) {
					unsigned long long leftKey = getKeyAtIdx(v, leftIdx);
					if (leftKey != 0) {
						assert(hashTable[leftKey] != NULL);
						hashTable[leftKey]->removeOccurrence(v, leftIdx);
					}
				}

				// Find the key to the right of this one and remove that occurrence of it from our structures
				// 1 3 0 5 0 0 6 2 2
				// We want leftKey = (6,2)
				// rightIdx = scanRight(v, idx) // should be 6
				// rightKey = getKeyAtIdx(rightIdx)
				// hashTable[rightKey]->removeOccurrence(v, rightIdx)

				int rightIdx = scanRight(v, idx); // should be 6
				if (rightIdx != -1) {
					unsigned long long rightKey = getKeyAtIdx(v, rightIdx);
					if (rightKey != 0) {
						assert(hashTable[rightKey] != NULL);
						hashTable[rightKey]->removeOccurrence(v, rightIdx);
					}
					// if (hashTable[rightKey] == NULL) {
					// 	cerr << rightIdx << endl;
					// 	cerr << rightKey << endl;
					// 	for (unsigned j = 0; j < versions[v].size(); j++)
					// 	{
					// 		cerr << versions[v][j] << " ";
					// 	}
					// 	cerr << endl;
					// }
				}

				// Store the association and which version it occurs in
				if (totalCount == 0)
					associations.push_back(Association(symbol, versions[v][idx], versions[v][rightIdx], numOccurrences, v));
				else
					associations.back().addVersion(v);


				// We have the current key, remove this occurrence of it from our structures
				// 1 3 0 5 0 0 6 2 2 key = (5,6) and idx = 3
				hashTable[key]->removeOccurrence(v, idx);


				// Now the replacement: 7 -> (5,6)
				// We modify the actual array of word Ids
				// 1 3 0 7 0 0 0 2 2
				versions[v][idx] = symbol;
				versions[v][rightIdx] = 0;

				// Now add the 2 new pairs
				// (3,7) at idx 1 and (7,2) at idx 3
				if (leftIdx != -1) {
					unsigned long long newLeftKey = getKeyAtIdx(v, leftIdx);
					if (newLeftKey != -1)
					{
						this->addOrUpdatePair(newLeftKey, v, leftIdx);
					}
				}
				if (rightIdx != -1) {
					unsigned long long newRightKey = getKeyAtIdx(v, idx);
					if (newRightKey != -1)
					{
						this->addOrUpdatePair(newRightKey, v, idx);
					}
				}

				totalCount++;
			}
		}
	}
}

// Release memory from all structures
void RepairAlgorithm::cleanup()
{
	for (auto it = hashTable.begin(); it != hashTable.end(); it++) {
		delete (*it).second;
	}

	for (size_t i = 0; i < versions.size(); ++i)
	{
		versions[i].clear();
	}
	versions.clear();

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
	while (associations[loc].getVersions().size() <= 0)
	{
		--loc;
		if (loc < 0)
		{
			return -1;
		}
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
			assert(versionNum < versions.size() && versionNum >= 0);
	
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
