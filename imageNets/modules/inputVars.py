import os
import os.path
import sys

def set_variables():
    """
    ask the user to choose which modes that he/she wants to test
    """
    in1 = raw_input("Use NN?[y], else use CNN: ")
    if (in1 == 'y') or (in1 == 'Y'):
        use_NN = True
    else:
        use_NN = False

    in2 = raw_input("Use pretrained model?[y], else train again: ")
    if (in2 == 'y') or (in2 == 'Y'):
        load_weights_bool = True
    else:
        load_weights_bool = False
    
    in3 = raw_input("Use only positional infos?[y], or a 3-dim model[d], else 1-dim: ")
    if (in3 == 'y') or (in3 == 'Y'):
        only_position_info = True
        img_channels = 1
    elif (in3 == 'd') or (in3 == 'D'):
        only_position_info = False
        img_channels = 3
    else:
        only_position_info = False
        img_channels = 1

    in4 = raw_input("|eta| < 0.9?[y], else whole eta range: ")
    if (in2 == 'y') or (in2 == 'Y'):
        eta_LT09 = True
    else:
        eta_LT09 = False

    return use_NN, load_weights_bool, img_channels, only_position_info, eta_LT09

def get_data_model_path( use_NN, load_weights_bool, img_channels, only_position_info, eta_LT09 ):

    trainTestValidFile='/media/hdd/eventInfo/train_valid_test'
    if eta_LT09:
        trainTestValidFile+='_etaLT09'
    # if we want to test something, it is better to use the pretrained models:
    model_path = '/home/ratzenboe/Documents/imageClassification/gettingStarted/pretrainedmodels/'
    if use_NN:
        model_fn = 'NN_3d_weights'
    else:
        model_fn = 'cnn'
    if img_channels == 3:
        model_fn += '_3d_weights'
        trainTestValidFile+='_3d'
    else:
        model_fn += '_weights'

    # if eta_LT09:
 
    model_fn += '_etaLT09'

    trainTestValidFile += '.h5'
    model_fn += '.h5'

    return trainTestValidFile, model_path+model_fn

