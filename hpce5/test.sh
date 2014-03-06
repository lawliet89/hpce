#!/bin/bash
set -eu

width=$((2<<12))
height=$((2<<12))
bits=8
levels=1
copies=1

if [[ $# -gt 1 ]]; then
	width=$2;
fi
if [[ $# -gt 2 ]]; then
	height=$3;
fi
if [[ $# -gt 3 ]]; then
	bits=$4;
fi
if [[ $# -gt 4 ]]; then
	copies=$5;
fi
if [[ $# -gt 5 ]]; then
	levels=$6;
fi

bytes=$(($width*$height*$bits/8))

make process
make "$1"

tempdir=$(mktemp -d "/tmp/process.XXXXXXXXXX") ||\
	{ echo "Failed to create temp directory"; exit 1; }
trap "rm -rf $tempdir" EXIT

expected="$tempdir/expected"
actual="$tempdir/actual"

echo "$width X $height $bits bits $copies copies $levels levels"

(./produce.sh $width $height $bits $copies |\
	tee  >(./process $width $height $bits $levels > "$actual") |\
 	"$1" $width $height $bits $levels > "$expected")

echo "Comparing..."
cmp "$actual" "$expected" && echo "Identical"
