import sys
import os
import argparse 
import warnings
import pickle
import fnmatch

import numpy as np
import pandas as pd


def main():

    if (sys.version_info < (3, 0)):
        raise OSError('Python 3+ is needed for this program to work ' \
            'but version {} is used!'.format(sys.version))

    files_list = []
    for filename in os.listdir(filespath):
        # take only root files
        if fnmatch.fnmatch(filename, 'evt_dic*.pkl'):
            files_list.append(filename)

    files_list.sort()
    print('Files: {}'.format(files_list))
    if not files_list: 
        raise IOError('There are no event dictionaries in the path {}!'.format(filespath)) 

    if len(files_list) is 1:
        print('There is only one file in the input path; no need to concatenate')
        return

    lst_of_evt_dics = []
    # load all event dictionaries 
    for filename in files_list:
        if not os.path.isfile(filespath + filename):
            raise IOError('File {} does not exist.'.format(filespath + filename))

        with open(filespath + filename, "rb") as f:
            lst_of_evt_dics.append(pickle.load(f))

    tot_evt_dic = {}
    for key in lst_of_evt_dics[0].keys():
        tot_evt_dic[key] = np.concatenate([evt_dic[key] for evt_dic in lst_of_evt_dics])

    print('::  Pickling the concatenated event dictionary to {}'.format(outfile))
    with open(outfile, 'wb') as f:
        pickle.dump(tot_evt_dic, f)

    return

   
if __name__ == "__main__":

    user_argv = None

    if(len(sys.argv) > 1):
        user_argv = sys.argv[1:]

    # commend line parser (right now not a own function as only 2 elements are used)
    parser = argparse.ArgumentParser()
    parser.add_argument('-filespath', '-evtfilesdir',
                        help='string (mandatory): the path where the evt_dictionary files are stored',
                        action='store',
                        dest='filespath',
                        default=None,
                        type=str)

    parser.add_argument('-outfile', 
                        help='string: the path+filename of the output file; default evt_dic_all.pkl', 
                        action='store',
                        dest='outfile',
                        default=None,
                        type=str)


    command_line_args = parser.parse_args(user_argv)

    filespath = command_line_args.filespath
    outfile = command_line_args.outfile

    if filespath is None or not os.path.isdir(filespath):
        raise IOError('No valid input path provided!\nPlease do so via ' \
                'command line argument: -filespath /path/to/rootfiles/')

    if not filespath.endswith('/'):
        filespath += '/'

    if not outfile:
        outfile = filespath + 'evt_dic_all.pkl'

    main()
