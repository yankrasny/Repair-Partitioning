Re-pair-gnu
===========

This is an implementation of the string re-pairing algorithm, intended for use in detecting text redundancy. Compiled with g++ on windows using mingw.

The input is several text files (the versions of a document). The output is a partitioning of each version such as many redundant fragments as possible show up in the partitions of each version. So for example, here are three versions of a (very) small document:

Version 1 -> Alice was beginning to get very tired of sitting by her sister on the bank, and of having nothing to do:

Version 2 -> Alice was starting to get very tired of sitting by her sister on the bank, and of having nothing to do:

Version 3 -> Alice was beginning to get very tired of sitting by her sister by the lake, and of having nothing to do:

Ignoring punctuation and spacing, we want to identify the common sequences of words, so first we convert the words to word IDs:

Version 1 -> [1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 8 17 18 4 19]

Version 2 -> [1 2 20 4 5 6 7 8 9 10 11 12 13 14 15 16 8 17 18 4 19]

Version 3 -> [1 2 3 4 5 6 7 8 9 10 11 12 10 14 21 16 8 17 18 4 19]

A good paritioning will find repeating fragments across versions, so first we identify those fragments:

A -> [1 2 3 4 5 6 7 8 9 10 11 12]

B -> [13 14 15 16 8 17 18 4 19]

C -> [1 2 20 4 5 6 7 8 9 10 11 12]

D -> [10 14 21 16 8 17 18 4 19]

Applying this back to the versions gives us:

Version 1 -> [A, B]

Version 2 -> [C, B]

Version 3 -> [A, D]

Note that fragments A and B occur more than once, so this is a good partitioning. 