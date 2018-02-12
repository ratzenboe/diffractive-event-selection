from __future__ import division

import sys
import os
import time
from select                                     import select
import argparse 
import ast 
import copy

import matplotlib
matplotlib.use('agg')
import matplotlib.pyplot                        as plt
import seaborn                                  as sns
sns.set()

import numpy                                    as np

from keras.models                               import load_model

from sklearn.model_selection                    import train_test_split
from sklearn.externals                          import joblib

from modules.control                            import config_file_to_dict
from modules.logger                             import logger
from modules.load_model_save                    import train_model
from modules.data_preparation                   import get_data, save_data_dictionary, \
                                                       get_data_dictionary, preprocess, \
                                                       fix_missing_values, shape_data
from modules.utils                              import print_dict, split_dictionary, \
                                                       pause_for_input, \
                                                       print_array_in_dictionary_stats
from modules.file_management                    import OutputManager
from modules.evaluation_plots                   import plot_ROCcurve, plot_MVAoutput, \
                                                       plot_cut_efficiencies

def main():

    start_time_main = time.time()
    ######################################################################################
    # STEP 0:
    # ----------------------------- fetching the data ------------------------------------
    # we fetch the data from 9 different root files where detector data is saved
    #       - AD
    #       - FMD
    #       - V0
    #       - EMC
    #       - PHOS
    #       - Evt-level data
    #       - tracking data
    #       - raw tracking data
    #       - calo cluster data
    # 
    # therefore we want to have a data structure where we have a dataframe for each  
    # of these 9 departments
    ######################################################################################    
    # data is a dictionary which contains the 9 numpy arrays in form 
    # -> data['raw_track'].shape = (n_evts, n_parts, n_features)
    #    data['fmd'].shape = (n_evts, n_cells, n_features)
    #    data['target'].shape = (n_evts, )
    #    etc. 
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
        path_dic          = data_params['path']
        branches_dic      = data_params['branches']
        max_entries_dic   = data_params['max_entries']
        std_scale_dic     = data_params['std_scale']
        target_list       = data_params['target']
        evt_id_string     = data_params['evt_id']
        n_track_id        = data_params['n_track_id']
        cut_list_n_tracks = data_params['cut_list_n_tracks']
        event_string      = data_params['event_string']
        missing_vals_dic  = data_params['missing_values']
        # ------------ run-parameters --------------
        frac_test_sample  = run_params['frac_test_sample']
        frac_val_sample   = run_params['frac_val_sample']
        batch_size        = run_params['batch_size']
        n_epochs          = run_params['n_epochs']
        do_standard_scale = run_params['do_standard_scale']
        # ------------ model-parametrs -------------
        rnn_layer         = model_params['rnn']
    except KeyError:
        raise KeyError('The variable names in the main either have a typo ' \
                'or do not exist in the config files!')

    out_path = om.get_session_folder()
    try:
        evt_dic_train = get_data_dictionary(output_path + 'evt_dic_train.npy')
        evt_dic_test = get_data_dictionary(output_path + 'evt_dic_test.npy')
        print('\n:: Event dictionary loaded from file: {}'.format(data_outfile))
    except (IOError, TypeError, ValueError):
        print('\nfetching data...\n')
        evt_dictionary = get_data(branches_dic      = branches_dic, 
                                  max_entries_dic   = max_entries_dic, 
                                  path_dic          = path_dic, 
                                  evt_id_string     = evt_id_string, 
                                  target_list       = target_list,
                                  n_track_id        = n_track_id,
                                  cut_list_n_tracks = cut_list_n_tracks,
                                  event_string      = event_string)


        evt_dictionary = fix_missing_values(evt_dictionary, missing_vals_dic)
        # saving the data as numpy record array

        ######################################################################################
        # STEP 1:
        # ------------------------------- Preprocessing --------------------------------------
        ######################################################################################
        print('\n:: Splitting data in training and test sample')
        # output type is the same as input type!
        evt_dic_train, evt_dic_test = split_dictionary(evt_dictionary, split_size=frac_test_sample)
        del evt_dictionary
        # evt_dic_train, evt_dic_test  = split_dictionary(evt_dic,
        #                                                 split_size=run_params['frac_test_sample'])
        # del evt_dic
            
        if do_standard_scale:
            print('\n:: Standarad scaling...')
            # returns a numpy array (due to fit_transform function)
            preprocess(evt_dic_train, std_scale_dic, out_path, load_fitted_attributes=False)
            preprocess(evt_dic_test,  std_scale_dic, out_path, load_fitted_attributes=True)

        print('\n:: Converting the data from numpy record arrays to standard numpy arrays...')
        shape_data(evt_dic_train)
        shape_data(evt_dic_test)

        print('\n:: Saving data in {}\n\n'.format(output_path+'evt_dic_train.npy'))
        save_data_dictionary(output_path + 'evt_dic_train.npy', evt_dic_train)
        save_data_dictionary(output_path + 'evt_dic_test.npy', evt_dic_test)

    print_array_in_dictionary_stats(evt_dic_train, 'Training data info:')
    print_array_in_dictionary_stats(evt_dic_test, 'Test data info:')
    

    pause_for_input('\n\n:: The model will be trained anew '\
            'if this is not desired please hit enter', timeout=1)
    ######################################################################################
    # STEP 2:
    # ------------------------------- Fitting the model -----------------------------------
    ######################################################################################
    # if we want cross validation (in most cases we do) we can in turn easily evaluate the
    # models by passing which metrics should be looked into
    start_time_training = time.time()

    print('\nFitting the model...')
    model = train_model(evt_dic_train, 
                        run_mode_user, 
                        val_data    = frac_val_sample,
                        batch_size  = batch_size,
                        n_epochs    = n_epochs,
                        rnn_layer   = rnn_layer,
                        out_path    = out_path)

    end_time_training = time.time()
    print('\n:: Finished training!')

    # Get the best model
    # model = load_model(out_path + 'weights_final.hdf5')

    ######################################################################################
    # STEP 3:
    # ----------------------------- Evaluating the model ---------------------------------
    ######################################################################################
    print('\nEvaluating the model on the training sample...')
    y_train_truth = evt_dic_train['target']
    # to predict the labels we have to ged rid of the target:
    evt_dic_train.pop('target')
    y_train_score = model.predict(evt_dic_train)

    num_trueSignal, num_trueBackgr = plot_MVAoutput(y_train_truth, y_train_score, 
                                                    out_path, label='train')

    MVAcut_opt = plot_cut_efficiencies(num_trueSignal, num_trueBackgr, out_path)

    plot_ROCcurve(y_train_truth, y_train_score, out_path, label='train')

    del y_train_truth, y_train_score
    del num_trueSignal, num_trueBackgr
    del evt_dic_train

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
    np.random.seed(7)

    user_argv = None

    if(len(sys.argv) > 1):
        user_argv = sys.argv[1:]

    parser = argparse.ArgumentParser()
    parser.add_argument('-run_mode', '-run_setting',
                        help='keyword to identify which run settings to \
			      choose from the config file (default: "run_params")',
                        action='store',
                        dest='run_mode',
                        default='SimpleGrid',
                        type=str)
    command_line_args = parser.parse_args(user_argv)

    run_mode_user = command_line_args.run_mode

    main()
