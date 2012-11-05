#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <locale>
#include "profiler.h"
#include "ObjectPool.h"
#include "RandomHeap.h"
#include "Tokenizer.h"
using namespace std;

unsigned currentID(0);
unsigned nextID()
{
	return ++currentID;
}

//Used to store associations from one symbol to two others
struct Association
{
	friend ostream& operator<<(ostream& os, const Association& a)
	{
		return os << a.symbol << " -> " << "(" << a.left << ", " << a.right << "), " << "freq: " << a.freq << endl;
	}

	unsigned symbol;
	unsigned left;
	unsigned right;
	unsigned freq;
	unsigned startPos; //TODO implement
	unsigned endPos; //TODO implement
	Association(unsigned symbol, unsigned left, unsigned right, unsigned freq) : symbol(symbol), left(left), right(right), freq(freq) {}
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
		// Profiler::getInstance().start("HashTableEntry.removeOccurrence");
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
				// Profiler::getInstance().end("HashTableEntry.removeOccurrence");
				return;
			}
			cerr << "we didn't find the target, wtf?" << endl;
			// Profiler::getInstance().end("HashTableEntry.removeOccurrence");
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
				// Profiler::getInstance().end("HashTableEntry.removeOccurrence");

				delete current;
				return;
			}
			prev = current;
			current = current->getNext();
			next = current->getNext();
		}
		//TODO make sure this never happens
		cerr << "we didn't find the target, wtf?" << endl;
		//system("pause");
		// Profiler::getInstance().end("HashTableEntry.removeOccurrence");
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
	// Profiler::getInstance().end("addOrUpdatePair");
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
} __attribute__((noinline))

void removeFromHeap(RandomHeap& myHeap, HeapEntry* hp)
{
	if (hp && !myHeap.empty())
	{
		// Profiler::getInstance().start("removeFromHeap");
		
		myHeap.deleteRandom(hp->getIndex());

		// Profiler::getInstance().end("removeFromHeap");
	}
} __attribute__((noinline))

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
	// Profiler::getInstance().end("removeOccurrence");
} __attribute__((noinline))

vector<unsigned>* getNewRightKey(unsigned symbol, Occurrence* succ)
{
	if (!succ) return NULL;
	unsigned symbolToTheRight = succ->getRight();
	vector<unsigned>* ret = new vector<unsigned>();
	ret->push_back(symbol);
	ret->push_back(symbolToTheRight);
	return ret;
} __attribute__((noinline))

vector<unsigned>* getNewLeftKey(unsigned symbol, Occurrence* prec)
{
	if (!prec) return NULL;
	unsigned symbolToTheLeft = prec->getLeft();
	vector<unsigned>* ret = new vector<unsigned>();
	ret->push_back(symbolToTheLeft);
	ret->push_back(symbol);
	return ret;
} __attribute__((noinline))

int binarySearch(const vector<Association>& associations, unsigned target, int leftPos, int rightPos)
{
	// cout << endl;
	// cout << "Target: " << target << endl;
	// cout << "Searching between: " << leftPos << " and " << rightPos << endl;
	// system("pause");

	//indexes that don't make sense, means we haven't found the target
	if (leftPos > rightPos)
		return -1;

	if (leftPos == rightPos)
	{
		if (associations[leftPos].symbol == target)
		{
			return leftPos;
		}
		return -1;
	}

	int mid = floor((leftPos + rightPos) / 2);
	unsigned midVal = associations[mid].symbol;

	// cout << "mid: " << mid << ", val: " << midVal << endl;
	// system("pause");

	//found it
	if (target == midVal)
		return mid;

	//target is on the left
	if (target < midVal)
		return binarySearch(associations, target, leftPos, mid);
	
	//target is on the right
	if (target > midVal)
		return binarySearch(associations, target, mid+1, rightPos);
} __attribute__((noinline))

vector<unsigned> expand(const vector<Association>& associations, int pos, map<unsigned, vector<unsigned> > knownExpansions)
{
	// cout << "Expanding position: " << pos << endl;
	// system("pause");
	if (knownExpansions[pos].size() > 0)
		return knownExpansions[pos];

	unsigned left = associations[pos].left;
	unsigned right = associations[pos].right;

	int lpos = binarySearch(associations, left, 0, pos);
	int rpos = binarySearch(associations, right, 0, pos);

	vector<unsigned> lret, rret;
	if (lpos == -1)	
	{
		lret = vector<unsigned>();		
		lret.push_back(left);
	}
	else
	{
		lret = expand(associations, lpos, knownExpansions);
	}

	if (rpos == -1)
	{
		rret = vector<unsigned>(); 		
		rret.push_back(right);
	}
	else
	{
		rret = expand(associations, rpos, knownExpansions);
	}

	// return lret + rret; //overload + for vector<unsigned> or just do something else
	//the stl code below might be slow, we don't have to use it if it's a problem
	lret.insert(lret.end(), rret.begin(), rret.end());
	knownExpansions[pos] = lret;
	return lret;
} __attribute__((noinline))

/*

Extract back to original string

*/
vector<unsigned> undoRepair(const vector<Association>& associations)
{
	// Profiler::getInstance().start("undoRepair");

	//knownExpansions is used for memoization
	map<unsigned, vector<unsigned> > knownExpansions = map<unsigned, vector<unsigned> >();
	vector<unsigned> result = expand(associations, associations.size() - 1, knownExpansions);
	
	// Profiler::getInstance().end("undoRepair");
	return result;
} __attribute__((noinline))

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
				associations.push_back(Association(symbol, curr->getLeft(), curr->getRight(), numOccurrences));
			
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
} __attribute__((noinline))

vector<unsigned> stringToWordIDs(const string& text)
{
	// Profiler::getInstance().start("stringToWordIDs");
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
	// Profiler::getInstance().end("stringToWordIDs");
	return ret;
}

char* getText(const string& filename, int& length)
{
	char* buffer;

	ifstream is;
	is.open ( filename.c_str(), ios::binary );

	if (is.fail())
	{
		cerr << "Could not open input file. Exiting..." << endl;
		exit(1);
	}
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

void cleanup(map<vector<unsigned>, HashTableEntry*>& hashTable)
{
	for (map<vector<unsigned>, HashTableEntry*>::iterator it = hashTable.begin(); it != hashTable.end(); it++)
	{
		delete it->second;
		it->second = NULL;
	}
}

void writeAssociations(const vector<Association>& associations, const string& filename)
{
	ofstream os(filename.c_str());
	for (size_t i = 0; i < associations.size(); i++)
	{
		os << associations[i];
	}
	os.close();
}

void getVersions(vector<char**>& versions)
{
	//TODO figure out the input for this, depends on Jinru's code
}


/*
We must be able to re-extract for the algorithm to be correct
Associations must be monotonically decreasing for the algorithm to be optimal
*/
bool checkOutput(vector<Association> associations, vector<unsigned> wordIDs)
{
	//Check to see whether it's correct
	vector<unsigned> extractedWordIDs = undoRepair(associations);
	for (size_t i = 0; i < extractedWordIDs.size(); i++)
	{
		//cerr << "Orig: " << wordIDs[i] << ", compare: " << extractedWordIDs[i] << endl; 
		if (extractedWordIDs[i] != wordIDs[i])
		{
			return false;
		}
	}

	cerr << "Passed correctness check..." << endl;

	//If it's correct, check to see whether it's optimal
	for (size_t i = 0; i < associations.size() - 1; i++)
	{
		cerr << "freq[i]: " << associations[i].freq << ", freq[i+1]: " << associations[i+1].freq << endl;
		// system("pause");
		if (associations[i].freq < associations[i+1].freq)
		{
			return false;
		}
	}

	cerr << "Passed optimal check..." << endl;

	return true;
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
		Profiler::getInstance().end("heap");
		Profiler::getInstance().writeResults("Output/profile-heap.txt");
		exit(0);
	}

	if (test == "repair")
	{
		//The original text, as one document (idea is to process concatenation of all versions)
		char* text;
		const char* filename;
		int fileSize;

		if (argc < 2)
			filename = "Input/longInput.txt";
		else
			filename = argv[1];

		text = getText(filename, fileSize);

		//For now just deal with one doc, no separators, TODO add them later
		//TODO this is roughly tolower, needs a string though, not a char*
		//std::transform(text.begin(), text.end(), text.begin(), ::tolower);

		vector<unsigned> wordIDs = stringToWordIDs(text);

		//Allocate the heap, hash table, and array of associations
		RandomHeap myHeap;
		map<vector<unsigned>, HashTableEntry*> hashTable = map<vector<unsigned>, HashTableEntry*>();
		vector<Association> associations = vector<Association>();
		
		// Profiler::getInstance().start("main");
		
		// Profiler::getInstance().start("extract");
		extractPairs(wordIDs, myHeap, hashTable);
		// Profiler::getInstance().end("extract");

		int numPairs = hashTable.size();

		// Profiler::getInstance().start("repair");
		doRepair(myHeap, hashTable, associations);
		// Profiler::getInstance().end("repair");

		// Profiler::getInstance().end("main");

		stringstream ss;
		ss << "Filename: " << filename << endl;
		ss << "Size: " << fileSize << " bytes" << endl; 
		ss << "Word Count: " << wordIDs.size() << endl;
		ss << "Unique Pair Count (in original file): " << numPairs << endl;

		writeAssociations(associations, "Output/associations.txt");
		
		cout << "Checking output... ";
		if (checkOutput(associations, wordIDs))
			cout << "Output ok!";
		else
			cout << "Check failed!";
		cout << endl;

		// Profiler::getInstance().setInputSpec(ss.str());	
		// Profiler::getInstance().writeResults("Output/profile-functions.txt");
		cleanup(hashTable);
		system("pause");
	}
}