#ifndef OCCURRENCE_H
#define OCCURRENCE_H

#include <string>
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

	Occurrence* getNext();
	Occurrence* getPrev();
	Occurrence* getPrec();
	Occurrence* getSucc();

	unsigned getLeft()	const;
	unsigned getRight()	const;

	unsigned getLeftPositionInSequence() const;

	void setNext(Occurrence* next);
	void setPrev(Occurrence* prev);
	void setPrec(Occurrence* prec);
	void setSucc(Occurrence* succ);

	unsigned long long getPair();
};

#endif