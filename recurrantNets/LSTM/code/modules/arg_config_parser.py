import os
import ast 
import sys
import argparse
import ConfigParser

def get_input_args(argv=None):
    
    parser = argparse.ArgumentParser(description='Basic framework for training of machine learning algorithms')

    parser.add_argument('-data',
                        help='specify which dataset to use ' \
                        '(currently available: higgs, breastcancer)',
                        action='store',
                        dest='data',
                        default=None,
                        type=str)

    parser.add_argument('-data_params', '-data_config',
                        help='keyword to identify a data configuration from the data config file',
                        action='store',
                        dest='data_params_id',
                        default=None,
			type=str)

    parser.add_argument('-predict',
                        help='predict mode for preprocessing, returns the files earlier',
                        action="store_true",
                        dest='predict',
                        default=False)

    parser.add_argument('-model',
                        help='specify a model to be used',
                        action='store',
                        dest='model_choice',
                        default='SVR',
                        type=str)

    parser.add_argument('-frac_val_sample',
                        help='fraction of entries in the validation sample',
                        action='store',
                        dest='frac_val_sample',
                        default=0.2,
                        type=float)

    parser.add_argument('-frac_test_sample',
                        help='fraction of entries in the test sample',
                        action='store',
                        dest='frac_test_sample',
                        default=0.1,
                        type=float)

    parser.add_argument('-nentries', '-num_entries',
                        help='specify number of rows to read from the datafile',
                        action='store',
                        dest='num_entries',
                        default=None,
                        type=int)

    parser.add_argument('-CV_nfolds', '-cross_validate_nfolds',
                        help='specify the number of folds used for cross-validation (default: 3)',
                        action='store',
                        dest='CV_nfolds',
                        default=5,
                        type=int)

    parser.add_argument('-cross-validate', '-cross-validation', '-cross_val', '-CV',
                        help='specify whether to perform cross-validation',
                        action='store_true',
                        dest='CV',
                        default=False)

    parser.add_argument('-rmCorr', 
                        help='remove highly correlated columns',
                        action='store_true',
                        dest='rmCorr',
                        default=False)

    parser.add_argument('-correlation', 
                        help='value of the correlation above columns get dropped',
                        action='store',
                        dest='correlation',
                        default=0.9,
                        type=float)
    
    parser.add_argument('-variance', 
                        help='value of the variance below columns get dropped',
                        action='store',
                        dest='variance',
                        default=0.01,
                        type=float)

    parser.add_argument('-rmVar', 
                        help='remove low variance columns',
                        action='store_true',
                        dest='rmVar',
                        default=False)

    parser.add_argument('-fillNAN', '-fillna',
                        help='fill in NAN values',
                        action='store_false',
                        dest='fillNAN',
                        default=True)

    parser.add_argument('-doMCA', '-MCA',
                        help='perform MCA on integer columns',
                        action='store_true',
                        dest='doMCA',
                        default=False)

    parser.add_argument('-stdScale', '-standardScale',
                        help='turn on standarad scaling',
                        action='store_true',
                        dest='stdScale',
                        default=False)

    parser.add_argument('-njobs',
                        help='specify number of parallel processes (default: -1)',
                        action='store',
                        dest='num_jobs',
                        default=-1,
                        type=int)

    parser.add_argument('-verbose',
                        help='make the program more chatty',
                        action='store_true',
                        dest='verbose_setting',
                        default=False)

    parser.add_argument('-classifier_params', '-classifier_config', '-model_params', \
                        '-model_config',
                        help='keyword to identify a classifier configuration from the \
                              classifier config file',
                        action='store',
                        dest='classifier_params_id',
                        default=None,
			type=str)


    return parser.parse_args(argv)


def get_run_params(parser_results):
    """
    Receives parser arguments as input and returns a dictionary of run parameters.
    """

    run_params = {
        'num_jobs':              parser_results.num_jobs,
        'num_entries':           parser_results.num_entries,
        'frac_val_sample':       parser_results.frac_test_sample,
        'frac_test_sample':      parser_results.frac_test_sample,
        'verbose_setting':       parser_results.verbose_setting,
        'stdScale':              parser_results.stdScale,
    }

    # data_params_id: section header in the config file
    if parser_results.data_params_id:
        run_params['data_params_id'] = parser_results.data_params_id
        
    else:
        try:
            run_params['data_params_id'] = os.environ['DATA_PARAMS']
            print'  Using data parameters: {} ' \
                  '(imported from environment variables)'.format(
                      run_params['data_params_id'])
            
        except KeyError:
            print'Error: variable "data_params" not specified. ' \
                  'Define it either via the "-data_params" flag or ' \
                  'set the environment variable DATA_PARAMS and run ' \
                  'the program again.'
            sys.exit(1)

    if parser_results.classifier_params_id:
        run_params['classifier_params_id'] = parser_results.classifier_params_id
        
    else:
        try:
            run_params['classifier_params_id'] = os.environ['MODEL_PARAMS']
            print'  Using model parameters: {} ' \
                  '(imported from environment variables)'.format(
                      run_params['classifier_params_id'])
            
        except KeyError:
            print'Error: variable "model_params" not specified. ' \
                  'Define it either via the "-model_params" flag or ' \
                  'set the environment variable MODEL_PARAMS and run ' \
                  'the program again.'
            sys.exit(1)

    return run_params



def get_data_params(identifier):
    """
    Returns the relevant section parameters (specified by 'identifier') of the data configuration file as a dictionary.
    """

    print'\nImport data configuration: {}'.format(identifier)
    
    config_file_path = 'config/data_params.conf'

    config = ConfigParser.ConfigParser()
    config.read(config_file_path)

    section = identifier
    options = config.options(section)

    data_config = {}

    for option in options:
        try:
            # literal eval trys to parse the value correctly, e.g. an integer
            # is no longer converted to a string, but to an integer
            data_config[option] = ast.literal_eval( config.get(section, option) )
            if data_config[option] == -1:
                print'Skipping: {}'.format(option)

        except:
            print'Exception on {}.'.format(option)
            data_config[option] = None

    return data_config


def get_classifier_params(identifier):
    """
    Returns the relevant section parameters (specified by 'identifier') of the classifier configuration file as a dictionary.
    """

    print'\nImport classifier configuration: {}'.format(identifier)
    
    config_file_path = 'config/classifier_params.conf'

    config = ConfigParser.ConfigParser()
    config.read(config_file_path)

    section = identifier
    options = config.options(section)

    data_config = {}

    for option in options:
        try:
            data_config[option] = ast.literal_eval( config.get(section, option) )
            if data_config[option] == -1:
                print'Skipping: {}'.format(option)

        except:
            print'Exception on {}.'.format(option)
            data_config[option] = None

    return data_config
