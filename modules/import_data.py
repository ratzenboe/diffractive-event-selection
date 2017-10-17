import os

import numpy as np
import pandas as pd
import root_numpy
import h5py

from modules.load_save_data import get_XY_data, load_data

img_dim = 32            # image dimensions (quadratic)

def import_data( from_hist_file=True, test_folder=None, valid_folder=None, only_testvalid=False ):
    "load data from a root tree and returns a panda dataframe with the relevant branches"
    
    if test_folder is None:
        test_folder = '/media/hdd/eventInfo/testFiles/'
        valid_folder = '/media/hdd/eventInfo/validationFiles/'
        finalTest_folder = '/media/hdd/eventInfo/'

    if from_hist_file and not only_testvalid:
        print 'loading test histograms'
        x_train = np.loadtxt(test_folder+'test_hists.txt')
        y_train = np.loadtxt(test_folder+'test_class.txt')
        
        print 'loading validation histograms'
        x_valid = np.loadtxt(valid_folder+'validation_hists.txt')
        y_valid = np.loadtxt(valid_folder+'validation_class.txt')

        print 'loading final validation histograms'
        x_test = np.loadtxt(finalTest_folder+'finalvalidation_hists.txt')
        y_test = np.loadtxt(finalTest_folder+'finalvalidation_class.txt')
        
        # files loaded, now they are shaped into the right size for tensorflow
        x_train = x_train.reshape(x_train.shape[0]/img_dim, img_dim, img_dim, 1)
        y_train = y_train.reshape(y_train.shape[0])

        x_valid = x_valid.reshape(x_valid.shape[0]/img_dim, img_dim, img_dim, 1)
        y_valid = y_valid.reshape(y_valid.shape[0])

        x_test = x_test.reshape(x_test.shape[0]/img_dim, img_dim, img_dim, 1)
        y_test = y_test.reshape(y_test.shape[0])


    elif from_hist_file and only_testvalid:
        print 'loading final validation hist:'
        x_test = np.loadtxt(finalTest_folder+'finalvalidation_hists.txt')
        y_test = np.loadtxt(finalTest_folder+'finalvalidation_class.txt')
 
        x_test = x_test.reshape(x_test.shape[0]/img_dim, img_dim, img_dim, 1)
        y_test = y_test.reshape(y_test.shape[0])

    else:
        print 'histograms are generated:'
        train_filename = test_folder+'etaPhiPt_allDiffMixed.root'
        valid_filename = valid_folder+'etaPhiPt_allDiffMixedValid.root'
        train_data = load_data( train_filename, selection='eventID<20000')
        validation_data = load_data( valid_filename, selection='eventID<5000' )
        test_data = load_data( train_filename, selection='eventID>30000 && eventID<35000')

        x_train, y_train = get_XY_data( train_data )
        x_valid, y_valid = get_XY_data( validation_data )
        x_test, y_test = get_XY_data( test_data )
        del train_data, validation_data, test_data

        x_train = x_train.reshape( x_train[0], img_dim, img_dim, 3 )
        x_valid = x_valid.reshape( x_valid[0], img_dim, img_dim, 3 )
        x_test = x_test.reshape( x_test[0], img_dim, img_dim, 3 )
 
    if only_testvalid:
        return x_test, y_test
    else:
        return x_train, y_train, x_valid, y_valid, x_test, y_test


def import_data_hdf5(inFile=None, only_testvalid=False ):
    "load data from a root tree and returns a panda dataframe with the relevant branches"
    
    if inFile is None:
        inFile = '/media/hdd/eventInfo/train_valid_test.h5'

    h5f = h5py.File(inFile, 'r')
    print 'loading test histograms'
    x_train = h5f['x_train'][:]
    y_train = h5f['y_train'][:]
    
    print 'loading validation histograms'
    x_valid = h5f['x_valid'][:]
    y_valid = h5f['y_valid'][:]

    print 'loading final validation histograms'
    x_test = h5f['x_test'][:]
    y_test = h5f['y_test'][:]

    if only_testvalid:
        return x_test, y_test
    else:
        return x_train, y_train, x_valid, y_valid, x_test, y_test

