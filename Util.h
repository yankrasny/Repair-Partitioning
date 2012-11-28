#ifndef UTIL_H
#define UTIL_H

const unsigned MAX_NUM_FRAGMENTS_PER_VERSION(100);
const unsigned MAX_FRAG_LENGTH(1000000);

extern unsigned currentFragID;
inline unsigned nextFragID()
{
	return ++currentFragID;
}

extern unsigned currentID;
inline unsigned nextID()
{
	return ++currentID;
}

inline unsigned long long combineToUInt64(unsigned long long left, unsigned long long right)
{
	return (left << 32) | right;
}

inline unsigned getLeft(unsigned long long key)
{
	return key >> 32;
}

inline unsigned getRight(unsigned long long key)
{
	return (key << 32) >> 32;
}

#endif