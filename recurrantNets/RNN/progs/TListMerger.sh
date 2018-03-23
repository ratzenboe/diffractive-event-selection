#!/bin/bash

if [ $# -ne 0 ]; then
    for i in "$@"
    do
    case $i in
        -filesdir=*)
        filesdir="${i#*=}"
        shift # past argument=value
        ;; 
    esac
    done
fi


echo 
echo "----------------------------------------------------------"
echo "Merging the histograms..."

aliroot -b -l -q "${EXEDIR}MergeTLists.C(\"${filesdir}\")"   2>&1 | tee histmerger.log

exit
