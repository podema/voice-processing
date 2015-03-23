#!/bin/bash
(perl -pe 'print rand, "\t"' $* | sort | cut -f 2) || exit 1
exit 0

