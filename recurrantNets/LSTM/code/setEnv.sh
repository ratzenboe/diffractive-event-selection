#!/bin/sh
# script creates folder structure where we store the code, modules and output

echo "Choose default model: [1=LSTM, 2=RNN, 3=GRU], followed by [ENTER]"
read MODEL
# SVR default and optimized
if [ "$MODEL" = "1" ]; then
    echo "setting MODEL_PARAMS environment to LSTM..."
    export MODEL_PARAMS='LSTM'
elif [ "$MODEL" = "1" ]; then
    echo "setting MODEL_PARAMS environment to RNN..."
    export MODEL_PARAMS='RNN'
# GBR default and optimized
elif [ "$MODEL" = "2" ]; then
    echo "setting MODEL_PARAMS environment to GRU..."
    export MODEL_PARAMS='GRU'
else
    echo "Not a valid input, no environment varibales have been set!"
fi

echo "Choose dataset: [1=PYTHIA8-ownSim, 2=gridSim], followed by [ENTER]"
read DATA

if [ "$DATA" = "1" ]; then
    echo "setting DATA_PARAMS environment to PYTHIA8-ownSim..."
    export DATA_PARAMS='P8own'
elif [ "$DATA" = "2" ]; then
    echo "setting DATA_PARAMS environment to grid simulations..."
    export DATA_PARAMS='GridSim'
else
    echo "Not a valid input, no environment varibales have been set!"
fi
