#ifndef UNDO_REPAIR_H
#define UNDO_REPAIR_H

#include <unordered_map>
#include "MetaClasses.h"

class Association;

int binarySearch(const std::vector<Association>& associations, unsigned target, int leftPos, int rightPos);

std::vector<unsigned> expand(const std::vector<Association>& associations, int pos, std::unordered_map<unsigned, std::vector<unsigned> > knownExpansions);

/*

undoRepair can be used to expand all of the nodes. 
I guess knownExpansions can store fragID -> fragContent
	how? it's a map<unsigned, vector<unsigned> >, like {4 -> (5,6), 8 -> (9,10), etc. }
	Well there you go, it already does it, and we're probably not worried about assigning fragment IDs for indexing (Jinru's index.cc does that most likely)


*/

std::vector<unsigned> undoRepair(const std::vector<Association>& associations);


#endif