# diffractive-event-selection
This repository is a collection of python files, that are used to classify 4 different event classes.
The input files are created by Pythia 8, using the MBR model to simulate diffractive events.
The modules folder contains multiple files for pre-processing and preparing the data as well as plot-files for displaying the ROC-curves, MVA outputs and significance. The pre-processing takes the simulated events and fills a histogram per event
which is saved in a .h5 (h5py, hdf5) file format for quicker and easier accsess.
As the dataset is of various multiplicity the particles are filled into a eta-phi space with pt as the intensity=color 
(and dE/dx from the TPC as 2nd and beta from TOF as a 3rd dimension, like rgb picture)
2 different classifiers are currently in use, a convolutional model (CNN) and a standard NN which is created by using the
2d-histogram and flatten it into 1d space.

Currently the CNN outperforms the NN
