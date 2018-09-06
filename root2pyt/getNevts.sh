#!/bin/bash

# execute getnEvents.C
out=$(aliroot -b -l -q "${EXEDIR}GetNEventsInTChain.C(\"${ROOTFILESDIR}\")" 2>&1)

# parse the output to extract the number of events
out=($out)
nout=${#out[@]}
cnt=${out[$(($nout-1))]}
cnt=($(echo $cnt | sed "s/)/ /g"))
cnt=${cnt[1]}

echo $cnt
