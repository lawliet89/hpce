HPCE Courswork 5
Ryan Savitski and Yong Wen Chua
===============================================================================
General Approach:

Pipeline of four concurrent threads (via std::thread): read io, first pass, second pass, write io. Threads operate in a synchronised streaming fashion on blocks(chunks) of data. Note that chunk size is independent of row width (can be smaller than for big images or capture several rows at a time for small images, this is both a latency and memory usage optimisation). O(2N*w) data is used for intermediate storage

Algorithmic approach

A single pass thread does all of the N level operations, in other words, it does a min or max operation across a diamond-shaped unrolled SE.

Consider the SE for one output pixel with N=3:

   1
  333
 55555
7777777
 55555
  333
   1

Note that adjacent output pixels share the majority of input pixel dependencies, therefore an approach that can reuse the partial results is crucial for high N values.

A possible approach (not used here) is to construct differential histograms for the pixels entering and leaving the SE as it is slid horizontally. However, this causes very irregular read patterns into the cached circular buffer of 2N*w and is hard to parallelise.

Our approach uses the ability to decompose the 2d SE into a set of horizontal 1d stripes that are then reduced vertically for the correct output value. 

Looking at 1d stripes centered around a specific input pixel, notice the symmetry of the SE. This means that a result of window of (e.g.) size 3 will be used twice, once for the top bit of a diamond and once for the bottom bit of a different diamond, corresponding to two output pixels with a relative vertical offset.

For a given input pixel and N levels, we require N windowed minima results centered across that pixel.

These values are obtained by using a sliding window algortihm (computing min/max within a fixed size window across a vector of values of a given row). The key point is that a window stepped by 1 value horizontally will produce the necessary windowed result for two more output pixels. The algorithm used (ascending minimum) does O(1) work per input pixel for a window. Given N windows to be done per row, this means O(N) work per input pixel, however, the windows are done fully independently and can therefore be done in parallel. So given N processors, each input pixel could be processed in O(1) time. 

Credit for the base version of the ascending minima algorithm goes to Richard Harter:
http://web.archive.org/web/20120805114719/http://home.tiac.net/~cri/2001/slidingmin.html



For the initial  top pixel (window size 1 if you will)

slidinbg window
O(2N*w) 
// Approach based on the ascending minima algorithm by Richard Harter:

The data usage and



in-place mem
operating on fully packed data
Optimal memory usage (2N + 2 chunks)

O(n) -> O(1) time with On procs

streaming, sequential reads
RMW operations to accumulators separated by entire lines of input, so there is no contention on reasonably sized images

template compile-time?

no time for optimising threading
no time for special binary case (can shrink windows to size of 1 entry and use binary logic instead of comparisons)


===============================================================================
Verification Methodology

Mostly through a script (test.sh) that compares processed /dev/urandom data against the provided implementation for given parameters. Using this manually and through automatic parameter-fuzzing wrapper script.

===============================================================================
Work Partition

Initial algorithm design by both group members, with some concept whiteboarding with the group of Oskar Weigl and Thomas Morrison.

Implementation:
> Ryan Savitski - sliding window code.
> Yong Wen - main code, synchronisation primitives, pipeline setup, io read/write threads, auxiliary functions (basically everything except include/window_1d.hpp).

===============================================================================
Compiling and Running

No dependencies on tbb or opencl, but if compiling under g++ an explicit -pthread flag needs to be passed in for std::thread (provided makefile handles this). 

Target binary generated: src/v2.
