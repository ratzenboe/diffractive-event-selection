from __future__ import division

import sys
import os
import time
import csv
from select                                     import select

import matplotlib
matplotlib.use('agg')
import matplotlib.pyplot                        as plt
import seaborn                                  as sns
sns.set()

import numpy                                    as np
import pandas                                   as pd

from sklearn.model_selection                    import train_test_split, GridSearchCV, \
                                                       cross_validate
from sklearn.ensemble                           import GradientBoostingRegressor
from sklearn.svm                                import SVR
from sklearn.linear_model                       import BayesianRidge
from sklearn.metrics                            import mean_squared_error, mean_absolute_error, \
                                                       explained_variance_score, r2_score
from sklearn.externals                          import joblib
from sklearn                                    import preprocessing

from modules.arg_config_parser                  import get_input_args, get_run_params, \
                                                       get_data_params, get_classifier_params 
from modules.logger                             import logger
from modules.load_model                         import load_model
from modules.load_grid                          import load_grid
from modules.model_evaluation                   import model_evaluation
from modules.utils                              import get_output_paths, pause_for_input, \
                                                       print_gridSearch_results, \
                                                       print_CV_results, print_dict
# want to have a wrapper-function that takes in the run_params and depending on these
# calles a series of sub-functions that all do some data cleaning, e.g. removing highly
# correlated columns, removing low variance columns etc
# we should always fill the categorical values before the 
from modules.CleanData                          import rmCorrelated, rmLowVariance, \
                                                       fillCategorical, fillNumerical, \
                                                       containsNAN, doMCA, parseObjectColums
from modules.indivPreprocess                    import preprecessBIKE


def main():

    ######################################################################################
    # STEP 0:
    # ----------------------------- fetching the data ------------------------------------
    ######################################################################################    
    print'\nfetching data...\n'
    data = pd.read_csv(data_params['test_path'], 
                 sep              = data_params['sep'],
                 error_bad_lines  = data_params['error_bad_lines'],
                 low_memory       = data_params['low_memory'],  
                 skip_blank_lines = data_params['skip_blank_lines'], 
                 na_values        = data_params['na_values'],  
                 keep_default_na  = data_params['keep_default_na'],
                 verbose          = data_params['verbose'],
                 header           = data_params['header'])

    X = data.drop(data_params['control'], axis=1)
    control = data[data_params['control']]

    ######################################################################################
    # STEP 1:
    # ------------------------------- Preprocessing --------------------------------------
    ######################################################################################
    if data_params['data'] == 'bike':
        X = preprecessBIKE(X)
    # in the first step we fill up the NAN values, this we can do all the time,
    # not depending on the parser arguments
    if containsNAN(X) == 0:
        X = parseObjectColums(X, run_params)
    if containsNAN(X) > 0 and run_params['fillNAN']:
        # fillCategorical parses all object columns
        # after the next 2 lines the dataframe should only contain
        # ints & floats
        X = fillCategorical(X, run_params)
        X = fillNumerical(X)
    if run_params['rmCorr']:
        X = rmCorrelated(X, run_params)
    if run_params['rmVar']:
        X = rmLowVariance(X, run_params)
    if run_params['doMCA']:
        X = doMCA(X)

    if run_params['stdScale']:
        print'standarad scaling...'
        # returns a numpy array (due to fit_transform function)
        X = standardScale(X, run_params)

    if not run_params['stdScale']:
        X = X.as_matrix()

    print'X shape: {}'.format(X.shape)
    print'\n{}\n'.format(30*'-')

    ######################################################################################
    # STEP 2:
    # ------------------------------ Fitting the model -----------------------------------
    ######################################################################################
    pretrained_model_filename = output_prefix + model_saves_prefix + 'model_save.pkl'
    if not os.path.exists(pretrained_model_filename):
        print'Error loading model. File {} does not exist.'.format(pretrained_model_filename)
        sys.exit(1)
    model = joblib.load(pretrained_model_filename)
    y_score_updated = model.predict(X)

    ######################################################################################
    # STEP 2:
    # ---------------------------- Saving the prediction ---------------------------------
    ######################################################################################
    print('\nClassifying data...')
    y_score_df = pd.DataFrame(np.column_stack((control, y_score_updated)),
                              columns=['\"id\"','\"cnt\"'])

    y_score_df['"id"'] = y_score_df['"id"'].astype(int)
    y_score_df.loc[y_score_df['"cnt"'] <= 0., '"cnt"'] = 0. 
    y_score_df.to_csv(output_prefix + 'BIKE_prediction.csv',
                      sep=data_params['sep'],
                      header=True,
                      index=False,
                      quoting=csv.QUOTE_NONE)


if __name__ == "__main__":


    user_argv = None
    
    if(len(sys.argv) > 1):
        user_argv = sys.argv[1:]
        
    parser_results = get_input_args(user_argv)
    print('Received input arguments: ', parser_results)
    run_params = get_run_params(parser_results)
    data_params = get_data_params(run_params['data_params_id'])
    classifier_params = get_classifier_params(run_params['classifier_params_id'])

    output_prefix, model_saves_prefix = get_output_paths(run_params)

    sys.stdout = logger(run_params)

    main()
