# Repair Partitioning

Repair is an algorithm traditionally used for text compression. In most implementations, words are first converted to symbols, and then the algorithm processes the string of symbols. The logic behind it is wonderfully simple:

* Identify all pairs of symbols in the string (the string [1 2 3 1 2] gives us the pairs (1,2), (2,3), and (3,1))
* Replace the highest occurring pair (in our case (1,2)) with a new symbol (we use the next available number; in our case, 5)
* Store the association 5 -> (1,2)
* Continue until there is one symbol remaining

You can then recreate the original string from the last symbol by recursively expanding it.

In this implementation, I consider versions of a text document. By running Repair on all these versions, I expect that repeating fragments will get the same symbol. If two versions of a document are similar, then their Repair Trees will be similar as well. This can be used to help build a more efficient text index for versioned systems.

## Example Usage
```
make
repair help (to see the args)
repair [args]
```

Included in this repo are some example inputs, so for example, you can run the following:

``` repair ./Input/ints/ ```

Or for a larger example:

``` repair ./Input/alice/ ```

## Description

A library for partitioning documents into fragments. The more a fragment occurs, the more likely it is to be chosen.

In RePair, pairs of symbols are continuously replaced until there is one symbol left. For example, given the following string of symbols: ```[1 2 3 1 2 4]```

The pair (1,2) occurs most often, so it is replaced by the next available symbol, 5.
```
5 -> (1,2)
[5 3 5 4]
```

We have a tie now, so we might replace the pairs as follows:
```
6 -> (5,3)
[6 5 4]

7 -> (6,5)
[7 4]

8 -> (7,4)
[8]
```

Applying the same algorithm to several versions of a document, we expect to find many similar fragments across versions.
This can aid in versioned indexing or other applications where it is useful to identify redundant text fragments. 