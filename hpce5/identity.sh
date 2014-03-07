#!/bin/bash
set -eu
width=$((2<<12))
height=$((2<<12))
bits=8
copies=1
levels=3
HPCE_DEBUG="true"

if [[ $# -gt 0 ]]; then
	width=$1;
fi
if [[ $# -gt 1 ]]; then
	height=$2;
fi
if [[ $# -gt 2 ]]; then
	bits=$3;
fi
if [[ $# -gt 3 ]]; then
	copies=$4;
fi
if [[ $# -gt 4 ]]; then
	levels=$5;
fi
echo "$width X $height $bits bits $copies copies $levels levels"
make && ./produce.sh $width $height $bits $copies >  test.dat
cat test.dat | src/v2 $width $height $bits $levels > actual.dat
cmp test.dat actual.dat
