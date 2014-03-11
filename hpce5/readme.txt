===============================================================================
HPCE Coursework 5 
Ryan Savitski & Yong Wen Chua
===============================================================================
General Approach:

Pipeline of four concurrent threads (via std::thread): read io, first pass,
second pass, write io. Threads operate in a synchronised streaming fashion on
blocks (chunks) of data. IO threads can sleep while waiting for processing to be
done (for large N on slow machines), but usually run without blocking, enabling
fully block-free streaming processing. Note that chunk size is independent of
row width (can be smaller than for big images or capture several rows at a time
for small images, this is both a latency and memory usage optimisation). 

The chunked streaming pipeline lets the io speed to be determined by the
computation speed, therefore latency will not suffer from starving or overruning
the processing threads.

Algorithmic approach:

A single pass thread does all of the N level operations, in other words, it does
a min or max operation across a diamond-shaped unrolled SE.

Consider the SE for one output pixel with N=3:

   1 
  333 
 55555 
7777777
 55555 
  333 
   1

Note that adjacent output pixels share the majority of input pixel dependencies,
therefore an approach that can reuse the partial results is crucial for high N
values (as at that point, the computational throughput contributes to latency).

A possible approach (not used here) is to construct differential histograms for
the pixels entering and leaving the SE as it is slid horizontally. However, this
causes very irregular read patterns into the cached buffer and is hard to
parallelise.

Our approach uses the ability to decompose the 2d SE into a set of horizontal 1d
stripes that are then reduced vertically for the correct output value. 

Looking at 1d stripes centered around a specific input pixel, notice the
symmetry of the SE. This means that a result of window of (e.g.) size 3 will be
used twice, once for the top bit of a diamond and once for the bottom bit of a
different diamond, corresponding to two output pixels with a relative vertical
offset.

For a given input pixel and N levels, we require N windowed minima results
centered across that pixel.

These values are obtained by using a sliding window algorithm, computing min/max
within a fixed size window across a vector of values of a given row. The key
point is that a window stepped by 1 value horizontally will produce the
necessary result for the two next output pixels (considering each window
contributing to two vertically offset outputs). The algorithm used (ascending
minimum) does O(1) work per input pixel for a window. Given N windows to be done
per row, this means O(N) work per input pixel, however, the windows are done
fully independently and can therefore be done in parallel. So given N
processors, each input pixel could be processed in O(1) time. 

This approach is a natural fit for the data layout, which is streamed in
scanline order. Since the windows are slid over the pixels in a given row
sequentially, this means that all accesses are sequential (therefore benefitiing
from hardware prefetching) and, since we do all N windows at the same time, that
the input data does not need to be retained and revisited later.

So, for an input pixel in the stream, a pass steps all windows forwards by one
pixel and spreads the results to 2N vertical accumulators (meaning that the
final single pixel at the bottom of the diamond only needs to do one comparison
before being pushed into the output stream as the remaider of the SE has already
been accumulated both horizontally and vertically).

There is a natural requirement of 2*N*w retained state, which is stored in a
circular buffer of vertical accumulators in this approach.

A data layout optimisation for these accumulators is used: consider a bit of
input image read into a circular buffer, the pixel values actually correspond to
the first 1d stripe pass (of trivial size 1). Instead of copying these values
into dedicated accumulator storage, we instead treat the input buffer as the
vertical accumulators, where the input data is already initialised correctly
purely by being passed to the processing thread through this buffer. Also, since
it is a circular buffer of (optimal) 2*N*w size, old accumulators are destroyed
naturally by being overwritten with fresh data as soon as they're no longer
needed. So the input pixel in the input buffer is turned into a vertical
accumulator for the diamond for which it is the top pixel. 

Note that there are 2*N*w active vertical accumulators at a time, corresponding
to the all state that is yet to be turned into output as it is waiting for
subsequent data.

The read-modify-write operations that the sliding windows do to the accumulators
should not produce any contention for reasonably sized images as a given
accumulator is only written to once per entire image row of data.


Credit for the base version of the ascending minima algorithm: Richard Harter:
http://web.archive.org/web/20120805114719/http://home.tiac.net/~cri/2001/slidingmin.html
===============================================================================
Future work

Did not have time to fully parallelise window processing, the threads support
doing a variable amount of windows, but the all-to-one synchronisation has not
been implemented (would be similar to GPU threads, with window threads all
having to be done with an input chunk before the producer would read and supply
the next chunk).

There is no special case implementation for binary data, even though it enables
the minima queues to always have a single entry (therefore dropping all circular
buffer code from the window queues). Furthermore, comparison and RMW operations
can be instead done as binary operators.

In hindsight, the chunk size being independent of the row width was a suboptimal
decision from a latency point of view, if the image were always padded
horizontally to be a nice power of two (or similar, to enable nice chunk sizes
that are a factor of row width), it would enable a lot of the complexity to be
moved from outside of the hot loop. At the moment the row boundaries can happen
asynchronous to chunk boundaries and therefore need to be checked for on every
pixel. Similarly, wrapping checks for the circular buffer keeping vertical
accumulators have to be performed for every lookup. If the chunk size was a
factor of row width, all of these checks could be done only on the chunk
boundaries. Additionally, horizontal windup and draining of 1d windows around
the window edges could be removed as the windows would be in a consistent state
by the time the padding is processed. Of course, there would be a bigger memory
footprint, but it is worthwhile simplifying the hot loop (both for performance
and code simplicity) if the metric is pixel latency.

===============================================================================
Verification Methodology

Mostly through a script (test.sh) that compares processed /dev/urandom data
against the provided implementation for given parameters. Using this manually
and through automatic parameter-fuzzing wrapper script.

===============================================================================
Work Partition

Initial algorithm design by both group members, with some concept whiteboarding
with the group of Oskar Weigl and Thomas Morrison.

Implementation:
Ryan Savitski - sliding window code.  
Yong Wen - main code, synchronisation primitives, pipeline setup, io read/write
           threads, auxiliary functions
           (basically everything except include/window_1d.hpp).

===============================================================================
Noteworthy source files

v2.cpp                - main 
read_write_sync.cpp   - synchronisation primitives
include/window_1d.hpp - algorithm kernel (templated, hence in header)

===============================================================================
Compiling and Running

No dependencies on tbb or opencl, but if compiling under g++ an explicit
-pthread flag needs to be passed in (provided makefile handles this). 

Target binary generated: src/v2.
