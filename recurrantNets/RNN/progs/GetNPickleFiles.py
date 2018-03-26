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

from tqdm import tqdm

try:
    import configparser
except ImportError:
    import ConfigParser as configparser


def main():

    if (sys.version_info < (3, 0)):
        raise OSError('Python 3+ is needed for this program to work ' \
            'but version {} is used!'.format(sys.version))

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
    print(len(list_of_path_ints[0]))

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

    command_line_args = parser.parse_args(user_argv)

    filespath = command_line_args.filespath

    if filespath is None or not os.path.isdir(filespath):
        raise IOError('No valid input path provided!\nPlease do so via ' \
                'command line argument: -filespath /path/to/rootfiles/')

    if not filespath.endswith('/'):
        filespath += '/'

    main()
