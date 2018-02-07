from __future__ import division

import os
import math
import sys
import warnings

import numpy as np
import pandas as pd
import root_numpy

from modules.utils import pause_for_input

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

    if not isinstance(max_entries, int) and max_entries is not None:
        raise TypeError('The variable "max_entries" has to be a either an integer ' \
                'or None but {} was provided'.format(type(max_entries)))


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


def event_grouping(inp_data, max_entries_per_evt, list_of_features, evt_id_string,
                   targets, list_of_events):
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
        __________________________________________________________________________________

        list_of_events:
            An index-list containing the event numbers of the desired events (right amount
            of tracks)
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

    if not isinstance(list_of_features, list):
        raise TypeError('Attention: the variable "list_of_features" should be a ' \
                'list but instead is a {}!'.format(type(list_of_features))) 
        
    if not isinstance(evt_id_string, str):
        raise TypeError('Attention: the variable "evt_id_string" should be a ' \
                'string but insted is a {}!'.format(type(evt_id_string)))

    if not isinstance(targets, list):
        raise TypeError('Attention: the variable "targets" should be a ' \
                'list but insted is a {}!'.format(type(targets)))

    if not isinstance(list_of_events, list):
        raise TypeError('Attention: the variable "list_of_events" should be a ' \
                'list but instead is a {}!'.format(type(list_of_events)))

    # remove event id from the list of features
    list_of_features = filter(lambda x: x != evt_id_string, list_of_features)

    all_events = []
    y_data = []
    if len(list_of_events) == 0:
        warnings.warn('No entries found in input data!\nReturning the unprocessed input.')
        return all_events, y_data
    
    current_n_evts = 0
    for evt_int in list_of_events:
        current_n_evts += 1
        if current_n_evts%1000 == 0:
            print(':: {} events from {} fetched'.format(current_n_evts, len(list_of_events)))
        # get relevant data for one event 
        evt_data = inp_data.loc[inp_data[evt_id_string] == evt_int, list_of_features]
        if evt_data.empty:
            evt_data = pd.DataFrame([])
            warnings.warn('The event {} has no information stored!'.format(evt_int))
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
            
            if 106 and 1 in target_list:
                y_data.append(1)
            else:
                y_data.append(0)

        evt_data = evt_data.as_matrix()
        evt_list = pad_array(evt_data, max_entries_per_evt)
        all_events.append(evt_list)

    all_events = np.array(all_events) 
    y_data = np.array(y_data)


    return all_events, y_data


def get_data(branches_dic, max_entries_dic, path_dic, evt_id_string, target_list, n_track_id,
             cut_list_n_tracks, event_string):
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
        __________________________________________________________________________

        n_track_id:
            The branch name which stores the number of particles that are detected
            in the TPC
        __________________________________________________________________________
    
        cut_list_n_tracks:
            list of ints determining the number of possible tracks that will be
            allowed in an event (default [2, 4, 6]) 
        __________________________________________________________________________

        event_string:
            string corresponding to the dictionary key of the event 
            (default 'event')
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

    if not isinstance(cut_list_n_tracks, list) and cut_list_n_tracks is not None:
        warnings.warn('The variable "cut_list_n_tracks" is not set properly!' \
                'It has to be a list of integers or None, ' \
                'but here is type {}.'.format(type(cut_list_n_tracks))) 

    # selection criterion regarding the number of tracks
    # we get a list of events (subset of events) that have the right number of
    # tracks (with 2,4 or 6 tracks) Only these events will be part 
    # of the final event dictionary
    try:
        data = load_data(path_dic[event_string], branches=[n_track_id,evt_id_string])
        n_evts_total = data.shape[0]
        if isinstance(cut_list_n_tracks, list):
            # array that only contains the indices of events with the right amount
            # of tracks
            list_of_events = data[evt_id_string][data[n_track_id].isin(
                cut_list_n_tracks)].values.tolist()
            # the integers in this list are long-ints -> convert them here to standard ints
            list_of_events = map(int, list_of_events)
            percentage_of_all = n_evts_total/len(list_of_events)
            print(':: Processing {}/{} events ({:.2f}%) with the following number of ' \
                    'tracks: {}'.format(
                        len(list_of_events), n_evts_total, percentage_of_all, cut_list_n_tracks))
        elif cut_list_n_tracks is not None:
            # if the variable is not properly set in the config files then 
            # we take all events that have at least 1 track
            list_of_events = data[evt_id_string][data[n_track_id]>0].values.tolist()
            list_of_events = map(int, list_of_events)

    except (IOError, KeyError):
        raise NameError('The event data cannot be loaded! Either the path {} ' \
                'does not exist or the column {} ' \
                'does not exist in the data.\nCheck the config file for any' \
                'name errors!'.format(path_dic[event_string], n_track_id))

    for key, list_of_features in branches_dic.iteritems():
        # list_of_features.remove(data_params['evt_id'])
        print('\n{} Loading {} data {}'.format(10*'-', key, 10*'-'))

        data = load_data(path_dic[key], branches=list_of_features)

        evt_dictionary[key], y_data = event_grouping(
                                            inp_data            = data, 
                                            max_entries_per_evt = max_entries_dic[key],
                                            list_of_features    = list_of_features,
                                            evt_id_string       = evt_id_string,
                                            targets             = target_list,
                                            list_of_events      = list_of_events)
        # if the y_data array has a size, then we add the 
        # target information to the evt_dictionary 
        if isinstance(y_data, list):
            y_data = np.array(y_data)
        if y_data.size is not 0:
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



def preprocess(evt_dic, std_scale_dic, out_path, load_fitted_attributes=False):
    """
    Args
        evt_dic:
            dictionary containing the event information; requires a certain 
            structure predefined in the config files
        __________________________________________________________________________
        
        std_scale_dic:
            dictionary containing the branches which will be standard scaled
        __________________________________________________________________________

        out_path:
            string, path to where the file will be saved and read
        __________________________________________________________________________

        load_fitted_attributes:
            bool, if the attributes will be picked up from the file or
            will be newly fitted
    ______________________________________________________________________________

    Operation breakdown

        Performes preprocessing on the data (right now only standarad scaling)
        the fitted attributes are then saved in a file
    ______________________________________________________________________________

    Return 
        
        the preprocessed/fitted dictionary 

    """

    load_save_file = out_path + 'scaling_attributes.npy'
    
    if not load_fitted_attributes:
        # we loop through all datasets that are in the dictionary std_scale
        # std_scale looks like:
        #   {'event': ['a', 'b',...], 'track': [...], ...}
        for key, columns in std_scale_dic.iteritems():
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

        for key, columns in std_scale_dic.iteritems():
            for col in columns:
                try: 
                    evt_dic[key][col] -= scaling_attributes[col][key]['mean']
                    evt_dic[key][col] /= scaling_attributes[col][key]['std']
                except KeyError:
                    print('Warning: Feature {} not found in the data!'.format(col))

    return evt_dic 

