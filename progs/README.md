## Execution order

Before anything can be executed we have to enter the ali-environment. This can be fixed in the `.bashrc`.

1. As we have to execute multiple files it is easier to just store these files all in one directoy. We change into this directory and the next steps assume
    that we are always in this directory
    ```
    cd /path/to/exedir
    ```
1. From the CEP-buffers which are directly filled on the grid we feed the information to a standard linear TTree. 
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




