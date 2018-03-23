#!/bin/bash

file1=/data1/pbuehler/alice/Pythia8CEPBuffers/all_feature_plots_1.root
file2=/home/sratzenboeck/output_plotTList/all_feature_plots_2.root
EXEDIR=/home/sratzenboeck/progs/
OUTPUTDIR=/home/sratzenboeck/output_ratioplts/

# parse command line arguments 
if [ $# -ne 0 ]; then
    for i in "$@"
    do
    case $i in
        -file1=*)
        file1="${i#*=}"
        shift # past argument=value
        ;;
        -file2=*)
        file2="${i#*=}"
        shift # past argument=value
        ;;
        -outputdir=*)
        OUTPUTDIR="${i#*=}"
        shift # past argument=value
        ;; 
        -exedir=*)
        EXEDIR="${i#*=}"
        shift # past argument=value
        ;;
    esac
    done
fi

if [ ! -f $file1 ] || [ ! -f $file2 ]; then
    echo "File " $file1 "or " $file2 " not found!"
    exit
fi

if [[ "$EXEDIR" != */ ]]; then
    EXEDIR="${EXEDIR}/"
fi

if [[ "$OUTPUTDIR" != */ ]]; then
    OUTPUTDIR="${OUTPUTDIR}/"
fi

if [ ! -d ${OUTPUTDIR} ]; then
    mkdir ${OUTPUTDIR}
fi

aliroot -b -l -q "${EXEDIR}CreateRatioPlots.C(\"${file1}\",\"${file2}\",\"${OUTPUTDIR}\")" 2>&1 | tee ratioplot.log






