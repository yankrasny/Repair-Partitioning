#ifndef UNDO_REPAIR_H
#define UNDO_REPAIR_H

#include <unordered_map>
#include "MetaClasses.h"

class Association;

int binarySearch(const std::vector<Association>& associations, unsigned target, int leftPos, int rightPos);

std::vector<unsigned> expand(const std::vector<Association>& associations, int pos, std::unordered_map<unsigned, std::vector<unsigned> > knownExpansions);

std::vector<unsigned> undoRepair(const std::vector<Association>& associations);

#endif