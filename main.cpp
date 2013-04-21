#include "prototype/prototype.h"
#include "prototype/prototype2.h"
#include <assert.h>
using namespace std;

unsigned currentFragID = 0;
unsigned currentID = 0;
unsigned currentOffset = 0;

int main(int argc, char* argv[])
{
	Prototype2 prototype2;
	return prototype2.run(argc, argv);
}