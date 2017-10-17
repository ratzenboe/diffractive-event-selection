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

from modules.import_data import import_data_hdf5

batch_size = 128        # number of samples per gradient update
nb_classes = 4          # number of different classes (ND, SD, DD, CD)
nb_epoch = 10           # number of training epochs
nb_datasplits = 10      # datasplits to speed up data->histogram making (=panda-manipulations)
nb_test_evts = 20000    # number of evts in test sample
nb_valid_evts = 5000    # number of evts in validation sample

img_dim = 32            # image dimensions (quadratic)
img_channels = 3        # number of channels of the picture (1 for b/w, 3 for rgb)
pool_size = (2,2)       # size of pooling area for max pooling
prob_drop_conv = 0.2    # drop-out probability at conv layer
prob_drop_hidden = 0.5  # drop-out probability at fc layer
output_folder = '/home/ratzenboe/Documents/imageClassification/plots/'
load_weights_bool = True
use_NN = True

def reduce_y_dimension( y_truth, y_score, class_number ):
    """
    as we have a mulitlabel classification, we need to a binary class. to plot 
    the roc-curve. class_number: (0=ND, 1=SD, 2=DD, 3=CD)
    """

    # y_truth_output = pd.DataFrame(y_truth)
    # y_truth_output = y_truth_output.loc[:,class_number].as_matrix()
    # y_score_output = pd.DataFrame(y_score)
    # y_score_output = y_score_output.loc[:,class_number].as_matrix()
    y_truth_output = y_truth[:,class_number]
    y_score_output = y_score[:,class_number]

    return y_truth_output, y_score_output


def plot_ROC( y_true, y_score, class_number, NN=None):
    "plots the ROC curve"

    #first change to the class number we want to check out
    y_tr_binary, y_score_binary = reduce_y_dimension(y_true, y_score, class_number)

    fpr, tpr, threshold = roc_curve(y_tr_binary, y_score_binary, pos_label=1)
    roc_auc = roc_auc_score(y_tr_binary, y_score_binary)

    titleStr = ''
    if NN is None:
        titleStr = ''
    elif NN:
        titleStr = 'NN'
    else:
        titleStr = 'Conv2D'

    plt.figure()
    plt.plot(fpr,tpr,label='ROC curve (AUC = %0.3f)'%roc_auc)
    plt.plot([0,1],[0,1], 'k--')
    plt.xlim([-0.05, 1.05])
    plt.ylim([-0.05, 1.05])
    plt.xlabel('False Positive Rate')
    plt.ylabel('True Positive Rate')
    plt.title('Receiver operating characteristic curve: '+titleStr)
    plt.grid(True)

    plt.legend(loc=4)
    class_str = ''
    if class_number==0:
        class_str = 'ND'
    if class_number==1:
        class_str = 'SD'    
    if class_number==2:
        class_str = 'DD'    
    if class_number==3:
        class_str = 'CD'    

    plt.savefig(output_folder + class_str + '_' + titleStr + '_roc_curve.pdf')
    
    return

def plot_MVAoutput( y_truth, y_score, class_number, nbins=100, NN=None):
    """
    plots the MVA output as histogram and returns the underlaying
    distributions of the positive and negative class
    """
    
    # split the array and use only the column containing the
    # probabilities of an event to belong the the "signal" class
    # (can be done in case of 'binary' classification)
    # hence: first make it binary
    y_tr_binary, y_score_binary = reduce_y_dimension(y_truth, y_score, class_number)

    y_score_truePos = y_score_binary[np.array(y_tr_binary==1.)]
    y_score_trueNeg = y_score_binary[np.array(y_tr_binary==0.)]

    plt.figure()
    n_total, bins_total, patches_total = plt.hist( y_score_binary,
                                                    bins=nbins,
                                                    alpha=0.5,
                                                    color='black',
                                                    label='MVA output' )
    n_trueNeg, bins_trueNeg, patches_trueNeg = plt.hist(y_score_trueNeg,
                                                        bins=nbins,
                                                        alpha=0.5,
                                                        color='#dd0000',
                                                        label='true negative')

    n_truePos, bins_truePos, patches_truePos = plt.hist(y_score_truePos,
                                                        bins=nbins,
                                                        alpha=0.5,
                                                        color='green',
                                                        label='true positive')
    titleStr = ''
    if NN is None:
        titleStr = ''
    elif NN:
        titleStr = 'NN'
    else:
        titleStr = 'Conv2D'

    plt.title('MVA output distribution (positive class): '+titleStr)
    plt.xlim(-0.05, 1.05)
    plt.xlabel('MVA output')
    plt.ylabel('Entries')
    plt.grid(True)
    plt.yscale('log')
    plt.legend()
    

    class_str = ''
    if class_number==0:
        class_str = 'ND'
    if class_number==1:
        class_str = 'SD'    
    if class_number==2:
        class_str = 'DD'    
    if class_number==3:
        class_str = 'CD'    

    plt.savefig(output_folder + class_str + '_' + titleStr + '_MVA_output_distr.pdf')

    return n_truePos, n_trueNeg

def conv2D_model(   filter_sizes=[32,64,128],
                    kernel_size=(3,3), 
                    pool_size=(2,2), 
                    dropout1=0.25, 
                    dropout2=0.5, 
                    nb_classes=4,
                    activation='relu',
                    input_shape=(img_dim, img_dim, 3),
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
                input_shape = (img_dim, img_dim, 3) ):
    
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

# ------------------ main ------------------------
def main():

    # if we want to test something, it is better to use the pretrained models:
    model_path = '/home/ratzenboe/Documents/imageClassification/gettingStarted/'
    if use_NN:
        if img_channels is 3: 
            model_fn = 'NN_3d_weights.h5'
        else:
            model_fn = 'NN_weights.hdf5'
    elif img_channels is 3:
        model_fn = 'cnn_3d_weights.h5'
    else:
        model_fn = 'cnn_weights.hdf5'
    # load data from files:
    print 'loading data...'
    if load_weights_bool and (os.path.isfile(model_path+model_fn)):
        x_test, y_test = import_data_hdf5( only_testvalid=True )
    else:
        x_train, y_train, x_valid, y_valid, x_test, y_test = import_data_hdf5(only_testvalid=False)
        y_train = keras.utils.to_categorical(y_train, nb_classes)
        y_valid = keras.utils.to_categorical(y_valid, nb_classes)


    print 'building model...'
    if use_NN:
         model = NN_model(  nb_layers=4, 
                            nb_classes=4, 
                            slope=1.2,
                            layer_size=150, 
                            activation='relu', 
                            dropout_rate=0.5,
                            kernel_initializer='glorot_normal', 
                            bias_initializer='glorot_normal',
                            normalize_batch=True, 
                            input_shape=(img_dim, img_dim, img_channels) )
    else:
        model = conv2D_model(   filter_sizes=[32,64,128],
                                kernel_size=(3,3), 
                                pool_size=pool_size, 
                                dropout1=0.25, 
                                dropout2=0.5, 
                                nb_classes=4,
                                input_shape=(img_dim, img_dim, img_channels),
                                normalize_batch=True )

    # convert class vectors to binary class matrices
    if load_weights_bool and (os.path.isfile(model_path+model_fn)):
        # load weights, to load weights into different architecture use by_name=True
        print 'using pretrained model!'
        model.load_weights(model_path+model_fn)
    else:
        print 'training model on imported data...'
        model.fit( x_train, y_train,
                    batch_size=batch_size,
                    shuffle=True,
                    epochs=nb_epoch,
                    verbose=1,
                    validation_data=(x_valid, y_valid) )

        score = model.evaluate( x_valid, y_valid, verbose=0 )
        print 'test loss: {}'.format(score[0])
        print 'test accuracy: {}'.format(score[1])

        # save weights in a file:
        if use_NN:
            weights_file='/home/ratzenboe/Documents/imageClassification/gettingStarted/NN_3d_weights.h5'
        else:
            weights_file='/home/ratzenboe/Documents/imageClassification/gettingStarted/cnn_3d_weights.h5'
        model.save_weights(weights_file)

    print 'making test predictions on new dataset (calc y_score)'
    y_test_score = model.predict( x_test )
    y_test_true = keras.utils.to_categorical(y_test, nb_classes)

    for i in range(100,110):
        print 'y_test_score: {}'.format(y_test_score[i])
        print 'y_test_true: {}'.format(y_test_true[i])

    print 'y_test_score type: {}'.format(type(y_test_score))
    print 'y_test_score shape: {}'.format(y_test_score.shape)
    print 'roc-auc-score: {}'.format( roc_auc_score(y_test_true, y_test_score) )


    plot_ROC(y_test_true, y_test_score, class_number=3, NN=use_NN)
    plot_ROC(y_test_true, y_test_score, class_number=2, NN=use_NN)
    plot_ROC(y_test_true, y_test_score, class_number=1, NN=use_NN)
    plot_ROC(y_test_true, y_test_score, class_number=0, NN=use_NN)
    plot_MVAoutput(y_test_true, y_test_score, nbins=100, class_number=0, NN=use_NN)
    plot_MVAoutput(y_test_true, y_test_score, nbins=100, class_number=1, NN=use_NN)
    plot_MVAoutput(y_test_true, y_test_score, nbins=100, class_number=2, NN=use_NN)
    plot_MVAoutput(y_test_true, y_test_score, nbins=100, class_number=3, NN=use_NN)
    #print 'Type of saved root data: {}'.format(type( data ))
    #print 'Data shape: {}'.format(data.shape)
    #print 'Max evt Number: {}'.format(maxEvt)

    return

if __name__ == "__main__":
    main()

