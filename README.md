## Diffractive background studies

This collection of `c++` and `python 3+` programs is used to **(1)** study the background component in simulated (*Pythia-8*) diffractive events at ALICE and **(2)**
to reject the background component in the measured data.

1. The background study (1) builds upon and uses `c++` classes stored in `./lib`. These classes range from base classes necessary to execute tasks to plotting classes. 
    In oder to be able to use these classes, the `rootlogon`-file has to be modified. See `rootlogon-example.C` for the necessary lines which have to be added. 
1. In order to study the background component ALICE tasks are available in `./tasks/`. These tasks require an additional folder containing the *Offline Analysis DataBase* - 
    an `OADB` folder - to be placed into the `./tasks/` folder. 
    A short description on how to run the tasks is presented in the main folder `./tasks/README.md`. 


