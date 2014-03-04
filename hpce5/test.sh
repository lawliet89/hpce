#!/bin/bash
set -eu

width=$((2<<12))
height=$((2<<12))
bits=8
levels=1

bytes=$(($width*$height*$bits/8))

make process
make "$@"

tempdir=$(mktemp -d "/tmp/process.XXXXXXXXXX") ||\
	{ echo "Failed to create temp directory"; exit 1; }
trap "rm -rf $tempdir" EXIT

expected="$tempdir/expected"
actual="$tempdir/actual"
mkfifo "$expected"
mkfifo "$actual"

echo "Producing $bytes bytes"
(head --bytes="$bytes" /dev/urandom |\
	tee  >(./process $width $height $bits $levels > "$actual") |\
 	"$@" $width $height $bits $levels > "$expected") &

echo "Comparing"
cmp -l "$actual" "$expected"
