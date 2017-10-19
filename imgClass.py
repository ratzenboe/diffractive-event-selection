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
from modules.plot import plot_ROC, plot_MVAoutput, plot_ROC_MVA_eff_sign_allDiff
from modules.models import conv2D_model, NN_model
from modules.inputVars import set_variables
from modules.load_save_data import heaviside_numpy

batch_size = 128        # number of samples per gradient update
nb_classes = 4          # number of different classes (ND, SD, DD, CD)
nb_epoch = 5           # number of training epochs
nb_datasplits = 10      # datasplits to speed up data->histogram making (=panda-manipulations)
nb_test_evts = 20000    # number of evts in test sample
nb_valid_evts = 5000    # number of evts in validation sample

img_dim = 32            # image dimensions (quadratic)
pool_size = (2,2)       # size of pooling area for max pooling
prob_drop_conv = 0.2    # drop-out probability at conv layer
prob_drop_hidden = 0.5  # drop-out probability at fc layer
output_folder = '/home/ratzenboe/Documents/imageClassification/plots/'

# ### ---------------------input variables that change main ------------------------
# load_weights_bool = True
# use_NN = False
# img_channels = 1            # number of channels of the picture (1 for b/w, 3 for rgb)
# only_position_info = False
# ### ------------------------------------------------------------------------------
# ------------------------------------- main -------------------------------------
def main():
    ## ------------------- determine what we want to do --------------------------
    use_NN, load_weights_bool, img_channels, only_position_info, eta_LT09 = set_variables()
    # ----------------------------------------------------------------------------
    # --------------------- choose model and file path ---------------------------
    trainTestValidFile='/media/hdd/eventInfo/train_valid_test'
    if eta_LT09:
        trainTestValidFile+='_etaLT09'
    # if we want to test something, it is better to use the pretrained models:
    model_path = '/home/ratzenboe/Documents/imageClassification/gettingStarted/'
    if use_NN:
        model_fn = 'NN_3d_weights'
    else:
        model_fn = 'cnn'
    if img_channels == 3:
        model_fn += '_3d_weights'
        trainTestValidFile+='_3d'
    else:
        model_fn += '_weights'

    if eta_LT09:
        model_fn += '_etaLT09.h5'

    trainTestValidFile += '.h5'
    model_fn += '.h5'
    # ----------------------------------------------------------------------------
    # --------------------- load data from files ---------------------------------
    print 'loading data...'
    if load_weights_bool and (os.path.isfile(model_path+model_fn)):
        x_test, y_test = import_data_hdf5( inFile=trainTestValidFile, 
                                           only_testvalid=True )
        if only_position_info:
            x_test = heaviside_numpy(x_test)
    else:
        x_train, y_train, x_valid, y_valid, x_test, y_test = import_data_hdf5(
                                                             inFile=trainTestValidFile,
                                                             only_testvalid=False )
        if only_position_info:
            x_train = heaviside_numpy(x_train)
            x_valid = heaviside_numpy(x_valid)
            x_test = heaviside_numpy(x_test)

        y_train = keras.utils.to_categorical(y_train, nb_classes)
        y_valid = keras.utils.to_categorical(y_valid, nb_classes)
    # ----------------------------------------------------------------------------
    


    # ------------------- build model, and then fit it ---------------------------
    print 'building model...'
    if use_NN:
         model = NN_model(  nb_layers=4, 
                            nb_classes=4, 
                            slope=1.,
                            layer_size=100, 
                            activation='relu', 
                            dropout_rate=0.25,
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
                                normalize_batch=False )

    # convert class vectors to binary class matrices
    if load_weights_bool and (os.path.isfile(model_path+model_fn)):
        # load weights, to load weights into different architecture use by_name=True
        print 'using pretrained model!'
        model.load_weights(model_path+model_fn)
    else:
        print 'training model on imported data...'
        model.fit(  x_train, y_train,
                    batch_size=batch_size,
                    shuffle=True,
                    epochs=nb_epoch,
                    verbose=1,
                    validation_data=(x_valid, y_valid) )

        score = model.evaluate( x_valid, y_valid, verbose=0 )
        print 'test loss: {}'.format(score[0])
        print 'test accuracy: {}'.format(score[1])

        # save weights in a file:
        weights_file=model_path+model_fn
        model.save_weights(weights_file)
    # ---------------------------------------------------------------------------------
    # -------------------- test prediction on new dataset -----------------------------
    print '\nmaking test predictions on new dataset (calc y_score)'
    y_test_score = model.predict( x_test )
    y_test_true = keras.utils.to_categorical(y_test, nb_classes)

    for i in range(100,110):
        print 'y_test_score: {}'.format(y_test_score[i])
        print 'y_test_true: {}'.format(y_test_true[i])

    print '\nroc-auc-score: {}'.format( roc_auc_score(y_test_true, y_test_score) )
    # ---------------------------------------------------------------------------------
    # -------------- plot ROC MVA output, effs and significance -----------------------

    print '\ncreating ROC-curves, plotting MVA outputs and significane...'
    plot_ROC_MVA_eff_sign_allDiff( y_test_true, y_test_score, nbins=100, useNN=use_NN )
    # ---------------------------------------------------------------------------------

    return

if __name__ == "__main__":
    main()

