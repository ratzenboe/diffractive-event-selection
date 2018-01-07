import os
import math
import sys

import numpy as np
import root_numpy
import pandas as pd
import h5py

def unison_shuffled_copies( a, b ):
    assert len(a) == len(b)
    p = np.random.permutation(len(a))
    return a[p], b[p]

def load_data( filename, branches=None, start=None, stop=None, selection=None ):
    "load data from a root tree and returns a panda dataframe with the relevant branches"

    data = pd.DataFrame( root_numpy.root2array( filename,
                                                branches=branches,
                                                start=start,
                                                stop=stop,
                                                selection=selection ) )

    nb_entries = data.shape[0]

    return data, nb_entries


def save_data_h5py( h5f, x_data, y_data, train_test_valid ):
    "save histograms in a .h5 file (hdf5-format)"
    
    h5f.create_dataset( 'x_'+train_test_valid, data=x_data, dtype='f' )
    h5f.create_dataset( 'y_'+train_test_valid, data=y_data, dtype='i' )


def load_data(data_params):
    # load data and get number of entries in the tree: 
    print 'fetching training data from {}'.format(train_filename)
    data = {}
    for key in data_params['branches']:
        data[key] = load_data( data_params['path'][key], 
                                      branches=data_params['branches'][key] )[0]

    print 'data loaded!'
    y = data['event'][data_params['target']]
    data['event'].Drop(data_params['target'], axis=1, inplace=True)

    h5f = h5py.File(data_params['data_h5_storage'], 'w')
    save_data_h5py(h5f, data, y, 'data')
    h5f.close()

    return data, y
