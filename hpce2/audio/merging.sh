#!/bin/bash
set -eu

freq1=
freq2=
main() {
	if [[ $# -eq 0 ]]; then
		freq1=1000
		freq2=500
	elif [[ $# -eq 1 ]]; then
		freq1=$1
		freq2=500
	else
		freq1=$1
		freq2=$2
	fi

	echo "Merging $freq1 Hz with $freq2 Hz"
	./merge <(./signal_generator $freq1) <(./signal_generator $freq2)
}

main "$@"
