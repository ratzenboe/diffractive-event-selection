from __future__ import division
from __future__ import unicode_literals

import os
import math
import sys
import warnings
import pickle

import numpy as np
import pandas as pd
import root_numpy

from modules.utils import pause_for_input

def pad_dataframe(df, max_entries):
    """
    Args

        df:
            pandas dataframe of the shape (n_particles, n_features) 
        ____________________________________________________________

        max_entries:
            int > 0
    ________________________________________________________________

    Operation breakdown
        
        Extends "dataframe" to a fixed size, if "max_entries" exceeds the 
        df size the df is filled with -999 columns. 
        This is then masked in the NN model
    ________________________________________________________________

    Returns
        
        the dataframe with shape (max_entries, n_features)
        with n_particles <= max_entries

    """

    if isinstance(df, np.ndarray):
        warnings.warn('The variable "df" is a numpy.ndarray ' \
                'but it should be a pandas dataframe. We convert it now!')
        df = pd.DataFrame(df)

    if not isinstance(df, pd.DataFrame):
        raise TypeError('The variable "df" has to be a pandas dataframe ' \
                'but {} was provided'.format(type(df)))


    if not isinstance(max_entries, int) and max_entries is not None:
        raise TypeError('The variable "max_entries" has to be a either an integer ' \
                'or None but {} was provided'.format(type(max_entries)))


    if max_entries is None:
        return df

    length = df.shape[0]
    if length > max_entries:
        warnings.warn('The input dataframe exceeds the maxiumum entries! It is cut from {} ' \
                'instances to {} entries! This may affect the performance. Please adjust ' \
                'the maximum entries in the data-config file accordingly!'.format(
                    length, max_entries))
        # pause_for_input('A warning was issued, do you want to abort the program?', timeout=1)
        # max_entries-1 will give return a dataframe with length max_entries as the 
        # first entry is 0
        return df.loc[:max_entries-1]

    elif length < max_entries:
        for i in range(length, max_entries):
            df.loc[i] = float(-999)
        return df

    elif length == max_entries:
        return df


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
    signal_evts = 0
    if len(list_of_events) == 0:
        warnings.warn('No entries found in input data!\nReturning the unprocessed input.')
        pause_for_input('A warning was issued, do you want to abort the program?', timeout=4)
        return all_events, y_data
    
    current_n_evts = 0
    for evt_int in list_of_events:
        current_n_evts += 1
        if current_n_evts%1000 == 0:
            print(':: {} events from {} fetched'.format(current_n_evts, len(list_of_events)))
        # get relevant data for one event 
        evt_dataframe = inp_data.loc[inp_data[evt_id_string] == evt_int, list_of_features]
        # the dataframe has some arbitraty indices because we just slice some
        # instances out of it: we fix the index here
        evt_dataframe = evt_dataframe.reset_index(drop=True)

        ###########################################################################
        # we are already finished getting the dataframe however we have to
        # extract the target values:
        if evt_dataframe.empty:
            evt_dataframe = pd.DataFrame([])
            warnings.warn('The event {} has no information stored!'.format(evt_int))
        # the target is only present in the event column
        target_list = []
        if set(targets) <= set(list_of_features):
            for trgt in targets:
                # the target is represented by the last incident in the particle list
                # in the grid case the event is however just a single entry 
                # in this case .iloc[-1] does not matter
                y = evt_dataframe[trgt].iloc[-1]
                # if the target is in the columns we have to drop it
                evt_dataframe = evt_dataframe.drop(trgt, axis=1)
                target_list.append(y)
            
            if 106 and 1 in target_list:
                y_data.append(1)
                signal_evts += 1
            else:
                y_data.append(0)

        #######################################################################
        # Attention:
        #
        #   standard scaling is not working properly if we only use numpy arrays
        #   due to the lack of column control (cannot access column by name,
        #   as is the case for dataframes)
        #   
        #   at this point in the progrom we still have dataframes that we can 
        #   simply transform into a numpy records format; the records format can
        #   be accessed in the same way a dataframe can access its columns with
        #   the advantage that we can kind of stack these together and are able to 
        #   produce different shapes (compared to the row-column shape of dataframes)
        #   
        #   Stacking can simply be done by creating a list (here: all_events) and 
        #   appending the record arrays piece by piece; afterwards the list is 
        #   transformed into a numpy array; IMPORTANT: before sending the data
        #   to the model the data have to be transformed from a records format to
        #   the standard numpy array format by doning:
        #       
        #               np.array(rec_array_list.tolist()) )
        #       
        #   this has to be done after standard scaling or if the data are not scaled
        #   right before they are put into the model
        #
        #######################################################################
        
        evt_filled_up_dataframe = pad_dataframe(evt_dataframe, max_entries_per_evt)
        # transform dataframe to numpy records array (no indexing, as ordering does not
        # contain information)
        all_events.append(evt_filled_up_dataframe.to_records(index=False))

    # this is neccessary (see above)
    all_events = np.array(all_events) 
    y_data = np.array(y_data)

    ###############################################################################
    # print signal information if available:
    if len(y_data) != 0:
        print('\n:: {} signal events found in data. ({:.4f}%)'.format(
            signal_evts, signal_evts/len(list_of_events)))

    return all_events, y_data


def get_data(branches_dic, max_entries_dic, path_dic, evt_id_string, target_list, 
             cut_dic, event_string):
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
    
        cut_dic:
            dictionary of variables that will recieve a standard cut 
            e.g. 'n_tracks': determining the number of possible tracks that will be
            allowed in an event 
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

    # selection criterion regarding the number of tracks
    # we get a list of events (subset of events) that have the right number of
    # tracks (with 2,4 or 6 tracks) Only these events will be part 
    # of the final event dictionary
    try:
        branches_list = list(cut_dic.keys())
        branches_list.append(event_string)
        data = load_data(path_dic[event_string], branches=branches_list)
        if isinstance(cut_dic, dict):
            get_evt_id_list(data, cut_dic) 
        elif cut_dic is None:
            # if the variable is not properly set in the config files then 
            # we take all events that have at least 1 track
            list_of_events = data[evt_id_string][data[n_track_id]>0].values.tolist()
            list_of_events = map(int, list_of_events)

        del data

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
            
        del data
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
    data = data.astype(float)

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

    return 


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
        raise TypeError('The element stored in {} is not a dictionary ' \
                         'but instead a {}'.format(infile, type(evt_dic)))

    # for key, array in evt_dic.iteritems():
    #     if not isinstance(array, np.recarray):
    #         raise TypeError('The element {} in the event dictionary is not a ' \
    #                         'numpy records arreay but instead a {}'.format(key, type(array)))

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
        scaling_attr = {}
        for key, columns in std_scale_dic.iteritems():
            # we have to fit new scaling attributes
            scaling_attr[key] = {}
            for col in columns:
                try:
                    # here we can access the columns directly (due to record array) 
                    # and exclude the masking value of -999 from the mean calculation
                    # evt_dic[key] is accessing the record array, col the right column
                    mean_std_array = evt_dic[key][col][np.where(evt_dic[key][col] != -999.0)]
                    # if only -999 in the array then we get nans for mean and std-dev
                    # hence we check the lenght of the non-(-999) data
                    if mean_std_array.size == 0:
                        mean = 0.
                        std = 1.
                    else:
                        mean = mean_std_array.mean()
                        std  = mean_std_array.std()
                        # if all values are equal
                        if std == 0:
                            std = 1.

                    evt_dic[key][col][np.where(evt_dic[key][col] != -999.0)] -= mean
                    evt_dic[key][col][np.where(evt_dic[key][col] != -999.0)] /= std
                except KeyError:
                    print('Warning: Feature {} not found in the data!'.format(col))
                # cannot save only the column, but we also have to 
                # save the key, as some columns appear in different keys
                # e.g. time in fmd, ad, v0
                scaling_attr[key][col] = {'mean': mean, 'std': std}

        np.save(load_save_file, scaling_attr)

    else:
        # load previously fitted attributes
        scaling_attr = np.load(load_save_file).item()

        for key, values_dic in scaling_attr.iteritems():
            for key_inner, values_inner in values_dic.iteritems():
                try: 
                    evt_dic[key][key_inner][np.where(
                        evt_dic[key][key_inner] != -999.0)] -= values_inner['mean']
                    evt_dic[key][key_inner][np.where(
                        evt_dic[key][key_inner] != -999.0)] /= values_inner['std']
                except KeyError:
                    print('Warning: Feature {} not found in the data!'.format(key_inner))


    return evt_dic 


def fix_missing_values(event_data, missing_vals_dic):
    """
    Args
        event_data:
            dicionary containing record arrays that hold the data; the data have
            to be present in a record array as they are adressed via column names
        ___________________________________________________________________________

        missing_vals_dic:
            dictionary where the keys are the data names (e.g. track, event, etc) 
            and the values are dictionaries with branch names and their missing
            values, for example:

                { 'track': {'tof_bunch_crossing': -100, 'pid_tof_signal': 99999} 
                , 'event': {...} }
    _______________________________________________________________________________

    Operation breakdown:
        
        the keys from the missing values are looped through and also correspond to 
        the keys in the event_data dictionary. The missing_vals_dic then presents
        features that have missing values that differ from the default missing value
        -999. These missing values are then changed to -999 (to be properly masked)
    _______________________________________________________________________________

    Return
        
        event dictionary with the correct missing values.

    """
    try:
        print('Fixing missing values in feature:')
        for key, values_dic in missing_vals_dic.iteritems():
            for key_inner, values_inner in values_dic.iteritems():
                # key_inner is the feature (e.g. tof_bunch_crossing)
                # values_inner is the missing value, e.g. -100
                print('\n:: {} (from {})...'.format(key_inner, key))
                event_data[key][key_inner][np.where(
                    event_data[key][key_inner] == values_inner)] = float(-999)

        print('\n{}'.format(50*'-'))
    except IndexError:
        raise TypeError('Standard numpy arrays are passed to the function! ' \
                'However the event-dictionary should contain numpy record arrays ' \
                'that can be indexed by column-name!')


    return event_data


def shape_data(evt_data):
    """
    Args
        event_data:
            dicionary containing record arrays that hold the data; the data have
            to be present in a record array as they are adressed via column names
    _______________________________________________________________________________

    Operation breakdown:

        the data is converted from numpy records array into standard numpy format;
        If the data has unnecessary dimensions in the second axis (as is the case 
        e.g. for event) this dimension is removed.

        (this function should be used right before the data are fit with a model)
        
    _______________________________________________________________________________

    Return

        the event dicitonary containing standard numpy arrays ready for the model

    """
    # convert to standard numpy array
    try:
        for key in evt_data.keys():
            if not isinstance(evt_data[key], np.ndarray):
                raise TypeError('The key {} does not hold a numpy ndarray' \
                        'but rather a {}!'.format(type(evt_data[key])))
            evt_data[key] = np.array(evt_data[key].tolist())

            # remove unnecessary dimensions
            # only if dimenstions are greater than 1
            if len(evt_data[key].shape) >= 2:
                if evt_data[key].shape[1] == 1:
                    evt_data[key] = np.squeeze(evt_data[key], axis=1)

    except AttributeError:
        raise TypeError('The variable "evt_data" is not a dictionary but ' \
                'instead a {}!'.format(type(evt_data)))
    

    return evt_data



def get_evt_id_list(data, cut_dic, event_id_string):
    """
    Args
        data:
            pandas dataframe containing the event column and additionally 
            the columns where a cut will be applied (stored in the cut_dic)
        ____________________________________________________________________

        cut_dic:
            dictionary where 
                key = column in the data
                value = a list of possible values or a function
        ____________________________________________________________________

        event_id_string:
            string, column name of the event id
    _________________________________________________________________________

    Operation breakdown
        
        the dictionary is looped over and the event ids that correspond to
        the individual cuts are written to a list
    _________________________________________________________________________

    Return
        
        list, event ids that fulfill the cut criteria

    """
    n_evts_total = data.shape[0]
    # array that only contains the indices of events with the right amount
    # of tracks
    for key, value in cut_dic.iteritems():
        if isinstance(value, list):
            data = data.loc[data[key].isin(value)]
        elif callable(value):
            # if the value is a function(e.g. lambda x: x < 3.1415)
            data = data[data[key].apply(value)]
        else:
            raise TypeError('The {} in cut_dic is not a supported ' \
                    'type ({})'.format(key, type(value)))

    list_of_events = data[evt_id_string].values.tolist()
    if not isinstance(list_of_events):
        raise TypeError('The event-id-list is not a of type list!')
    # the integers in this list are long-ints -> convert them here to standard ints
    list_of_events = map(int, list_of_events)

    # print-out
    percentage_of_all = n_evts_total/len(list_of_events)
    print(':: Processing {}/{} events ({:.2f}%) with the following number of ' \
            'tracks: {}'.format(
                len(list_of_events), n_evts_total, percentage_of_all, cut_list_n_tracks))
 

    return list_of_events


