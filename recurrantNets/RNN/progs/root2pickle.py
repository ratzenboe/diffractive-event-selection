from __future__ import division

import sys
import os
import argparse 

import numpy as np
import pandas as pd
import pickle
import fnmatch

import root_numpy

def main():

    if not sys.version_info[:2] == (2, 7):
        raise OSError('The python version must be 2.7 but instead ' \
            'version {} is used!'.format(sys.version))

    files_list = []
    for filename in os.listdir(filespath):
        # take only root files
        if fnmatch.fnmatch(filename, '*.root'):
            files_list.append(filename)

    files_list.sort()
    print('{} root files found'.format(len(files_list)))

    # the fuction partition return as the first element 
    # the part of the string before the part '_info'
    # as the files have the tree-name saved in their name we use this to determine 
    # each individual name
    numb_files = nfiles
    if all_files:
        numb_files = len(files_list)
    if base < len(files_list):
        for i in range(base, base+numb_files):
            if i >= len(files_list):
                break
            
            treename = files_list[i].partition('_info')[0]
            print('::  converting {} into pickle format'.format(files_list[i]))
            data = pd.DataFrame(root_numpy.root2array(filespath+files_list[i], treename=treename))
            data = data.astype(float)
            # [:-5] cuts away the .root ending
            data.to_pickle(outpath + files_list[i][:-5] + '_data_pandas.pkl')
            print('::  Saving pickled file in {}'.format(outpath + files_list[i][:-5] + '_data_pandas.pkl\n'))
    

if __name__ == "__main__":

    user_argv = None

    if(len(sys.argv) > 1):
        user_argv = sys.argv[1:]

    # commend line parser (right now not a own function as only 2 elements are used)
    parser = argparse.ArgumentParser()
    parser.add_argument('-filespath', '-rootfilesdir',
                        help='string (mandatory): the path where the root files are stored',
                        action='store',
                        dest='filespath',
                        default=None,
                        type=str)

    parser.add_argument('-basefile', '-base',
                        help='int: the lowest number corresponding to the *_base.root file \
                                files from this number on will be processed',
                        action='store',
                        dest='base',
                        default=0,
                        type=int)

    parser.add_argument('-nfiles', 
                        help='int: number of files to be processed',
                        action='store',
                        dest='nfiles',
                        default=1,
                        type=int)

    parser.add_argument('-all_files', 
                        help='bool: if true all files are processed at once',
                        action='store_true',
                        dest='all_files',
                        default=False)

    command_line_args = parser.parse_args(user_argv)

    base = command_line_args.base
    nfiles = command_line_args.nfiles
    all_files = command_line_args.all_files
    filespath = command_line_args.filespath

    if filespath is None or not os.path.isdir(filespath):
        raise IOError('No valid input path provided!\nPlease do so via ' \
                'command line argument: -filespath /path/to/rootfiles/')

    if not filespath.endswith('/'):
        filespath += '/'

    outpath = filespath + 'output_pickle_files/'
    if not os.path.isdir(outpath):
        os.makedirs(outpath)

    if all_files:
        base = 0

    main()
