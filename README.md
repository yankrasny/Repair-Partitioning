## Repair Partitioning

### Example Usage
```
make
repair help (to see the args)
repair [args]
```

Included in this repo are some example inputs, so for example, you can run the following:

``` repair ./Input/ints/ ```

Or for a larger example:

``` repair ./Input/alice/ ```

### Description

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