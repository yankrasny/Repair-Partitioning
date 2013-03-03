#ifndef OCCURRENCE_H
#define OCCURRENCE_H

#include "Util.h"

//An Occurrence, which interacts with other occurrences when it gets replaced by a symbol
class Occurrence
{
private:
	//linked list of occurrences needs each occurrence to be able to point to the next one (see HashTableEntry)
	Occurrence* next;
	Occurrence* prev;

	//To modify each other's left and right
	Occurrence* prec;
	Occurrence* succ;
	unsigned left;
	unsigned right;
	unsigned leftPositionInSequence;
public:
	Occurrence() : prec(NULL), succ(NULL), next(NULL), prev(NULL) {}

	Occurrence(unsigned long long key) : prec(NULL), succ(NULL), left(key >> 32), right((key << 32) >> 32), next(NULL), prev(NULL) {}

	Occurrence(unsigned long long key, unsigned leftPositionInSequence) : prec(NULL), succ(NULL), left(key >> 32), right((key << 32) >> 32), leftPositionInSequence(leftPositionInSequence), next(NULL), prev(NULL) {}

	Occurrence* getNext() { return next; }
	Occurrence* getPrev() { return prev; }
	Occurrence* getPrec() { return prec; }
	Occurrence* getSucc() { return succ; }

	unsigned getLeft()	const { return left; }
	unsigned getRight()	const { return right; }

	unsigned getLeftPositionInSequence() const { return leftPositionInSequence; }

	void setNext(Occurrence* next) { this->next = next; }
	void setPrev(Occurrence* prev) { this->prev = prev; }
	void setPrec(Occurrence* prec) { this->prec = prec; }
	void setSucc(Occurrence* succ) { this->succ = succ; }

	unsigned long long getPair()
	{
		return combineToUInt64(left, right);
	}
};

#endif