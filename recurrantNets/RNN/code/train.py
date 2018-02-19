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

from keras.models                               import load_model

from sklearn.model_selection                    import train_test_split
from sklearn.externals                          import joblib

from modules.control                            import config_file_to_dict
from modules.logger                             import logger
from modules.load_model                         import train_model
from modules.data_preparation                   import get_data, save_data_dictionary, \
                                                       get_data_dictionary, preprocess, \
                                                       fix_missing_values, shape_data, \
                                                       unpack_and_order_data_path
from modules.utils                              import print_dict, split_dictionary, \
                                                       pause_for_input, \
                                                       print_array_in_dictionary_stats
from modules.file_management                    import OutputManager
from modules.evaluation_plots                   import plot_ROCcurve, plot_MVAoutput, \
                                                       plot_cut_efficiencies

def main():

    if (sys.version_info > (3, 0)):
        load_pandas = True
        save_pandas = False
    else:
        # for python 2.7 we can load the data from root_numpy
        # in python 3+ we have to load it from a pickeled pandas file
        load_pandas = False
        save_pandas = True

    print('\n\n:: Running mode \n::    load_pandas: {} \n::    save_pandas: {}'.format(
        load_pandas, save_pandas))
    pause_for_input('You may review the load and save state of the program.', timeout=15)

    start_time_main = time.time()
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
        cut_dic           = data_params['cut_dic']
        event_string      = data_params['event_string']
        missing_vals_dic  = data_params['missing_values']
        # ------------ run-parameters --------------
        frac_test_sample  = run_params['frac_test_sample']
        frac_val_sample   = run_params['frac_val_sample']
        batch_size        = run_params['batch_size']
        n_epochs          = run_params['n_epochs']
        dropout           = run_params['dropout']
        class_weight      = run_params['class_weight']
        do_standard_scale = run_params['do_standard_scale']
        # ------------ model-parametrs -------------
        rnn_layer         = model_params['rnn']
    except KeyError:
        raise KeyError('The variable names in the main either have a typo ' \
                'or do not exist in the config files!')

    out_path = om.get_session_folder()
    try:
        evt_dictionary = get_data_dictionary(output_path + 'evt_dic.pkl')
        print('\n:: Event dictionary loaded from file: {}'.format(output_path + 'evt_dic.pkl'))
    except (IOError, TypeError, ValueError):
        # first we have to check the path variables which can refer to multiple paths
        # -> we loop though all paths and add the each event dictionary to a global one
        all_paths_dic, num_paths = unpack_and_order_data_path(path_dic)
        # create final event dictionary, target is always in it
        # the rest in added a few lines below
        evt_dictionary = {'target': []}
        # fill the (final) evt_dictionary with the correct keys
        # and empty arrays
        for key in path_dic.keys():
            evt_dictionary[key] = []

        temp_path_dic = {}
        for i in range(num_paths):
            # change path_dic to file-path i
            try:
                for key in path_dic.keys():
                    # all_paths_dic[key][i] access the path list key (e.g. event)
                    # and takes the i'th position (=i'th path) out and writes is to
                    # the temporary path dictionary: temp_path_dic which is passed to the 
                    # function
                    temp_path_dic[key] = all_paths_dic[key][i]

                print_dict(temp_path_dic)

            except KeyError:
                raise KeyError('The dictionary "path_dic" and "all_paths_dic" ' \
                    'contain a different set of keys!')


            print('\nfetching data...\n')
            tmp_evt_dictionary = get_data(branches_dic      = branches_dic, 
                                          max_entries_dic   = max_entries_dic, 
                                          path_dic          = temp_path_dic, 
                                          evt_id_string     = evt_id_string, 
                                          target_list       = target_list,
                                          cut_dic           = cut_dic,
                                          event_string      = event_string,
                                          save              = save_pandas,
                                          load              = load_pandas)

            print('type(target): {}'.format(type(tmp_evt_dictionary['target'])))
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

        evt_dictionary = fix_missing_values(evt_dictionary, missing_vals_dic)
        save_data_dictionary(output_path + 'evt_dic.pkl', evt_dictionary)
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
                        out_path    = out_path,
                        dropout     = dropout,
                        class_weight = class_weight)

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
