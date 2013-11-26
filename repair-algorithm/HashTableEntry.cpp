#include "HashTableEntry.h"
using namespace std;

HashTableEntry::HashTableEntry(HeapEntryPtr entry, unsigned version) : heapEntryPointer(entry), size(1)
{
	unsigned long long key = entry.getPtr()->getKey();
	occurrences = new Occurrence(key, version); // The head of the linked list
}

void HashTableEntry::increment()
{
	size++;
	heapEntryPointer.getPtr()->increment();
}

void HashTableEntry::decrement()
{
	size--;
	heapEntryPointer.getPtr()->decrement();
}

void HashTableEntry::removeOccurrence(Occurrence* target)
{
	if (!target || !heapEntryPointer.getPtr())
		return;

	Occurrence* next = target->getNext();
	Occurrence* prev = target->getPrev();

	doubleLinkOccurrences(prev, next);

	this->decrement();

	if (size < 1)
	{
		if (occurrences == target)
		{
			delete occurrences;
			occurrences = NULL;
			return;
		}
		cerr << "we didn't find the target, wtf?" << endl;
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

	// Adds an occurrence to the head of the linked list	
	oc->setNext(occurrences);
	occurrences->setPrev(oc);

	occurrences = oc;
	this->increment();
}

Occurrence* HashTableEntry::getHeadOccurrence() const
{
	return occurrences;
}

size_t HashTableEntry::getSize() const
{
	return size;
}

HeapEntryPtr HashTableEntry::getHeapEntryPointer() const
{
	return heapEntryPointer;
}
