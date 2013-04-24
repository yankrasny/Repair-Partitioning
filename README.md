# Repair Partitioning

Repair is an algorithm traditionally used for text compression. In most implementations, words are first converted to symbols, and then the algorithm processes the string of symbols. The logic behind it is wonderfully simple:

## Repair Algorithm and Example

* Identify all pairs of symbols in the string (the string [1 2 3 1 2] gives us the pairs (1,2), (2,3), and (3,1))
* Replace the highest occurring pair (in our case (1,2)) with a new symbol (we use the next available number; in our case, 5)
* Store the association 5 -> (1,2)
* Continue until there is one symbol remaining

You can then recreate the original string from the last symbol by recursively expanding it. Here's the full example:

```
[1 2 3 1 2]
5 -> (1,2)

[5 3 5]
6 -> (5,3)

[6 5]
7 -> (6,5)

[7]
```

Applying all the rules gives us a tree:
```
			 7
			/ \
		   6    5
		  / |  | \
		 5	3  1  2
		/ \
	   1   2
```
Read off the leaves (aka terminals) from left to right: [1 2 3 1 2] -> the original string!

## Accounting for Versions

In this implementation, I consider versions of a text document. By running Repair on all these versions, I expect that repeating fragments will get the same symbol. If two versions of a document are similar, then their Repair Trees will be similar as well. This can be used to help build a more efficient text index for versioned systems.

To visualize this, just try doing repair on these two strings, and compare the trees: [1 2 3 1 2 3 1 4], [1 2 1 2 2 3 1 4]. When choosing the most occurring pair, consider occurrences in both versions; that's the whole point!

## Paritioning Algorithm

Once you get the Repair Trees, you can cut them in a way that maintains boundaries of common fragments.

TODO: add details here on offsets and the output format.

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