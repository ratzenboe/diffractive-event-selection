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

| Parameter         |  Explanation |
|------------------ |-------------|
| max_entries       | If variable length sequences are possible (*e.g.* number of tracks, number of clusters) then max_entries defines the maximum number of saved entries (only relevant during event-dictionary creation, NOT during training)  |
| branches          | Features to use during training (evt_id in *event* should be left in the featues, gets removed automatically)  |
| missing_values    | If a feature has an internally saved special missing value, then it should be noted here. It will be altered to the default missing value of 999.0  |
| evt_id            | String defining the event id name. Important for checks if the event id feature is still in the set of featuers during training (should be removed before training)  |
| event_string      | String defining the event colom in the event dictionary (default "event"). Used during event-dictionary creation, not needed for training |
| cut_dic           | Dictionary specifying the pre-cut applied to the data before they are used during training | 
| remove_features   | List containing individual features which should be removed. For mode NN with 2 tracks this is not needed. | 
    
1. The `run_params.conf` file 

| Parameter         | Explanation   |
| ---------------   | -----------   |
| frac_val_sample   | The fraction of the whole data which is used to validate the performance during training. Default: 0.2 |
| frac_test_sample  | The fraction of the (remaining) training data (after the validation sample is split off used to test the final model performance. Default: 0.25, results in a final split of 60 - 20 - 20 (train-validation-test). |
| do_standard_scale     | Bool: if the data should be standard scaled for training. |
| data_set              | String: name of the dataset (not neccessary, just for print-outs |

1. The `model_params.conf` file 

| Parameter         | Explanation   |
| ---------------   | -----------   |
| rnn   | String: If the recursive mode is chosen this parameter defines which recursive unit is used. Default: 'LSTM'. |
| n_layers  | Int: Number of layers of the neural network which is attached to the intput. |
| layer_nodes     | Int: width of the networks hidden layers. |
| activation      | String: Network activation function. |
| batch_size      | Int: Size of the mini-batch fed into the neural network at once. |
| n_epochs      | Int: Number of training epochs. |
| dropout      | Float in (0., 1.) or list of floats with length = n_layers: Dropout fraction |
| k_reg      | Float: Kernel regularizer - not important and currently not in use! |
| class_weights     | Dictionary: Weights used to compensate for a class imbalance - currently not in use! | 


Additionally some command-line arguments can also be used. A detailed description is available via the `--help` command, *i.e.*:
```
python train.py --help
```
This command opens the help window featuring every commando-line argument with an explanation. **Attention:** there are default settings for every 
command-line parameter, however, some may not be correct, *e.g.* the `-inpath` argument has to be set if the default path `output/evt_dic.pkl` is non-existing.
The training is then done via
```
python train.py -necessaryarguments
```
and the outputs are stored in session folders tagged by the date `ml/output/session-date`. **Attention:** by default the session folders delete themselves if more than 20 folders
starting with the signature `session_` exist. Therefore, rename important result-folders.

After a model is trained, it can be used to classify data with the `classify.py` script. This script needs an already trained model which is used in oder to reject background events. 
Here command line arguments are also very important. The program creates `.txt` file with signal-event guesses which is saved in the folder where also the model is saved.
This `.txt` file can be used to plot invariant mass spectrum of the events which the model recognizes as signal. This is done with the `./plottingscripts/CEPBuffersToList_fromtxtfile.C` file.
