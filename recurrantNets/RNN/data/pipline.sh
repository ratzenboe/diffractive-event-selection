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

files_array=(/home/ratzenboe/Documents/CEP_ALICE/runners/files_*.txt)
echo "   Processing these files $files_array"
while true; do
    echo
    read -p "   Do you wish to create the raw files? ([y]/n)" yn
    case $yn in
        [Nn]* ) break;;
            * ) cd ${CEPDIR}/runners
                # as we create the the raw files we have to move all
                # current files out of the directroy because we dont want 
                # duplicates to occur
                mv /media/hdd/train_files/raw_trees_from_grid/AnalysisResults*.root /media/hdd/train_files/raw_trees_from_grid/old_files/.
                read -p "   How may events: " NEVTS
                NEVTS=${NEVTS:-500000}
                echo "   Creating $NEVTS events..."
                # looping through all files with files*.txt
                for file in "${files_array[@]}"; do
                    echo "   Using events in $file"
                    # the next line is possible as we are in the 
                    # right directory
                    mv $file files.txt
                    aliroot -q -b ${CEPDIR}/runners/runCEPAna_PYTHIA.C\(\"local\",\"test\",true,true,$NEVTS\)
                    mv ${CEPDIR}/runners/AnalysisResults.root /media/hdd/train_files/raw_trees_from_grid/AnalysisResults_$(date +%F-%H-%M).root
                    mv files.txt $file
                done
                # in the next step we have to be in the current path
                cd ${CURRENTPATH}
                break;;
    esac
done

array=(/media/hdd/train_files/raw_trees_from_grid/AnalysisResults*.root)
if [ ${#array[@]} -eq 0 ]; then
    echo
    echo "   Please move the raw files to /media/hdd/train_files/raw_trees_from_grid/"
    echo
    exit
fi
COUNTER=0
while true; do
    echo
    read -p "   Do you wish to transform the raw file buffer to a linear tree? ([y]/n)" yn
    case $yn in
        [Nn]* ) break;;
            * ) for file in "${array[@]}"; do 
                    # aliroot -q -b '/home/ratzenboe/Documents/ML_master_thesis/ML-repo/recurrantNets/RNN/data/CEPBuffersToTTree.C("'$file'")'
                    # the next line will hopefully work but it is not tested yet, therefore we still use the line above
                    aliroot -q -b '/home/ratzenboe/Documents/ML_master_thesis/ML-repo/recurrantNets/RNN/data/CEPBuffersToTTree.C("'$file'", '$COUNTER')'
                    COUNTER=$[$COUNTER +1]
                    mv $file /media/hdd/train_files/raw_trees_from_grid/old_files/.
                done
                break;;
    esac
done

