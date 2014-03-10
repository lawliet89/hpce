HPCE Courswork 5
Ryan Savitski and Yong Wen Chua
===============================================================================
Algorithms Overview

The application is designed as a system of independent modules that would work
well on their own as separate threads. These disparate threads can then be
strung together to form a multi-threaded application through the use of buffers
and synchronisation primitives.

Each module has a producer/consumer relationship with its upstream producer
and downstream consumer. Synchronisation is performed by a consistent set
of APIs and procedures using the ReadWriteSync class which essentially
encapsulates some mutex, conditional variables and locks, along with methods
for performing synchronisation using these primitives in an abstract manner.

Performing a n levels erosion or dilation can be thought of as a minimisation
or maximisation of the n Manhattan Distance Von Neumann neighbourhood of some
pixel. Since minimisation and maximisation are both associative, and
commutative, it does not matter which order the are performed.

Consider the each pass of dilation or erosion with n levels. For some pixel,
information for pixels n rows above and below would be necessary. As such, the
buffer holding intermediate max/min results would have to be at least be able to
2n rows of pixels plus the pixel itself. This is the minimum size of the buffer
between each module. The buffer size is rounded up to a power of two to try and
take advantage of the memory cache system, as well as to easily facillitate
calculating a factor of the buffer size.

Work is performed from the first module all the way to the last module in
units of chunks. For each chunk read from stdin, the passes would process
the chunks in turn, and finally written out to stdout as a full chunk. The
buffers are implemented as circular buffers, and to facillitate operations,
the chunk size should be a factor of the buffer size. As the buffer size is a
power of two, any powers of two smaller than the buffer size is necessarily
one of its factors. Thus, the chunk size is calculated this way, subjected to
a floor and ceiling value.

For each pixel, the threads handling the reading and writing from stdin and
stdout respectively will process a chunk, and then synchronise with its
downstream and upstream modules respectively. In particular, the modules have
to be mindful of the end of an image due to the fact that chunk size might not
necessarily be a factor of the image size. They have to be mindful not to expect
more bytes than is avaialble and write more bytes than required.

For the purposes of the discussion below, we will consider an erosion pass --
dilation will be analogous.

Consider an n levels erosion operation and the corresponding n distance Von
Neumann neighbourhood of some pixel in the shape of a diamond. Minimisation
of pixel values will first be done row-wise and then accumulated for each
column until the pixel at the bottom of the diamond at which point the pixel
would have been fully processed. If we think in terms of the overlapping diamonds
for every pixel in an image, we can see that each pixel is also part of a row of
the diamond of 2n other pixels. To process each pixel would mean to process
2n+1 minimisations over window sizes ranging from 1 to n+1. This window
minimisation can be performed efficiently in O(1) amortised using the algorithm
presented in http://goo.gl/eXGis3. Also, notice that except for the window of
size 2n+1, the minimisation done for an upper diamond of one pixel is essentially
that of a lower diamond of another pixel. Thus the work done can be almost halved.

The erosion and dilation modules will perform their respective minimisation and
maximisation described above. It also handles synchronisation using the primitives
described above. Finally, they also have to "prime" and "drain" their process
pipeline accordingly.

===============================================================================
Verification Methodology

Corectness is crucial in the operation of the algorithm. Correctness is checked
by performing a binary comparison of the ouputs from the reference implementation
and our implementation using an identical randomly generated input from
/dev/urandom. Tests were performed for images with dimensions of a power of two,
and "odd" sizes using the various levels.
===============================================================================
Work Partition

Conceptualisation of the algorithm design, and overall design took place
initially between the two of us.  After we had a fair idea of how the design
should look like, we then proceed to split the work accordingly.

Ryan would be in charge of coding up the eroding and dilating passes and Yong
Wen would be in charge of coding the overall structure and "glue" of the
application, including synchronisation of the threads.

The initial plan is to get a serial version of the algorithm working, with no
threading involved. Once that was acheived, the second step was to glue the
individual components as separate threads together via various synchronisation
mechanisms.

Testing of the algorithms correctness was done by Ryan as he implemented the
passes and overall testing for concurrency issues was done by Yong Wen.
===============================================================================
Compiling and Running

The code files should be compiled with the flag -pthread because of the use of
the std::thread library. The Makefile will compile the files properly into
src/v2.
