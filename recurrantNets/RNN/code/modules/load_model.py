import os
import sys

from keras.models import Sequential
from keras.layers.core import Activation, Dense, Dropout
from keras.layers import Masking, LSTM, GRU, Concatenate, Input, Lambda

def load_model(data, run_params): 
    model_name = run_params['classifier_params_id']
    if 'P8own' in model_name:
        # pass shape into Masking layer
        y_train = data['target']
        X_particles_train = data['tracks']

        PARTICLE_SHAPE = X_particles_train.shape[1:]

        particle_channel = Sequential()

        #adding layers to the jet and photon class neural networks
        particle_channel.add(Masking(mask_value=-999, 
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

    else if 'GridSim' in model_name:
        X_track_train = data['track']
        X_raw_track_train = data['raw_track']
        X_emcal_train = data['emcal']
        X_phos_train=data['phos']
        X_event_train=data['event']
        X_calo_cluster_train=data['calo_cluster']
        X_ad_train=data['ad']
        X_fmd_train=data['fmd']
        X_v0_train=data['v0']

        y_train = data['target']

        # the data for the RNNs are of shape (n_evts, n_timesteps=n_particles, n_features) 
        # the masking layer however is only intersted in (n_timesteps, n_features)
        TRACK_SHAPE = X_track_train.shape[1:]
        RAW_TRACK_SHAPE = X_raw_track_train.shape[1:]
        EMCAL_SHAPE = X_emcal_train.shape[1:]
        PHOS_SHAPE = X_phos_train.shape[1:]
        CALO_CLUSTER_SHAPE = X_calo_cluster_train.shape[1:]
        # this following data is of shape (n_evts, n_features)
        # the network is fed with n_features
        EVENT_SHAPE = X_event_train.shape[1]
        AD_SHAPE = X_ad_train.shape[1]
        FMD_SHAPE = X_fmd_train.shape[1]
        V0_SHAPE = X_v0_train.shape[1]

        # adding layers to the particle, raw_tracking emcal and phos info,
        # calo clusters
        track_channel.add(Masking(mask_value=-999, 
                                     input_shape=TRACK_SHAPE, 
                                     name='track_masking'))
        track_channel.add(GRU(16, name='track_gru'))
        track_channel.add(Dropout(0.3, name='track_dropout'))

        raw_track_channel.add(Masking(mask_value=-999,
                                     input_shape=RAW_TRACK_SHAPE, 
                                     name='raw_track_masking'))
        raw_track_channel.add(GRU(14, name='raw_track_gru'))
        raw_track_channel.add(Dropout(0.3, name='raw_track_dropout'))

        emcal_channel.add(Masking(mask_value=-999,
                                     input_shape=EMCAL_SHAPE, 
                                     name='emcal_masking'))
        emcal_channel.add(GRU(2, name='emcal_gru'))
        emcal_channel.add(Dropout(0.3, name='emcal_dropout'))

        phos_channel.add(Masking(mask_value=-999,
                                     input_shape=PHOS_SHAPE, 
                                     name='emcal_masking'))
        phos_channel.add(GRU(2, name='phos_gru'))
        phos_channel.add(Dropout(0.3, name='phos_dropout'))

        calo_cluster_channel.add(Masking(mask_value=-999,
                                     input_shape=CALO_CLUSTER_SHAPE, 
                                     name='calo_cluster_masking'))
        calo_cluster_channel.add(GRU(4, name='calo_cluster_gru'))
        calo_cluster_channel.add(Dropout(0.3, name='calo_cluster_dropout'))

        event_level.add(Lambda(lambda x: x, input_shape=(EVENT_SHAPE, ))) 

        ad_level.add(Lambda(lambda x: x, input_shape=(AD_SHAPE, ))) 
        ad_level.add(Dense(16, activation='relu'))
        ad_level.add(Dense(1, activation='relu'))

        fmd_level.add(Lambda(lambda x: x, input_shape=(FMD_SHAPE, ))) 
        fmd_level.add(Dense(64, activation='relu'))
        fmd_level.add(Dense(1, activation='relu'))

        v0_level.add(Lambda(lambda x: x, input_shape=(V0_SHAPE, ))) 
        v0_level.add(Dense(64, activation='relu'))
        v0_level.add(Dense(1, activation='relu'))

        combined_rnn = Sequential()
        combined_rnn.add(Concatenate([particle_channel, 
                                      raw_track_channel,
                                      emc_channel, 
                                      phos_level, 
                                      calo_cluster_channel,
                                      event_level,
                                      fmd_level,
                                      ad_level,
                                      v0_level] ))
        combined_rnn.add(Dense(36, activation='relu'))
        combined_rnn.add(Dropout(0.3))
        combined_rnn.add(Dense(24, activation='relu'))
        combined_rnn.add(Dropout(0.3))
        combined_rnn.add(Dense(12, activation='relu'))
        combined_rnn.add(Dropout(0.3))
        combined_rnn.add(Dense(1, activation='softmax'))
        combined_rnn.compile('adam', 'sparse_categorical_crossentropy')

        return combined_rnn

    else:
        print'ERROR: Unrecognized model {}'.format(run_params['classifier_params_id'])
        exit(1)

    return model
