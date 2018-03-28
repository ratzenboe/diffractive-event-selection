import os
import sys

import numpy as np
import copy

import keras
from keras.models import Model, Sequential
from keras.layers.core import Activation, Dense, Dropout
from keras.layers import Masking, LSTM, GRU, Input, BatchNormalization, Flatten
from keras.layers.advanced_activations import PReLU
from keras.callbacks import ModelCheckpoint

from modules.keras_callback import callback_ROC

def train_model(data, run_mode_user, val_data,
                batch_size=64, n_epochs=50, rnn_layer='LSTM', 
                out_path = 'output/', dropout = 0.2, class_weight={0: 1., 1: 1.},
                n_layers=3, layer_nodes=100, batch_norm=False, activation='relu', 
                flat=False, aux=False):
    """
    Args 
        data:
            the data dictionary
        ___________________________________________________________

        run_mode_user:
            string that decribes which model to use
        ___________________________________________________________

        val_data:
            tuple = (X_val_data, y_val_data)
        ___________________________________________________________

        batch_size:
            integer number determining the number of samples in 
            each batch
        ___________________________________________________________

        n_epochs:
            integer, number of training epochs
        ___________________________________________________________

        rnn_layer:
            string that is evaluated as a keras layer
    _______________________________________________________________

    Operation breakdown:
        a model (determined by run_mode_user) is fit to the data
        provided.
    _______________________________________________________________

    Return:
        the fitted model

    """
    if not isinstance(data, dict):
        raise TypeError('The data is not provided in a dictionary ' \
                'format but rather is of type {}'.format(type(data)))

    for key in data.keys():
        if not isinstance(data[key], np.ndarray):
            raise TypeError('The key {} of the "data"-dictionary ' \
                    'is not of type numpy ndarrays but rather {}'.format(key, type(data[key])))

    if not isinstance(run_mode_user, str):
        raise TypeError('The variable "run_mode_user" is not a string type ' \
                'but instead {}'.format(type(run_mode_user)))

    if not isinstance(val_data, tuple):
        raise TypeError('The variable "val_data" is not a data tuple ' \
                'but instead {}'.format(type(val_data)))

    if not isinstance(rnn_layer, str):
        raise TypeError('The variable "rnn_layer" is not a string type ' \
                'but insted {}'.format(type(rnn_layer)))


    if 'NN' in run_mode_user:
        if flat or len(data.keys())==2:
            history = train_flat_NN(data, val_data, batch_size, n_epochs,
                       out_path, dropout, class_weight,
                       n_layers, layer_nodes, batch_norm, activation)
        else:
            history = train_composite_NN(data, val_data, batch_size, n_epochs, rnn_layer,
                           out_path, dropout, class_weight,
                           n_layers, layer_nodes, batch_norm, activation, aux, koala=False)

        return history

    elif 'anomaly' in run_mode_user:
        history = train_autoencoder(data, val_data, batch_size, n_epochs, out_path, dropout, 
                          n_layers, batch_norm, activation)
        return history

    elif 'koala' in run_mode_user:
        history = train_composite_NN(data, val_data, batch_size, n_epochs, rnn_layer,
                           out_path, dropout, class_weight,
                           n_layers, layer_nodes, batch_norm, activation, aux, koala=True)

        return history


    else:
        raise NameError('ERROR: Unrecognized model {}'.format(run_mode_user))



def train_autoencoder(data, val_data, batch_size=32, n_epochs=50, out_path = 'output/', 
                      dropout=0.2, n_layers=[14,7,7,14], batch_norm=False, activation='relu'):
    """
    Autoencoder for anomaly detection
    """
    if not isinstance(n_layers, list):
        raise TypeError('The variable "n_layers" is not a list but rather ' \
                'of type {}.'.format(type(n_layers)))
    try:
        # we use an autoencoder thus the target is the 
        # X_train itself
        X_train = data['feature_matrix']
        # train only on bg data, as this will be the only available source
        bg_indices = np.array(np.where(data['target']==0)).ravel()
        X_train = X_train[bg_indices]
        train_data = {'feature_matrix': X_train}

    except KeyError:
        raise KeyError('The data-dictionary provided does not contain' \
                'all necessary keys for the selected run-mode (run_mode_user)')

    input_train = Input(shape=(X_train.shape[-1],), name='feature_matrix')
    if activation != 'relu':
        x = Dense(n_layers[0], kernel_initializer='glorot_normal')(input_train)
        x = (getattr(keras.layers, activation)())(x)
    else:
         x = Dense(n_layers[0], 
                  activation = activation, 
                  kernel_initializer = 'glorot_normal')(input_train)

    for layer_nodes in n_layers[1:]:
        if activation != 'relu':
            x = Dense(layer_nodes, kernel_initializer='glorot_normal')(x)
            x = (getattr(keras.layers, activation)())(x)
        else:
            x = Dense(layer_nodes, 
                      activation         = activation, 
                      kernel_initializer = 'glorot_normal')(x)
        if batch_norm:
            x = BatchNormalization()(x)
        if dropout > 0.0:
            x = Dropout(dropout)(x)

    main_output = Dense(X_train.shape[-1],
                        activation = 'sigmoid', 
                        name = 'main_output', 
                        kernel_initializer = 'glorot_normal')(x)
    model = Model(inputs=input_train, outputs=main_output)

    model.compile(optimizer='adam', loss='binary_crossentropy', metrics=['mse'])
            
    print(model.summary())
    try:
        checkpointer = ModelCheckpoint(filepath=out_path+'best_model.h5',
                                       verbose=0,
                                       save_best_only=True)
        history = model.fit(train_data,
                            X_train,  
                            epochs = n_epochs, 
                            batch_size = batch_size,
                            validation_data = val_data,
                            callbacks = [checkpointer]).history
                  # ,callbacks = [callback_ROC(train_data_dic, 
                  #                           output_targets, 
                  #                           output_prefix=out_path)])
    except KeyboardInterrupt:
        print('Training ended early.')

    return history


def train_koala(data, val_data, batch_size=64, n_epochs=50, out_path = 'output/', 
                dropout = 0.2, n_layers=3, layer_nodes=100, batch_norm=False, activation='relu'):
    """
    Train a model on two sets of randomly picked sets of (unknown signal background ratio)
    the targets are not 0:bg and 1:sig but rather the set the particle came from. 
    meaning 0:set0 and 1:set1; 
    see: https://arxiv.org/abs/1801.10158
    """
    try:
        X_train = data['feature_matrix']
        y_train = np.copy(data['target'])
        # divide the training data into 2 groups: 
        #   1 = samples with sig and bg
        #   0 = samples with modeled bg (by chossing 3 tracks and omitting one)
        # new targets according to these groups
        y_train[(y_train==1) | (y_train==0)] = 1
        y_train[y_train==99] = 0

        # the training data stays the same
        train_data = {'feature_matrix': X_train}

        sig_percent = y_train[y_train==1].shape[0]/y_train.shape[0]*100.
        print('{}/{} "signal" events in 0({:.3f}%)'.format(
            y_train[y_train==1].shape[0], y_train.shape[0], sig_percent))
        
    except KeyError:
        raise KeyError('The data-dictionary provided does not contain' \
                'all necessary keys for the selected run-mode (run_mode_user)')

    input_train = Input(shape=(X_train.shape[-1],), name='feature_matrix')
    if activation != 'relu':
        x = Dense(layer_nodes, kernel_initializer='glorot_normal')(input_train)
        x = (getattr(keras.layers, activation)())(x)
    else:
        x = Dense(layer_nodes, 
                  activation = activation, 
                  kernel_initializer = 'glorot_normal')(input_train)

    for i in range(0,n_layers-1):
        if activation != 'relu':
            x = Dense(layer_nodes, kernel_initializer='glorot_normal')(x)
            x = (getattr(keras.layers, activation)())(x)
        else:
            x = Dense(layer_nodes, 
                      activation         = activation, 
                      kernel_initializer = 'glorot_normal')(x)
        if batch_norm:
            x = BatchNormalization()(x)
        if dropout > 0.0:
             x = Dropout(dropout)(x)

    main_output = Dense(1, 
                        activation = 'sigmoid', 
                        name = 'main_output', 
                        kernel_initializer = 'glorot_normal')(x)
    model = Model(inputs=input_train, outputs=main_output)

    model.compile(optimizer='adam', loss='binary_crossentropy', metrics=['accuracy'])
            
    print(model.summary())
    try:
        checkpointer = ModelCheckpoint(filepath=out_path+'best_model.h5',
                           verbose=0,
                           save_best_only=True)

        history = model.fit(train_data,
                            y_train,  
                            epochs = n_epochs, 
                            batch_size = batch_size,
                            validation_data = val_data,
                            callbacks = [checkpointer]).history
    except KeyboardInterrupt:
        print('Training ended early.')

    return history


def train_composite_NN(data, val_data, batch_size=64, n_epochs=30, rnn_layer='LSTM', 
        out_path = 'output/', dropout = 0.2, class_weight={0: 1., 1: 1.},
        n_layers=3, layer_nodes=100, batch_norm=False, activation='relu', aux=False,
        koala=False):


        try:
            train_data = data.copy()
            X_track_train   = train_data.pop('track')
            X_event_train   = train_data.pop('event')
            X_fmd_train     = train_data.pop('fmd', None)
            X_v0_train      = train_data.pop('v0', None)
            X_ad_train      = train_data.pop('ad', None)
            X_emcal_train   = train_data.pop('emcal', None)
            X_phos_train    = train_data.pop('phos', None)
            X_calo_cluster_train = train_data.pop('calo_cluster', None)

            y_train         = train_data.pop('target')

            if koala and y_train[y_train==99].size != 0:
                y_train[(y_train==1) | (y_train==0)] = 1
                y_train[y_train==99] = 0

            if train_data:
                raise ValueError('The input dictionary contains unknown keys: {}'.format(
                    list(train_data.keys())))
            
            input_data = data.copy()
            input_data.pop('target')

        except KeyError:
            raise KeyError('The data-dictionary provided does not contain' \
                    'all necessary keys for the selected run-mode (run_mode_user)')

        # the data for the RNNs are of shape (n_evts, n_timesteps=n_particles, n_features) 
        # the masking layer however is only intersted in (n_timesteps, n_features)
        TRACK_SHAPE      = X_track_train.shape[1:]
        N_FEATURES_track = TRACK_SHAPE[-1]

        # this following data is of shape (n_evts, n_features)
        # the network is fed with n_features
        EVENT_SHAPE = (X_event_train.shape[-1],)

        # inputs network
        track_input = Input(shape=TRACK_SHAPE, name='track')
        event_input = Input(shape=EVENT_SHAPE, name='event')
        input_list = [track_input, event_input]
        # ------ Build the model ---------
        # RNNs feeding into the event level layer
        # track_mask = Masking(mask_value=float(-999), name='track_masking')(track_input)
        # event needs no masking
        try:
            track_rnn = Masking(mask_value=float(-999), name='track_masking')(track_input)
            track_rnn = (getattr(keras.layers, rnn_layer)(N_FEATURES_track, 
                name='track_rnn'))(track_rnn)
            if batch_norm:
                track_rnn = BatchNormalization(name='track_batch_norm')(track_rnn)
            if dropout > 0.0:
                track_rnn = Dropout(dropout, name='track_dropout')(track_rnn)

        except AttributeError:
            raise AttributeError('{} is not a valid Keras layer!'.format(rnn_layer))
        concatenate_list = [track_rnn, event_input]
        # ----------------------------- std input end -------------------------------

        if X_emcal_train is not None:
            EMCAL_SHAPE = X_emcal_train.shape[1:]
            N_FEATURES_emcal = EMCAL_SHAPE[-1]
            emcal_input = Input(shape=EMCAL_SHAPE, name='emcal')
            try:
                emcal_rnn = Masking(mask_value=float(-999), name='emcal_masking')(emcal_input)
                emcal_rnn = (getattr(keras.layers, rnn_layer)(N_FEATURES_emcal, 
                    name='emcal_rnn'))(emcal_rnn)
                if batch_norm:
                    emcal_rnn = BatchNormalization(name='emcal_batch_norm')(emcal_rnn)
                if dropout > 0.0:
                    emcal_rnn = Dropout(dropout, name='emcal_dropout')(emcal_rnn)
            except AttributeError:
                raise AttributeError('{} is not a valid Keras layer!'.format(rnn_layer))

            # input_list.extend(emcal_input)
            input_list.append(emcal_input)
            concatenate_list.append(emcal_rnn)

        if X_phos_train is not None:
            PHOS_SHAPE = X_phos_train.shape[1:]
            N_FEATURES_phos = PHOS_SHAPE[-1]
            phos_input = Input(shape=PHOS_SHAPE, name='phos')
            try:
                phos_rnn = Masking(mask_value=float(-999), name='phos_masking')(phos_input)
                phos_rnn = (getattr(keras.layers, rnn_layer)(N_FEATURES_phos, 
                    name='phos_rnn'))(phos_rnn)
                if batch_norm:
                    phos_rnn = BatchNormalization(name='phos_batch_norm')(phos_rnn)
                if dropout > 0.0:
                    phos_rnn = Dropout(dropout, name='phos_dropout')(phos_rnn)
            except AttributeError:
                raise AttributeError('{} is not a valid Keras layer!'.format(rnn_layer))

            input_list.append(phos_input)
            concatenate_list.append(phos_rnn)

        if X_calo_cluster_train is not None:
            CALO_CLUSTER_SHAPE = X_calo_cluster_train.shape[1:]
            N_FEATURES_calo_cluster = CALO_CLUSTER_SHAPE[-1]
            calo_cluster_input = Input(shape=CALO_CLUSTER_SHAPE, name='calo_cluster')
            try:
                calo_cluster_rnn = Masking(mask_value=float(-999), 
                                           name='calo_cluster_masking')(calo_cluster_input)
                calo_cluster_rnn = (getattr(keras.layers, rnn_layer)(N_FEATURES_calo_cluster, 
                    name='calo_cluster_rnn'))(calo_cluster_rnn)
                if batch_norm:
                    calo_cluster_rnn = BatchNormalization(name='calo_cluster_batch_norm')(
                            calo_cluster_rnn)
                if dropout > 0.0:
                    calo_cluster_rnn = Dropout(dropout, name='calo_cluster_dropout')(
                            calo_cluster_rnn)
            except AttributeError:
                raise AttributeError('{} is not a valid Keras layer!'.format(rnn_layer))

            input_list.append(calo_cluster_input)
            concatenate_list.append(calo_cluster_rnn)

        if X_ad_train is not None:
            # the data is in this form: (n_evts, n_cells, n_features)
            # here: (n_evts, 16 (cells), 1 (amplitude in each cell))
            # this means we have to flatten the input
            AD_SHAPE = X_ad_train.shape[1:]
            ad_input = Input(shape=AD_SHAPE, name='ad')
            dense_ad = Flatten()(ad_input)

            dense_ad = Dense(16, kernel_initializer='glorot_normal')(dense_ad)
            dense_ad = (getattr(keras.layers, activation)())(dense_ad)
            if batch_norm:
                dense_ad = BatchNormalization()(dense_ad)
            if dropout > 0.0:
                dense_ad = Dropout(dropout)(dense_ad)

            for i in range(2):
                dense_ad = Dense(16, kernel_initializer='glorot_normal')(dense_ad)
                dense_ad = (getattr(keras.layers, activation)())(dense_ad)
                if batch_norm:
                    dense_ad = BatchNormalization()(dense_ad)
                if dropout > 0.0:
                    dense_ad = Dropout(dropout)(dense_ad)

            dense_ad = Dense(1, 
                            activation = 'sigmoid', 
                            name = 'dense_ad_output', 
                            kernel_initializer = 'glorot_normal')(dense_ad)

            input_list.append(ad_input)
            concatenate_list.append(dense_ad)


        if X_fmd_train is not None:
            FMD_SHAPE = X_fmd_train.shape[1:]
            fmd_input = Input(shape=FMD_SHAPE, name='fmd')
            dense_fmd = Flatten()(fmd_input)

            dense_fmd = Dense(64, kernel_initializer='glorot_normal')(dense_fmd)
            dense_fmd = (getattr(keras.layers, activation)())(dense_fmd)
            if batch_norm:
                dense_fmd = BatchNormalization()(dense_fmd)
            if dropout > 0.0:
                dense_fmd = Dropout(dropout)(dense_fmd)

            for i in range(2):
                dense_fmd = Dense(64, kernel_initializer='glorot_normal')(dense_fmd)
                dense_fmd = (getattr(keras.layers, activation)())(dense_fmd)
                if batch_norm:
                    dense_fmd = BatchNormalization()(dense_fmd)
                if dropout > 0.0:
                    dense_fmd = Dropout(dropout)(dense_fmd)

            dense_fmd = Dense(1, 
                            activation = 'sigmoid', 
                            name = 'dense_fmd_output', 
                            kernel_initializer = 'glorot_normal')(dense_fmd)

            input_list.append(fmd_input)
            concatenate_list.append(dense_fmd)

        if X_v0_train is not None:
            V0_SHAPE = X_v0_train.shape[1:]
            v0_input = Input(shape=V0_SHAPE, name='v0')
            dense_v0 = Flatten()(v0_input)

            dense_v0 = Dense(64, kernel_initializer='glorot_normal')(dense_v0)
            dense_v0 = (getattr(keras.layers, activation)())(dense_v0)
            if batch_norm:
                dense_v0 = BatchNormalization()(dense_v0)
            if dropout > 0.0:
                dense_v0 = Dropout(dropout)(dense_v0)

            for i in range(2):
                dense_v0 = Dense(64, kernel_initializer='glorot_normal')(dense_v0)
                dense_v0 = (getattr(keras.layers, activation)())(dense_v0)
                if batch_norm:
                    dense_v0 = BatchNormalization()(dense_v0)
                if dropout > 0.0:
                    dense_v0 = Dropout(dropout)(dense_v0)

            dense_v0 = Dense(1, 
                            activation = 'sigmoid', 
                            name = 'dense_v0_output', 
                            kernel_initializer = 'glorot_normal')(dense_v0)

            input_list.append(v0_input)
            concatenate_list.append(dense_v0)
        # ----------------------------- fancy ml end -----------------------------------

        x = keras.layers.concatenate(concatenate_list)
        # add an auxilairy output
        output_list = []
        output_data = []
        if aux: 
            aux_output_track = Dense(1, activation='sigmoid', name='aux_output')(x)
            output_list.append(aux_output_track)
            output_data.append(y_train)
        for i in range(0,n_layers):
            x = Dense(layer_nodes, kernel_initializer='glorot_normal')(x)
            x = (getattr(keras.layers, activation)())(x)
            if batch_norm:
                x = BatchNormalization()(x)
            if dropout > 0.0:
                x = Dropout(dropout)(x)

        main_output = Dense(1, 
                            activation = 'sigmoid', 
                            name = 'main_output', 
                            kernel_initializer = 'glorot_normal')(x)
        output_list.insert(0, main_output)  
        output_data.append(y_train)
        model = Model(inputs=input_list, outputs=output_list)
  
        model.compile(optimizer='adam', loss='binary_crossentropy', metrics=['accuracy'])
        print(model.summary())
        try:
            checkpointer = ModelCheckpoint(filepath=out_path+'best_model.h5',
                               verbose=0,
                               save_best_only=True)
            history = model.fit(input_data,
                                output_data,  
                                epochs = n_epochs, 
                                batch_size = batch_size,
                                validation_data = val_data,
                                callbacks = [checkpointer]).history
                      # class_weight = class_weight)
                      # ,callbacks = [callback_ROC(train_data, 
                      #                           y_train, 
                      #                           output_prefix=out_path)])

        except KeyboardInterrupt:
            print('Training ended early.')

        return history



def train_flat_NN(data, val_data, batch_size=64, n_epochs=50, 
                  out_path = 'output/', dropout = 0.2, class_weight={0: 1., 1: 1.},
                  n_layers=3, layer_nodes=100, batch_norm=False, activation='relu'):
    """
    train a flat NN
    """

    try:
        X_train = data['feature_matrix']
        y_train = data['target']
        train_data = {'feature_matrix': X_train}
    except KeyError:
        raise KeyError('The data-dictionary provided does not contain' \
                'all necessary keys for the selected run-mode (run_mode_user)')

    print('X_train shape {}'.format(X_train.shape))

    if 99 in y_train:
        raise ValueError('The data contains more than 2 different targets!')

    input_train = Input(shape=(X_train.shape[-1],), name='feature_matrix')
    x = Dense(layer_nodes, kernel_initializer='glorot_normal')(input_train)
    x = (getattr(keras.layers, activation)())(x)

    for i in range(0,n_layers-1):
       x = Dense(layer_nodes, kernel_initializer='glorot_normal')(x)
       x = (getattr(keras.layers, activation)())(x)
       if batch_norm:
           x = BatchNormalization()(x)
       if dropout > 0.0:
            x = Dropout(dropout)(x)

    main_output = Dense(1, 
                        activation = 'sigmoid', 
                        name = 'main_output', 
                        kernel_initializer = 'glorot_normal')(x)
    model = Model(inputs=input_train, outputs=main_output)

    model.compile(optimizer='adam', loss='binary_crossentropy', metrics=['accuracy'])
            
    print(model.summary())
    try:
        checkpointer = ModelCheckpoint(filepath=out_path+'best_model.h5',
                           verbose=0,
                           save_best_only=True)

        history = model.fit(train_data,
                            y_train,  
                            epochs = n_epochs, 
                            batch_size = batch_size,
                            validation_data = val_data,
                            callbacks = [checkpointer]).history
                  # class_weight = class_weight)
                  # ,callbacks = [callback_ROC(train_data_dic, 
                  #                           output_targets, 
                  #                           output_prefix=out_path)])

    except KeyboardInterrupt:
        print('Training ended early.')

    return history


