#!/bin/bash

echo 
echo "----------------------------------------------------------"
echo "Converting the pickle files into an event dictionary..."

python3.6 ${EXEDIR}root2evtdic.py -filespath ${FILESDIR} -config_path ${EXEDIR}data_params.conf -base ${basevalue} -nfiles ${nfilesinbatch} -run_mode_user ${runmode} 2>&1 | tee log_root2evtdic.log


exit
