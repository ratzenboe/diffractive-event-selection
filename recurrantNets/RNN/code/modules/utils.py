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

    print('Dimensions of {} vector y: {}'.format(name, y.shape))

    for key in kwargs.keys():
        item = kwargs[key]
        print('Number of {}: {} ({:.2f} percent)'.format(key, y[y==item].shape[0], 
                                                   y[y==item].shape[0]*100/y.shape[0]))
    
    return


def pause_for_input(message, timeout=5):
    print(message)
    print('      (Program paused for {} seconds. Hit Enter to abort the program.)'.format(timeout))
    rlist = select([sys.stdin], [], [], timeout)[0]

    if rlist:
        print('Input received. Exit the program...')
        sys.exit()
    else:
        print('No input received. Continue running the program...')
    
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
        print('\n{}'.format(headline))
            
    for item in dictionary:
        print('  {:25}: {}'.format(item, dictionary[item]))

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
    if isinstance(evt_dic, dict):
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
                if key == 'target':
                    all_evts = evt_dic[key].shape[0]
                    sig_evts = evt_dic[key][evt_dic[key] == 1].shape[0]
                    three_tracks_bg = evt_dic[key][evt_dic[key] == 99].shape[0]
                    sig_bg_percentage = sig_evts/all_evts*100.
                    print('{}/{} signal events ({:.3f}%)'.format(
                        sig_evts, all_evts, sig_bg_percentage))
                    if three_tracks_bg >0:
                        print('{}/{} real bg (3 tracks) events ({:.3f}%)'.format(
                            three_tracks_bg, all_evts, three_tracks_bg/all_evts *100.))

            print('{}'.format(50*'-'))


        except AttributeError:
            raise TypeError('The evt_dic variable provided is not a dictionary but ' \
                    'rather a {}!'.format(type(evt_dic)))

    elif isinstance(evt_dic, np.ndarray):
        print('\n\n{}\n:: {}'.format(50*'-',message))
        print('type(array): {}'.format(type(evt_dic)))
        print('array.shape: {}'.format(evt_dic.shape))
        print('{}'.format(50*'-'))

    else:
        raise TypeError('The variable "evt_dic" is neither a dictionary nor a ' \
                'numpy.ndarray but rather a {}.'.format(type(evt_dic)))

    return


def get_subsample(evt_dic, n_samples_signal, bg_sig_ratio=1.):
    """
    Args
        evt_dic:
            the event-dictionary containing the data
        _____________________________________________________________

        n_samples_signal:
            int, number of signal samples 
        ______________________________________________________________

        bg_sig_ratio:
            float, desired ratio of background to signal which should 
            be in the data
    __________________________________________________________________

    Operation breakdown
        
        a subsample of the event dictionary is selected and returned
    __________________________________________________________________

    Return

        a smaller event dictionary

    """
    target_array = evt_dic['target']
    # np.where outputs a tuple, with 0 we get the numpy array
    index_sig = np.where(target_array == 1)[0]
    index_bg  = np.where(target_array == 0)[0]
    np.random.shuffle(index_sig)
    np.random.shuffle(index_bg)

    if n_samples_signal >= index_sig.shape[0]:
        warnings.warn('n_samples_signal is greater than the actual number of ' \
                'signal samples in the data. Proceeding with the full number of ' \
                'signal samples')
        n_samples_signal = -1

    n_samples_bg = int(n_samples_signal*bg_sig_ratio)
    if n_samples_bg >= index_bg.shape[0]:
        warnings.warn('n_samples_bg is greater than the actual number of ' \
                'background samples in the data. Proceeding with the full number of ' \
                'background samples')
        n_samples_bg = -1

    index_tot = np.concatenate([index_sig[:n_samples_signal], index_bg[:n_samples_bg]])
    np.random.shuffle(index_tot)

    evt_dic_subset = {}

    for key in evt_dic.keys():
        evt_dic_subset[key] = evt_dic[key][index_tot]


    return evt_dic_subset


def remove_field_name(np_recarray, name):
    """
    Remove the column "name" from the record array np_recarray
    """
    names = list(np_recarray.dtype.names)
    if name in names:
        names.remove(name)
    new_recarray = np_recarray[names]

    return new_recarray
        

def flatten_dictionary(evt_dic, feature_labels_dic=None, skip_list=['target']):
    """
    Args
        evt_dic:
            the event dictionary
        ___________________________________________________________

        feature_labels_dic:
            dictionary containing the feature labels
        ___________________________________________________________

        skip_list:
            keys (e.g. track to skip therefore not putting them into
            the final feature-matrix, at least 'target' has to be
            in the skip_list!
    _______________________________________________________________

    Operation breakdown

        flatten recursive tracks into a single 1D array
    _______________________________________________________________

    Return

        1: One numpy array that is made up from all entries 
           in the event dictionary
        2: If provided, a flattened feature label list  
    """
    if not isinstance(skip_list, list):
        raise TypeError('The variable "skip_list" has to be a list ' \
                'here {} was recieved!.'.format(type(skip_list)))

    if 'target' not in skip_list:
        skip_list.append('target')

    # write the arrays in a list
    arr_lst = []
    labels = []
    for key in sorted(evt_dic.keys()):
        if key in skip_list:
            continue
        if feature_labels_dic is not None and len(evt_dic[key].shape) == 2:
            labels.extend(feature_labels_dic[key])
        if len(evt_dic[key].shape) == 3:
            n_evts = evt_dic[key].shape[0]
            n_particles = evt_dic[key].shape[1]
            n_features = evt_dic[key].shape[2]
            evt_dic[key] = np.reshape(evt_dic[key], (n_evts, n_particles*n_features))

            # here we add an index to each particle feature
            if feature_labels_dic is not None:
                for i in range(1,n_particles+1):
                    target_str = str(i)
                    new_lst = [idx_lst + target_str for idx_lst in feature_labels_dic[key]]
                    labels.extend(new_lst)

        arr_lst.append(evt_dic[key])

    concat_arr = np.concatenate(arr_lst, axis=1)

    return concat_arr, labels


def special_preprocessing(run_mode_user, evt_dic, labels_dic=None, append_array=None):
    """
    preprocessing only applied to certain run_mode
    """
    skip_list = ['target']
    if 'NN' in run_mode_user or 'anomaly' in run_mode_user or 'koala' in run_mode_user:
        tmp_evt_dic = {'target': evt_dic['target']}
        if 'koala' in run_mode_user:
            # here we have to reject event level data as the distributions do not
            # align with the data
            skip_list.append('event')
        tmp_evt_dic['feature_matrix'], labels_list = flatten_dictionary(evt_dic, 
                                                                        labels_dic,
                                                                        skip_list)

        # here append the eta_phi_diff column
        if 'koala' in run_mode_user and append_array is not None:
            tmp_evt_dic['feature_matrix'] = np.c_[tmp_evt_dic['feature_matrix'], 
                                                  append_array]

        evt_dic = tmp_evt_dic


    return evt_dic, labels_list


def eta_phi_dist(arr):
    """
    returns the eta phi distance of a rec-array that has the field names phi and eta
    """
    return np.sqrt(np.power(arr['phi'][0]-arr['phi'][1],2) + 
                   np.power(arr['eta'][0]-arr['eta'][1],2))

def get_eta_phi_dist_array(evt_dic):
    """
    returns an numpy array that can be added to the features at any point in the program
    """
    try:
        track_arr = evt_dic['track']
        new_feature = np.apply_along_axis(eta_phi_dist, 1, track_arr)

        return new_feature

    except KeyError:
        raise KeyError('The key "track" is not in among the evt dictionary keys: ' \
                '{}'.format(list(evt_dic.keys())))


def engineer_features(evt_dic, replace=False):
    """
    append new features to the record array
    """
    # record arrays are in general buggy to extend 
    # therefore we take a save route via a pandas dataframe 
    # the read-out of the record arr is done by looping over them, writing each 
    # feature into the datafreame
    list_of_engineered_features = ['eta_phi_diff']

    # return a copy because we dont want to modify the original dictionary
    evt_dic_copy = evt_dic.copy()

    df = pd.DataFrame([])
    for name in evt_dic_copy['event'].dtype.names: 
        df[name] = evt_dic_copy['event'][name].ravel() 

    df['eta_phi_diff'] = get_eta_phi_dist_array(evt_dic_copy)

    evt_dic_copy['event'] = df.to_records(index=False) 

    if replace:
        evt_dic = evt_dic_copy

    return evt_dic_copy, list_of_engineered_features




