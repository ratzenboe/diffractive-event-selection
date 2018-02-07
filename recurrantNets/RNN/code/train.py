from __future__ import division

import sys
import os
import time
from select                                     import select
import argparse 
import ast 

import matplotlib
matplotlib.use('agg')
import matplotlib.pyplot                        as plt
import seaborn                                  as sns
sns.set()

import numpy                                    as np

from sklearn.model_selection                    import train_test_split
from sklearn.externals                          import joblib

from modules.control                            import config_file_to_dict
from modules.logger                             import logger
from modules.load_model                         import train_model
from modules.data_preparation                   import get_data, save_data_dictionary, \
                                                       get_data_dictionary
from modules.utils                              import print_dict
from modules.file_management                    import OutputManager

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
    print('\nfetching data...\n')
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

    # here is a collection of variables extracted from the 
    # config files
    path_dic          = data_params['path']
    branches_dic      = data_params['branches']
    max_entries_dic   = data_params['max_entries']
    std_scale_dic     = data_params['std_scale']
    target_list       = data_params['target']
    evt_id_string     = data_params['evt_id']
    n_track_id        = data_params['n_track_id']
    cut_list_n_tracks = data_params['cut_list_n_tracks']
    event_string      = data_params['event_string']

    data_outfile = om.get_session_folder() + 'all_evts.npy'
    try:
        evt_dictionary = get_data_dictionary(data_outfile)
    except (IOError, TypeError):
        evt_dictionary = get_data(branches_dic      = branches_dic, 
                                  max_entries_dic   = max_entries_dic, 
                                  path_dic          = path_dic, 
                                  evt_id_string     = evt_id_string, 
                                  target_list       = target_list,
                                  n_track_id        = n_track_id,
                                  cut_list_n_tracks = cut_list_n_tracks,
                                  event_string      = event_string)

        print('\n:: saving data in {}'.format(data_outfile))
        save_data_dictionary(data_outfile, evt_dictionary)

    print('\n\n:: Loading the data worked well')
    counter = 0

    for key, array in evt_dictionary.iteritems():
        print('\n{}'.format(key))
        print('type(array): {}'.format(type(array)))
        print('array.shape: {}'.format(array.shape))

    # sys.exit(1)
    ######################################################################################
    # STEP 1:
    # ------------------------------- Preprocessing --------------------------------------
    ######################################################################################
    print('Splitting data in training and test sample')
    # output type is the same as input type!
    evt_dic_train, evt_dic_test = split_dictionary(evt_dictionary,
                                                   split_size=run_params['frac_test_sample'])
    del evt_dictionary
    # evt_dic_train, evt_dic_test  = split_dictionary(evt_dic,
    #                                                 split_size=run_params['frac_test_sample'])
    # del evt_dic
        
    if run_params['stdScale']:
        print('standarad scaling...')
        # returns a numpy array (due to fit_transform function)
        preprocess(evt_dic_train, data_params, run_params)
        preprocess(evt_dic_valid, data_params, run_params, load_fitted_attributes=True)
        preprocess(evt_dic_test,  data_params, run_params, load_fitted_attributes=True)

    pause_for_input(run_params, timeout=3)
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
                        val_data    = run_params['frac_val_sample'],
                        batch_size  = run_params['batch_size'],
                        n_epochs    = run_params['n_epochs'],
                        rnn_layer   = model_params['rnn'])
 
    end_time_training = time.time()

    # Save the model
    joblib.dump(model, output_prefix + 'model_save.pkl')
    ######################################################################################
    # STEP 3:
    # ----------------------------- Evaluating the model ---------------------------------
    ######################################################################################
    model_evaluation(X_test, y_test, model)
    ######################################################################################
    # ------------------------------------ EOF -------------------------------------------
    # -------------------- print some runtime information --------------------------------
    ######################################################################################
    end_time_main = time.time()
    print('\n{}'.format(19*'-'))
    print('- RUNTIME -')
    print('{}'.format(19*'-'))
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
