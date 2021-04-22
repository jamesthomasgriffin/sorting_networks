# sorting_networks
C++ template for generating small sorting networks compatible with SIMD intrinsics

## Should you be interested?

Only if:

* you need to sort many short sequences of the same length (4 - 32ish?), and performance is important
* the length of sequence is known at compile time
* you can't implement the sorting network by hand (you don't know the sequence length when writing your code).
* comparisons and swaps of pairs of elements are cheap

Especially if:

* you want to use SIMD intrinsics

## Why this method over any other?

Sorting networks are branchless, operate on sequences in-place and require few registers, meaning the generated code can offer great performance.  Efficiency is O(n(log n)^2).  The run-time is uniform, i.e. independent of the ordering of the original sequence.

## Why not this method?

For longer sequences the size of the network will lead to a large network and a lot of generated code, this may impact performance.
Since the code is branchless there is no 'early-out', a different algorithm may be more efficient.

### Other repositories of interest

[Vectorized/Static-Sort](https://github.com/Vectorized/Static-Sort)
Comparible performance in scalar code (but templates over comparisons, not comparison/swaps)

[MarcelPiNacy/osn](https://github.com/MarcelPiNacy/osn)
Explicitly specified sorting networks up to 8 elements
