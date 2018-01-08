import os
import math
import sys

import numpy as np
import root_numpy
import pandas as pd
import h5py

def pad_array(array, max_entries):
    """
    truncate or extend numpy array to a fixed size
    if the maximum entries exceeds the size the array is
    filled with -999 columns. This is then masked in the NN model
    """
    if max_entries is None:
        return array.tolist()
    try: 
        length = array.shape[0]
        if length > max_entries:
            return array[:max_entries,:].tolist()
        elif length < max_entries:
            dummy_arr = np.array(array[0])
            dummy_arr.fill(-999)
            for i in range(length, max_entries):
                array = np.append(array, [dummy_arr], axis=0)
            return array.tolist()
        elif length == max_entries:
            return array.tolist()

    except AttributeError:
        print 'please provide numpy array to pad_array function'


def prepare_data(inp_data, max_entries_per_evt, list_of_features, data_params):
    """
    Function that loops trough the data array which we get from the TTree
    The event is stored as individual particles that all carry the same event number
    The events are stored in (5,x) arrays (5=eta, pt, phi, pdgID, charge) & x=number of 
    particles in the event and appended to a super array which stores all events 

    returns: two ndarrays:
                - all_events (n_evts, n_parts, n_features)
                - y_data     (n_evts,)
    """

    minEvtInt = inp_data[data_params['evt_id']].min()
    maxEvtInt = inp_data[data_params['evt_id']].max()
    evts_in_data = maxEvtInt - minEvtInt

    all_events = []
    y_data = []
    # this loop works also if there is only one evt in the inp_data 
    for evtInt in range(minEvtInt, maxNumber+1):
        if evtInt%1000 == 0:
            print '{} events from {} fetched'.format(evtInt, minEvtInt+maxNumber)
        # get relevant data for one event 
        evt_data = inp_data.loc[inp_data.eventID == evtInt, list_of_features]

        if data_params['target'] in list_of_features:
            y = evt_data[data_params['target']].iloc[-1]
            evt_data = evt_data.drop([data_params['target']], axis=1)
            y_data.append(y)
        # drop data with 0-charge, and with a too small pt, then we get closer to the
        # real experimental setup
        evt_data = evt_data.drop(evt_data[(evt_data.pT < 0.12) & (evt_data.charge == 0.)].index)
        # only TPC, TOF eta space
        evt_data = evt_data.drop(evt_data[abs(evt_data.eta) > 0.9].index)
        # as_matrix() transfroms dataframe to numpy array -> needed for pad_array
        evt_data = evt_data.as_matrix()
        evt_list = pad_array(evt_data, max_entries_per_evt)
        all_events.append(evt_list)

    all_events = np.array(all_events) 
    y_data = np.array(y_data)


    return all_events, y_data


def get_data(data_params)
    """
    a global function combining various functions to load the data and put it in the right
    form for ML
    """
    evt_dictionary = {}
    for key, value in data_params['branches'].iteritems():
        list_of_features = value
        list_of_features.remove(data_params['evt_id'])
        
        data = load_data(data_params['path'][key], branches=data_params['branches'][key])[0]
        evt_dictionary[key], y_data = prepare_data(
                                            inp_data            = data, 
                                            max_entries_per_evt = data_params['max_entries'][key],
                                            list_of_features    = list_of_features,
                                            data_params         = data_params)
        # if the y_data array has a size, then we add the 
        # target information to the evt_dictionary 
        if y_data:
            evt_dictionary['target'] = y_data

    return evt_dictionary



def load_data(filename, branches=None, start=None, stop=None, selection=None):
    """
    load data from a root tree and returns a panda dataframe with the relevant branches

    sidenote: 
        the raw data from the root file is then put into a shaping function that prepares the
        data for the RNNs (prepare_data) 
    """
    if not os.path.exists(filename):
        print'Error when loading the data. File {} is not a regular file.'.format(filename)
        sys.exit(1)

    data = pd.DataFrame( root_numpy.root2array( filename,
                                                branches=branches,
                                                start=start,
                                                stop=stop,
                                                selection=selection ) )

    nb_entries = data.shape[0]

    return data, nb_entries


def save_data_h5py(outfile, all_evt_data, file_addition=''):
    """
    save histograms in a .h5 file (hdf5-format)
    """

    h5f = h5py.File(outfile, 'w')

    data_name = 'all_evts' 
    data_name += file_addition
    h5f.create_dataset(data_name, data=all_evt_data)

    h5f.close()
 
    return 


def preprocess(evt_dic, data_params, run_params, load_fitted_attributes=False):
    """
    Performes preprocessing on the data, right now only standarad scaling
    the fitted attributes are then saved in a file
    """

    output_prefix, model_saves_prefix = get_output_paths(run_params)
    load_save_file = output_prefix + model_saves_prefix + 'scaling_attributes.npy'
    
    if not load_fitted_attributes:
        # we loop through all datasets that are in the dictionary std_scale
        # std_scale looks like:
        #   {'fmd': ['a', 'b',...], 'ad': [...], ...}
        for key, columns in data_params['std_scale'].iteritems():
            # we have to fit new scaling attributes
            scaling_attr = {}
            for col in columns:
                try:
                    mean = evt_dic[key][col].mean()
                    std  = evt_dic[key][col].std()
                    evt_dic[key][col] -= mean
                    evt_dic[key][col] /= std
                except KeyError:
                    print'Warning: Feature {} not found in the data!'.format(col)
                    
                # cannot save only the column, but we also have to 
                # save the key, as some columns appear in different keys
                # e.g. time in fmd, ad, v0
                scaling_attr[col] = {key: {'mean': mean, 'std': std} }

        np.save(load_save_file, scaling_attributes)

    else:
        # load previously fitted attributes
        scaling_attr = np.load(load_save_file).item()

        for key, columns in data_params['std_scale'].iteritems():
            for col in columns:
                try: 
                    evt_dic[key][col] -= scaling_attributes[col][key]['mean']
                    evt_dic[key][col] /= scaling_attributes[col][key]['std']
                except KeyError:
                    print'Warning: Feature {} not found in the data!'.format(col)

    return evt_dic 

