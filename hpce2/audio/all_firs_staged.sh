#!/bin/bash
set -eu

tempdir='../tmp'
stage1=$(mktemp "$tempdir/XXXXXXXXXX")
stage2=$(mktemp "$tempdir/XXXXXXXXXX")
stage3=$(mktemp "$tempdir/XXXXXXXXXX")
stage4=$(mktemp "$tempdir/XXXXXXXXXX")
stage5=$(mktemp "$tempdir/XXXXXXXXXX")

trap "rm -f $stage1 $stage2 $stage3 $stage4 $stage5" EXIT

cat - | ./fir_filter coeffs/f500.csv > "$stage1"
cat "$stage1" | ./fir_filter coeffs/f600.csv > "$stage2"
cat "$stage2" | ./fir_filter coeffs/f800.csv > "$stage3"
cat "$stage3" | ./fir_filter coeffs/f1000.csv > "$stage4"
cat "$stage4" | ./fir_filter coeffs/f1200.csv > "$stage5"
cat "$stage5" | ./fir_filter coeffs/f1400.csv
