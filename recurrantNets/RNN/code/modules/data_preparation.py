import os
import math
import sys
import warnings

import numpy as np
import pandas as pd
import root_numpy

def pad_array(array, max_entries):
    """
    truncate or extend numpy array to a fixed size
    if the maximum entries exceeds the size the array is
    filled with -999 columns. This is then masked in the NN model
    """

    if max_entries is None:
        return array.tolist()
    try: 
        length = array.shape[0]
        if length > max_entries:
            return array[:max_entries,:].tolist()

        elif length < max_entries:
            dummy_arr = np.zeros(shape=(array.shape[1],), dtype='float')
            dummy_arr.fill(float(-999))
            for i in range(length, max_entries):
                array = np.append(array, [dummy_arr], axis=0)
            return array.tolist()

        elif length == max_entries:
            return array.tolist()

    except AttributeError:
        print('please provide numpy array to pad_array function')


def event_grouping(inp_data, max_entries_per_evt, list_of_features, evt_id_string, targets)
    """
    Args
        inp_data: 
            data in a the shape of ((n_evt*n_parts), n_features), 
            e.g. (1000, 3) for 10 evts with each 10 particles and 3 features
            will be converted into an event-wise arrangement (n_evt, n_parts, n_feats)
        __________________________________________________________________________________

        max_entries_per_evt:
            if an event contains more or less particles/entries than this number
            the event is either discarded or padded with -999 columns (for the Masking layer)
        __________________________________________________________________________________
                
        list_of_features: 
            list of features to work with (maybe all features or a subset of them
        __________________________________________________________________________________

        evt_id_string:
            the name of the event id column, as this is not consistend accross files
        __________________________________________________________________________________

        targets:
            the target value(s)
            may also be multiple values, as there may be multiple information available
            that determine the final target.
        __________________________________________________________________________________
    
    Operation breakdown:
        Function that loops trough the data array which comes from a TTree
        All paricles/etc that belong to the same event are grouped together and are stored 

    returns: 
        two ndarrays:
            - all_events (n_evts, n_parts, n_features)
            - y_data     (n_evts,)
    """

    min_evt_nb = inp_data[evt_id_string].min()
    max_evt_nb = inp_data[evt_id_string].max()
    evts_in_data = max_evt_nb - min_evt_nb

    all_events = []
    y_data = []
    if evts_in_data == 0:
        warnings.warn('No entries found in input data!\nReturning the unprocessed input.')
        return all_events, y_data
    
    for evt_int in range(min_evt_nb, max_evt_nb+1):
        if evt_int%1000 == 0:
            print('{} events from {} fetched'.format(evt_int, min_evt_nb+max_evt_nb))
        # get relevant data for one event 
        evt_data = inp_data.loc[inp_data[evt_id_string] == evt_int, list_of_features]

        # the target is only present in the event column
        target_list = []
        if set(targets) <= set(list_of_features):
            for trgt in targets:
                # the target is represented by the last incident in the particle list
                # in the grid case the event is however just a single entry 
                # in this case .iloc[-1] does not matter
                y = evt_data[trgt].iloc[-1]
                # if the target is in the columns we have to drop it
                evt_data = evt_data.drop(trgt, axis=1)
                target_list.append(y)
            
            if 106 and 1 target_list:
                y_data.append(1)
            else:
                y_data.append(0)

        evt_data = evt_data.as_matrix()
        evt_list = pad_array(evt_data, max_entries_per_evt)
        all_events.append(evt_list)

    all_events = np.array(all_events) 
    y_data = np.array(y_data)


    return all_events, y_data


def get_data(data_params):
    """
    a global function combining various functions to load the data and put it in the right
    form for ML
    """
    evt_dictionary = {}
    for key, value in data_params['branches'].iteritems():
        list_of_features = value
        # list_of_features.remove(data_params['evt_id'])

        data = load_data(data_params['path'][key], branches=list_of_features)

        evt_dictionary[key], y_data = event_grouping(
                                            inp_data            = data, 
                                            max_entries_per_evt = data_params['max_entries'][key],
                                            list_of_features    = list_of_features,
                                            data_params         = data_params)
        # if the y_data array has a size, then we add the 
        # target information to the evt_dictionary 
        if y_data:
            evt_dictionary['target'] = y_data

    return evt_dictionary



def load_data(filename, branches=None, start=None, stop=None, selection=None):
    """
    load data from a root tree and returns a panda dataframe with the relevant branches

    sidenote: 
        the raw data from the root file is then put into a shaping function that prepares the
        data for the RNNs (event_grouping) 
    """
    if not os.path.exists(filename):
        raise IOError('File {} does not exist.'.format(filename))

    data = pd.DataFrame(root_numpy.root2array(filename, branches=branches))

    return data


def save_data_dictionary(outfile, all_evt_data):
    """
    save histograms in a numpy format
    """
    np.save(outfile, all_evt_data)


def get_data_dictionary(infile):
    """
    retrieve the dictionay containing the data
    """
    if not os.path.isfile(infile):
        raise IOError('File {} does not exist.'.format(infile))

    evt_dic = np.load(infile)[()]
    if not isinstance(evt_dic, dict):
        raise TypeError('The element stored in {} is not a dictionary \
                         but instead a {}'.format(infile, type(evt_dic)))
    
    return evt_dic

def preprocess(evt_dic, data_params, run_params, load_fitted_attributes=False):
    """
    Performes preprocessing on the data, right now only standarad scaling
    the fitted attributes are then saved in a file
    """

    output_prefix, model_saves_prefix = get_output_paths(run_params)
    load_save_file = output_prefix + model_saves_prefix + 'scaling_attributes.npy'
    
    if not load_fitted_attributes:
        # we loop through all datasets that are in the dictionary std_scale
        # std_scale looks like:
        #   {'fmd': ['a', 'b',...], 'ad': [...], ...}
        for key, columns in data_params['std_scale'].iteritems():
            # we have to fit new scaling attributes
            scaling_attr = {}
            for col in columns:
                try:
                    mean = evt_dic[key][col].mean()
                    std  = evt_dic[key][col].std()
                    evt_dic[key][col] -= mean
                    evt_dic[key][col] /= std
                except KeyError:
                    print('Warning: Feature {} not found in the data!'.format(col))
                    
                # cannot save only the column, but we also have to 
                # save the key, as some columns appear in different keys
                # e.g. time in fmd, ad, v0
                scaling_attr[col] = {key: {'mean': mean, 'std': std} }

        np.save(load_save_file, scaling_attributes)

    else:
        # load previously fitted attributes
        scaling_attr = np.load(load_save_file).item()

        for key, columns in data_params['std_scale'].iteritems():
            for col in columns:
                try: 
                    evt_dic[key][col] -= scaling_attributes[col][key]['mean']
                    evt_dic[key][col] /= scaling_attributes[col][key]['std']
                except KeyError:
                    print('Warning: Feature {} not found in the data!'.format(col))

    return evt_dic 

