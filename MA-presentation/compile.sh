#!/bin/bash

texfile=$(pwd)/abstract.tex
if [ $# -ne 0 ]; then
    for i in "$@"
    do
    case $i in
        -texfile=*)
        texfile="${i#*=}"
        shift # past argument=value
        ;;
    esac
    done
fi

#parse command line 

OUTPUTPATH='output/'

mkdir -p $OUTPUTPATH

filebase=$(echo $texfile | cut -f1 -d".")
auxfile=$filebase".aux"
auxfile=${auxfile##*/}
echo $auxfile

# pdflatex -output-directory $OUTPUTPATH $texfile && \
# bibtex ${OUTPUTPATH}$auxfile && \
pdflatex -output-directory $OUTPUTPATH $texfile && \
pdflatex -output-directory $OUTPUTPATH $texfile 

if [ $? -eq 0 ]; then
	echo ""
	echo "Compilation successful."
	echo ""
else
	echo ""
	echo "Compilation failed."
	echo ""
fi
