#!/bin/bash

echo 
echo "----------------------------------------------------------"
echo "Converting the root files into pickle format..."
echo "Filesdir: "$FILESDIR

python2.7 ${EXEDIR}root2pickle.py -filespath ${FILESDIR} -base ${basevalue} -nfiles ${nfilesinbatch} 2>&1 | tee log_root2pickle.log

exit
