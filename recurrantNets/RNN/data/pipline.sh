#!/bin/bash
# Script that walkes through the pipeline and executes 
# the steps the user agrees upon

export CURRENTPATH=$(pwd)
# standard 
if [ -z "${ALICE_PHYSICS}" ]; then
    echo
    echo "   Please execute this script only in the alice environment (start it with 'alienter')"
    echo
    exit
fi

while true; do
    echo
    read -p "   Do you wish to create the raw files? ([y]/n)" yn
    case $yn in
        [Nn]* ) break;;
            * ) cd ${CEPDIR}/runners
                aliroot -q -b ${CEPDIR}/runners/runCEPAna_PYTHIA.C\(\"local\",\"test\"\)
                mv ${CEPDIR}/runners/AnalysisResults.root /media/hdd/train_files/raw_trees_from_grid/.
                cd ${CURRENTPATH}
                break;;
    esac
done

array=(/media/hdd/train_files/raw_trees_from_grid/*.root)
if [ ${#array[@]} -eq 0 ]; then
    echo
    echo "   Please move the raw files to /media/hdd/train_files/raw_trees_from_grid/"
    echo
    exit
fi

while true; do
    echo
    read -p "   Do you wish to transform the raw file buffer to a linear tree? ([y]/n)" yn
    case $yn in
        [Nn]* ) break;;
            * ) for file in "${array[@]}"; do 
                    aliroot -q -b '/home/ratzenboe/Documents/ML_master_thesis/ML-repo/recurrantNets/RNN/data/CEPBuffersToTTree.C("'$file'")'
                done
                break;;
    esac
done

