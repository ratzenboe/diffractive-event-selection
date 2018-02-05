import os
import math
import sys
import warnings

import numpy as np
import pandas as pd
import root_numpy

def pad_array(array, max_entries):
    """
    Args

        array:
            numpy array of the shape (n_particles, n_features) 
        ____________________________________________________________

        max_entries:
            int > 0
    ________________________________________________________________

    Operation breakdown
        
        Extends "array" to a fixed size, if "max_entries" exceeds the 
        array size the array is filled with -999 columns. 
        This is then masked in the NN model
    ________________________________________________________________

    Returns
        
        the array with shape (max_entries, n_features)
        with n_particles <= max_entries

    """

    if not isinstance(array, np.ndarray):
        raise TypeError('The variable "array" has to be a numpy.ndarray ' \
                'but {} was provided'.format(type(array)))

    if not isinstance(max_entries, int):
        raise TypeError('The variable "max_entries" has to be a int ' \
                'but {} was provided'.format(type(max_entries)))


    if max_entries is None:
        return array.tolist()

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
            the target value(s), list
            may also be multiple values, as there may be multiple information available
            that determine the final target.
    _______________________________________________________________________________________
    
    Operation breakdown

        Function that loops trough the data array which comes from a TTree
        All paricles/etc that belong to the same event are grouped together and are stored 
    _______________________________________________________________________________________

    Returns
        two ndarrays:
            - all_events (n_evts, n_parts, n_features)
            - y_data     (n_evts,)

    """
    if not isinstance(inp_data, pd.DataFrame):
        raise TypeError('Attention: the variable "inp_data" should be a pandas.DataFrame ' \
                'but insted is a {}!'.format(type(inp_data)))

    if not isinstance(max_entries_per_evt, int):
        raise TypeError('Attention: the variable "max_entries_per_evt" should be a ' \
                'integer but instead is a {}!'.format(type(max_entries_per_evt)))

     if not isinstance(list_of_features, list):
        raise TypeError('Attention: the variable "list_of_features" should be a ' \
                'list but instead is a {}!'.format(type(list_of_features))) 

    if not isinstance(list_of_features, list):
        raise TypeError('Attention: the variable "list_of_features" should be a ' \
                'list but instead is a {}!'.format(type(list_of_features))) 
        
    if not isinstance(evt_id_string, str):
        raise TypeError('Attention: the variable "evt_id_string" should be a ' \
                'string but insted is a {}!'.format(type(evt_id_string)))

    if not isinstance(targets, list):
        raise TypeError('Attention: the variable "targets" should be a ' \
                'list but insted is a {}!'.format(type(targets)))

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


def get_data(branches_dic, max_entries_dic, path_dic, evt_id_string, target_list):
    """
    Args
        
        branches_dic:
            A dictionary that contains the branches stored in the TTree
        __________________________________________________________________________

        max_entries_dic:
            Maximum entries/particles/hits that will be stored in the event
            dictionary. This determines e.g. the maximum number of tracks. Events
            with more tracks will not be considered (also events with 0 tracks in
            the tpc)
        __________________________________________________________________________

        path_dic:
            Dictionary containing the path to the root files
        __________________________________________________________________________

        evt_id_string:
            Every root tree (e.g the fmd data TTree) contains information about
            the event it belongs to. Different datasets may have different naming
            conventions describing the event number. 
        __________________________________________________________________________

        target_list:
            The target may depend on numerous varibles stored in the TTree.
            This list captures these variables. 
    ______________________________________________________________________________

    Operation breakdown

        A wrapper function calling multiple function in that assemble the event 
        dictionary which contains multiple numpy arrays that in turn 
        contain event wise data.
    ______________________________________________________________________________

    Return

        The event dictionary in the form:
            - evt_dic = {event:  np-array(n_evts, n_features), 
                         tracks: np-array(n_evts, n_particles, n_features),
                         .... }

    """
    evt_dictionary = {}
    for key, list_of_features in branches_dic.iteritems():
        # list_of_features.remove(data_params['evt_id'])

        data = load_data(path_dic[key], branches=list_of_features)

        evt_dictionary[key], y_data = event_grouping(
                                            inp_data            = data, 
                                            max_entries_per_evt = max_entries_dic[key],
                                            list_of_features    = list_of_features,
                                            evt_id_string       = evt_id_string,
                                            targets             = target_list )
        # if the y_data array has a size, then we add the 
        # target information to the evt_dictionary 
        if y_data:
            evt_dictionary['target'] = y_data

    return evt_dictionary



def load_data(filename, branches=None, selection=None):
    """
    Args
        filename:
            Path to the root file where the raw data is stored
        __________________________________________________________________________

        branches:
            List of branches that will be loaded from the root TTree. If None then
            all branches will be loaded, otherwise only the provided sub-set of
            branches will be in the data.
        __________________________________________________________________________

        selection:
            Some pre-selective cuts on the variables in the TTree. E.g. "time>0."
    ______________________________________________________________________________

    Operation breakdown
        
        The data in form of a TTree is loaded via root_numpy and stored 
        in a pandas dataframe (for easier slicing and processing of the data)
    ______________________________________________________________________________

    Return

        Pandas dataframe containing the TTree

    """
    if not os.path.exists(filename):
        raise IOError('File {} does not exist.'.format(filename))

    data = pd.DataFrame(root_numpy.root2array(filename, 
                                              branches = branches,
                                              selection = selection))

    return data


def save_data_dictionary(outfile, all_evt_data):
    """
    Args
        oufile:
            The path where the data dictionary will be saved.
        __________________________________________________________________________

        all_evt_data:
            The dictionary containing the data.
    ______________________________________________________________________________

    Operation breakdown
    
        save histograms in a numpy format

    """
    if not isinstance(all_evt_data, dict):
        raise TypeError('The variable "all_evt_data" does not contain ' \
                'the event dictionary but is of type {}!'.format(type(all_evt_data)))

    if not isinstance(outfile, str):
        raise TypeError('The "outfile" variable is not a string ' \
                'but rather a {}!.'.format(type(outfile)))

    np.save(outfile, all_evt_data)


def get_data_dictionary(infile):
    """
    Args
        infile:
            File where the data dictionary is saved. 
    ______________________________________________________________________________

    Operation breakdown

        Retrieve the dictionay containing the data

    """
    if not os.path.isfile(infile):
        raise IOError('File {} does not exist.'.format(infile))

    evt_dic = np.load(infile)[()]
    if not isinstance(evt_dic, dict):
        raise TypeError('The element stored in {} is not a dictionary \
                         but instead a {}'.format(infile, type(evt_dic)))
    
    return evt_dic




# #############################################################################
# --------------- will be updated if data loading part works ------------------
# #############################################################################
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

