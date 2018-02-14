import os
import sys

import numpy as np
import copy

import keras
from keras.models import Model
from keras.layers.core import Activation, Dense, Dropout
from keras.layers import Masking, LSTM, GRU, Input, BatchNormalization

from modules.keras_callback import callback_ROC

def train_model(data, run_mode_user, val_data=0.2, 
                batch_size=64, n_epochs=50, rnn_layer='LSTM', 
                out_path = 'output/', dropout = 0.2, class_weight={0: 1., 1: 1.}):
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
        combined_rnn.add(Dropout(dropout))
        combined_rnn.add(Dense(24, activation='relu'))
        combined_rnn.add(Dropout(dropout))
        combined_rnn.add(Dense(12, activation='relu'))
        combined_rnn.add(Dropout(dropout))
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
            X_event_train     = data['event']
            y_train           = data['target']
            train_data_dic    = copy.deepcopy(data)
            train_data_dic.pop('target')

        except KeyError:
            raise KeyError('The data-dictionary provided does not contain' \
                    'all necessary keys for the selected run-mode (run_mode_user)')

        # the data for the RNNs are of shape (n_evts, n_timesteps=n_particles, n_features) 
        # the masking layer however is only intersted in (n_timesteps, n_features)
        TRACK_SHAPE = X_track_train.shape[1:]
        N_FEATURES_track = TRACK_SHAPE[-1]

        # this following data is of shape (n_evts, n_features)
        # the network is fed with n_features
        EVENT_SHAPE = (X_event_train.shape[-1],)
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
            N_FEATURES_emcal = EMCAL_SHAPE[-1]

            PHOS_SHAPE = X_phos_train.shape[1:]
            N_FEATURES_phos = PHOS_SHAPE[-1]

            CALO_CLUSTER_SHAPE = X_calo_cluster_train.shape[1:]
            N_FEATURES_calo_cluster = CALO_CLUSTER_SHAPE[-1]

            AD_SHAPE = (X_ad_train.shape[-1],)
            FMD_SHAPE = (X_fmd_train.shape[-1],)
            V0_SHAPE = (X_v0_train.shape[-1],)
         


        # inputs network
        track_input     = Input(shape=TRACK_SHAPE, dtype='float32', name='track')
        event_input     = Input(shape=EVENT_SHAPE, dtype='float32', name='event')

        input_list = [track_input, event_input]
        # ------ Build the model ---------
        # RNNs feeding into the event level layer
        track_mask     = Masking(mask_value=float(-999), name='track_masking')(track_input)
        # event needs no masking
        try:
            track_rnn = (getattr(keras.layers, rnn_layer)(N_FEATURES_track, 
                name='track_rnn'))(track_mask)
            track_batch_norm = BatchNormalization(name='track_batch_norm')(track_rnn)
            track_dropout = Dropout(dropout, name='track_dropout')(track_batch_norm)

        except AttributeError:
            raise AttributeError('{} is not a valid Keras layer!'.format(rnn_layer))

        # aux_output_track = Dense(1, activation='softmax', 
        #         name='aux_output_track')(track_dropout)

        # output_list = [aux_output_track]

        concatenate_list = [track_dropout, event_input]
        val_data_list = None

        if isinstance(val_data, dict):
            try: 
                val_data_list = [val_data['track'], val_data['event']]
            except KeyError:
                warnings.warn('The provided valiation dictionary does not contain ' \
                        'all necessary columns!\nConsequently 20% of the training data ' \
                        'are now used to fit the model!')
                val_data_list = None
                val_data = 0.2
                pass


        if 'Full' in run_mode_user:
            # here we now add additional channels to the network
            emcal_input = Input(shape=EMCAL_SHAPE, dtype='float32', name='emcal')
            phos_input = Input(shape=PHOS_SHAPE, dtype='float32', name='phos')
            calo_cluster_input = Input(shape=CALO_CLUSTER_SHAPE, dtype='float32', 
                    name='calo_cluster')
            ad_input = Input(shape=AD_SHAPE, dtype='float32', name='ad')
            fmd_input = Input(shape=FMD_SHAPE, dtype='float32', name='fmd')
            v0_input = Input(shape=V0_SHAPE, dtype='float32', name='v0') 

            full_input_list = [emcal_input, phos_input, calo_cluster_input, ad_input,
                    fmd_input, v0_input]

            # ------ Build the model ---------
            # RNNs
            emcal_masking = Masking(mask_value=float(-999), name='emcal_masking')(emcal_input)
            phos_masking  = Masking(mask_value=float(-999), name='phos_masking')(phos_input)
            calo_cluster_masking = Masking(mask_value=float(-999), 
                    name='calo_cluster_masking')(calo_cluster_input)
            try:
                emcal_rnn = (getattr(keras.layers, rnn_layer)(N_FEATURES_emcal, 
                    name='emcal_rnn'))(emcal_masking)
                emcal_dropout = Dropout(dropout, name='emcal_dropout')(emcal_rnn)

                phos_rnn = (getattr(keras.layers, rnn_layer)(N_FEATURES_phos, 
                    name='phos_rnn'))(phos_masking)
                phos_dropout = Dropout(dropout, name='phos_dropout')(phos_rnn)

                calo_cluster_rnn = (getattr(keras.layers, rnn_layer)(N_FEATURES_calo_cluster, 
                    name='calo_cluster_rnn'))(calo_cluster_masking)
                calo_cluster_dropout = Dropout(dropout, 
                        name='calo_cluster_dropout')(calo_cluster_rnn)

            except AttributeError:
                raise AttributeError('{} is not a valid Keras layers!'.format(rnn_layer))

 
            # NNs feeding into the event-level net
            dense_ad = Dense(16, activation='relu')(ad_input)
            ad_output = Dense(1, activation='softmax')(dense_ad)

            dense_fmd = Dense(64, activation='relu')(fmd_input)
            fmd_output = Dense(1, activation='softmax')(dense_fmd)

            dense_v0 = Dense(16, activation='relu')(v0_input)
            v0_output = Dense(1, activation='softmax')(dense_v0)

            # merge the output of these layers into one layer
            # and append an auxilliary output
            merge_full = keras.layers.concatenate([emcal_dropout, phos_dropout, 
                calo_cluster_dropout, ad_output, fmd_output, v0_output])
            aux_output_full = Dense(1, activation='softmax', 
                    name='aux_output_full')(merge_full)

            concatenate_list.append(merge_full)

            # extend the input and output list
            # output_list.append(aux_output_full)
            input_list.extend(full_input_list)
            if isinstance(val_data, dict):
                try: 
                    full_val_data_list = [val_data['emcal'], val_data['phos'], 
                            val_data['calo_cluster'], val_data['ad'], val_data['fmd'], 
                            val_data['v0']]
                except KeyError:
                    warnings.warn('The provided valiation dictionary does not contain ' \
                            'all necessary columns!\nConsequently 20% of the training data ' \
                            'are now used to fit the model!')
                    val_data_list = None
                    val_data = 0.2
                    pass



        # stack the layers on top of a fully connected DNN
        x = keras.layers.concatenate(concatenate_list)

        x = Dense(64, activation='relu')(x)
        x = BatchNormalization()(x)
        x = Dropout(dropout)(x)
        x = BatchNormalization()(x)
        x = Dense(64, activation='relu')(x)
        x = BatchNormalization()(x)
        x = Dropout(dropout)(x)
        x = Dense(64, activation='relu')(x)
        x = BatchNormalization()(x)
        x = Dropout(dropout)(x)
        main_output = Dense(1, activation='softmax', name='main_output')(x)
        # output_list.insert(0,main_output)
        model = Model(inputs=input_list, outputs=main_output)
  
        # the main output has the full weight (main output is the first element
        # loss_weights = [1.]
        # output_targets = [y_train]
        # for i in range(len(output_list)-1):
        #     # the auxiliary outputs have to be weighted (here all: 0.2)
        #     loss_weights.append(0.2)
        #     # the target is always the same
        #     output_targets.append(y_train)


        model.compile(optimizer='adam', loss='sparse_categorical_crossentropy')
        print(model.summary())
        try:
            model.fit(train_data_dic,
                      y_train,  
                      epochs = n_epochs, 
                      batch_size = batch_size,
                      validation_data = val_data_list,
                      validation_split = val_data,
                      class_weight = class_weight)
                      # ,callbacks = [callback_ROC(train_data_dic, 
                      #                           output_targets, 
                      #                           output_prefix=out_path)])

        except KeyboardInterrupt:
            print('Training ended early.')

        return model

    else:
        raise NameError('ERROR: Unrecognized model {}'.format(run_mode_user))


