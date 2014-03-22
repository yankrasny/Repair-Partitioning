#include "Repair.h"
#include <map>
using namespace std;

/*
If the pair exists, add this occurrence of it
If not, create a new entry in the heap and hash table, and add this occurrence of it
*/
void RepairAlgorithm::addOccurrence(unsigned long long key, unsigned version, int idx)
{
    // cerr << "addOccurrence(" << getKeyAsString(key) << ", " << version << ", " << idx << ")" << endl;
    if (hashTable.count(key) > 0) // We've seen this pair before
    {
        hashTable[key]->addOccurrence(version, idx);
    }
    else // First time we've seen this pair
    {
        HeapEntry* entry = myHeap.insert(key);

        // Create a hash table entry, and initialize it with its heap entry pointer
        // This creates the first occurrence (see the constructor)
        hashTable[key] = new HashTableEntry(entry, version, idx);
    }
}

/*
Remove an occurrence of a pair from the hash table
If there are no more occurrences of this pair left, release and erase the entries
in both structures. 
    EDGE CASE: Recall that each hash table entry points to a heap entry
    When deleting a heap entry, we swap with the last heap entry to achive O(1) deletion
    This messes up the hash table entry's pointer to that last heap entry
    But, the deleteAtIdx function returns the new location of the previously last heap entry
    See the section that uses indexOfEntryThatGotSwapped for the solution
*/
bool RepairAlgorithm::removeOccurrence(unsigned long long key, unsigned v, int idx)
{
    // cerr << "removeOccurrence(" << getKeyAsString(key) << ", " << v << ", " << idx << ")" << endl;

    // Assertions
    checkVersionAndIdx(v, idx);
    assert(!myHeap.empty());
    assert(hashTable.count(key) > 0);
    assert(hashTable[key] != NULL);

    // Remove this occurrence at this key
    hashTable[key]->removeOccurrence(v, idx);
    
    // If we've removed all the occurrences at this key, remove the heap entry as well
    if (hashTable[key]->getSize() < 1) {
        int idxInHeap = hashTable[key]->getHeapEntryPointer()->getIndex();

        // We can already set our heap entry pointer to null because we'll be deleting by idxInHeap
        hashTable[key]->setHeapEntryPointer(NULL);

        // To understand this, look at the implementation of myHeap.deleteAtIdx(idxInHeap)
        // If indexOfEntryThatGotSwapped is -1, that means there was no swap
        int indexOfEntryThatGotSwapped = myHeap.deleteAtIndex(idxInHeap);
        if (indexOfEntryThatGotSwapped != -1) {
            unsigned long long keyOfEntryThatGotSwapped = myHeap.getAtIndex(indexOfEntryThatGotSwapped)->getKey();

            // When we delete from the heap, we use a swap with the last element
            // Well, hashTable[keyOfLastElement] was pointing to it, so that's not very nice
            // Our delete function returns to us the new location of the swapped last element specifically so we can use it here like so
            assert(hashTable.count(keyOfEntryThatGotSwapped) > 0);
            assert(hashTable[keyOfEntryThatGotSwapped] != NULL);
            hashTable[keyOfEntryThatGotSwapped]->setHeapEntryPointer(myHeap.getAtIndex(indexOfEntryThatGotSwapped));
        }

        // hashTable.erase only decrements the size of the hash table. It doesn't delete the entry.
        // The value in this table is a pointer.
        delete hashTable[key];
        hashTable.erase(key);

        return true;
    }
    return false;
}

/*
Assert that the version and idx are within logical boundaries
*/
void RepairAlgorithm::checkVersionAndIdx(unsigned v, int idx)
{
    if (!(v >= 0 && v < this->versions.size()))
    {
        cerr << "v is out of bounds...";
        cerr << "v:" << v << " and idx: " << idx << endl;
    }
    if (!(idx >= 0 && idx < versions[v].size()))
    {
        cerr << "idx is out of bounds... ";
        cerr << "v:" << v << " and idx: " << idx << endl;
    }
    assert(v >= 0 && v < this->versions.size());
    assert(idx >= 0 && idx < versions[v].size());
}

/*
Scan left from this idx and return the next valid idx
-1 is used to indicate that there are no valid idxs in this direction
*/
int RepairAlgorithm::scanLeft(unsigned v, int idx)
{
    checkVersionAndIdx(v, idx);
    while (idx > 0) {
        if (versions[v][--idx] != 0) return idx;
    }
    return -1;
}

/*
Scan right from this idx and return the next valid idx
-1 is used to indicate that there are no valid idxs in this direction
*/
int RepairAlgorithm::scanRight(unsigned v, int idx)
{
    checkVersionAndIdx(v, idx);
    while (idx < versions[v].size() - 1) {
        if (versions[v][++idx] != 0) return idx;
    }
    return -1;
}

/*
Get the pair of unsigned at this idx if it's valid, and the next valid idx to the right
*/
unsigned long long RepairAlgorithm::getKeyAtIdx(unsigned v, int idx)
{
    checkVersionAndIdx(v, idx);
    if (versions[v][idx] == 0) {
        return 0;
    }
    int rightIdx = scanRight(v, idx);
    if (rightIdx == -1) {
        return 0;
    }

    if (versions[v][idx] == 0 || versions[v][rightIdx] == 0) {
        return 0;
    }

    // Squeeze the pair of two unsigned numbers together for storage
    return combineToUInt64(versions[v][idx], versions[v][rightIdx]);
}

/*
Run through all positions of all versions
Add each pair to the structures
*/
void RepairAlgorithm::extractPairs()
{
    unsigned long long currPair;
    for (size_t v = 0; v < versions.size(); v++)
    {
        if (versions[v].size() < 1)
        {
            continue;
        }
        // Go through the string and get all overlapping pairs, and process them
        for (size_t i = 0; i < versions[v].size() - 1; i++)
        {
            currPair = getKeyAtIdx(v, i);

            // 0 is returned if either the left or right element is 0
            if (currPair == 0)
                continue;

            // Add this occurrence of the pair to our structures
            this->addOccurrence(currPair, v, i);
        }
    }


    // auto histogram = map<unsigned, unsigned>();
    // // auto sizes = vector<size_t>();
    // for (auto it = hashTable.begin(); it != hashTable.end(); it++)
    // {
    //     auto locsInDoc = (*it).second->getLocationsInDoc();
    //     for (auto it2 = locsInDoc.begin(); it2 != locsInDoc.end(); it2++)
    //     {
    //         auto locsInVersion = (*it2).second;
    //         unsigned theSize = locsInVersion.size();
    //         if (histogram.count(theSize) > 0) {
    //             histogram[theSize]++;
    //         } else {
    //             histogram[theSize] = 0;
    //         }
    //         // sizes.push_back(locsInVersion.size());
    //     }
    // }

    // ofstream os("histogram.txt");

    // // sort(histogram.begin(), histogram.end());
    // for (auto it = histogram.begin(); it != histogram.end(); it++) {
    //     unsigned size = (*it).first;
    //     unsigned numTimes = (*it).second;
    //     os << "size: " << size << ", numTimes: " << numTimes << endl;
    // }
    // os.close();
    // exit(0);

    // int sum_of_elems = 0;
    // for (int n : sizes) 
    //     sum_of_elems += n;  

    // cerr << "sum(sizes) = " << sum_of_elems << endl;

    // sort(sizes.begin(), sizes.end(), std::greater<int>());

    // for (size_t i = 0; i < sizes.size(); i++)
    // {
    //     if (i % 100 == 0) {
    //         system("pause");
    //     }
    //     cerr << sizes[i] << endl;
    // }


    // int sizes = 0;
    // for (int i = 0; i < versions.size(); i++)
    // {
    //     sizes += versions[i].size();
    // }
    // cerr << "# distinct pairs: " << hashTable.size() << endl;
    // cerr << "total # words: " << sizes << endl;
    // exit(1);
}

/*
    While the heap is not empty, get the max and process it (that is, replace all occurrences and modify all prec and succ pointers)
    The max will keep getting removed, as well as the occurrences it touches
    Two new occurrences will be added (resulting from the replacement)
    So 3 occurrences are removed and 2 are added during each iteration
        Don't forget to link the new occurrences together

    Example of one iteration: abcd -> axd (replacing bc with symbol x)
        New occurrences to add:     ax, xd
        Old occurrences to remove:  ab, bc, cd

    We used to rely on the linked lists in the end to do the partitioning
    Now we're going to use associations
    -> Now we're going to build some trees inside here -yk, 2/24/13

*/
void RepairAlgorithm::doReplacements(unsigned repairStoppingPoint)
{
    unsigned symbol;
    unsigned long long key;
    HeapEntry* hp;
    HashTableEntry* max;

    while (!myHeap.empty())
    {
        // Get the max from the heap
        hp = myHeap.getMax();
        assert(hp != NULL);

        // The key is a pair of unsigned ints represented as one 64 bit unsigned int
        // This allows us to pull the max from the heap, and then
        // get the corresponding hash table entry in constant time
        key = hp->getKey();
        assert(hashTable.count(key));

        max = hashTable[key];
        assert(max != NULL);

        size_t numOccurrences = max->getSize();

        // TODO Consider using this at some point
        // if (numOccurrences < repairStoppingPoint)
        //     return;

        // Will use this as the new symbol (say we're replacing 1 2 3 4 with 1 5 4, this is 5)
        symbol = nextWordID();

        // EDGE CASE: given runs of the same symbol in a row, we have consecutive equivalent occurrences of the same pair
        // When we do the removes and adds for one of them, the other one becomes invalid
        // Without checking for this case, we proceed to do the removes and adds for the now invalid pair, causing a runtime error
        // So, if there are two adjacent indexes (abs(idx - prevIdx) < 2) then we just skip replacement for that occurrence, as it is invalid
        int prevIdx;

        // Don't continue to loop over all versions if we're done with this pair
        bool maxDeleted = false;

        // For all versions
        for (size_t v = 0; v < versions.size(); v++)
        {
            if (maxDeleted) {
                break;
            }
            if (!max->hasLocationsAtVersion(v)) {
                continue;
            }

            // cerr << endl << "Replacement (" << numOccurrences << "): [" << symbol << " -> " << getKeyAsString(key) << "]" << endl;
            // printVector(v);

            // First call remove on all identical overlapping pairs, and note their indexes
            prevIdx = -1;
            auto indexes = max->getLocationsAtVersion(v);
            auto removed = set<int>();
            bool justRemoved = false;
            for (auto it = indexes.begin(); it != indexes.end(); ++it)
            {
                int idx = *it;
                if (prevIdx >= 0) { // prevIdx is -1 for the first idx
                    if (scanRight(v, prevIdx) == idx) { // check that idx and prevIdx are consecutive
                        if (!justRemoved) { // remove every second occurrence in a line of the same occurrences
                            maxDeleted = removeOccurrence(key, v, idx);
                            removed.insert(idx);
                            justRemoved = true; // justRemoved must only be true in this case, so we have to have those elses where it's false
                        } else { // this one
                            justRemoved = false;
                        }
                    } else { // and this one
                        justRemoved = false;
                    }
                }
                prevIdx = idx;
            }

            // Note that indexes no longer contains the deleted ones,
            // however leftIdx and rightIdx in the algorithm below still can
            // That is why we need to keep track of them using removed
            indexes = max->getLocationsAtVersion(v);

            // For all locations of the pair in the current version
            for (auto it = indexes.begin(); it != indexes.end(); ++it)
            {
                int idx = *it;

                // printSection(v, idx, 6);

                // Find the key to the left of this one and remove that occurrence of it from our structures
                int leftIdx = scanLeft(v, idx);
                if (removed.count(leftIdx) < 1) {
                    if (leftIdx != -1) {
                        unsigned long long leftKey = getKeyAtIdx(v, leftIdx);
                        if (leftKey != 0) {
                            assert(hashTable.count(leftKey));
                            assert(hashTable[leftKey] != NULL);
                            maxDeleted = removeOccurrence(leftKey, v, leftIdx);
                        }
                    }
                }
                // Find the key to the right of this one and remove that occurrence of it from our structures
                int rightIdx = scanRight(v, idx);
                if (removed.count(rightIdx) < 1) {
                    if (rightIdx != -1) {
                        unsigned long long rightKey = getKeyAtIdx(v, rightIdx);
                        if (rightKey != 0) {
                            assert(hashTable.count(rightKey));
                            assert(hashTable[rightKey] != NULL);
                            maxDeleted = removeOccurrence(rightKey, v, rightIdx);
                        }
                    }
                }
                // We have the current key, remove this occurrence of it from our structures
                if (key != 0) {
                    assert(hashTable.count(key));
                    assert(hashTable[key] != NULL);
                    maxDeleted = removeOccurrence(key, v, idx);
                }

                // Store the association and which version it occurs in
                if (associations.count(symbol) < 1)
                {
                    associations[symbol] = Association(symbol, versions[v][idx], versions[v][rightIdx], numOccurrences, v);
                }
                else
                {
                    associations[symbol].addVersion(v);
                }

                // Now the replacement: we modify the actual array of word Ids
                versions[v][idx] = symbol;
                if (rightIdx != -1) {
                    versions[v][rightIdx] = 0;
                }

                // Now add the 2 new pairs
                if (leftIdx != -1) {
                    unsigned long long newLeftKey = getKeyAtIdx(v, leftIdx);
                    if (newLeftKey != 0)
                    {
                        this->addOccurrence(newLeftKey, v, leftIdx);
                    }
                }
                if (rightIdx != -1) {
                    unsigned long long newRightKey = getKeyAtIdx(v, idx);
                    if (newRightKey != 0)
                    {
                        this->addOccurrence(newRightKey, v, idx);
                    }
                }
            }
        }
    }
    // cerr << "# associations: " << associations.size() << endl;
    // exit(1);
}

/*
Print the whole version v (useful for small examples)
*/
void RepairAlgorithm::printVector(unsigned v)
{
    // Print the current vector in one line
    cerr << "Version " << v << ": ";
    for (unsigned i = 0; i < versions[v].size(); i++) {
        cerr << versions[v][i] << " ";
    }
    cerr << endl;
}

/*
Print a section of the specified version
*/
void RepairAlgorithm::printSection(unsigned v, unsigned idx, unsigned range)
{
    checkVersionAndIdx(v, idx); 
    if (!(range > 0 && range < versions[v].size()))
    {
        return;
    }
    cerr << endl;
    for (size_t i = idx - range / 2; i <= idx + range / 2; i++)
    {
        if (i < 0 || i > versions[v].size() - 1)
        {
            continue;
        }
        if (i == idx)
            cerr << i << ": " << "[" << versions[v][i] << "]" << endl;  
        else
            cerr << i << ": " << versions[v][i] << endl;
        
    }
    cerr << endl;
}

/*
Release memory from core repair structures
*/
void RepairAlgorithm::clearRepairStructures()
{
    for (auto it = hashTable.begin(); it != hashTable.end(); it++) {
        delete (*it).second;
    }
    hashTable.clear();
    assert(hashTable.empty());

    while (!myHeap.empty())
    {
        myHeap.deleteAtIndex(myHeap.getSize() - 1);
    }
    assert(myHeap.empty());
}

/*
Clear associations and reset the global Ids
*/
void RepairAlgorithm::clearAssociationsAndReset()
{
    for (size_t i = 0; i < versions.size(); ++i)
    {
        versions[i].clear();
    }
    versions.clear();
    assert(versions.empty());

    this->associations.clear();
    resetCurrentWordID();
    resetFragID();
}

/*************************************************************************************************/
/*
Algorithm Step 2: Build and cut trees
    Takes the resulting vector<Association> from repair 
    and builds a tree for each version
*/
/*************************************************************************************************/

/*
Recursively build a tree from associations
Expand an association to its left and right symbols
Each symbol is a key to another association, there's your recursion
*/
RepairTreeNode* RepairAlgorithm::buildTree(unsigned symbol, unsigned versionNum)
{
    // Allocate the current node and set its symbol
    RepairTreeNode* root = new RepairTreeNode(symbol);

    // Keep track of which versions we've processed in order to choose a root (see getNextRootLoc)
    associations[symbol].removeFromVersions(versionNum);
    
    unsigned left = associations[symbol].getLeft();
    unsigned right = associations[symbol].getRight();

    if (associations.count(left) < 1)
        root->setLeftChild(new RepairTreeNode(left));
    else
        root->setLeftChild(buildTree(left, versionNum));

    if (associations.count(right) < 1)
        root->setRightChild(new RepairTreeNode(right));
    else
        root->setRightChild(buildTree(right, versionNum));

    return root;
}

/*
Release the memory used by one tree starting at the root
*/
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

/*
Part of our top down tree building algorithm
If this symbol has some versions left then it can be a root node
*/
int RepairAlgorithm::getNextRootSymbol(unsigned symbol)
{
    while (associations[symbol].getVersions().size() <= 0)
    {
        if (--symbol < 1)
        {
            return -1;
        }
    }
    return symbol;
}

/*
Starting at the last association created, build trees for each version top down
Once you build one, get the partitioning for it and store it in the output
Immediately delete the tree
Organize the output as needed by the consumers of this class

TODO fix: this function is pretty long, we can break it up
*/
void RepairAlgorithm::getOffsetsAllVersions(unsigned* offsetsAllVersions, unsigned* versionPartitionSizes)
{
    int symbol = currWordID();
    RepairTreeNode* currRoot = NULL;
    int versionNum = 0;

    // A set of partition lists, sorted by version number (see Repair.h for typedef)
    SortedPartitionsByVersion offsetMap = SortedPartitionsByVersion();
    PartitionList theList;

    RepairDocumentPartition partitionAlg = RepairDocumentPartition(this->associations, this->versions.size(),
        this->numLevelsDown, this->minFragSize, this->fragmentationCoefficient);

    vector<unsigned> bounds;
    while (true)
    {
        symbol = getNextRootSymbol(symbol);
        if (symbol == -1) break;

        while (true)
        {
            versionNum = associations[symbol].getVersionAtBegin();
            if (versionNum == -1) break;

            assert(versionNum < versions.size() && versionNum >= 0);
            // cerr << "Build tree: v" << versionNum << endl;
    
            // Allocates a lot of memory, see delete at the end of this function
            currRoot = buildTree(symbol, versionNum);

            // Let's see if this tree is reasonable
            // int countNodes = currRoot->getCountNodes();

            this->calcOffsets(currRoot);
            resetOffset();

            theList = PartitionList(versionNum);
            bounds = vector<unsigned>();

            // TODO write some loops to run partitioning with different param values
            // Pseudocode written, make it happen for real
            /*
            for (fragCoefficientArray : fragCoeff) {
                for (numLevelsDownArray : numLevelsDown) {
                    for (minFragSizeArray : minFragSize) {
                        partitionAlg.getPartitioningOneVersion(currRoot, bounds, versions[versionNum].size(), params...);                        
                    }
                    // Now score the partitioning
                    // The best one wins and gets used below
                }
            }
            */
            
            partitionAlg.getPartitioningOneVersion(currRoot, bounds, versions[versionNum].size());

            // A partitioning is defined as at least one fragment
            // So at least 2 fragment boundaries
            assert(bounds.size() > 1);

            for (size_t i = 0; i < bounds.size(); i++)
            {
                theList.push(bounds[i]);
            }

            // Now theList should be populated, just insert it into the sorted set
            offsetMap.insert(theList);

            // Deallocate all those tree nodes
            this->deleteTree(currRoot);
        }
        --symbol;
    }

    unsigned totalOffsetInArray = 0;
    unsigned numVersions = 0;

    // offsetMap have not been populated with all partitionings
    // some versions might be of size 0, so the algorithm might just do nothing for some of them
    if (versions.size() != offsetMap.size())
    {
        // cerr << "offsetMap.size(): " << offsetMap.size() << endl;
        set<unsigned> versionNums = set<unsigned>();
        for (auto it = offsetMap.begin(); it != offsetMap.end(); it++)
        {
            // Reusing the same var from above, should be ok
            theList = *it;
            unsigned versionNum = theList.getVersionNum();
            versionNums.insert(versionNum);
        }

        for (size_t v = 0; v < versions.size(); ++v)
        {
            if (versionNums.count(v) < 1)
            {
                // We didn't find the version number, handle the possible cases
                // cerr << "Version not found in offsetMap: " << v << endl;
                if (versions[v].size() < this->minFragSize)
                {
                    theList = PartitionList(v);
                    theList.push(0);
                    theList.push(versions[v].size());
                    offsetMap.insert(theList);
                } else {
                    // What else is a possible cause?
                    cerr << "Version not found in offsetMap: " << v << endl;
                }
            }
        }
    }

    // Now assert that we've populated results for all those poorly behaved versions 
    assert(versions.size() == offsetMap.size());

    // Post processing to get offsetMap into offsets and versionPartitionSizes    
    for (auto it = offsetMap.begin(); it != offsetMap.end(); it++)
    {
        // Reusing the same var from above, should be ok
        theList = *it;
        for (size_t j = 0; j < theList.size(); j++)
        {
            offsetsAllVersions[totalOffsetInArray + j] = theList.get(j);
        }
        versionPartitionSizes[numVersions++] = theList.size();
        totalOffsetInArray += theList.size();
    }

    offsetMap.clear();
}

/*
Given a root node, assign offsets and sizes to it and recursively to all children nodes
*/
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