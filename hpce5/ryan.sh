#!/bin/bash

make;

(cat temp/input.raw | src/v2 512 512 8 7 > temp/00.raw) &
(cat temp/input.raw | ./process 512 512 8 7 > temp/99.raw) &

echo "waiting for threads...";
wait;
echo "--------------------------------";
echo "comparing:";
cmp -l temp/00.raw temp/99.raw;
