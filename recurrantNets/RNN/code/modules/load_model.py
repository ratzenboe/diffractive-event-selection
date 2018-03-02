import os
import sys

import numpy as np
import copy

import keras
from keras.models import Model, Sequential
from keras.layers.core import Activation, Dense, Dropout
from keras.layers import Masking, LSTM, GRU, Input, BatchNormalization
from keras.layers.advanced_activations import PReLU
from keras.callbacks import ModelCheckpoint

from modules.keras_callback import callback_ROC

def train_model(data, run_mode_user, val_data,
                batch_size=64, n_epochs=50, rnn_layer='LSTM', 
                out_path = 'output/', dropout = 0.2, class_weight={0: 1., 1: 1.},
                n_layers=3, layer_nodes=100, batch_norm=False, activation='relu'):
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
        combined_rnn.compile('adam', 'binary_crossentropy')

        print(combined_rnn.summary())
        try:
            combined_rnn.fit(X_particles_train, y_train, 
                             validation_data = val_data,
                             batch_size = batch_size,
                             epochs = n_epochs)
        except KeyboardInterrupt:
            print('Training ended early.')

        return combined_rnn

    elif 'NN' in run_mode_user:
        try:
            X_train = data['feature_matrix']
            y_train = data['target']
            train_data = data.copy() 
            train_data.pop('target')
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
           if activation == 'PReLU':
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
                      # class_weight = class_weight)
                      # ,callbacks = [callback_ROC(train_data_dic, 
                      #                           output_targets, 
                      #                           output_prefix=out_path)])

        except KeyboardInterrupt:
            print('Training ended early.')

        return history

    elif 'Grid' in run_mode_user:
        try:
            X_track_train     = data['track']
            X_event_train     = data['event']
            y_train           = data['target']
            train_data = data.copy() 
            train_data.pop('target')

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
            track_rnn = (getattr(keras.layers, rnn_layer)(N_FEATURES_track, 
                name='track_rnn'))(track_input)
            if batch_norm:
                track_rnn = BatchNormalization(name='track_batch_norm')(track_rnn)
            if dropout > 0.0:
                track_rnn = Dropout(dropout, name='track_dropout')(track_batch_norm)

        except AttributeError:
            raise AttributeError('{} is not a valid Keras layer!'.format(rnn_layer))

        # aux_output_track = Dense(1, activation='softmax', 
        #         name='aux_output_track')(track_dropout)

        # output_list = [aux_output_track]

        concatenate_list = [track_rnn, event_input]
        # stack the layers on top of a fully connected DNN
        x = keras.layers.concatenate(concatenate_list)
        for i in range(0,n_layers):
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
        model = Model(inputs=input_list, outputs=main_output)
  
        # the main output has the full weight (main output is the first element
        # loss_weights = [1.]
        # output_targets = [y_train]
        # for i in range(len(output_list)-1):
        #     # the auxiliary outputs have to be weighted (here all: 0.2)
        #     loss_weights.append(0.2)
        #     # the target is always the same
        #     output_targets.append(y_train)

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
                      # ,callbacks = [callback_ROC(train_data, 
                      #                           y_train, 
                      #                           output_prefix=out_path)])

        except KeyboardInterrupt:
            print('Training ended early.')

        return history


    elif 'anomaly' in run_mode_user:
        history = train_autoencoder(data, val_data, batch_size, n_epochs, out_path, dropout, 
                          n_layers, batch_norm, activation)
        return history

    elif 'koala' in run_mode_user:
        history = train_koala(data, batch_size, n_epochs, out_path, dropout, 
                          n_layers, layer_nodes, batch_norm, activation)
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


def train_koala(data, batch_size=64, n_epochs=50, out_path = 'output/', 
                dropout = 0.2, n_layers=3, layer_nodes=100, batch_norm=False, activation='relu'):
    """
    Train a model on two sets of randomly picked sets of (unknown signal background ratio)
    the targets are not 0:bg and 1:sig but rather the set the particle came from. 
    meaning 0:set0 and 1:set1; 
    see: https://arxiv.org/abs/1801.10158
    """
    try:
        X_train = data['feature_matrix']
        y_train = data['target']
        # divide the training data into 2 groups: 
        #   1 = samples with sig and bg
        #   0 = samples with modeled bg (by chossing 3 tracks and omitting one)
        # new targets according to these groups
        y_train[(y_train==1) | (y_train==0)] = 1
        y_train[y_train==99] = 0

        # the training data stays the same
        train_data = {'feature_matrix': X_train}

        sig_percent = y_train[y_train==1].shape[0]/y_train.shape[0]*100.
        print('{}/{} signal events in 0({:.3f}%)'.format(
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
                            validation_split = 0.2,
                            callbacks = [checkpointer]).history
    except KeyboardInterrupt:
        print('Training ended early.')

    return history


