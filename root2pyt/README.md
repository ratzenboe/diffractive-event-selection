## Root2pyt

The purpose of these programs is to generate a data structure suited for machine learning tasks. The data is initially available in the form of root files, more precisely in the 
form of special classes (called *CEPBuffers*, written in `c++`) which store the necessary information. The programs explained in the following section transform data saved in various
files to a single python dictionary which is easy to handle and can be used for all machine learning purposes.

### Execution order

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
