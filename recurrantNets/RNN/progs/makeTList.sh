#!/bin/bash

ROOTFILESDIR=/media/hdd/train_files/raw_trees_from_grid/old_files/
EXEDIR=/home/ratzenboe/Documents/ML_master_thesis/ML-repo/recurrantNets/RNN/data/grid_runners/
OUTPUTDIR=/media/hdd/train_files/raw_trees_from_grid/output_dir/
nev=100000
label=0


if [ $# -ne 0 ]; then
    for i in "$@"
    do
    case $i in
        -rootdir=*)
        ROOTFILESDIR="${i#*=}"
        shift # past argument=value
        ;;
        -exedir=*)
        EXEDIR="${i#*=}"
        shift # past argument=value
        ;;
        -outputdir=*)
        OUTPUTDIR="${i#*=}"
        shift # past argument=value
        ;; 
        -nev=*)
        nev="${i#*=}"
        shift # past argument=value
        ;;
        -label=*)
        label="${i#*=}"
        shift # past argument=value
        ;;
    esac
    done
fi


echo 
echo "----------------------------------------------------------"
echo "label: "${label}
aliroot -b -l -q "${EXEDIR}CEPBuffersToList.C(\"${ROOTFILESDIR}\",\"${OUTPUTDIR}\",\"${EXEDIR}CEPfilters.C\",${nskip},${nev},TString(\"${label}\"),${filter})"   2>&1 | tee treetrafo.log

exit
