#include "prototype/prototype.h"
#include <assert.h>
using namespace std;

unsigned currentFragID = 0;
unsigned currentID = 0;
unsigned currentOffset = 0;

int main(int argc, char* argv[])
{
	Prototype prototype;
	return prototype.run(argc, argv);
}