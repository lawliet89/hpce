HPCE Courswork 5
Ryan Savitski and Yong Wen Chua
===============================================================================
Algorithms Overview

The application is designed as a system of independent modules that would work
well on their own as separate threads.
===============================================================================
Verification Methodology
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
