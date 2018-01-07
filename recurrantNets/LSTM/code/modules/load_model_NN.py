import os
import os.path
import math

import numpy as np
import pandas as pd
import root_numpy
import matplotlib
import matplotlib.pyplot as plt

import keras
from keras.models import Sequential
from keras.layers import Activation, Dropout, Flatten, Dense
from keras.layers import Conv2D, MaxPooling2D, ZeroPadding2D, BatchNormalization
from keras.optimizers import RMSprop
from keras.preprocessing.image import ImageDataGenerator
from keras import optimizers
from keras import initializers
from keras import backend as K

from sklearn.metrics import roc_auc_score, roc_curve

def NN_model( nb_layers_final=4, 
              nb_layers_hierachical=2,
              nb_classes=2, 
              layer_size_final=100, 
              layer_size_hierachical=50, 
              slope=1.0,
              activation='relu', 
              dropout_rate=0.5,
              kernel_initializer='glorot_normal', 
              bias_initializer='glorot_normal',
              normalize_batch=False,
              input_shape_fmd = (140,) 
              input_shape_ad = (16,) 
              input_shape_v0 = (64,) 
              input_shape_emcal = (None,) 
              input_shape_phos = (None,) 
              input_shape_ = (None,) 
              
              ):
    
    "NN model" 
    
    model = Sequential()
    model.add(Dense(layer_size, 
                    input_shape=input_shape, 
                    activation=activation, 
                    kernel_initializer=kernel_initializer,
                    bias_initializer=bias_initializer))
    model.add(Flatten())

    for layer in range(nb_layers-1):
        if normalize_batch==True:
            model.add(BatchNormalization())

        model.add(Dropout(dropout_rate))
        model.add(Dense(layer_size, 
                        activation=activation, 
                        kernel_initializer=kernel_initializer,
                        bias_initializer = bias_initializer))
        layer_size = int(round(layer_size*slope))
        if layer_size < 1:
            layer_size = 1

    if normalize_batch:
        model.add(BatchNormalization())
    model.add(Dense(nb_classes, activation='softmax'))

    model.compile(loss=keras.losses.categorical_crossentropy,
                    optimizer=keras.optimizers.Adadelta(),
                    metrics=['accuracy'])

    print(model.summary())
    return model
