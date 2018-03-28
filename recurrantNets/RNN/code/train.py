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
                                                       fix_missing_values, shape_data, \
                                                       get_sub_dictionary, save_data_dictionary
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

    start_time_main = time.time()

    print('run_mode_user: {}'.format(run_mode_user))
    run_params = config_file_to_dict(config_path + 'run_params.conf')
    data_params = config_file_to_dict(config_path + 'data_params.conf')
    model_params = config_file_to_dict(config_path + 'model_params.conf')

    data_params = data_params[run_mode_user]
    model_params = model_params[run_mode_user]
    run_params = run_params[run_mode_user]

    # here is a collection of variables extracted from the config files
    try:
        # ----------- data-parameters --------------
        branches_dic      = data_params['branches']
        evt_id_string     = data_params['evt_id']
        cut_dic           = data_params['cut_dic']
        missing_vals_dic  = data_params['missing_values']
        remove_features   = data_params['remove_features']
        # ------------ run-parameters --------------
        frac_test_sample  = run_params['frac_test_sample']
        frac_val_sample   = run_params['frac_val_sample']
        do_standard_scale = run_params['do_standard_scale']
        # ------------ model-parametrs -------------
        rnn_layer         = model_params['rnn']
        batch_size        = model_params['batch_size']
        n_epochs          = model_params['n_epochs']
        dropout           = model_params['dropout']
        class_weight      = model_params['class_weight']
        activation        = model_params['activation']
        n_layers          = model_params['n_layers']
        layer_nodes       = model_params['layer_nodes']
        batch_norm        = model_params['batch_norm']
        # printing model parameters:
        print('\n{}'.format(30*'-'))
        print('::  Model Parameters:')
        print('::    rnn_layer: {}'.format(rnn_layer))
        print('::    batch_size: {}'.format(batch_size))
        print('::    n_epochs: {}'.format(n_epochs))
        print('::    dropout: {}'.format(dropout))
        print('::    class_weight: {}'.format(class_weight))
        print('::    activation: {}'.format(activation))
        print('::    n_layers: {}'.format(n_layers))
        print('::    layer_nodes: {}'.format(layer_nodes))
        print('::    batch_norm: {}'.format(batch_norm))
        print('::    aux-output: {}'.format(aux))
        print('::    flatten: {}'.format(flat))
        print('{}'.format(30*'-'))
    except KeyError:
        raise KeyError('The variable names in the main either have a typo ' \
                'or do not exist in the config files!')

    out_path = om.get_session_folder()
    try:
        # uncomment the next line if we want to produce the evt-dictionary with the
        # saved pickle files (the event.pkl, etc)
        # raise TypeError('We want to produce the evt_dic again')
        evt_dictionary = get_data_dictionary(inpath)
        print('\n:: Event dictionary loaded from file: {}'.format(
            output_path + 'evt_dic.pkl'))
        evt_dictionary = get_sub_dictionary(evt_dictionary, branches_dic)
    except(OSError, IOError, TypeError, ValueError):
        raise IOError('The event dictionary cannot be loaded from {}!'.format(inpath))

    evt_dictionary = fix_missing_values(evt_dictionary, missing_vals_dic)
    evt_dictionary, list_of_engineered_features = engineer_features(evt_dictionary, replace=False)
    
    # remove a feature if it is in the cut_dic and contains no further info 
    remove_features.append(evt_id_string)
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

    if 'koala' not in run_mode_user:
        not_99_indices = np.arange(evt_dictionary['target'].shape[0])[evt_dictionary['target']!=99]
        for key in evt_dictionary.keys():
            evt_dictionary[key] = evt_dictionary[key][not_99_indices]

    ######################################################################################
    # STEP 1:
    # ------------------------------- Preprocessing --------------------------------------
    ######################################################################################

    print('\n:: Splitting data in training and test sample')
    # output type is the same as input type!
    evt_dic, evt_dic_val = split_dictionary(evt_dictionary, split_size=frac_val_sample)
    del evt_dictionary
    evt_dic_train, evt_dic_test  = split_dictionary(evt_dic, split_size=frac_test_sample)
    del evt_dic

    print('\n::  Standarad scaling...')
    # returns a numpy array (due to fit_transform function)
    preprocess(evt_dic_train, out_path, load_fitted_attributes=False)
    preprocess(evt_dic_test,  out_path, load_fitted_attributes=True)
    preprocess(evt_dic_val,   out_path, load_fitted_attributes=True)

    if plot:
        print('\n::  Plotting the standard scaled features...')
        plot_all_features(evt_dic_train, out_path, post_fix='_std_scaled', real_bg=False)

    # before we lose track of the column names we save the eta-phi-diff columns
    # which we will (is needed for the koala mode)
    eta_phi_dist_feature_arr_train = evt_dic_train['event']['eta_phi_diff'].ravel()
    eta_phi_dist_feature_arr_val   = evt_dic_val['event']['eta_phi_diff'].ravel()
    eta_phi_dist_feature_arr_test  = evt_dic_test['event']['eta_phi_diff'].ravel()
 
    print('\n::  Converting the data from numpy record arrays to standard numpy arrays...')
    evt_dic_train, feature_names_dic = shape_data(evt_dic_train)
    shape_data(evt_dic_test)
    shape_data(evt_dic_val)

    if 'NN' in run_mode_user:
        flatten_feature(evt_dic_train, 'track')
        evt_dic_train['feature_matrix'] = np.c_[evt_dic_train.pop('track'), 
                                                evt_dic_train.pop('event')]
       
        flatten_feature(evt_dic_test, 'track')
        evt_dic_test['feature_matrix'] = np.c_[evt_dic_test.pop('track'), 
                                               evt_dic_test.pop('event')]
       
        flatten_feature(evt_dic_val, 'track')
        evt_dic_val['feature_matrix'] = np.c_[evt_dic_val.pop('track'), 
                                              evt_dic_val.pop('event')]
  
    print_dict(evt_dic_train)

    # saveing the feature_list to do shap predictions
    # om.save(feature_lst, 'feature_list')

    print_array_in_dictionary_stats(evt_dic_train, 'Training data info:')
    print_array_in_dictionary_stats(evt_dic_test, 'Test data info:')
    print_array_in_dictionary_stats(evt_dic_val, 'Validation data info:')

    ######################################################################################
    # STEP 2:
    # ------------------------------- Fitting the model -----------------------------------
    ######################################################################################
    # if we want cross validation (in most cases we do) we can in turn easily evaluate the
    # models by passing which metrics should be looked into

    if 'koala' in run_mode_user:
        y_val_data[(y_val_data==1) | (y_val_data==0)] = 1
        y_val_data[y_val_data==99] = 0
        if aux:
            y_val_data = {
                          'main_output': evt_dic_val['target'], 
                          'aux_output': evt_dic_val['target']
                         }
            evt_dic_val.pop('target')
        else: 
            y_val_data = evt_dic_val.pop('target')
        X_val_data = evt_dic_val

    else:
        if aux:
            y_val_data = {
                          'main_output': evt_dic_val['target'], 
                          'aux_output': evt_dic_val['target']
                         }
            evt_dic_val.pop('target')
        else:
            y_val_data = evt_dic_val.pop('target')

        X_val_data = evt_dic_val

    # to predict the labels we have to ged rid of the target:
    start_time_training = time.time()

    print('\nFitting the model...')
    pause_for_input('\n\n:: The model will be trained anew '\
            'if this is not desired please hit enter', timeout=5)
    history = train_model(evt_dic_train,
                          run_mode_user, 
                          val_data    = (X_val_data, y_val_data),
                          batch_size  = batch_size,
                          n_epochs    = n_epochs,
                          rnn_layer   = rnn_layer,
                          out_path    = out_path,
                          dropout     = dropout,
                          class_weight = class_weight,
                          n_layers    = n_layers,
                          layer_nodes = layer_nodes, 
                          batch_norm  = batch_norm,
                          activation  = activation,
                          flat        = flat,
                          aux         = aux)

    plot_model_loss(history, out_path)
    print('\n:: Finished training!')
    load_model_path = out_path + 'best_model.h5'

    end_time_training = time.time()

    # Get the best model
    model = load_model(load_model_path)
    ######################################################################################
    # STEP 3:
    # ----------------------------- Evaluating the model ---------------------------------
    ######################################################################################
    # save the test dictionary for easy testing later on
    save_data_dictionary(out_path+'evt_dic_train.pkl', evt_dic_train)

    print('\nEvaluating the model on the training sample...')
    # returns the poped element
    # evt_dic_train['target'][evt_dic_train['target']==99] = 0
    y_train_truth = evt_dic_train.pop('target')
    y_train_score = model.predict(evt_dic_train)
    if isinstance(y_train_score, list):
        # if one or several aux-outputs exist the main output is on the
        # first position in the list
        y_train_score = y_train_score[0]

    if not 'anomaly' in run_mode_user:
        num_trueSignal, num_trueBackgr = plot_MVAoutput(y_train_truth, y_train_score, 
                                                        out_path, label='train')
        MVAcut_opt = plot_cut_efficiencies(num_trueSignal, num_trueBackgr, out_path)
        del num_trueSignal, num_trueBackgr
    else:
        y_train_truth, y_train_score = plot_autoencoder_output(y_train_score, 
                                                               evt_dic_train['feature_matrix'],
                                                               y_train_truth,
                                                               out_path,
                                                               label='train')

    plot_ROCcurve(y_train_truth, y_train_score, out_path, label='train')

    del y_train_truth, y_train_score
    del evt_dic_train

    save_data_dictionary(out_path+'evt_dic_test.pkl', evt_dic_test)

    print('\n::  Evaluating the model on the test sample...')
    # evt_dic_test['target'][evt_dic_test['target']==99] = 0
    print_array_in_dictionary_stats(evt_dic_test, 'Test data info:')
    y_test_truth = evt_dic_test.pop('target')
    # to predict the labels we have to ged rid of the target:
    y_test_score = model.predict(evt_dic_test)
    if isinstance(y_test_score, list):
        y_test_score = y_test_score[0]

    if not 'anomaly' in run_mode_user:
        num_trueSignal, num_trueBackgr = plot_MVAoutput(y_test_truth, y_test_score, 
                                                        out_path, label='test')
        MVAcut_opt = plot_cut_efficiencies(num_trueSignal, num_trueBackgr, out_path)
        del num_trueSignal, num_trueBackgr
    else:
        y_test_truth, y_test_score = plot_autoencoder_output(y_test_score, 
                                                             evt_dic_test['feature_matrix'],
                                                             y_test_truth,
                                                             out_path,
                                                             label='test')

    plot_ROCcurve(y_test_truth, y_test_score, out_path, label='test')

    del y_test_truth, y_test_score
    del evt_dic_test

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

    parser.add_argument('-plot', 
                        help='bool: if used the feature plots will be produced \
			      this usually takes a long time',
                        action='store_true',
                        dest='plot',
                        default=False)

    parser.add_argument('-flat', 
                        help='bool: has only an effect if run_mode NN is used \
                              if used then the rnn structure of the tracks is flattend',
                        action='store_true',
                        dest='flat',
                        default=False)

    parser.add_argument('-aux', 
                        help='bool: has only an effect if run_mode NN is used \
                              if used then an auxiliary output is used after the concatination',
                        action='store_true',
                        dest='aux',
                        default=False)

    command_line_args = parser.parse_args(user_argv)

    run_mode_user = command_line_args.run_mode
    plot = command_line_args.plot
    flat = command_line_args.flat
    aux = command_line_args.aux
    inpath = command_line_args.inpath

    if not os.path.isfile(inpath) or not inpath.endswith('.pkl'):
        raise IOError('No valid "inpath" for the event dictionary provided!\nPlease do so via ' \
                'command line argument: -inpath /path/to/evt_dic_folder/evt_dic.pkl')

    main()
