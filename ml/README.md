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
    | max_entries       | If variable length sequences are possible (*e.g.* number of tracks, number of clusters) then max_entries defines the maximum number 
                            of saved entries (only relevant during event-dictionary creation, NOT during training)  |
    | branches          | Features to use during training (evt_id in *event* should be left in the featues, gets removed automatically)  |
    | missing_values    | If a feature has an internally saved special missing value, then it should be noted here. It will be altered to the default missing value of 999.0  |
    | evt_id            | String defining the event id name. Important for checks if the event id feature is still in the set of featuers during 
                            training (should be removed before training)  |
    | event_string      | String defining the event colom in the event dictionary (default "event"). Used during event-dictionary creation, not needed for training |
    | cut_dic           | Dictionary specifying the pre-cut applied to the data before they are used during training | 
    | remove_features   | List containing individual features which should be removed. For mode NN with 2 tracks this is not needed. | 
    
1. The `
    One for each detector and a event and track seperate tree to cope with the un-matching number events (#tracks != #events, etc). This is started via
    ```
    ./.linearizeCEPBuffers -rootdir=/path/to/rootfiles-dir/ -exedir=/path/to/exedir/ -outputdir=/path/where/output/goes/ -nev=#evts
    ```
1. In the next step we transform the root files into the event dictionary format which is readable in `python3`
    ```
    ./.root2evtdic -filesdir=/path/where/previous/output/went/ -nproc=#processes -runmode=runmode
    ```
    The #processes corresponds to the number of jobs that the problem is split into. Hence how often the program will be started. The `runmode` corresponds to what will be read from the config file which should be saved under `/path/to/exedir/data_params.conf`. If this is not the case the progam will note that not all necessary files are present in the exe-dir.

1. The output of the last program creates multiple event dictionaries that can be concatenated with
    ```
    python3 concat_evt_dic.py -filespath /path/to/evtdics/
    ```

## Plots

We can plot the features in the (raw-)buffers easily by using the following pipeline:

1. We initialize the feature plotting by using the following lines
    ```
    ./.feature_extraction_to_tlist 
    ```
    The `nev`, number of evts, is the number of events per batch. The `filter` defines if the data plotted will have some active filters (*e.g.* !V0). 
1. The previous step outputs many individual small plots each only covering `nev`. We now merge the files with
    ```
    ./TListMerger.sh -filesdir=/path/to/previous/output/ -exedir=/exe/dir/
    ```

### Ratio plots

If we ran the previous plotting script with multiple cuts selected or over diffenrent datasets we can compare the featrue distributions via the following scipt
```
./ratioplot.sh -file1=/path/to/file1 -file2=/path/to/file2 -outputdir=/out/path/ -exedir=/path/to/exedir/
```
**Warning:** this script may fail in `root-v5` as the `TRatioPlot` class may not be implemented. Execute in `root-v6`




