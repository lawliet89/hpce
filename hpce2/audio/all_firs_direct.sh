#!/bin/bash
set -eu

cat - | ./fir_filter coeffs/f500.csv | ./fir_filter coeffs/f600.csv |\
	./fir_filter coeffs/f800.csv | ./fir_filter coeffs/f1000.csv |\
	./fir_filter coeffs/f1200.csv | ./fir_filter coeffs/f1400.csv
