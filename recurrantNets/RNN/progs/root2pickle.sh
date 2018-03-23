#!/bin/bash

if [ $# -ne 0 ]; then
    for i in "$@"
    do
    case $i in
        -exedir=*)
        EXEDIR="${i#*=}"
        shift # past argument=value
        ;;
        -filespath=*)
        OUTPUTDIR="${i#*=}"
        shift # past argument=value
        ;; 
        -base=*)
        basevalue="${i#*=}"
        shift # past argument=value
        ;; 
        -nfiles=*)
        nfilesinbatch="${i#*=}"
        shift # past argument=value
        ;; 
    esac
    done
fi


echo 
echo "----------------------------------------------------------"
echo "Converting the root files into pickle format..."

source /home/ratzenboe/envs/tfenv/bin/activate
source /home/ratzenboe/root/newBuild/bin/thisroot.sh
python ${EXEDIR}root2pickle.py -filespath ${OUTPUTDIR} -base ${basevalue} -nfiles ${nfilesinbatch} -all_files 2>&1 | tee root2pickle.log

exit
