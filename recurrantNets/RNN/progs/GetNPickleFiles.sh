#!/bin/bash

out=$(python ${EXEDIR}GetNPickleFiles.py -filespath ${FILESDIR} 2>&1)
echo $out
