#!/bin/bash
if [[ $# < 1 ]]; then
echo "identify yourself with -u option"
echo "select the audio credential file with -f option"
fi

for cmd in $*
if [[$cmd==-u]]; then
echo "ok"
fi
done

