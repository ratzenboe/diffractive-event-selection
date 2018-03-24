from __future__ import division

import sys
import os
import time
from select                                     import select
import argparse 
import ast 
import copy

import fnmatch

import matplotlib
matplotlib.use('agg')
import matplotlib.pyplot                        as plt
import seaborn                                  as sns
sns.set()

import numpy                                    as np
import pandas                                   as pd

from keras.models                               import load_model

from sklearn.model_selection                    import train_test_split
from sklearn.externals                          import joblib

from modules.control                            import config_file_to_dict
from modules.logger                             import logger
from modules.load_model                         import train_model
from modules.data_preparation                   import get_data_dictionary, preprocess, \
                                                       fix_missing_values, shape_data
from modules.utils                              import print_dict, split_dictionary, \
                                                       pause_for_input, get_subsample, \
                                                       print_array_in_dictionary_stats, \
                                                       remove_field_name, flatten_dictionary, \
                                                       special_preprocessing, engineer_features
from modules.file_management                    import OutputManager
from modules.evaluation_plots                   import plot_ROCcurve, plot_MVAoutput, \
                                                       plot_cut_efficiencies, plot_all_features, \
                                                       plot_model_loss, plot_autoencoder_output

def main():

    if (sys.version_info < (3, 0)):
        raise OSError('This program needs python3 to work! Currently {} is used.'.format(
            sys.version_info))

    start_time_main = time.time()

    print('run_mode_user: {}'.format(run_mode_user))
    run_params = config_file_to_dict(config_path + 'run_params.conf')
    data_params = config_file_to_dict(config_path + 'data_params.conf')

    data_params = data_params[run_mode_user]
    run_params = run_params[run_mode_user]

    # here is a collection of variables extracted from the config files
    try:
        # ----------- data-parameters --------------
        path_dic          = data_params['path']
        branches_dic      = data_params['branches']
        max_entries_dic   = data_params['max_entries']
        std_scale_dic     = data_params['std_scale']
        target_list       = data_params['target']
        evt_id_string     = data_params['evt_id']
        cut_dic           = data_params['cut_dic']
        event_string      = data_params['event_string']
        missing_vals_dic  = data_params['missing_values']
        remove_features   = data_params['remove_features']
        # ------------ run-parameters --------------
        do_standard_scale = run_params['do_standard_scale']
        # ------------ model-parametrs -------------
    except KeyError:
        raise KeyError('The variable names in the main either have a typo ' \
                'or do not exist in the config files!')

    try:
        # uncomment the next line if we want to produce the evt-dictionary with the
        # saved pickle files (the event.pkl, etc)
        # raise TypeError('We want to produce the evt_dic again')
        evt_dictionary = get_data_dictionary(output_path + 'evt_dic.pkl')
        print('\n:: Event dictionary loaded from file: {}'.format(
            output_path + 'evt_dic.pkl'))
    except(OSError, IOError, TypeError, ValueError):
        raise IOError('The event dictionary cannot be loaded from {}!'.format(
            output_path+'evt_dic.pkl'))

    evt_dictionary = fix_missing_values(evt_dictionary, missing_vals_dic)
    evt_dictionary = engineer_features(evt_dictionary)[0]
    
    list_of_engineered_features = engineer_features(evt_dictionary, replace=False)[1]

    # remove a feature if it is in the cut_dic and contains no further info 
    for key in evt_dictionary.keys():
        if key == 'target':
            continue
        # if the loaded data contains features that are no longer in the list of 
        # desired features (in the config files) we add them to the remove_features-list
        remove_features.extend(list(set(evt_dictionary[key].dtype.names) - set(branches_dic[key])))
        # remove possible duplicate entries
        remove_features = list(set(remove_features))
        # have to remove feature from the remove-list that we have engineered!
        remove_features = [x for x in remove_features if x not in list_of_engineered_features]
        for feature_name in remove_features:
            evt_dictionary[key] = remove_field_name(evt_dictionary[key], feature_name)
        print('Features left in {}: {}'.format(key, list(evt_dictionary[key].dtype.names)))

    if plot:
        print('\n::  Plotting the features...')
        plot_all_features(evt_dictionary, std_scale_dic, model_path, real_bg=False)
    ######################################################################################
    # STEP 1:
    # ------------------------------- Preprocessing --------------------------------------
    ######################################################################################
    print('\n::  Standarad scaling...')
    # returns a numpy array (due to fit_transform function)
    preprocess(evt_dic, std_scale_dic, model_path, load_fitted_attributes=True)

    # before we lose track of the column names we save the eta-phi-diff columns
    # which we will (is needed for the koala mode)
    eta_phi_dist_feature_arr = evt_dic['event']['eta_phi_diff'].ravel()
 
    #####################################################################################
    # TODO
    # function that extracts the evt-id from each 'event'-array and puts it into a list
    # function that removes the evt-id from the 'event'-array
    #####################################################################################

    print('\n::  Converting the data from numpy record arrays to standard numpy arrays...')
    shape_data(evt_dic)

    evt_dic = special_preprocessing(run_mode_user, 
                                    evt_dic, 
                                    append_array=eta_phi_dist_feature_arr,
                                    flat=flat)[0]


    ######################################################################################
    # STEP 2:
    # ------------------------------- Fitting the model -----------------------------------
    ######################################################################################
    # if we want cross validation (in most cases we do) we can in turn easily evaluate the
    # models by passing which metrics should be looked into

    # Get the best model
    model = load_model(model_path + 'best_model.h5')
    ######################################################################################
    # STEP 3:
    # ----------------------------- Evaluating the model ---------------------------------
    ######################################################################################
    # save the test dictionary for easy testing later on

    print('\nEvaluating the model on the training sample...')
    # returns the poped element
    # evt_dic_train['target'][evt_dic_train['target']==99] = 0
    evt_dic.pop('target')
    y_score = model.predict(evt_dic_train)
    if isinstance(y_score, list):
        # if one or several aux-outputs exist the main output is on the
        # first position in the list
        y_score = y_score[0]

    sig_list = []
    for idx, val in enumerate(y_score):
        if val >= mva_cut:
            # evt_id_list is created in a function that will be created in the near future
            sig_list.append(evt_id_list[idx])

    thefile = open(model_path+outfile, 'w')
    for item in sig_list:
        thefile.write("%s\n" % item)
    

    ######################################################################################
    # ------------------------------------ EOF -------------------------------------------
    # -------------------- print some runtime information --------------------------------
    ######################################################################################
    end_time_main = time.time()
    print('\n{p} RUNTIME {p}'.format(p=20*'-'))
    print('\nTotal runtime: {} seconds'.format(end_time_main - start_time_main))
    print('\nTraining time: {} seconds'.format(end_time_training - start_time_training))
    print('\n{}\n'.format(30*'-'))
    
if __name__ == "__main__":

    output_path = 'output/'
    data_path   = '../data/'
    config_path = 'config/'
    om = OutputManager(output_path, keep_sessions=20)
    sys.stdout = logger(om.get_session_folder())

    # fix random seed for reproducibility
    # np.random.seed(7)

    user_argv = None

    if(len(sys.argv) > 1):
        user_argv = sys.argv[1:]

    # commend line parser (right now not a own function as only 2 elements are used)
    parser = argparse.ArgumentParser()
    parser.add_argument('-run_mode', '-run_setting',
                        help='keyword to identify which run settings to \
			      choose from the config file (default: "run_params")',
                        action='store',
                        dest='run_mode',
                        default='NN',
                        type=str)

    parser.add_argument('-modelpath', 
                        help='str: path where the model can be found',
                        action='store',
                        dest='model_path',
                        default=None,
                        type=str)

    parser.add_argument('-mva_cut', 
                        help='float: cut value that seperates sig-bg',
                        action='store',
                        dest='mva_cut',
                        default=None,
                        type=float)

    parser.add_argument('-outfile', 
                        help='str: filename where the evt-ids will be saved',
                        action='store',
                        dest='outfile',
                        default=None,
                        type=str)


    command_line_args = parser.parse_args(user_argv)

    run_mode_user = command_line_args.run_mode
    model_path = command_line_args.model_path
    mva_cut = command_line_args.mva_cut
    outfile = command_line_args.outfile

    if not outfile:
        outfile = 'signal_evt_ids.txt'

    if not oufile.endswith('.txt'):
        outfile += '.txt'

    if not model_path.endswith('/'):
        model_path += '/'

    if model_path is None or not os.path.isdir(model_path):
        raise IOError('No valid "model_path" provided!\nPlease do so via ' \
                'command line argument: -modelpath /path/to/modelfolder/')

    if not mva_cut or mva_cut < 0. or mva_cut > 1.:
        raise IOError('MVA cut-value has no valid value: {}'.format(mva_cut))

    main()
