#!/bin/bash
curl $1 | ../local/bin/lame --mp3input --decode -t - -
