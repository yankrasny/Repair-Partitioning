/*

Current Status:

8/27/12 (I think)
[Done] Just changed all the chars to unsigned, and implemented word -> wordID translation
	[Done] We have a vector of wordIDs to work with, it's no longer char based
		[See below] Extracting the pairs and filling the two data strcutures (HT + heap) is buggy

9/7/12
[Done] Refactored and fixed a bunch of problems with extract pairs
[Done] Implemented a consistent understanding of adding occurrences and setting prec/succ
	[Fixed] Still need to clean up the classes based on all of this
		[See below] About to implement all removals, this should address the above

[Fixed] OK so now, we have this huge if else structure for all the cases with the linked list
	[Fixed]	This needs to be refactored (a lot of statements in there are repeated)
	[See below]	Also, there are access violations, so need to think more about what's happening with memory I'm using

9/9/12
[Fixed]	Refactored that huge if else structure, now it's logically related to the edge cases
[See below] The linked list of Occurrences isn't being properly handled, something to do with removal of occurrences...

9/10/12
[Fixed] Removing the hashTable[key] was happening every time (added the appropriate condition)
[Fixed] myHeap.front() is not giving the highest occurring element, so the comparison function is either wrong on not being used
[TempFix] Heap Entries are being sort of deleted, but not fully, so ending up with numOccurrences = 23123129083 or something
	=> Instead of an efficient remove for the heap, ran through the whole thing

9/11/12 (Early)
[Update] No runtime errors
[] Not sure if associations are correct (consider implementing fillTree for this)
[Fixed] not all associations have consecutive IDs, and they should
[Done] The associations now print at the end of the program

9/12/12
[] Introduce separators and concept of versions

9/18/12
[shit] No progress since 9/12, have been working on other projects...

9/20/12
[Fixed] Some bugs (program used to crash on non trivial input)

9/21/12
[Fixed] Linear removal is likely the cause of poor performance


9/24/12
[Done] No progress since last time (learned a bit about stl heaps, might have to write my own)
[Not needed] I did try to use the valid flag approach, with unfortunate results
	[Not needed] Something got corrupted, like the heap (not a heap, the heap).... not sure wtf was wrong

[Great] Checked out Jinru's code for winnowing and discovered the data type for representing fragment boundaries
	[Great] An int* called starts, here's an example [0, 10, 17, 24, 38]
		[Great] In this example, the fragments are (0,9), (10,16), (17,23), (24,37), (38, docSize)

[Done] Have an idea of how to implement deletion of an arbitrary element in the heap
	[Done] It's stored as a vector, and the tree is conceptual
		[Done] So swap with the back(), pop_back(), and reheapify
		[Done] Wait, didn't I try this already, and wasn't the heap invalid? Let's see...


9/26/12
Talked to Sergey, and have a new plan
[Done] Profile: created a profiler, now using it
[Done] Instead of using new and delete, use an object pool
[Done] Implement the damn log(n) remove

[Result] Basically we need to create an object pool and also fix the O(n) remove

9/27/12
[Done] Implementing Object Pool: Results are weak. Gotta fix the algorithm itself (re-examine pooling later)

10/8/12
[I know] Well that was a long break...

10/9/12
[Done] Writing RandomHeap (THAT SHIT WORKS)
[Not 30 minutes...] Integrate into this code (just keep track of positions, not that hard)
	DEBUG THIS FOR LIKE 30 MINUTES AND YOU GOT IT!
		===>LOL, nope

10/12/12
[Done] Still debugging runtime errors
[Done] Now it looks like prec and succ are not properly being removed (they have mem addresses that seem to have been freed, ex: 0xfefefefe)
[Done] The heap seems to work nicely on its own, priorities and indexes look ok
[Fixed] Memory is being corrupted when calling mapExists() More 0xfefefefe type stuff

10/17/12
[Great] After an awesome session with Costas, I'm much closer to solving this
[Done] Latest error: prec and succ aren't being set properly somewhere
[Great] Note: heap seems ok, finally

10/19/12
[Done] The problem is now in the linking to prec and succ, check the code for that in repair, as the link function itself is easy
[IT WORKS BITCHES]

------------------------------------------------------------
What have we learned so far?

1) Think before you implement anything, however simple.
2) Version control, always!
3) Don't worry about wasting time, sometimes proper implementation can take a while.

*/

//Shit + Alt + Enter to toggle fullscreen

#include<iostream>
#include<fstream>
#include<vector>
#include<map>
#include<algorithm>
#include<iterator>
#include<sstream>
#include<locale>
#include"profiler.h"
#include"ObjectPool.h"
#include"RandomHeap.h"
using namespace std;

unsigned currentID(0);
unsigned nextID()
{
	return ++currentID;
}

/*
Borrowed from Sergey N.
*/
void tokenize(const std::string& str,  std::vector<std::string>& tokens,
              const std::string& delimiters, const bool trimEmpty) 
{
        std::string::size_type pos, lastPos = 0;
        while(true)
        {
                pos = str.find_first_of(delimiters, lastPos);
                if(pos == std::string::npos)
                {
                        pos = str.length();

                        if(pos != lastPos || !trimEmpty)
                                tokens.push_back(std::string(str.data()+lastPos,
                                                (std::string::size_type)pos-lastPos ));

                        break;
                }
                else
                {
                        if(pos != lastPos || !trimEmpty)
                                tokens.push_back(std::string(str.data()+lastPos,
                                                (std::string::size_type)pos-lastPos ));
                }

                lastPos = pos + 1;
        }
}

//Used to store associations from one symbol to two others
struct Association
{
	friend ostream& operator<<(ostream& os, const Association& a)
	{
		return os << a.symbol << " -> " << "(" << a.left << ", " << a.right << ")" << endl;
	}

	unsigned symbol;
	unsigned left;
	unsigned right;
	Association(unsigned symbol, unsigned left, unsigned right) : symbol(symbol), left(left), right(right) {}
};

//ObjectPool<HeapEntry> heapPool = ObjectPool<HeapEntry>(1000);

//An Occurrence, which interacts with other occurrences when it gets replaced by a symbol
class Occurrence
{
private:
	//linked list of occurrences needs each occurrence to be able to point to the next one (see HashTableEntry)
	Occurrence* next;

	//To modify each other's left and right
	Occurrence* prec;
	Occurrence* succ;
	unsigned left;
	unsigned right;
public:
	Occurrence() : prec(NULL), succ(NULL), next(NULL) {}

	Occurrence(unsigned left, unsigned right) : prec(NULL), succ(NULL), left(left), right(right), next(NULL) {}

	Occurrence(vector<unsigned> pair) : prec(NULL), succ(NULL), left(pair[0]), right(pair[1]), next(NULL) {}

	void init(vector<unsigned> pair)
	{
		left = pair[0];
		right = pair[1];
	}

	Occurrence* getNext()	
	{
		return next;
	}
	Occurrence* getPrec()	{return prec;}
	Occurrence* getSucc()	{return succ;}

	unsigned getLeft()	const {return left;}
	unsigned getRight()	const {return right;}

	void setNext(Occurrence* next)	{this->next = next;}
	void setPrec(Occurrence* prec)	{this->prec = prec;}
	void setSucc(Occurrence* succ)	{this->succ = succ;}

	vector<unsigned> getPair()
	{
		vector<unsigned> ret;
		ret.push_back(left);
		ret.push_back(right);
		return ret;
	}
};
//ObjectPool<Occurrence> occurrencePool = ObjectPool<Occurrence>(5000);

void doubleLinkOccurrences(Occurrence* prev, Occurrence* curr)
{
	// Profiler::getInstance().start("link");
	if (prev && curr)
	{
		//Set the preceding pointer of the current element
		curr->setPrec(prev);

		//Set the succeeding pointer of the previous element
		prev->setSucc(curr);
	}
	// Profiler::getInstance().end();
}

class HashTableEntry
{
	HeapEntry* heapPointer;
	Occurrence* occurrences;
	size_t size;
public:
	HashTableEntry(HeapEntry* hp, Occurrence* prec, Occurrence* succ) : heapPointer(hp), size(1)
	{
		unsigned left = hp->getKey()[0];
		unsigned right = hp->getKey()[1];
		
		occurrences = new Occurrence(left, right); //The head of the linked list (Occurrences have a next pointer)

		doubleLinkOccurrences(prec, occurrences);
		doubleLinkOccurrences(occurrences, succ);

		//occurrences = occurrencePool.getNew();
		//occurrences->init(hp->getKey());
	}
	HashTableEntry(HeapEntry* hp, Occurrence* oc) : heapPointer(hp), size(1)
	{
		occurrences = oc;
	}

	void increment()
	{
		size++;
		heapPointer->increment();
	}
	void decrement()
	{
		size--;
		heapPointer->decrement();
	}
	void removeOccurrence(Occurrence* target)
	{
		//Profiler::getInstance().start("HashTableEntry.removeOccurrence");
		if (!target || !heapPointer)
			return;

		decrement();

		if (size < 1)
		{
			if (occurrences == target)
			{
				//occurrencePool.destroy(target);
				delete occurrences;
				occurrences = NULL;
				return;
			}
			return;
		}
		
		Occurrence* current = occurrences;
		Occurrence* next = occurrences->getNext();
		Occurrence* prev(NULL);

		//TODO this might also make things slow (can we do this without looping over the list?)
		while (current->getNext())
		{
			if (current == target)
			{
				//found it, do the removal
				if (!prev)
				{
					//target was the first one, just point occurrences to the next one
					occurrences = next;
				}
				else
				{
					//need to link the previous one to the next one
					prev->setNext(next);
				}
				
				//occurrencePool.destroy(current);
				//Profiler::getInstance().end();

				delete current;
				return;
			}
			prev = current;
			current = current->getNext();
			next = current->getNext();
		}
		//TODO make sure this never happens
		//cerr << "we didn't find the target, wtf?" << endl;
		//system("pause");
		//Profiler::getInstance().end();
	}
	void addOccurrence(Occurrence* oc)
	{
		//Adds an occurrence to the head of the linked list	
		if (occurrences)
		{
			oc->setNext(occurrences);
		}
		occurrences = oc;
		increment();
	}
	Occurrence* getHeadOccurrence() const
	{
		return occurrences;
	}
	size_t getSize() const
	{
		return size;
	}
	HeapEntry* getHeapPointer() const
	{
		return heapPointer;
	}
};
//ObjectPool<HashTableEntry> hashTablePool = ObjectPool<HashTableEntry>(2000);

void addOrUpdatePair(RandomHeap& myHeap, map<vector<unsigned>, HashTableEntry*>& hashTable, vector<unsigned>* keyPtr, Occurrence* prec = NULL, Occurrence* succ = NULL)
{
	if (keyPtr == NULL)
		return;

	// Profiler::getInstance().start("addOrUpdatePair");
	
	vector<unsigned> key = *keyPtr;
	HeapEntry* hp;

	if (mapExists(hashTable, key))
	{
		hashTable[key]->addOccurrence(new Occurrence(key));
	}
	else //First time we've seen this pair
	{
		//Create a heap entry, and initialize the count to 1
		hp = new HeapEntry(key, 1);

		//HeapEntry* hp = heapPool.getNew();
		//hp->init(key, 1);

		//Create a hash table entry, and initialize it with its heap entry pointer
		hashTable[key] = new HashTableEntry(hp, prec, succ); //This creates the first occurrence (see the constructor)
		
		//The order of these calls matters: do this first and the hash table entry won't know the index
		//Is this still true? TODO
		myHeap.insert(hp);
	}
	// Profiler::getInstance().end();
}

void extractPairs(vector<unsigned> wordIDs, RandomHeap& myHeap, map<vector<unsigned>, HashTableEntry*>& hashTable)
{
	//The current pair (will be reused in the loop)
	vector<unsigned>* currPair = new vector<unsigned>();

	//The previous entry in the HT (used to set preceding and succeeding pointers)
	Occurrence* prevOccurrence(NULL);

	//Go through the string and get all overlapping pairs, and process them
	for (size_t i = 0; i < wordIDs.size() - 1; i++)
	{
		currPair->push_back(wordIDs[i]);
		currPair->push_back(wordIDs[i+1]);

		addOrUpdatePair(myHeap, hashTable, currPair, NULL, NULL);

		//The first occurrence was the last one added because we add to the head
		Occurrence* lastAddedOccurrence = hashTable[*currPair]->getHeadOccurrence();

		//Checks for existence of prev, and links them to each other
		doubleLinkOccurrences(prevOccurrence, lastAddedOccurrence);

		//Update the previous occurrence variable
		prevOccurrence = lastAddedOccurrence;

		currPair->clear();
	}
	delete currPair;
}

void removeFromHeap(RandomHeap& myHeap, HeapEntry* hp)
{
	if (hp && !myHeap.empty())
	{
		//Profiler::getInstance().start("removeFromHeap");
		
		myHeap.deleteRandom(hp->getIndex());

		//Profiler::getInstance().end();
	}
}

void removeOccurrence(RandomHeap& myHeap, map<vector<unsigned>, HashTableEntry*>& hashTable, Occurrence* oc)
{
	if (!oc)
	{
		return;
	}
	// Profiler::getInstance().start("removeOccurrence");
	vector<unsigned> key = oc->getPair();
	if (mapExists(hashTable, key))
	{
		HeapEntry* hp = hashTable[key]->getHeapPointer();
		hashTable[key]->removeOccurrence(oc);
		if (hashTable[key]->getSize() < 1)
		{
			removeFromHeap(myHeap, hp);
			hashTable.erase(key);
		}
	}
	// Profiler::getInstance().end();
}

vector<unsigned>* getNewRightKey(unsigned symbol, Occurrence* succ)
{
	if (!succ) return NULL;
	unsigned symbolToTheRight = succ->getRight();
	vector<unsigned>* ret = new vector<unsigned>();
	ret->push_back(symbol);
	ret->push_back(symbolToTheRight);
	return ret;
}

vector<unsigned>* getNewLeftKey(unsigned symbol, Occurrence* prec)
{
	if (!prec) return NULL;
	unsigned symbolToTheLeft = prec->getLeft();
	vector<unsigned>* ret = new vector<unsigned>();
	ret->push_back(symbolToTheLeft);
	ret->push_back(symbol);
	return ret;
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
*/
void doRepair(RandomHeap& myHeap, map<vector<unsigned>, HashTableEntry*>& hashTable, vector<Association>& associations)
{
	while (!myHeap.empty())
	{
		unsigned symbol;
		unsigned symbolToTheLeft;
		unsigned symbolToTheRight;
		
		//Get the max from the heap
		HeapEntry hp = myHeap.getMax();

		//The string of 2 chars, used to key into the hashmap
		vector<unsigned> key = hp.getKey();
		
		//Get the hash table entry (so all occurrences and so on)
		HashTableEntry* max = hashTable[key];
		size_t numOccurrences = max->getSize();

		Occurrence* curr;
		Occurrence* prec;
		Occurrence* succ;

		//Will use this as the new symbol (say we're replacing abcd with axd, this is x)
		symbol = nextID();

		//For all occurrences of this entry, do the replacement and modify the corresponding entries
		for (size_t i = 0; i < numOccurrences; i++)
		{
			Occurrence* newLeftOcc(NULL);
			Occurrence* newRightOcc(NULL);
			
			//Get the occurrence and its neighbors
			curr = max->getHeadOccurrence();

			//If curr is null, we have a problem. This should never happen.
			if (!curr)
				break;

			prec = curr->getPrec();
			succ = curr->getSucc();

			//Store the association before you do anything else
			if (i == 0)
				associations.push_back(Association(symbol, curr->getLeft(), curr->getRight()));
			
			//Now go through all the edge cases (because of the links we have to make, there are a lot)
			bool onLeftEdge(false);
			bool onRightEdge(false);
			bool nearLeftEdge(false);
			bool nearRightEdge(false);

			vector<unsigned>* newLeftKey;
			vector<unsigned>* newRightKey;

			//Use these bools instead of calling the functions repeatedly
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

			//Just creates the occurrence in the hash table and heap, doesn't link it to its neighbors
			addOrUpdatePair(myHeap, hashTable, newLeftKey);
			addOrUpdatePair(myHeap, hashTable, newRightKey);

			if (!nearLeftEdge && !onLeftEdge)
			{
				//Have 2 neighbors to the left
				newLeftOcc = hashTable[*newLeftKey]->getHeadOccurrence();
				doubleLinkOccurrences(prec->getPrec(), newLeftOcc);
			}
			if (!nearRightEdge && !onRightEdge)
			{
				//Have 2 neighbors to the right
				newRightOcc = hashTable[*newRightKey]->getHeadOccurrence();
				doubleLinkOccurrences(newRightOcc, succ->getSucc());
			}
			if (!onRightEdge && !onLeftEdge)
			{
				//A neighbor on each side, link them
				newLeftOcc = hashTable[*newLeftKey]->getHeadOccurrence();
				newRightOcc = hashTable[*newRightKey]->getHeadOccurrence();
				doubleLinkOccurrences(newLeftOcc, newRightOcc);
			}
			
			//cerr << "Removing curr: " << curr->getLeft() << "," << curr->getRight() << endl;
			removeOccurrence(myHeap, hashTable, curr);
			
			if (!onRightEdge)
			{
				//cerr << "Removing succ: " << succ->getLeft() << "," << succ->getRight() << endl;
				removeOccurrence(myHeap, hashTable, succ);
			}
			if (!onLeftEdge)
			{
				//cerr << "Removing prec: " << prec->getLeft() << "," << prec->getRight() << endl;
				removeOccurrence(myHeap, hashTable, prec);
			}
			delete newLeftKey;
			delete newRightKey;
			newLeftKey = NULL;
			newRightKey = NULL;
		}
	}
}

vector<unsigned> stringToWordIDs(const string& text)
{
	map<unsigned, unsigned> uniqueWordIDs = map<unsigned, unsigned>();

	vector<unsigned> ret = vector<unsigned>();

    istringstream iss(text);
	
	vector<string> tokens;
	//copy(istream_iterator<string>(iss),
	//		 istream_iterator<string>(),
	//		 back_inserter<vector<string> >(tokens));
	
	//remove punctuation and produce vector of strings (called tokens)
	string delimiters = "',.:;\"/!?() []{}\n";
	bool trimEmpty = false;	
	tokenize(text, tokens, delimiters, trimEmpty);

	//Would it be faster to download the code for a hash function? TODO
	locale loc;
	const collate<char>& coll = use_facet<collate<char> >(loc);
	unsigned myHash;

	map<unsigned, unsigned>::iterator it;
	for (size_t i = 0; i < tokens.size(); i++)
	{
		myHash = coll.hash(tokens[i].data(), tokens[i].data() + tokens[i].length());
		it = uniqueWordIDs.find(myHash);
		unsigned theID;
		if (it != uniqueWordIDs.end())
		{
			//found it, so use its ID
			theID = it->second;
		}
		else
		{
			//Didn't find it, give it an ID
			theID = nextID();
			uniqueWordIDs[myHash] = theID;
			//it->second = nextID();
		}
		ret.push_back(theID);
	}
	return ret;
}

char* getText(const string& filename)
{
	int length;
	char* buffer;

	ifstream is;
	is.open ( filename.c_str(), ios::binary );

	// get length of file:
	is.seekg (0, ios::end);
	length = is.tellg();
	is.seekg (0, ios::beg);

	// allocate memory:
	buffer = new char [length];

	// read data as a block:
	is.read (buffer,length);
	is.close();

	return buffer;
}

void cleanup(RandomHeap& myHeap, map<vector<unsigned>, HashTableEntry*>& hashTable, vector<Association>& associations)
{
	for (map<vector<unsigned>, HashTableEntry*>::iterator it = hashTable.begin(); it != hashTable.end(); it++)
	{
		delete it->second;
		it->second = NULL;
	}
	myHeap.cleanup();
	associations.clear();
}

void showAssociations(const vector<Association>& associations)
{
	for (size_t i = 0; i < associations.size(); i++)
	{
		cout << associations[i];
	}
}

void getVersions(vector<char**>& versions)
{
	//TODO figure out the input for this, depends on Jinru's code
}

int main(int argc, char* argv[])
{
	//createOutputDir();

	//heap, repair
	string test = "repair";

	if (test == "heap")
	{
		Profiler::getInstance().start("heap");
		RandomHeapTest test = RandomHeapTest(1000);		
		Profiler::getInstance().end();
		Profiler::getInstance().writeResults("Output/profile-heap.txt");
		exit(0);
	}

	if (test == "repair")
	{
		//The original text, as one document (idea is to process concatenation of all versions)
		char* text;
		const char* filename;

		if (argc < 2)
			filename = "Input/longInput.txt";
		else
			filename = argv[1];

		text = getText(filename);

		//For now just deal with one doc, no separators, TODO add them later
		//TODO this is roughly tolower, needs a string though, not a char*
		//std::transform(text.begin(), text.end(), text.begin(), ::tolower);

		vector<unsigned> wordIDs = stringToWordIDs(text);

		//Allocate the heap, hash table, and array of associations
		RandomHeap myHeap;
		map<vector<unsigned>, HashTableEntry*> hashTable = map<vector<unsigned>, HashTableEntry*>();
		vector<Association> associations = vector<Association>();

		Profiler::getInstance().setInputSpec(filename);
		Profiler::getInstance().start("extract");
		extractPairs(wordIDs, myHeap, hashTable);
		Profiler::getInstance().end();

		Profiler::getInstance().start("repair");
		doRepair(myHeap, hashTable, associations);
		Profiler::getInstance().end();		

		showAssociations(associations);
		cleanup(myHeap, hashTable, associations);		
		Profiler::getInstance().writeResults("Output/profile-functions.txt");
		system("pause");
	}
}