from __future__ import division

import sys
import os
import time
from select                                     import select

import matplotlib
matplotlib.use('agg')
import matplotlib.pyplot                        as plt
import seaborn                                  as sns
sns.set()

import numpy                                    as np
import pandas                                   as pd

from sklearn.model_selection                    import train_test_split
from sklearn.externals                          import joblib
from sklearn                                    import preprocessing

from modules.arg_config_parser                  import get_input_args, get_run_params, \
                                                       get_data_params, get_classifier_params 
from modules.logger                             import logger
from modules.load_model                         import load_model
from modules.model_evaluation                   import model_evaluation
from modules.utils                              import get_output_paths, pause_for_input, \
                                                       print_gridSearch_results, \
                                                       print_CV_results, print_dict
from modules.CleanData                          import standardScale 

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
    print'\nfetching data...\n'
    # data is a dictionary which contains the 9 dataframes. 
    # the dictonary 'event' contained the target 'mc_process_type' which is moved to y
    X,y = load_data(data_params)
    
    ######################################################################################
    # STEP 1:
    # ------------------------------- Preprocessing --------------------------------------
    ######################################################################################
    print'X shape: {}'.format(X.shape)
    print'\n{}\n'.format(30*'-')
    print'Splitting data in training and test sample'
    # output type is the same as input type!
    X_train, X_test, y_train, y_test = train_test_split(X, y,
                                            test_size=run_params['frac_test_sample'])
        
    if run_params['stdScale']:
        print'standarad scaling...'
        # returns a numpy array (due to fit_transform function)
        X_train = standardScale(X_train, run_params)
        X_test  = standardScale(X_test,  run_params, load_fitted_attributes=True)

    # want the data in a numpy format
    if not run_params['stdScale']:
        X_train = X_train.as_matrix()
        X_test  = X_test.as_matrix()
    y_train = y_train.as_matrix()
    y_test  = y_test.as_matrix()

    pause_for_input(run_params, timeout=3)
    model = load_model(run_params, classifier_params)
    ######################################################################################
    # STEP 2:
    # ------------------------------- Fitting the model -----------------------------------
    ######################################################################################
    # if we want cross validation (in most cases we do) we can in turn easily evaluate the
    # models by passing which metrics should be looked into
    start_time_training = time.time()
    if not run_params['CV']:
        print'\nGrid search on {}'.format(run_params['classifier_params_id'])
        param_grid = load_grid(run_params, classifier_params)
        model = GridSearchCV(model, param_grid         = param_grid,
                                    scoring            = data_params['cross_validate_metrics'],
                                    cv                 = run_params['CV_nfolds'],
                                    refit              = data_params['gs_refit'],
                                    return_train_score = True,
                                    verbose            = 1,
                                    n_jobs             = run_params['num_jobs'],
                                    pre_dispatch       = '2*n_jobs')
    else:
        print'\nCross validation...'
        scores = cross_validate(model, X_train, y_train,
                                scoring            = data_params['cross_validate_metrics'],
                                cv                 = run_params['CV_nfolds'],
                                return_train_score = True)
    # we have to fit the model anyhow (even with CV) to make evaluations on the test
    # data sampple
    print'\nFitting the model...'
    model = model.fit(X_train, y_train)
    if run_params['CV']:
        print_CV_results(scores)
    else:
        print_gridSearch_results(model, data_params, run_params)
    # save the training time
    end_time_training = time.time()

    # Save the model
    joblib.dump(model, output_prefix + model_saves_prefix + 'model_save.pkl')
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
    print'\n{}'.format(19*'-')
    print'- RUNTIME -'
    print'{}'.format(19*'-')
    print'\nTotal runtime: {} seconds'.format(end_time_main - start_time_main)
    print'\nTraining time: {} seconds'.format(end_time_training - start_time_training)
    print'\n{}\n'.format(30*'-')
    
if __name__ == "__main__":


    user_argv = None
    
    if(len(sys.argv) > 1):
        user_argv = sys.argv[1:]
        
    parser_results = get_input_args(user_argv)
    print('Received input arguments: ', parser_results)
    run_params = get_run_params(parser_results)
    sys.stdout = logger(run_params)
    print_dict(run_params, 'Continue with the following run parameters:')

    data_params = get_data_params(run_params['data_params_id'])
    print_dict(data_params)

    classifier_params = get_classifier_params(run_params['classifier_params_id'])
    print_dict(classifier_params)

    output_prefix, model_saves_prefix = get_output_paths(run_params)


    main()
