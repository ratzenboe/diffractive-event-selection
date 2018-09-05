## Training a model

Training a model is done within this folder. It is important to use `python 3+` and the data object, the *event dictionary*, created in an earlier step. 
If the event dictionary is not yet created do not proceed but instead change into the folder  `../progs` and follow the instructions decribed in the `README.md` there.

In order to train a model on the data the file `train.py` is executed. First and foremost the `run_mode` is specified via a command line input. Mainly two run-modi have been used during
this thesis, *i.e* `NN` for standard training of a neural network, and `koala` for a special training method described by [Methodiev *et al.*](https://arxiv.org/abs/1708.02949).

The main file `train.py` has many input parameters mostly steered via the **config files** described in the next section.

### Config files

The config files are a set of three steering files which contain numerous parameters to change. Among these parameters are *e.g.* the features used for training a model and
the hyperparameters of the model itself which include for example width and depth of a model. The three config files are located in the folder `./config` and deal with the
following parameters:

1. The `data_params.conf` file handles important information about the features used during training. 
| Paramerter name   |  Expanation |
|------------------ |-------------|
| max_entries       | If variable length sequences are possible (*e.g.* number of tracks, number of clusters) then max_entries defines the maximum number of saved entries (only relevant during event-dictionary creation, NOT during training)  |
| branches          | Features to use during training (evt_id in *event* should be left in the featues, gets removed automatically)  |
| missing_values    | If a feature has an internally saved special missing value, then it should be noted here. It will be altered to the default missing value of 999.0  |
| evt_id            | String defining the event id name. Important for checks if the event id feature is still in the set of featuers during training (should be removed before training)  |
| event_string      | String defining the event colom in the event dictionary (default "event"). Used during event-dictionary creation, not needed for training |
| cut_dic           | Dictionary specifying the pre-cut applied to the data before they are used during training | 
| remove_features   | List containing individual features which should be removed. For mode NN with 2 tracks this is not needed. | 
    
1. The `run_params.conf` file 

