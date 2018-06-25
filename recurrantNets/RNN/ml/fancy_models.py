import os
import sys

import numpy as np
import copy

import keras
from keras.models import Model, Sequential
from keras.layers.core import Activation, Dense, Dropout
from keras.layers import Masking, LSTM, GRU, Input, BatchNormalization, Flatten, AveragePooling1D, MaxPooling1D, Conv1D, Bidirectional
from keras.layers.advanced_activations import PReLU
from keras.callbacks import ModelCheckpoint
from keras import regularizers

from modules.keras_callback import callback_ROC

special_activations = ['PReLU', 'LeakyReLU', 'ELU']

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


def train_hypernet(data, val_data, batch_size=64, n_epochs=30, rnn_layer='LSTM', 
        out_path = 'output/', dropout = 0.2, class_weight={0: 1., 1: 1.},
        n_layers=3, layer_nodes=100, batch_norm=False, activation='relu', k_reg=0.01,
        aux=False, flat=False, koala=False):


        try:
            train_data = data.copy()
            if flat:
                event_name = 'feature_matrix'
                X_event_train = train_data.pop('feature_matrix')
                X_track_train = None
            else:
                event_name = 'event'
                X_event_train   = train_data.pop('event')
                X_track_train   = train_data.pop('track')

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
        # input_list = [track_input, event_input]
        input_list = []
        # this following data is of shape (n_evts, n_features)
        # the network is fed with n_features
        if len(X_event_train.shape)==1:
            EVENT_SHAPE = (1,)
        else:
            EVENT_SHAPE = (X_event_train.shape[-1],)
        event_input = Input(shape=EVENT_SHAPE, name=event_name)
        input_list.append(event_input)
        # ------ Build the model ---------
        # RNNs feeding into the event level layer
        # track_mask = Masking(mask_value=float(-999), name='track_masking')(track_input)
        # event needs no masking
        if not flat:
            TRACK_SHAPE      = X_track_train.shape[1:]
            N_FEATURES_track = TRACK_SHAPE[-1]
            track_input = Input(shape=TRACK_SHAPE, name='track')
            input_list.append(track_input)
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
            concatenate_list_track_evt = [track_rnn, event_input]
        # ----------------------------- std input end -------------------------------

        concatenate_list_calorimeters = []
        concatenate_list_veto_detectors = []
        if X_emcal_train is not None:
            EMCAL_SHAPE = X_emcal_train.shape[1:]
            N_FEATURES_emcal = EMCAL_SHAPE[-1]
            emcal_input = Input(shape=EMCAL_SHAPE, name='emcal')
            try:
                emcal_rnn = Masking(mask_value=float(-999), name='emcal_masking')(emcal_input)
                emcal_rnn = (getattr(keras.layers, rnn_layer)(
                    N_FEATURES_emcal, name='emcal_rnn'))(emcal_rnn)
                # emcal_rnn = (getattr(keras.layers, rnn_layer)(N_FEATURES_emcal, 
                #     name='emcal_rnn_backwards', go_backwards=True))(emcal_rnn)
                # emcal_rnn = Bidirectional((getattr(keras.layers, rnn_layer)(N_FEATURES_emcal, 
                #     return_sequences=True, name='emcal_rnn')))(emcal_rnn)
                # emcal_rnn = Bidirectional(getattr(keras.layers, rnn_layer)(N_FEATURES_emcal, 
                #     name='emcal_rnn_back'))(emcal_rnn)
                if batch_norm:
                    emcal_rnn = BatchNormalization(name='emcal_batch_norm')(emcal_rnn)
                # emcal_rnn = Dropout(0.25, name='emcal_dropout')(emcal_rnn)
               # if batch_norm:
               #      emcal_rnn = BatchNormalization(name='emcal_batch_norm_2')(emcal_rnn)
               #  if dropout > 0.0:
               #      emcal_rnn = Dropout(dropout, name='emcal_dropout_2')(emcal_rnn)
            except AttributeError:
                raise AttributeError('{} is not a valid Keras layer!'.format(rnn_layer))

            # dense_emcal = Flatten()(emcal_rnn)
            # dense_emcal = Dense(10, kernel_initializer='glorot_normal', 
            #         activity_regularizer=regularizers.l1(0.01))(emcal_rnn)
            # dense_emcal = (getattr(keras.layers, activation)())(dense_emcal)
            # if batch_norm:
            #     dense_emcal = BatchNormalization()(dense_emcal)
            # if dropout > 0.0:
            #     dense_emcal = Dropout(dropout)(dense_emcal)

            # for i in range(2):
            #     dense_emcal = Dense(10, kernel_initializer='glorot_normal',
            #             activity_regularizer=regularizers.l1(0.01))(dense_emcal)
            #     dense_emcal = (getattr(keras.layers, activation)())(dense_emcal)
            #     if batch_norm:
            #         dense_emcal = BatchNormalization()(dense_emcal)
            #     if dropout > 0.0:
            #         dense_emcal = Dropout(dropout)(dense_emcal)

            # dense_emcal = Dense(1, 
            #                 activation = 'sigmoid', 
            #                 name = 'dense_emcal_output', 
            #                 kernel_initializer = 'glorot_normal')(dense_emcal)

            # # input_list.extend(emcal_input)
            input_list.append(emcal_input)
            concatenate_list_calorimeters.append(emcal_rnn)

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
                # if dropout > 0.0:
                #     phos_rnn = Dropout(dropout, name='phos_dropout')(phos_rnn)
            except AttributeError:
                raise AttributeError('{} is not a valid Keras layer!'.format(rnn_layer))

            input_list.append(phos_input)
            concatenate_list_calorimeters.append(phos_rnn)

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
                # if dropout > 0.0:
                #     calo_cluster_rnn = Dropout(dropout, name='calo_cluster_dropout')(
                #             calo_cluster_rnn)
            except AttributeError:
                raise AttributeError('{} is not a valid Keras layer!'.format(rnn_layer))

            input_list.append(calo_cluster_input)
            concatenate_list_calorimeters.append(calo_cluster_rnn)

        if X_ad_train is not None:
            # the data is in this form: (n_evts, n_cells, n_features)
            # here: (n_evts, 16 (cells), 1 (amplitude in each cell))
            # this means we have to flatten the input
            AD_SHAPE = X_ad_train.shape[1:]
            ad_input = Input(shape=AD_SHAPE, name='ad')
            conv_ad = Conv1D(16, kernel_size=3, activation='relu')(ad_input)
            conv_ad = AveragePooling1D(pool_size=2)(conv_ad)
            dense_ad = Flatten()(conv_ad)

            dense_ad = Dense(20, kernel_initializer='glorot_normal')(dense_ad)
            dense_ad = (getattr(keras.layers, activation)())(dense_ad)
            if batch_norm:
                dense_ad = BatchNormalization()(dense_ad)
            if dropout > 0.0:
                dense_ad = Dropout(dropout)(dense_ad)

            for i in range(2):
                dense_ad = Dense(20, kernel_initializer='glorot_normal')(dense_ad)
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
            concatenate_list_veto_detectors.append(dense_ad)


        if X_fmd_train is not None:
            FMD_SHAPE = X_fmd_train.shape[1:]
            fmd_input = Input(shape=FMD_SHAPE, name='fmd')
            conv_fmd = Conv1D(128, kernel_size=4, activation='relu')(fmd_input)
            conv_fmd = AveragePooling1D(pool_size=3)(conv_fmd)
            conv_fmd = Conv1D(64, kernel_size=3, activation='relu')(conv_fmd)
            conv_fmd = MaxPooling1D(pool_size=2)(conv_fmd)
            dense_fmd = Flatten()(conv_fmd)

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
            concatenate_list_veto_detectors.append(dense_fmd)

        if X_v0_train is not None:
            V0_SHAPE = X_v0_train.shape[1:]
            v0_input = Input(shape=V0_SHAPE, name='v0')
            conv_v0 = Conv1D(128, kernel_size=4, activation='relu')(v0_input)
            conv_v0 = AveragePooling1D(pool_size=3)(conv_v0)
            conv_v0 = Conv1D(64, kernel_size=3, activation='relu')(conv_v0)
            conv_v0 = MaxPooling1D(pool_size=2)(conv_v0)
            dense_v0 = Flatten()(conv_v0)


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
            concatenate_list_veto_detectors.append(dense_v0)
        # ----------------------------- fancy ml end -----------------------------------

        tot_concat_lst = []
        if not flat:
            concat_trk_evt = keras.layers.concatenate(concatenate_list_track_evt)
            tot_concat_lst.append(concat_trk_evt)
        else:
            concat_trk_evt = event_input
            tot_concat_lst.append(event_input)

        if len(concatenate_list_calorimeters)>1:
            concat_calos = keras.layers.concatenate(concatenate_list_calorimeters)
            tot_concat_lst.append(concat_calos)
        elif len(concatenate_list_calorimeters)==1:
            concat_calos = concatenate_list_calorimeters[0]
            tot_concat_lst.append(concat_calos)        
        
        if len(concatenate_list_veto_detectors)>1:
            concat_veto_dets =  keras.layers.concatenate(concatenate_list_veto_detectors)
            tot_concat_lst.append(concat_veto_dets)
        elif len(concatenate_list_veto_detectors)==1:
            concat_veto_dets = concatenate_list_veto_detectors[0]
            tot_concat_lst.append(concat_veto_dets)        
 
        if len(tot_concat_lst)>1:
            x = keras.layers.concatenate(tot_concat_lst)
        else:
            x = concat_trk_evt
        # add an auxilairy output
        output_list = []
        output_data = []
        if aux: 
            aux_output_evt_trk = Dense(1, # activity_regularizer=regularizers.l1(0.01),
                    activation='sigmoid', name='aux_evt_trk')(concat_trk_evt)
            if concatenate_list_calorimeters:
                aux_output_calos = Dense(1, # activity_regularizer=regularizers.l1(0.01), 
                        activation='sigmoid', name='aux_calos')(concat_calos)
            if concatenate_list_veto_detectors:
                aux_output_veto_dets = Dense(1, 
                                             # activity_regularizer=regularizers.l1(0.01),
                                             activation='sigmoid',
                                             name='aux_veto_dets')(concat_veto_dets)
            output_list.append(aux_output_evt_trk)
            output_data.append(y_train)
            if concatenate_list_calorimeters:
                output_list.append(aux_output_calos)
                output_data.append(y_train)
            if concatenate_list_veto_detectors:
                output_list.append(aux_output_veto_dets)
                output_data.append(y_train)

        for i in range(0,n_layers):
            x = Dense(layer_nodes, 
                      # activity_regularizer=regularizers.l1(0.01),
                      kernel_initializer='glorot_normal')(x)
            x = (getattr(keras.layers, activation)())(x)
            if batch_norm:
                x = BatchNormalization()(x)
            if dropout > 0.0:
                x = Dropout(dropout)(x)

        main_output = Dense(1, 
                            # activity_regularizer=regularizers.l1(0.01),
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


def train_calo_net(data, val_data, batch_size=16, n_epochs=30, rnn_layer='LSTM', 
        out_path = 'output/', dropout = 0.2, class_weight={0: 1., 1: 1.},
        n_layers=3, layer_nodes=100, batch_norm=False, activation='relu', k_reg=0.01,
        aux=False, flat=False, koala=False):


        try:
            train_data = data.copy()
            if flat:
                event_name = 'feature_matrix'
                X_event_train = train_data.pop('feature_matrix')
                X_track_train = None
            else:
                event_name = 'event'
                X_event_train   = train_data.pop('event')
                X_track_train   = train_data.pop('track', None)

            # X_fmd_train     = train_data.pop('fmd', None)
            # X_v0_train      = train_data.pop('v0', None)
            # X_ad_train      = train_data.pop('ad', None)
            # X_emcal_train   = train_data.pop('emcal', None)
            # X_phos_train    = train_data.pop('phos', None)
            X_calo_cluster_train = train_data.pop('calo_cluster')

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

        input_list = []
        # this following data is of shape (n_evts, n_features)
        # the network is fed with n_features
        if len(X_event_train.shape)==1:
            EVENT_SHAPE = (1,)
        else:
            EVENT_SHAPE = (X_event_train.shape[-1],)
        event_input = Input(shape=EVENT_SHAPE, name=event_name)
        input_list.append(event_input)
        # ------ Build the model ---------
        # RNNs feeding into the event level layer
        # track_mask = Masking(mask_value=float(-999), name='track_masking')(track_input)
        # event needs no masking
        if not flat and X_track_train is not None:
            TRACK_SHAPE      = X_track_train.shape[1:]
            N_FEATURES_track = TRACK_SHAPE[-1]
            track_input = Input(shape=TRACK_SHAPE, name='track')
            input_list.append(track_input)
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
        else:
            concatenate_list = [event_input]
        # ----------------------------- std input end -------------------------------

        # if X_emcal_train is not None:
        #     EMCAL_SHAPE = X_emcal_train.shape[1:]
        #     N_FEATURES_emcal = EMCAL_SHAPE[-1]
        #     emcal_input = Input(shape=EMCAL_SHAPE, name='emcal')
        #     try:
        #         emcal_rnn = Masking(mask_value=float(-999), name='emcal_masking')(emcal_input)
        #         emcal_rnn = (getattr(keras.layers, rnn_layer)(N_FEATURES_emcal, 
        #             name='emcal_rnn'))(emcal_rnn)
        #         if batch_norm:
        #             emcal_rnn = BatchNormalization(name='emcal_batch_norm')(emcal_rnn)
        #         if dropout > 0.0:
        #             emcal_rnn = Dropout(dropout, name='emcal_dropout')(emcal_rnn)
        #     except AttributeError:
        #         raise AttributeError('{} is not a valid Keras layer!'.format(rnn_layer))

        #     # input_list.extend(emcal_input)
        #     input_list.append(emcal_input)
        #     concatenate_list.append(emcal_rnn)

        # if X_phos_train is not None:
        #     PHOS_SHAPE = X_phos_train.shape[1:]
        #     N_FEATURES_phos = PHOS_SHAPE[-1]
        #     phos_input = Input(shape=PHOS_SHAPE, name='phos')
        #     try:
        #         phos_rnn = Masking(mask_value=float(-999), name='phos_masking')(phos_input)
        #         phos_rnn = (getattr(keras.layers, rnn_layer)(N_FEATURES_phos, 
        #             name='phos_rnn'))(phos_rnn)
        #         if batch_norm:
        #             phos_rnn = BatchNormalization(name='phos_batch_norm')(phos_rnn)
        #         if dropout > 0.0:
        #             phos_rnn = Dropout(dropout, name='phos_dropout')(phos_rnn)
        #     except AttributeError:
        #         raise AttributeError('{} is not a valid Keras layer!'.format(rnn_layer))

        #     input_list.append(phos_input)
        #     concatenate_list.append(phos_rnn)

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


        if len(concatenate_list)>1:
            x = keras.layers.concatenate(concatenate_list)
        else: 
            x = concatenate_list[0]
        # add an auxilairy output
        output_list = []
        output_data = []
        if aux: 
            aux_output_track = Dense(1, activation='sigmoid', name='aux_evt_trk')(x)
            output_list.append(aux_output_track)
            output_data.append(y_train)
        for i in range(0,n_layers):
            if activation in special_activations:
                x = Dense(layer_nodes, kernel_regularizer=regularizers.l2(k_reg),
                          kernel_initializer='glorot_normal')(x)
                x = (getattr(keras.layers, activation)())(x)
            else:
                x = Dense(layer_nodes, activation = activation, 
                        kernel_regularizer=regularizers.l2(k_reg),
                        kernel_initializer = 'glorot_normal')(x)
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
                                # callbacks = [checkpointer]).history
                                callbacks = [callback_ROC(input_data, output_data, 
                                                          output_prefix=out_path),
                                             checkpointer]).history

        except KeyboardInterrupt:
            print('Training ended early.')

        return history


