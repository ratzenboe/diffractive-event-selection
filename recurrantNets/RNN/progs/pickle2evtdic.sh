#!/bin/bash

# parse command line arguments 
if [ $# -ne 0 ]; then
    for i in "$@"
    do
    case $i in
        -exedir=*)
        EXEDIR="${i#*=}"
        shift # past argument=value
        ;;
        -outputdir=*)
        OUTPUTDIR="${i#*=}"
        shift # past argument=value
        ;; 
        -runmode=*)
        runmode="${i#*=}"
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
echo "Converting the pickle files into an event dictionary..."

source /home/ratzenbo/envs/MLenv/bin/activate

python ${EXEDIR}pickle2evtdic.py -filespath ${OUTPUTDIR}output_pickle_files/ -config_path ${EXEDIR}data_params.conf -base ${basevalue} -nfiles ${nfilesinbatch} -run_mode_user ${runmode} 2>&1 | tee pickle2evtdic.log


exit
