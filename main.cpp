#include "prototype/RepairPartitioningPrototype.h"
#include <assert.h>
using namespace std;

unsigned currentFragID = 0;
unsigned currentID = 0;
unsigned currentOffset = 0;

int main(int argc, char* argv[])
{
	RepairPartitioningPrototype prototype;
	return prototype.run(argc, argv);
}