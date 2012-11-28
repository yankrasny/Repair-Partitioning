#include "Occurrence.h"
#include "Util.h"

Occurrence* Occurrence::getNext()	{return next;}
Occurrence* Occurrence::getPrev()	{return prev;}
Occurrence* Occurrence::getPrec()	{return prec;}
Occurrence* Occurrence::getSucc()	{return succ;}

unsigned Occurrence::getLeft()	const {return left;}
unsigned Occurrence::getRight()	const {return right;}

unsigned Occurrence::getLeftPositionInSequence()	const {return leftPositionInSequence;}

void Occurrence::setNext(Occurrence* next)	{this->next = next;}
void Occurrence::setPrev(Occurrence* prev)	{this->prev = prev;}
void Occurrence::setPrec(Occurrence* prec)	{this->prec = prec;}
void Occurrence::setSucc(Occurrence* succ)	{this->succ = succ;}

unsigned long long Occurrence::getPair()
{
	return combineToUInt64(left, right);
}
