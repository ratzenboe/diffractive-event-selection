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

def conv2D_model(   filter_sizes=[32,64,128],
                    kernel_size=(3,3), 
                    pool_size=(2,2), 
                    dropout1=0.25, 
                    dropout2=0.5, 
                    nb_classes=4,
                    activation='relu',
                    input_shape=(32, 32, 3),
                    normalize_batch=False ):
    "convoltional model builder"

    model = Sequential()
    model.add(Conv2D(   filter_sizes[0], 
                        kernel_size=kernel_size,
                        activation=activation,
                        input_shape=input_shape ))

    model.add(Conv2D(filter_sizes[1], kernel_size, activation=activation))
    model.add(MaxPooling2D(pool_size=pool_size))
    model.add(Dropout(dropout1))
    model.add(Flatten())
    if normalize_batch:
        model.add(BatchNormalization())
    model.add(Dense(filter_sizes[2], activation=activation))
    if normalize_batch:
        model.add(BatchNormalization())
    model.add(Dropout(dropout2))
    model.add(Dense(nb_classes, activation='softmax'))

    model.compile(loss=keras.losses.categorical_crossentropy,
                    optimizer=keras.optimizers.Adadelta(),
                    metrics=['accuracy'])

    print(model.summary())
    return model


def NN_model(   nb_layers=4, 
                nb_classes=4, 
                layer_size=100, 
                slope=1.0,
                activation='relu', 
                dropout_rate=0.5,
                kernel_initializer='glorot_normal', 
                bias_initializer='glorot_normal',
                normalize_batch=False,
                input_shape = (32, 32, 3) ):
    
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
