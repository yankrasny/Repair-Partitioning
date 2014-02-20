#include <assert.h>

#include "prototype/RepairPartitioningPrototype.h"

// Disable debugging (use this in production)
// #define NDEBUG 1

using namespace std;

unsigned currentFragID = 0;
unsigned currentWordID = 0;
unsigned currentOffset = 0;

int main(int argc, char* argv[])
{
	// cerr << "Association: " << sizeof(Association) << endl;
	// cerr << "HeapEntry: " << sizeof(HeapEntry) << endl;
	// cerr << "HashTableEntry: " << sizeof(HashTableEntry) << endl;
	// cerr << "unsigned: " << sizeof(unsigned) << endl;
	// cerr << "unsigned long: " << sizeof(unsigned long) << endl;
	// cerr << "unsigned long long: " << sizeof(unsigned long long) << endl;
	// exit(0);
	RepairPartitioningPrototype prototype;
	return prototype.run(argc, argv);
}