#!/bin/bash

echo 
echo "----------------------------------------------------------"
echo "label: "${label}
echo "filter: "${filter}
aliroot -b -l -q "${EXEDIR}CEPBuffersToList.C(\"${ROOTFILESDIR}\",\"${OUTPUTDIR}\",\"${EXEDIR}CEPfilters.C\",${nskip},${nev},TString(\"${label}\"),${filter})"   2>&1 | tee treetrafo.log

exit
