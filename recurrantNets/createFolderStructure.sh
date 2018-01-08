#!/bin/sh
# script creates folder structure where we store the code, modules and output

echo "Type name of parent directory, followed by [ENTER]:"
read DIRECTORY

if [ ! -d "$DIRECTORY" ]; then
    mkdir -p $DIRECTORY/data
    mkdir -p $DIRECTORY/code/modules
    mkdir -p $DIRECTORY/code/output
    mkdir -p $DIRECTORY/code/config
fi
