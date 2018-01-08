from __future__ import division
import sys
import os
import time
import random

from select import select

import numpy as np
import pandas as pd

def print_number(y, name, **kwargs):

    print'Dimensions of {} vector y: {}'.format(name, y.shape)

    for key, item in kwargs.iteritems():
        print'Number of {}: {} ({:.2f} percent)'.format(key, y[y==item].shape[0], 
                                                   y[y==item].shape[0]*100/y.shape[0])
    
    return


def pause_for_input(run_params, timeout=5):
    print'\nInfo: The model will be trained anew. Existing saves will be overwritten.'
    print'      (Program paused for {} seconds. Hit Enter to abort the training.)'.format(timeout)
    rlist = select([sys.stdin], [], [], timeout)[0]

    if rlist:
        print'Input received. Exit the program...'
        sys.exit()
    else:
        print'No input received. Continue running the program...'
        print'Creating the model using {}...'.format(run_params['classifier_params_id'])
    
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
    as we only ever work with the event dicitionary (due to the many sub items in it)  
    the usual train_test_split from sklearn does not suffice

    returns the large sample as the first argument, the small one is the second return
    """
    sample_size = split_size
    if split_size < 1.:
        sample_size = int(split_size*len(evt_dictionary))
    randIndx = random.sample(xrange(len(evt_dictionary)), sample_size)
    randIndex_comp = list(set(range(len(evt_dictionary))) - set(randIndex))
    rand_smpl_compl = [ evt_dictionary[i] for i in randIndex_comp ]
    rand_smpl_small = [ evt_dictionary[i] for i in randIndex ]

    return valid_sample, rand_smpl_small

