#!/bin/bash
../local/bin/sox -t raw -b 16 -c 2 -e signed-integer -r 44.1k -L - -d
