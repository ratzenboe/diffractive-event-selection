import sys
import os
import time
import argparse 
import ast 
import copy
import warnings
import pickle

import timeit

import fnmatch

import numpy as np
import pandas as pd

try:
    import configparser
except ImportError:
    import ConfigParser as configparser


def print_dict(dictionary, headline=None):
    """
    Prints the contents of a dictionary to the terminal, line by line.
    """

    if headline:
        print('\n{}'.format(headline))
            
    for item in dictionary:
        print('  {:25}: {}'.format(item, dictionary[item]))

    return


class fileparser(configparser.ConfigParser):
    """
    Extends the functionality of the Python configparser.
    """

    def as_dict(self):
        """
        Returns the entire config file as a dictionary.
        """

        d = dict(self._sections)
        for k in d:
            d[k] = dict(**d[k])
            d[k].pop('__name__', None)
            for j in d[k].keys():
                d[k][j] = ast.literal_eval(d[k][j])
        
        return d


def config_file_to_dict(filename):
    """
    Imports an entire config file and returns it as a dictionary.
    """
    if not isinstance(filename, str):
        raise ValueError('Passed argument is not a string but instead {}'.format(type(filename)))
 
    if not os.path.isfile(filename):
        raise IOError('File {} does not exist.'.format(filename))
    
    parser = fileparser(configparser.ConfigParser())
    parser.optionxform=str # make key parsing case-sensitive
    parser.read(filename)

    return parser.as_dict()


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
    for key in cut_dic.keys():
        value = cut_dic[key]
        if isinstance(value, list):
            data = data.loc[data[key].isin(value)]
        elif isinstance(value, int) or isinstance(value, float):
            # if the value is a function(e.g. lambda x: x < 3.1415)
            data = data[data[key].apply(lambda x: x == value)]
        elif isinstance(value, tuple):
            if len(value) == 2:
                # sort the tuple as the values may not be in the right order 
                # becomes a list but we do not care 
                value = sorted(value)
                data = data[data[key].apply(lambda x: x > value[0] and x < value[1])]
        else:
            raise TypeError('The {} in cut_dic is not a supported ' \
                    'type ({})'.format(key, type(value)))

    list_of_events = data[event_id_string].values.tolist()
    if not isinstance(list_of_events, list):
        raise TypeError('The event-id-list is not a of type list!')
    # the integers in this list are long-ints -> convert them here to standard ints
    list_of_events = list(map(int, list_of_events))

    # print-out
    percentage_of_all = len(list_of_events)/n_evts_total
    print('\n::  Processing {}/{} events ({:.3f}%) with the following number of ' \
            'cuts:\n'.format(len(list_of_events), n_evts_total, percentage_of_all))
    print_dict(cut_dic)

    return list_of_events


def event_grouping(inp_data, max_entries_per_evt, evt_id_string, 
        targets, list_of_events, max_tracks=2, rm_evt_id_string=False):
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
        __________________________________________________________________________________

        max_tracks:
            int, the maximum number of tracks used to distinguish the modeled background (99)
            from the signal events (0,1)
        __________________________________________________________________________________

        rm_evt_id_string:
            bool, after classification is done we want to use the event number to trace back
            which events we can keep: therefore we save the evt-id in the event array
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
    if rm_evt_id_string:
        list_of_features = list(filter(lambda x: x != evt_id_string, list(inp_data.columns)))
    else:
        list_of_features = list(inp_data.columns)
    
    all_events = []
    y_data = []
    signal_evts = 0
    real_bg_evts = 0
    if len(list_of_events) == 0:
        warnings.warn('No entries found in input data!\nReturning the unprocessed input.')
        pause_for_input('A warning was issued, do you want to abort the program?', timeout=4)
        return all_events, y_data
    
    time_needed_for_loc = []
    time_needed_for_pad_df = []
    for evt_int in list_of_events:
        # get relevant data for one event 
        start_time = time.time()
        evt_dataframe = inp_data.loc[inp_data[evt_id_string] == evt_int, list_of_features]
        time_needed_for_loc.append((time.time() - start_time))
        # the dataframe has some arbitraty indices because we just slice some
        # instances out of it: we fix the index here
        evt_dataframe = evt_dataframe.reset_index(drop=True)

        ###########################################################################
        # we are already finished getting the dataframe however we have to
        # extract the target values:
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

            ###################################################################
            # we select (mostly) 2 tracks out of 3, 4, ... or more to simulate the background
            # this is done in the pad_dataframe function
            # HERE: we mark them as a special target, namely 99
            #       we also know that the targets are only present in the
            #       event therefore we can do evt_dataframe['n_tracks']
            try:
                if int(evt_dataframe['n_tracks'].iloc[-1]) > max_tracks:
                    target_list.append(99)
            except KeyError:
                raise KeyError('The key "n_tracks" is not in the evt_dataframe! Therefore it ' \
                        'is not among the list of features: \n {}'.format(list_of_features))

            if 99 in target_list: 
                y_data.append(99)
                real_bg_evts += 1
            elif 106 in target_list and 1 in target_list:
                y_data.append(1)
                signal_evts += 1
            else:
                y_data.append(0)


        if 'charge_sign' in list(evt_dataframe.columns):
            check_charge = True
        else:
            check_charge = False
        
        time_start = time.time()
        evt_filled_up_dataframe = pad_dataframe(evt_dataframe, max_entries_per_evt, check_charge)
        time_needed_for_pad_df.append((time.time() - time_start))
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
        all_events.append(evt_filled_up_dataframe.to_records(index=False))

    ###############################################################################
    # print signal information if available:
    if len(y_data) != 0:
        print('\n:: {} signal events found in data. ({:.3f}%)'.format(
            signal_evts, signal_evts/len(list_of_events)*100.))
        print(':: {} real bg (3+tracks) events found in data. ({:.3f}%)\n'.format(
            real_bg_evts, real_bg_evts/len(list_of_events)*100.))

    print(':: Average time for: "inp_data.loc[inp_data[evt_id_string] == evt_int, list_of_features]" : {:.4f} s'.format(
        sum(time_needed_for_loc) / float(len(time_needed_for_loc))))
    print(':: Average time for: "pad_dataframe" : {:.4f} s'.format(
        sum(time_needed_for_pad_df) / float(len(time_needed_for_pad_df))))
     ###############################################################################

    return all_events, y_data


def pad_dataframe(df, max_entries, check_charge=False):
    """
    Args

        df:
            pandas dataframe of the shape (n_particles, n_features) 
        ____________________________________________________________

        max_entries:
            int > 0
        ____________________________________________________________

        check_charge:
            bool: if too many tracks are provided this bool controls 
            if we should check for the charge of the particles to 
            sum up to 0 (should be false for any cases except for tracks) 
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
        if not check_charge:
            warnings.warn('The input dataframe exceeds the maxiumum entries! It is cut from {} ' \
                    'instances to {} entries! This may affect the performance. Please adjust ' \
                    'the maximum entries in the data-config file accordingly!'.format(
                        length, max_entries))

        if check_charge:
            while True:
                df_new = df.sample(n=max_entries).reset_index(drop=True)
                try:
                    if int(df_new['charge_sign'].sum()) == 0:
                        df = df_new
                        break
                except KeyError:
                    raise KeyError('The feature "charge_sign" is not stored in the read data!')
        else:
            df = df.sample(n=max_entries).reset_index(drop=True)

        return df

    elif length < max_entries:
        diff = max_entries - length
        new_df = pd.DataFrame(float(-999), index=np.arange(diff), columns=df.columns)
        concat_df = pd.concat([df, new_df], ignore_index=True)
        if concat_df.shape[0] != max_entries:
            raise ValueError('The dataframe was not padded properly!')

        return concat_df

    elif length == max_entries:
        # this is only true for the track features
        if check_charge:
            if int(df['charge_sign'].sum()) != 0:
                raise ValueError('The tracks provided do not sum up to a neutral particle!')
        return df



def main():

    if (sys.version_info < (3, 0)):
        raise OSError('Python 3+ is needed for this program to work ' \
            'but version {} is used!'.format(sys.version))

    ######################################################################################
    # STEP 0:
    # ----------------------------- fetching the data ------------------------------------
    # we fetch the data from 8 different root files where detector data is saved
    #       - AD
    #       - FMD
    #       - V0
    #       - EMC
    #       - PHOS
    #       - Evt-level data
    #       - tracking data
    #       - calo cluster data
    # 
    # therefore we want to have a data structure where we have a dataframe for each  
    # of these 8 departments
    ######################################################################################    
    # data is a dictionary which contains the 9 numpy arrays in form 
    # -> data['track'].shape = (n_evts, n_parts, n_features)
    #    data['fmd'].shape = (n_evts, n_cells, n_features)
    #    data['target'].shape = (n_evts, )
    #    etc. 
    data_params = config_file_to_dict(config_path)
    data_params = data_params[run_mode_user]
    # here is a collection of variables extracted from the config files
    try:
        # ----------- data-parameters --------------
        max_entries_dic   = data_params['max_entries']
        target_list       = data_params['target']
        evt_id_string     = data_params['evt_id']
        cut_dic           = data_params['cut_dic']
        event_string      = data_params['event_string']
    except KeyError:
        raise KeyError('The variable names in the main either have a typo ' \
                'or do not exist in the config files!')

    files_lst = []
    for filename in os.listdir(filespath):
        if fnmatch.fnmatch(filename, '*_pandas.pkl'):
            files_lst.append(filename)
    
    # first we make a list of all the file types in the filespath
    unique_files = [filename.partition('_info')[0] for filename in files_lst]
    unique_files = list(set(unique_files))
    unique_files.sort()

    # write the paths in a dictionary
    path_dic = {}
    list_of_path_ints = []
    for file_type in unique_files:
        # get all files which have the appropriate data 
        file_type_lst = [filename for filename in files_lst if file_type+'_info' in filename]
        file_type_lst.sort()
        path_dic[file_type] = file_type_lst
        # save ints at end of files and (later) check if the files endings correspond to each other
        int_list = [int(''.join(list(filter(str.isdigit, path_str)))) for path_str in file_type_lst]
        list_of_path_ints.append(int_list)
    # correspondance check (check if the first integer series (of e.g. ad) is repeaded in all other 
    # detectors
    if list_of_path_ints.count(list_of_path_ints[0]) != len(list_of_path_ints):
        raise IOError('\nThere is no correspondance between the files ' \
                'provided (elements are not the same)!')
    # number of file-collections (file-collection = ad, fmd, event, track, ...)
    n_diff_paths = len(list_of_path_ints[0])

    #########################################################################################
    # variable modifications
    if base >= n_diff_paths:
        raise ValueError('The value "base" {} exceeds the number of different paths: {}'.format(
            base, n_diff_paths))

    if all_files or base + nfiles > n_diff_paths:
        num_paths = n_diff_paths
    else:
        num_paths = base + nfiles

    if file_suffix and not all_files:
        suffix = '_' + str(int(base/(num_paths-base))) + '_' + file_suffix
    elif not all_files:
        suffix = '_' + str(int(base/(num_paths-base)))
    elif not file_suffix:
        suffix = ''
    else:
        suffix = '_' + file_suffix
    # 
    #########################################################################################


    evt_dictionary = {'target': []}
    # fill the (final) evt_dictionary with the correct keys
    # and empty arrays
    for key in path_dic.keys():
        evt_dictionary[key] = []

    temp_path_dic = {}
    for i in range(base, num_paths):
        # change path_dic to file-path i
        for key in path_dic.keys():
            temp_path_dic[key] = path_dic[key][i]
        ##################################################################################
        # at first we make a preselection of possible events (stored in event info)
        # currently cuts are only supported at event-level
        data = pd.read_pickle(filespath + temp_path_dic['event'])
        list_of_events = get_evt_id_list(data, cut_dic, evt_id_string)
        
        ##################################################################################
        # now we have a subset of the data -> extract that subset
        tmp_evt_dictionary = {}
        for key in temp_path_dic.keys():
            print('\n{} Loading {} data {}'.format(10*'-', key, 10*'-'))
            data = pd.read_pickle(filespath + temp_path_dic[key])
            ##############################################################################
            # here the data get transformed into the records array shape
            if str(key) == 'event':
                rm_evt_id_string = False
                print('rm: False, key={}'.format(key))
            else:
                rm_evt_id_string = True
                print('rm: True, key={}'.format(key))

            tmp_evt_dictionary[key], y_data = event_grouping(
                                                        inp_data = data,
                                                        max_entries_per_evt=max_entries_dic[key],
                                                        evt_id_string = evt_id_string,
                                                        targets = target_list,
                                                        list_of_events = list_of_events,
                                                        max_tracks = max_entries_dic['track'],
                                                        rm_evt_id_string = rm_evt_id_string)

            # if the y_data array has a size, then we add the 
            # target information to the tmp_evt_dictionary 
            if not isinstance(y_data, list):
                raise TypeError('The variable "y_data" is not a list but rather ' \
                        'a {}.'.format(type(y_data)))
            if y_data:
                tmp_evt_dictionary['target'] = y_data

        # check if the keys are the same
        if set(tmp_evt_dictionary.keys()) != set(evt_dictionary.keys()):
            raise KeyError('The temporary event-dictionary is not compatible ' \
                    'to the global one:\n tmp_evt_dic.keys(): {} \n evt_dic.keys(): {}'.format(
                        set(tmp_evt_dictionary.keys()), set(evt_dictionary.keys())))

        for key in tmp_evt_dictionary.keys():
            evt_dictionary[key] += tmp_evt_dictionary[key]
        
        del tmp_evt_dictionary

    # we loop over the entries and transform the list of record arrays into
    # a numpy record array
    for key in evt_dictionary.keys():
        evt_dictionary[key] = np.array(evt_dictionary[key]) 

    # # saveing the record array only works in python 3
    outfile = outpath + 'evt_dic' + suffix + '.pkl'
    print('\n::  Saving the data in {}'.format(outfile))
    with open(outfile, 'wb') as f:
        pickle.dump(evt_dictionary, f)

    return

   
if __name__ == "__main__":

    user_argv = None

    if(len(sys.argv) > 1):
        user_argv = sys.argv[1:]

    # commend line parser (right now not a own function as only 2 elements are used)
    parser = argparse.ArgumentParser()
    parser.add_argument('-filespath', '-pklfilesdir',
                        help='string (mandatory): the path where the pickle files are stored',
                        action='store',
                        dest='filespath',
                        default=None,
                        type=str)

    parser.add_argument('-config_path', 
                        help='string (mandatory): path to the data config file',
                        action='store',
                        dest='config_path',
                        default=None,
                        type=str)

    parser.add_argument('-basefile', '-base',
                        help='int (default: 0): the lowest number corresponding to the *_base.root file \
                                files from this number on will be processed',
                        action='store',
                        dest='base',
                        default=0,
                        type=int)

    parser.add_argument('-nfiles', 
                        help='int (default: 1): number of files to be processed',
                        action='store',
                        dest='nfiles',
                        default=1,
                        type=int)

    parser.add_argument('-all_files', 
                        help='bool (default: False): if true all files are processed at once',
                        action='store_true',
                        dest='all_files',
                        default=False)

    parser.add_argument('-filesuffix', 
                        help='string(or int) (default: None) suffix to the output file',
                        action='store',
                        dest='file_suffix',
                        default=None,
                        type=str)

    parser.add_argument('-run_mode_user', '-run_mode',
                        help='string (default: koala): the key word that is found in the config file',
                        action='store',
                        dest='run_mode_user',
                        default='koala',
                        type=str)

    command_line_args = parser.parse_args(user_argv)

    base = command_line_args.base
    nfiles = command_line_args.nfiles
    all_files = command_line_args.all_files
    filespath = command_line_args.filespath
    file_suffix = command_line_args.file_suffix
    config_path = command_line_args.config_path
    run_mode_user = command_line_args.run_mode_user

    if file_suffix:
        file_suffix = str(file_suffix)

    if config_path is None or not os.path.isfile(config_path):
        raise IOError('No valid "config_path" provided!\nPlease do so via ' \
                'command line argument: -config_path /path/to/config_file.conf')

    if filespath is None or not os.path.isdir(filespath):
        raise IOError('No valid input path provided!\nPlease do so via ' \
                'command line argument: -filespath /path/to/rootfiles/')

    if not filespath.endswith('/'):
        filespath += '/'

    outpath = filespath + 'output_evt_dic/'
    if not os.path.isdir(outpath):
        os.makedirs(outpath)

    if all_files:
        base = 0

    main()
