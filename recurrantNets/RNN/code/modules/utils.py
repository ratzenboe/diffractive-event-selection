from __future__ import division
import sys
import os
import time

from select import select

import numpy as np
import pandas as pd

def print_CV_results(scores):
    """
    Outputs the metrics evaluated by cross-validation (mean and standard deviation).
    """

    print'\nCross-validation results:'

    for metric in sorted(scores.keys()):
        print'  {:22s}: {:8.4} +/- {:8.4}'.format(
            metric, np.mean(scores[metric]), np.var(scores[metric]))

    return


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


def file_manipulation(func, data_params, **kwargs):
    """
    Simple wrapper for a pandas file import method.
    """
    
    if not os.path.exists(data_params['data_path']):
        print'Error when loading data. File {} file not found.'.format(data_params['data_path'])
        sys.exit()

    print'Reading file {}...'.format(data_params['data_path'])
    
    data = func(data_params['data_path'], **kwargs)

    print'data: {}  rows, {} columns'.format(data.shape[0], data.shape[1])
    print'data: branches = {}'.format(list(data.columns.values))

    return data


def print_dict(dictionary, headline=None):
    """
    Prints the contents of a dictionary to the terminal, line by line.
    """

    if headline:
        print'\n{}'.format(headline)
            
    for item in dictionary:
        print'  {:25}: {}'.format(item, dictionary[item])

    return


def print_gridSearch_results(model, data_params, run_params):

    output_prefix, model_saves_prefix = get_output_paths(run_params)
    print('\n')
    print('{}\nGrid search summary (used folds: {})\n{}\n'.format(
        '-'*35, model.n_splits_, '-'*35))
    print('  Best estimator: {} '.format(model.best_estimator_))
    print('\n  Best score: {} (optimized metric: {})'.format(
        model.best_score_, data_params['gs_refit']))
    print('\n')

    for item in sorted(model.cv_results_.keys()):
        keywords = ['mean_', 'std_']

        if not any(filter(lambda x: item.startswith(x), keywords)):
            continue
        print('  {:35s}: {:8.4}'.format(item, model.cv_results_[item][model.best_index_]))

    np.save(output_prefix + model_saves_prefix + 'gridsearch_cv_results.npy', 
                model.cv_results_)

