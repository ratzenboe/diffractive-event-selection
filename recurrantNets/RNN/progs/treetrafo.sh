#!/bin/bash

echo 
echo "----------------------------------------------------------"
echo "label: "${label}
aliroot -b -l -q "${EXEDIR}CEPBuffersToTTree_tchain.C(\"${ROOTFILESDIR}\",\"${OUTPUTDIR}\",\"${EXEDIR}CEPfilters.C\",${nskip},${nev},TString(\"${label}\"))"   2>&1 | tee treetrafo.log

exit
