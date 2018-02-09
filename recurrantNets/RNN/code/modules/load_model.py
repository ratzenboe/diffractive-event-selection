import os
import sys

import numpy as np

import keras
from keras.models import Sequential
from keras.layers.core import Activation, Dense, Dropout
from keras.layers import Masking, LSTM, GRU, Concatenate, Input, Lambda

def train_model(data, run_mode_user, val_data=0.2, batch_size=64, n_epochs=50, rnn_layer='LSTM'):
    """
    Args 
        data:
            the data dictionary
        ___________________________________________________________

        run_mode_user:
            string that decribes which model to use
        ___________________________________________________________

        val_data:
            either another data dictonary 
            or a floating value between 0 and 1 determining the 
            validation data size (second may be better as we do 
            not train only once but for multiple epochs)
        ___________________________________________________________

        batch_size:
            integer number determining the number of samples in 
            each batch
        ___________________________________________________________

        n_epochs:
            integer, number of training epochs

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

    for key, value in data.iteritems():
        if not isinstance(value, np.ndarray):
            raise TypeError('The key {} of the "data"-dictionary ' \
                    'is not of type numpy ndarrays but rather {}'.format(key, type(value)))

    if not isinstance(run_mode_user, str):
        raise TypeError('The variable "run_mode_user" is not a string type ' \
                'but instead {}'.format(type(run_mode_user)))

    if not isinstance(val_data, float) and not isinstance(val_data, dict):
        raise TypeError('The variable "val_data" is neither a dictionary ' \
                'nor a float but instead {}'.format(type(val_data)))

    if isinstance(val_data, dict):
        for key, value in data.iteritems():
            if not isinstance(value, np.ndarray):
                raise TypeError('The key {} of the "value"-dictionary ' \
                        'is not of type numpy ndarrays but rather {}'.format(key, type(value)))

    if not isinstance(rnn_layer, str):
        raise TypeError('The variable "rnn_layer" is not a string type ' \
                'but insted {}'.format(type(rnn_layer)))


    if 'P8own' in run_mode_user:
        # pass shape into Masking layer
        y_train = data['target']
        X_particles_train = data['tracks']

        PARTICLE_SHAPE = X_particles_train.shape[1:]

        particle_channel = Sequential()

        #adding layers to the jet and photon class neural networks
        particle_channel.add(Masking(mask_value=float(-999), 
                                     input_shape=PARTICLE_SHAPE, 
                                     name='particle_masking'))
        particle_channel.add(GRU(5, name='particle_gru'))
        particle_channel.add(Dropout(0.3, name='particle_dropout'))

        combined_rnn = Sequential()
        combined_rnn.add(Concatenate([particle_channel], mode='concat'))  
        combined_rnn.add(Dense(36, activation='relu'))
        combined_rnn.add(Dropout(0.3))
        combined_rnn.add(Dense(24, activation='relu'))
        combined_rnn.add(Dropout(0.3))
        combined_rnn.add(Dense(12, activation='relu'))
        combined_rnn.add(Dropout(0.3))
        combined_rnn.add(Dense(1, activation='softmax'))
        # compile
        combined_rnn.compile('adam', 'sparse_categorical_crossentropy')

        print(combined_rnn.summary())
        try:
            combined_rnn.fit(X_particles_train, y_train, 
                             validation_data = (val_data['tracks'], val_data['target']),
                             batch_size = batch_size,
                             epochs = n_epochs)
        except KeyboardInterrupt:
            print('Training ended early.')

        return combined_rnn

    elif 'Grid' in run_mode_user:
        try:
            X_track_train     = data['track']
            X_raw_track_train = data['raw_track']
            X_event_train     = data['event']
            y_train           = data['target']
        except KeyError:
            raise KeyError('The data-dictionary provided does not contain' \
                    'all necessary keys for the selected run-mode (run_mode_user)')

        # the data for the RNNs are of shape (n_evts, n_timesteps=n_particles, n_features) 
        # the masking layer however is only intersted in (n_timesteps, n_features)
        TRACK_SHAPE = X_track_train.shape[1:]
        RAW_TRACK_SHAPE = X_raw_track_train.shape[1:]
        # this following data is of shape (n_evts, n_features)
        # the network is fed with n_features
        EVENT_SHAPE = X_event_train.shape[1]
        COMBINED_SHAPE = TRACK_SHAPE[0] + RAW_TRACK_SHAPE[0] + EVENT_SHAPE
        if 'Full' in run_mode_user:
            try:
                X_emcal_train        = data['emcal']
                X_phos_train         = data['phos']
                X_calo_cluster_train = data['calo_cluster']
                X_ad_train           = data['ad']
                X_fmd_train          = data['fmd']
                X_v0_train           = data['v0']
            except KeyError:
                raise KeyError('The data-dictionary provided does not contain' \
                        'all necessary keys for the selected run-mode ' \
                        '(run_mode_user = {})'.format(run_mode_user))

            EMCAL_SHAPE = X_emcal_train.shape[1:]
            PHOS_SHAPE = X_phos_train.shape[1:]
            CALO_CLUSTER_SHAPE = X_calo_cluster_train.shape[1:]
            AD_SHAPE = X_ad_train.shape[1]
            FMD_SHAPE = X_fmd_train.shape[1]
            V0_SHAPE = X_v0_train.shape[1]
         
            COMBINED_SHAPE += EMCAL_SHAPE[0] + PHOS_SHAPE[0] + CALO_CLUSTER_SHAPE[0]
            COMBINED_SHAPE += AD_SHAPE + FMD_SHAPE + V0_SHAPE


        # basis network
        track_channel = Sequential()
        raw_track_channel = Sequential()
        event_level = Sequential()

        # ------ Build the model ---------
        # RNNs feeding into the event level layer
        track_channel.add(Masking(mask_value=float(-999), 
                                     input_shape=TRACK_SHAPE, 
                                     name='track_masking'))
        try:
            track_channel.add(getattr(keras.layers, rnn_layer)(16, name='track_rnn'))
            track_channel.add(Dropout(0.3, name='track_dropout'))

            raw_track_channel.add(Masking(mask_value=float(-999),
                                         input_shape=RAW_TRACK_SHAPE, 
                                         name='raw_track_masking'))
            raw_track_channel.add(getattr(keras.layers, rnn_layer)(14, name='raw_track_rnn'))
            raw_track_channel.add(Dropout(0.3, name='raw_track_dropout'))
        except AttributeError:
            raise AttributeError('{} is not a valid Keras layer!'.format(rnn_layer))

        # event level layer
        event_level.add(Lambda(lambda x: x, input_shape=(EVENT_SHAPE, ))) 

        layer_list = [track_channel, raw_track_channel, event_level]
        data_list = [X_track_train, X_raw_track_train, X_event_train]
        val_data_list = None

        if isinstance(val_data, dict):
            try: 
                val_data_list = [val_data['track'], val_data['raw_track'], val_data['event']]
            except KeyError:
                warnings.warn('The provided valiation dictionary does not contain ' \
                        'all necessary columns!\nConsequently 20% of the training data ' \
                        'are now used to fit the model!')
                val_data_list = None
                val_data = 0.2
                pass


        if 'Full' in run_mode_user:
            # here we now add additional channels to the network
            emcal_channel = Sequential()
            phos_channel = Sequential()
            calo_cluster_channel = Sequential()
            ad_level = Sequential()
            fmd_level = Sequential()
            v0_level = Sequential()

            # ------ Build the model ---------
            # RNNs
            emcal_channel.add(Masking(mask_value=float(-999),
                                         input_shape=EMCAL_SHAPE, 
                                         name='emcal_masking'))
            try:
                emcal_channel.add(getattr(keras.layers, rnn_layer)(2, name='emcal_rnn'))
                emcal_channel.add(Dropout(0.3, name='emcal_dropout'))

                phos_channel.add(Masking(mask_value=float(-999),
                                             input_shape=PHOS_SHAPE, 
                                             name='emcal_masking'))
                phos_channel.add(getattr(keras.layers, rnn_layer)(2, name='phos_rnn'))
                phos_channel.add(Dropout(0.3, name='phos_dropout'))

                calo_cluster_channel.add(Masking(mask_value=float(-999),
                                             input_shape=CALO_CLUSTER_SHAPE, 
                                             name='calo_cluster_masking'))
                calo_cluster_channel.add(getattr(keras.layers, rnn_layer)(4, name='calo_cluster_rnn'))
                calo_cluster_channel.add(Dropout(0.3, name='calo_cluster_dropout'))
            except AttributeError:
                raise AttributeError('{} is not a valid Keras layers!'.format(rnn_layer))

 
            # NNs feeding into the event-level net
            ad_level.add(Lambda(lambda x: x, input_shape=(AD_SHAPE, ))) 
            ad_level.add(Dense(16, activation='relu'))
            ad_level.add(Dense(1, activation='relu'))

            fmd_level.add(Lambda(lambda x: x, input_shape=(FMD_SHAPE, ))) 
            fmd_level.add(Dense(64, activation='relu'))
            fmd_level.add(Dense(1, activation='relu'))

            v0_level.add(Lambda(lambda x: x, input_shape=(V0_SHAPE, ))) 
            v0_level.add(Dense(64, activation='relu'))
            v0_level.add(Dense(1, activation='relu'))

            sim_layer_list = [emcal_channel, phos_channel, calo_cluster_channel, 
                              ad_level, fmd_level, v0_level]
            sim_data_list = [X_emcal_train, X_phos_train, X_calo_cluster_train,
                             X_ad_train, X_fmd_train, X_v0_train]

            layer_list.extend(sim_layer_list)
            data_list.extend(sim_data_list)

            if isinstance(val_data, dict):
                try: 
                    sim_val_data_list = [val_data['emcal'], val_data['phos'], val_data['calo_cluster'],
                                         val_data['ad'], val_data['fmd'], val_data['v0']]
                except KeyError:
                    warnings.warn('The provided valiation dictionary does not contain ' \
                            'all necessary columns!\nConsequently 20% of the training data ' \
                            'are now used to fit the model!')
                    val_data_list = None
                    val_data = 0.2
                    pass



        # stack the layers on top of a fully connected DNN
        combined_rnn = Sequential()
        #######################################################
        # rewrite model based on https://keras.io/getting-started/functional-api-guide/
        # the add(Concatenate(..)) does not work properly here!
        combined_rnn.add(Concatenate([layer_list], input_shape=(COMBINED_SHAPE,)))
        combined_rnn.add(Dense(36, activation='relu'))
        combined_rnn.add(Dropout(0.3))
        combined_rnn.add(Dense(24, activation='relu'))
        combined_rnn.add(Dropout(0.3))
        combined_rnn.add(Dense(12, activation='relu'))
        combined_rnn.add(Dropout(0.3))
        combined_rnn.add(Dense(1, activation='softmax'))
        combined_rnn.compile('adam', 'sparse_categorical_crossentropy')

        print(combined_rnn.summary())
        try:
            combined_rnn.fit(data_list, y_train, 
                             validation_split = val_data,
                             validation_data  = val_data_list,
                             batch_size       = batch_size,
                             epochs           = n_epochs,
                             callbacks        = callback_ROC(data_list, y_train)) 

        except KeyboardInterrupt:
            print('Training ended early.')

        return combined_rnn

    else:
        raise NameError('ERROR: Unrecognized model {}'.format(run_mode_user))


