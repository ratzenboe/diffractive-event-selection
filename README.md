## Diffractive background studies

This collection of `c++` and `python 3+` programs is used to **(1)** study the background component in simulated (*Pythia-8*) diffractive events at ALICE and **(2)**
to reject the background component in the measured data.

1. The background study (1) builds upon and uses `c++` classes stored in `./lib/`. These classes range from base classes necessary to execute tasks to plotting classes. 
    In oder to be able to use these classes, the `rootlogon`-file has to be modified. See `rootlogon-example.C` for the necessary lines which have to be added. 
1. In order to study the background component ALICE tasks are available in `./tasks/`. These tasks require an additional folder containing the *Offline Analysis DataBase* - 
    an `OADB` folder - to be placed into the `./tasks/` folder. 
    A short description on how to run the tasks is presented in the main folder `./tasks/README.md`. 
1. If the *EventDef* class has been used to create a precise list of the decay modes with their respective sub-decays then a `decaymodes.tex` file is created (located in the 
    respective task folder where the `.go` script has been evoked) which is a table written in latex format. This table can be transferred into a `.pdf` format by 
    copying the `decaymodes.tex` file into the `./latex/` folder and invoking `./latex/compile.sh`. The pdf file is stored in `./latex/output/doc.pdf`.
1. Plotting is usually done via a special class saved in `./lib` called the *PlotTask* class. It can be used to make a wide variety of plots 
    (*e.g.* ratio plots, significance plots, etc.). It can be used to plot the task-output histograms as well standarad histograms saved in a `.root` format. The
    aim of this class is to plot histograms in a uniform style. With multiple implemented functions (see `./lib/PlotTask.h`) a plot can be made more 
    quickly and simply without having to deal with individual root funtions for individual hisotgrams. 
    In the folder `./plottingscripts/` some macros can be found which make use of this class. 
1. Additional smaller `c++` macros which are not crucial to the background study but may become useful in the study of background events are saved in `./misc`

The background rejection is done with machine learning methods with `python 3+`. 

1. In oder to use the data stored in the *CEP-buffers* (*i.e.* CEPEventBuffer, CEPTrackBuffer,and the raw buffers) a few steps have to be taken. 
    The necessary programs to do so are stored in the folder `./root2pyt/` and the procedure is described there.
1. After the data is converted into the special format (refered to as the *event-dictionary*) machine learning algorithms (neural networks using the `Keras` software framework) 
    can be trained using the programs (and instructions) provided in the `./ml/` folder.


