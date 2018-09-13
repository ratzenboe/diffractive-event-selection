## Running the tasks

In order to run the background (`./BG`) or the calorimeter specific task (`./EMCAL`) first change in one of the respective subfolders.
Then the task is mainly steered via the macro `run{EMCAL,BG}Ana.C` which is exucuted via the `.go` script. The script then processes all (root-) files listed in `files.txt`. 
The root-files have to be include **ESD** objects in order to be correctely processed.
