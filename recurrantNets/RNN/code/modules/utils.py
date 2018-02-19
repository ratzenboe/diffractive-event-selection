from __future__ import division

import sys
import os
import time
import random
import warnings

from select import select

import numpy as np
import pandas as pd

def print_number(y, name, **kwargs):

    print'Dimensions of {} vector y: {}'.format(name, y.shape)

    for key in kwargs.keys():
        item = kwargs[key]
        print'Number of {}: {} ({:.2f} percent)'.format(key, y[y==item].shape[0], 
                                                   y[y==item].shape[0]*100/y.shape[0])
    
    return


def pause_for_input(message, timeout=5):
    print(message)
    print'      (Program paused for {} seconds. Hit Enter to abort the program.)'.format(timeout)
    rlist = select([sys.stdin], [], [], timeout)[0]

    if rlist:
        print'Input received. Exit the program...'
        sys.exit()
    else:
        print'No input received. Continue running the program...'
    
    return


def get_output_paths(run_params):
    """
    Ensures that all output paths exist before returning them as strings.
    """

    output_prefix = 'output/' + run_params['data_params_id'] +'/'
    model_saves = run_params['classifier_params_id'] + '/model_saves/'
    
    if not os.path.exists(output_prefix):
        os.makedirs(output_prefix)

    if not os.path.exists(output_prefix + model_saves):
        os.makedirs(output_prefix + model_saves)
        
    return output_prefix, model_saves


def print_dict(dictionary, headline=None):
    """
    Prints the contents of a dictionary to the terminal, line by line.
    """

    if headline:
        print'\n{}'.format(headline)
            
    for item in dictionary:
        print'  {:25}: {}'.format(item, dictionary[item])

    return


def split_dictionary(evt_dictionary, split_size):
    """
    Args
        evt_dictionary:
            dictionary, containing the event numpy arrays stored in its
            keys (see config files for structural information)
        ________________________________________________________________________

        split_size:
            float or int, the fraction or number of the numpy arrays that will 
            be used as test example
    ____________________________________________________________________________

    Operation breakdown
    
        as we only ever work with the event dicitionary (due to the many sub-items 
        in it)  the usual train_test_split from sklearn does not suffice; we split
        the dictionary by taking the lenght of the numpy arrays and putting this in
        a range of numbers; then this range is shuffled, thus shuffling all numpy 
        arrays at the same time; The the dictionary is split into a big and small one
        according to the split_size - fraction
    ____________________________________________________________________________

    Return
        
        2 dictionaries with shoretened numpy arrays in the following convetion:
            - 1st dictionary is the BIG one
            - 2nd is the small one

    """
    if not isinstance(evt_dictionary, dict):
        raise TypeError('The variable "evt_dictionary" is expected to be of type ' \
                'dictionary but instead it is a {}!'.format(type(evt_dictionary)))
    if not isinstance(split_size, int) and not isinstance(split_size, float):
        raise TypeError('The variable "split_size" is expected to be of type ' \
                'int or float but instead it is a {}!'.format(type(split_size)))

    # output dictionaries
    big_sample = {}
    small_sample = {}

    sample_size = split_size
    n_evts = evt_dictionary['target'].shape[0]
    if split_size > n_evts:
        warnings.warn('The test sample size (variable "split_size") exceeds the ' \
                'the number of instances. Consequently a test split of 0.2 is used!')
        split_size = 0.2

    if split_size < 1.:
        sample_size = int(split_size*n_evts)
    # create a random subsample
    idx = np.arange(n_evts)
    np.random.shuffle(idx)

    for key in evt_dictionary.keys():
        value = evt_dictionary[key]
        if value.shape[0] != n_evts:
            raise ValueError('The {} level data does not agree with the ' \
                    'rest. It has stored {} events in contrast to the expected {}!'.format(
                        key, value.shape[0], n_evts))


        small_sample[key] = value[idx][:sample_size]
        big_sample[key] = value[idx][sample_size:]


    return big_sample, small_sample


def print_array_in_dictionary_stats(evt_dic, message='Event dictionary'):
    """
    Prints array shape and type of all dictionary entries
    """
    try:
        print('\n\n{}\n:: {}'.format(50*'-',message))
        for key in evt_dic.keys():
            array = evt_dic[key]
            if not isinstance(array, np.ndarray):
                raise TypeError('The key {} is not a numpy ndarray ' \
                        'but rather a {}!'.format(type(array)))
            print('\n{}'.format(key))
            print('type(array): {}'.format(type(array)))
            print('array.shape: {}'.format(array.shape))
        print('{}'.format(50*'-'))

    except AttributeError:
        raise TypeError('The evt_dic variable provided is not a dictionary but ' \
                'rather a {}!'.format(type(evt_dic)))


    return


