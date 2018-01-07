import os
import math
import sys

import numpy as np
import root_numpy
import pandas as pd
import h5py

import matplotlib
import matplotlib.pyplot as plt

from lorentz_vec import get_p_e 
from bethe_bloch import ALEPH_bethe_bloch_ToF_beta

#fist we vectorize the functions get_p_e and ALEPH_bethe_bloch_ToF_beta
bb_ToF_vect = np.vectorize(ALEPH_bethe_bloch_ToF_beta)
get_p_e_vect = np.vectorize(get_p_e)


def unison_shuffled_copies( a, b ):
    assert len(a) == len(b)
    p = np.random.permutation(len(a))
    return a[p], b[p]

# first: we need a function that makes a histogram and puts it into an array
# the function puts pT into the eta-phi place, for all entries with the same eventID
# the diffrCode is converted into a 4x1 vector ( ND, SD, DD, CD ), it is 1 for the certain
# diffractive case and 0 for the others
def get_XY_data( inp_data, hist_dim=32, etaMin=-13, etaMax=13, maxNumber=None, n_dim=1 ):
    "Function that creates an array of histograms from the rootNumpy array"

    # first: arrays that define the histogram:
    # x is eta -> we definde eta from -13 to 13, 
    xedges = np.linspace(etaMin, etaMax, hist_dim+1)
    # y is phi in [-pi, pi]
    yedges = np.linspace(-math.pi, math.pi, hist_dim+1)

    minEvtInt = inp_data['eventID'].min()
    maxEvtInt = inp_data['eventID'].max()
    evts_in_data = maxEvtInt - minEvtInt
    # secondly: have to read out the data from one evt into an array:
    # have to know how data is saved in the dataframe
    # here we also fill up the 4x1 target vector
    if (maxNumber is None) or (maxNumber > evts_in_data):
        maxNumber = evts_in_data

    x_data = np.zeros( (maxNumber, hist_dim, hist_dim, n_dim ) )
    y_data = np.zeros( (maxNumber) )

    for evtInt in range( minEvtInt, minEvtInt+maxNumber ):
        if evtInt%1000 == 0:
            print '{} events from {} fetched'.format(evtInt, minEvtInt+maxNumber)
        # get histogram
        evt_data = inp_data.loc[inp_data.eventID == evtInt, ['eta', 
                                                             'phi', 
                                                             'pT', 
                                                             'pdgID', 
                                                             'diffrCode',
                                                             'charge']]

        # drop data with 0-charge, and with a too small pt, then we get closer to the
        # real experimental setup
        evt_data = evt_data.drop(evt_data[(evt_data.pT < 0.12) & (evt_data.charge == 0.)].index)

        # eta selection
        # evt_data = evt_data.drop(evt_data[(evt_data.eta < -7.) | 
        #                                   (evt_data.eta > 6.3) |
        #                                   ((evt_data.eta > -4.9) & (evt_data.eta < -3.7))].index)
        # only TPC, TOF eta space
        evt_data = evt_data.drop(evt_data[abs(evt_data.eta) > etaMax].index)
        # we get p with the inputs eta(as vector) phi, pT, pdgID
        if evt_data.size == 0:
            continue

        if n_dim == 3:
            p_data = evt_data[['eta', 'phi', 'pT', 'pdgID']].T.as_matrix()
            # returns p and e, if |eta| > 0.9, e is returned as -1 (needed for isInTPCTOF)
            p_arr = get_p_e_vect( eta = p_data[0], 
                                  phi = p_data[1], 
                                  pt  = p_data[2], 
                                  pdg = p_data[3] )

            # bb_tof_arr[0] = bethebloch array, bb_tof_arr[1] = tof array
            # returns 0,0 if pdg is neutral particle, and if isInTPCTOF is less than 0.
            bb_tof_arr = bb_ToF_vect( p=p_arr[0], 
                                      pdg=p_data[3],
                                      isInTPCTOF=p_arr[1] )

            # the dEdx numbers go up to a very high number, therefore we divide by the largest number
            bb_arr = bb_tof_arr[0]/2000.

            # bethe bloch histogram
            x_data[evtInt-minEvtInt,:,:,1] = np.histogram2d(evt_data.eta.as_matrix(), 
                                                            evt_data.phi.as_matrix(), 
                                                            bins=(xedges, yedges), 
                                                            weights=bb_arr )[0]

            # tof histogram
            x_data[evtInt-minEvtInt,:,:,2] = np.histogram2d(evt_data.eta.as_matrix(), 
                                                            evt_data.phi.as_matrix(), 
                                                            bins=(xedges, yedges), 
                                                            weights=bb_tof_arr[1] )[0]

        # make pT-histogram
        x_data[evtInt-minEvtInt,:,:,0] = np.histogram2d(evt_data.eta.as_matrix(), 
                                                        evt_data.phi.as_matrix(), 
                                                        bins=(xedges, yedges), 
                                                        weights=evt_data.pT.as_matrix() )[0]
       # want the corrensponding diffractive code, which is our training goal
        y_data[evtInt-minEvtInt] = evt_data['diffrCode'].iloc[-1]

    # x_data, y_data = unison_shuffled_copies(x_data, y_data)
    # the 1 corresponds to the diff-class guess
    # this is done afterwards, we write the data in the shape ( number, hist_dim, hist_dim )
    # to the file and afterwards recalculate the 
    # for tensorFlow backend: (x_data.shape[0], hist_dim, hist_dim, 1)
    # x_data = x_data.reshape( x_data.shape[0], hist_dim, hist_dim, 1 )
    # returns the histograms in shuffled order
    return x_data, y_data


def heaviside_numpy(arr):
    """
    numpy array input is transformed into a 0/1 array (just position is highlighted)
    use the numpy.sign function that returns -1 for x<0, 0 for x=0 and 1 for x>0
    we want: 0 for x<=0, else 1, hence, add 1 and multply by 0.5
    as this gives 0.5 for the case 0 (most cases) we subtract a value from the np-arr
    between 0.5 and 1 and redo the heviside calculation to arrive at the desired output
    """
    arr = 0.5 * (np.sign(arr) + 1)
    arr = arr-0.6
    return 0.5 * (np.sign(arr) + 1)


def plot_feature_hists( data_in, feature='pT', nbins=100 ):
    """
    want to show the difference in the input variables for the different classes:
        - phi
        - eta
        - pt
        - dE/dx
        - beta(TOF)
        - number of particles per evt
    """
    data = data_in
    if feature == 'N':
        # adds a column freq to the dataframe, which accounts for the number of the same
        # eventID, hence particles in the event
        data['freq'] = data.groupby('eventID')['eventID'].transform('count')
        data.drop_duplicates(subset='eventID', keep='first', inplace=True)
        
        feature = 'freq'

    ND_data = data.loc[data.diffrCode==0, [feature]]
    SD_data = data.loc[data.diffrCode==1, [feature]]
    DD_data = data.loc[data.diffrCode==2, [feature]] 
    CD_data = data.loc[data.diffrCode==3, [feature]]
    
    plt.figure()
    plt.hist( ND_data[feature].as_matrix(), 
                       bins=nbins, alpha=0.5, color='black', label='ND' )
    plt.hist( SD_data[feature].as_matrix(), 
                        bins=nbins, alpha=0.5, color='green', label='SD' )
    plt.hist( DD_data[feature].as_matrix(), 
                        bins=nbins, alpha=0.5, color='red', label='DD' )
    plt.hist( CD_data[feature].as_matrix(), 
                        bins=nbins, alpha=0.5, color='yellow', label='CD' )
    
    plt.title(feature+' comparison')
    plt.xlabel(feature)
    plt.ylabel('Entries')
    plt.grid(True)
    plt.yscale('log')
    plt.legend()

    plt.show()


    
def load_data( filename, branches=None, start=None, stop=None, selection=None ):
    "load data from a root tree and returns a panda dataframe with the relevant branches"

    data = pd.DataFrame( root_numpy.root2array( filename,
                                                branches=branches,
                                                start=start,
                                                stop=stop,
                                                selection=selection ) )

    nb_entries = data.shape[0]

    return data, nb_entries

def save_data_h5py( h5f, x_data, y_data, train_test_valid ):
    "save histograms in a .h5 file (hdf5-format)"
    
    h5f.create_dataset( 'x_'+train_test_valid, data=x_data, dtype='f' )
    h5f.create_dataset( 'y_'+train_test_valid, data=y_data, dtype='i' )


def save_data( x_file, y_file, x_data, y_data ):
    "save the histograms in a .txt so that the process only has to be made once"

    with file(x_file, 'w') as of:
        # a header is added to the file:
        # any line starting with # will be ignored by numpy.loadtxt
        of.write('# Array shape: {0}\n'.format(x_data.shape))

        # Iterating through a ndimensional array produces slices along
        # the last axis. This is equivalent to data[i,:,:] in this case
        # for data_slice in x_data:
        # now we have more than one dimension in the picture:
        for i in range(0,x_data.shape[0]):
            for j in range(0,3):
                # Writing out a break to indicate different slices
                if j == 0:
                    of.write('# New histogram\n')
                    of.write('# pt:\n')
                if j == 1:
                    of.write('# dE/dx TPC:\n')
                if j == 2:
                    of.write('# beta in TOF:\n')

                # The formatting string indicates that I'm writing out
                # the values in left-justified columns 5 characters in width
                # with 3 decimal places.
                np.savetxt(of, x_data[i,:,:,j], fmt='%-5.3f') 

    with file(y_file, 'w') as y_of:
        y_of.write('# Array shape: {0}\n'.format(y_data.shape))
        np.savetxt(y_of, y_data, fmt='%i') 

    return

def main():
    # dimensions of the image
    train_filename = '/media/hdd/eventInfo/testFiles/etaPhiPt_allDiffMixed.root'
    valid_filename = '/media/hdd/eventInfo/validationFiles/etaPhiPt_allDiffMixedValid.root'
    # both_filename = '/media/hdd/eventInfo/etaPhiPt_all.root'

    # branch_names = [
    #         'eventID',
    #         'diffrCode',
    #         'eta',
    #         'phi',
    #         'pT',
    #         'charge',
    #         'pdgID'
    #         ]

    small_branch_name = [
            'eventID',
            'diffrCode',
            'eta',
            'phi',
            'pT',
            ]

    # load data and get number of entries in the tree: 
    print 'fetching training data from {}'.format(train_filename)
    train_data = load_data( train_filename, 
                            branches=small_branch_name, 
                            selection='eventID<=20000' )[0]
    print 'fetching validation data from {}'.format(valid_filename)
    validation_data = load_data( valid_filename, 
                                    branches=small_branch_name, 
                                    selection='eventID<4000' )[0]

    print 'load final validation data'
    final_valid_data = load_data( '/media/hdd/eventInfo/testFiles/etaPhiPt_allDiffMixed.root',
                                    branches=small_branch_name,
                                    selection='eventID>30000 && eventID<35000' )[0]
    print 'data loaded!'

    print 'converting data to histograms...'
    X_train, y_train = get_XY_data( train_data, etaMax=13, etaMin=-13, maxNumber=20000, n_dim=1 )
    print 'first histogram finished'
    print 'validaiton data to histograms...'
    X_valid, y_valid = get_XY_data( validation_data,etaMax=13,etaMin=-13,maxNumber=4000,n_dim=1 )
    print 'final validation data to histograms...' 
    x_test, y_test = get_XY_data( final_valid_data,etaMax=13,etaMin=-13,maxNumber=5000,n_dim=1 )
    print 'data conversion finished!'

    # plot_feature_hists( final_valid_data, feature='N' )
    # sys.exit()

    del train_data, validation_data, final_valid_data

    # outFile_test_hist = '/media/hdd/eventInfo/testFiles/test_hists.txt'
    # outFile_test_class = '/media/hdd/eventInfo/testFiles/test_class.txt'
    # outFile_validation_hist = '/media/hdd/eventInfo/validationFiles/validation_hists.txt'
    # outFile_validation_class = '/media/hdd/eventInfo/validationFiles/validation_class.txt'
    # outFile_finalval_hist = '/media/hdd/eventInfo/finalvalidation_hists.txt'
    # outFile_finalval_class = '/media/hdd/eventInfo/finalvalidation_class.txt'

    outFile = '/media/hdd/eventInfo/train_valid_test.h5'
    h5f = h5py.File(outFile, 'w')
    save_data_h5py( h5f, X_train, y_train, 'train' )
    save_data_h5py( h5f, X_valid, y_valid, 'valid' )
    save_data_h5py( h5f, x_test, y_test, 'test' )
    h5f.close()
 
    # print 'save histograms in file...'
    # save_data( outFile_test_hist, outFile_test_class, X_train, y_train ) 
    # print 'training histograms and classes saved!'
    # save_data( outFile_validation_hist, outFile_validation_class, X_valid, y_valid ) 
    # print 'validation histograms and classes saved!'
    # save_data( outFile_finalval_hist, outFile_finalval_class, x_test, y_test )
    # print 'test histograms and classes saved!'

    # print 'histograms written to files:\n\t {}\n\t{}'.format(outFile_test_hist, 
                                                                # outFile_validation_hist)
    return

if __name__ == "__main__":
    main()

