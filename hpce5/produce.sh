#!/bin/bash
set -eu

width=$((2<<12))
height=$((2<<12))
bits=8
copies=1

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

bytes=$(($width*$height*$bits/8*$copies))

echo "Producing $bytes bytes" 1>&2
head --bytes="$bytes" /dev/urandom
