#include <iostream>
#include "Repair.h"
#include "HashTableEntry.h"

HashTableEntry::HashTableEntry(HeapEntry* hp, Occurrence* prec, Occurrence* succ, unsigned leftPosition) : heapPointer(hp), size(1)
{
	unsigned long long key = hp->getKey();

	occurrences = new Occurrence(key, leftPosition); //The head of the linked list (Occurrences have a next pointer)

	doubleLinkNeighbors(prec, occurrences);
	doubleLinkNeighbors(occurrences, succ);
}
HashTableEntry::HashTableEntry(HeapEntry* hp, Occurrence* oc) : heapPointer(hp), size(1)
{
	occurrences = oc;
}

void HashTableEntry::increment()
{
	size++;
	heapPointer->increment();
}
void HashTableEntry::decrement()
{
	size--;
	heapPointer->decrement();
}
void HashTableEntry::removeOccurrence(Occurrence* target)
{
	if (!target || !heapPointer)
		return;

	Occurrence* next = target->getNext();
	Occurrence* prev = target->getPrev();

	doubleLinkOccurrences(prev, next);

	decrement();
	if (size < 1)
	{
		if (occurrences == target)
		{
			delete occurrences;
			occurrences = NULL;
			return;
		}
		std::cerr << "we didn't find the target, wtf?" << std::endl;
		return;
	}

	if (occurrences == target)
	{
		occurrences = next;
	}
	delete target;
	target = NULL;
}
void HashTableEntry::addOccurrence(Occurrence* oc)
{
	if (!oc || !occurrences)
		return;

	//Adds an occurrence to the head of the linked list	
	oc->setNext(occurrences);
	occurrences->setPrev(oc);

	occurrences = oc;
	increment();
}
Occurrence* HashTableEntry::getHeadOccurrence() const
{
	return occurrences;
}
size_t HashTableEntry::getSize() const
{
	return size;
}
HeapEntry* HashTableEntry::getHeapPointer() const
{
	return heapPointer;
}
