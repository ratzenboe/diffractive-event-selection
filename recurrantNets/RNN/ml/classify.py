from __future__ import division

import sys
import os
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
                                                       fix_missing_values, shape_data, \
                                                       get_sub_dictionary
from modules.utils                              import print_dict, split_dictionary, \
                                                       pause_for_input, get_subsample, \
                                                       print_array_in_dictionary_stats, \
                                                       remove_field_name, flatten_dictionary, \
                                                       special_preprocessing, engineer_features, \
                                                       flatten_feature
from modules.file_management                    import OutputManager
from modules.evaluation_plots                   import plot_ROCcurve, plot_MVAoutput, \
                                                       plot_cut_efficiencies, plot_all_features, \
                                                       plot_model_loss, plot_autoencoder_output

def main():

    if (sys.version_info < (3, 0)):
        raise OSError('This program needs python3 to work! Currently {} is used.'.format(
            sys.version_info))

    print('run_mode_user: {}'.format(run_mode_user))
    run_params = config_file_to_dict(config_path + 'run_params.conf')
    data_params = config_file_to_dict(config_path + 'data_params.conf')

    data_params = data_params[run_mode_user]
    run_params = run_params[run_mode_user]

    # here is a collection of variables extracted from the config files
    try:
        # ----------- data-parameters --------------
        branches_dic      = data_params['branches']
        evt_id_string     = data_params['evt_id']
        remove_features   = data_params['remove_features']
        print('evt_id_string: {}'.format(evt_id_string))
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
        evt_dictionary = get_data_dictionary(inpath)
        print('\n:: Event dictionary loaded from file: {}'.format(inpath))
        # train only on events with no clusters
        indices = np.arange(evt_dictionary['event']['has_no_calo_clusters'].shape[0])[(evt_dictionary['event']['has_no_calo_clusters']==True).ravel()]
        for key in evt_dictionary.keys():
                evt_dictionary[key] = evt_dictionary[key][indices]

    except(OSError, IOError, TypeError, ValueError):
        raise IOError('The event dictionary cannot be loaded from {}!'.format(inpath))

    evt_dictionary = get_sub_dictionary(evt_dictionary, branches_dic)

    if evt_id_string in evt_dictionary['event'].dtype.names:
        print('{} in event category!'.format(evt_id_string))
        print('{}'.format(evt_dictionary['event'].dtype.names))
    else:
        print('{} NOT in event category!'.format(evt_id_string))
        print('{}'.format(evt_dictionary['event'].dtype.names))
    evt_dictionary['event'], evt_id_np = remove_field_name(evt_dictionary['event'], evt_id_string)
    print('\nType evt-id-np: {}'.format(type(evt_id_np)))
    print(evt_id_np)

    evt_dictionary, list_of_engineered_features = engineer_features(evt_dictionary, replace=False)
    
    # function that extracts the evt-id from each 'event'-array and puts it into a list
    # we do not need the 99 events any more
    not_99_indices = np.arange(evt_dictionary['target'].shape[0])[evt_dictionary['target']!=99]
    for key in evt_dictionary.keys():
        evt_dictionary[key] = evt_dictionary[key][not_99_indices]

    evt_id_list = list(map(int, evt_id_np.ravel().tolist()))
    # remove a feature if it is in the cut_dic and contains no further info 
    branches_dic['event'].remove(evt_id_string)
    print(remove_features)
    for key in evt_dictionary.keys():
        if key == 'target':
            continue
        remove_features = list(set(remove_features))
        # have to remove feature from the remove-list that we have engineered!
        remove_features = [x for x in remove_features if x not in list_of_engineered_features]
        for feature_name in remove_features:
            print('key: {}'.format(key))
            evt_dictionary[key] = remove_field_name(evt_dictionary[key], feature_name)[0]
        print('Features left in {}: {}'.format(key, list(evt_dictionary[key].dtype.names)))

    if evt_id_string in evt_dictionary['event'].dtype.names:
        raise KeyError('Attention, the event id key is still in the data! '\
                'By not removing it the machine will treat it as a feature') 
    # if plot:
    #     print('\n::  Plotting the features...')
    #     plot_all_features(evt_dictionary, out_path, real_bg=False)

    ######################################################################################
    # STEP 1:
    # ------------------------------- Preprocessing --------------------------------------
    ######################################################################################
    print('\n::  Standarad scaling...')
    # returns a numpy array (due to fit_transform function)
    preprocess(evt_dictionary, model_dir, load_fitted_attributes=True)

    #####################################################################################

    print('\n::  Converting the data from numpy record arrays to standard numpy arrays...')
    shape_data(evt_dictionary)

    # if 'NN' in run_mode_user:
    #     flatten_feature(evt_dictionary, 'track')
    #     evt_dictionary['feature_matrix'] = np.c_[evt_dictionary.pop('track'), 
    #                                              evt_dictionary.pop('event')]
    print_array_in_dictionary_stats(evt_dictionary, 'Training data info:')
    # Get the best model
    model = load_model(model_path)
    ######################################################################################
    # STEP 3:
    # ----------------------------- Evaluating the model ---------------------------------
    ######################################################################################
    # save the test dictionary for easy testing later on

    print('\nEvaluating the model on the training sample...')
    y_target = evt_dictionary.pop('target')
    y_score = model.predict(evt_dictionary)
    if isinstance(y_score, list):
        # if one or several aux-outputs exist the main output is on the
        # first position in the list
        y_score = y_score[0]

    full_recon = 0
    feed_down = 0
    sig_list = []
    for idx, val in enumerate(y_score):
        if val >= mva_cut:
            # evt_id_list is created in a function that will be created in the near future
            sig_list.append(evt_id_list[idx])
            if y_target[idx] == 1:
                full_recon += 1
            else:
                feed_down += 1

    num_trueSignal, num_trueBackgr = plot_MVAoutput(y_target, y_score, 
                                                    model_dir, label='classify')
    if num_trueSignal is not None:
        MVAcut_opt = plot_cut_efficiencies(num_trueSignal, num_trueBackgr, model_dir, 'classify')
        plot_ROCcurve(y_target, y_score, model_dir, label='classify')
    del num_trueSignal, num_trueBackgr



    print('::  Cut quality:')
    print('   Signal events after cut: {}/{}'.format(full_recon, full_recon+feed_down))
    print('   Signal efficiency: {}'.format(full_recon/(full_recon+feed_down)))

    print('::  Saving the predicted signal evts in {}...'.format(model_dir+outfile))
    thefile = open(model_dir+outfile, 'w')
    for item in sig_list:
        thefile.write("%s\n" % item)

    
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
    parser.add_argument('-inpath', '-evtdicpath', '-evtdic',
                        help='path to the event dictionary',
                        action='store',
                        dest='inpath',
                        default='output/evt_dic.pkl',
                        type=str)

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
                        default='signal_evt_ids.txt',
                        type=str)


    command_line_args = parser.parse_args(user_argv)

    run_mode_user = command_line_args.run_mode
    model_path = command_line_args.model_path
    mva_cut = command_line_args.mva_cut
    outfile = command_line_args.outfile
    inpath = command_line_args.inpath

    if not os.path.isfile(inpath) or not inpath.endswith('.pkl'):
        raise IOError('No valid "inpath" for the event dictionary provided!\nPlease do so via ' \
                'command line argument: -inpath /path/to/evt_dic_folder/evt_dic.pkl')


    if not outfile.endswith('.txt'):
        outfile += '.txt'

    if not model_path.endswith('/') and not model_path.endswith('.hdf5'):
        model_path += '/'

    if model_path.endswith('/'):
        model_path += 'best_model.h5'

    model_dir = os.path.split(model_path)[0]+'/'

    if model_path is None or not os.path.isfile(model_path):
        raise IOError('No valid "model_path" provided!\nPlease do so via ' \
                'command line argument: -modelpath /path/to/modelfolder/model_file.hdf5')

    if not mva_cut or mva_cut < 0. or mva_cut > 1.:
        raise IOError('MVA cut-value has no valid value: {}'.format(mva_cut))

    main()
