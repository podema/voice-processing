#!/bin/bash

if [[ $# != 2 ]]; then
   echo "$0 input.wav output.mcp"
   exit 1
fi

# Put dir of SPTK in front of the path, to be sure the sptk are used.
# E.g: lpc: we want to use the sptk linear prediction 
#           and not the CUPS line printer control.

dirsptk=$(which x2x)
dirsptk=${dirsptk%\/x2x}
if [[ $? != 0 ]]; then
   echo "SPTK not found in path ...; install and check PATH"
   exit 1
fi
PATH="$dirsptk:$PATH"



# TODO
# This is a very trivial feature extraction:
# Please, read sptk documentation and some papers,
# and apply a better front end to represent the speech signal


# Compute the LPC Cepstrum using 

sampling_freq=8000
frame_len=240 #30 ms.
frame_shift=120 #15ms
LPC_ORDER=10
NCEPS=12

base=/tmp/wav2mcp$$
sox $1 -r $sampling_freq $base.raw
x2x +sf < $base.raw | frame -l $frame_len -p $frame_shift |\
    window -w 1 -l $frame_len |\
    lpc -l $frame_len -m $LPC_ORDER |\
     lpc2c -m $LPC_ORDER -M $NCEPS > $base.cep

#    x2x +fa | wc -l; exit


ncol=$((NCEPS+1))
nrow=`x2x +fa < $base.cep | wc -l | perl -ne 'print $_/'$ncol', "\n";'`

# Write fmatrix header: nrow and ncol
echo $nrow $ncol | x2x +aI > $2

# Append the data:
cat $base.cep >> $2

\rm -f $base.raw $base.cep
exit 0
