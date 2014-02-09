#!/bin/bash
set -eu

tempdir=$(mktemp -d "../tmp/corrupt.XXXXXXXXXX") ||\
	{ echo "Failed to create temp directory"; exit 1; }
trap "rm -rf $tempdir" EXIT

# 				OUT
#		1				2
#	3		4		5		STDIN
# 500 600 800 1000 1200 1400

pipes=()
for i in {1..5}; do
		mkfifo "$tempdir/pipe$i"
done

./merge "$tempdir/pipe1" "$tempdir/pipe2" &\
	./merge "$tempdir/pipe3" "$tempdir/pipe4" > "$tempdir/pipe1" &\
	./merge "$tempdir/pipe5" <(cat -) > "$tempdir/pipe2" &\
	./merge <(./signal_generator 500) <(./signal_generator 600) > "$tempdir/pipe3" &\
	./merge <(./signal_generator 800) <(./signal_generator 1000) > "$tempdir/pipe4" &\
	./merge <(./signal_generator 1200) <(./signal_generator 1400) > "$tempdir/pipe5"
