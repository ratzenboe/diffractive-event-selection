#!/bin/bash

echo 
echo "----------------------------------------------------------"
echo "label: "${label}
aliroot -b -l -q "${EXEDIR}CEPBuffersToTTree_tchain.C(\"${ROOTFILESDIR}\",\"${OUTPUTDIR}\",${nskip},${nev},TString(\"${label}\"))"   2>&1 | tee treetrafo.log

exit
