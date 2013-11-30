#include <assert.h>

#include "prototype/RepairPartitioningPrototype.h"

// Disable debugging (use this in production)
// #define NDEBUG



using namespace std;

unsigned currentFragID = 0;
unsigned currentWordID = 0;
unsigned currentOffset = 0;

int main(int argc, char* argv[])
{
	RepairPartitioningPrototype prototype;
	return prototype.run(argc, argv);
}