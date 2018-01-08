import os
import os.path
import math

import numpy as np
import pandas as pd
import root_numpy
import matplotlib
import matplotlib.pyplot as plt

from sklearn.metrics import roc_auc_score, roc_curve

def reduce_y_dimension( y_truth, y_score, class_number ):
    """
    as we have a mulitlabel classification, we need to a binary class. to plot 
    the roc-curve. class_number: (0=ND, 1=SD, 2=DD, 3=CD)
    """
    y_truth_output = y_truth[:,class_number]
    y_score_output = y_score[:,class_number]

    return y_truth_output, y_score_output

def plot_ROC( y_true, y_score, class_number, NN=None, output_folder=None ):
    "plots the ROC curve"

    if output_folder is None:
        output_folder='/home/ratzenboe/Documents/imageClassification/plots/'
    class_str = ''
    if class_number==0:
        class_str = 'ND'
    if class_number==1:
        class_str = 'SD'    
    if class_number==2:
        class_str = 'DD'    
    if class_number==3:
        class_str = 'CD'    
    output_folder+='_'+class_str    

    #first change to the class number we want to check out
    y_tr_binary, y_score_binary = reduce_y_dimension(y_true, y_score, class_number)

    fpr, tpr, threshold = roc_curve(y_tr_binary, y_score_binary, pos_label=1)
    roc_auc = roc_auc_score(y_tr_binary, y_score_binary)

    plt.figure()
    plt.plot(fpr,tpr,label='ROC curve (AUC = %0.3f)'%roc_auc)
    plt.plot([0,1],[0,1], 'k--')
    plt.xlim([-0.05, 1.05])
    plt.ylim([-0.05, 1.05])
    plt.xlabel('False Positive Rate')
    plt.ylabel('True Positive Rate')
    plt.title('Receiver operating characteristic curve: '+titleStr)
    plt.grid(True)

    plt.legend(loc=4)

    plt.savefig(output_folder+'_roc_curve.pdf')
    
    return

def plot_MVAoutput( y_truth, y_score, class_number, nbins=100, output_folder=None ):
    """
    plots the MVA output as histogram and returns the underlaying
    distributions of the positive and negative class
    """
    
    # split the array and use only the column containing the
    # probabilities of an event to belong the the "signal" class
    # (can be done in case of 'binary' classification)
    # hence: first make it binary

    if output_folder is None:
        output_folder='/home/ratzenboe/Documents/imageClassification/plots/'    
    class_str = ''
    if class_number==0:
        class_str = 'ND'
    if class_number==1:
        class_str = 'SD'    
    if class_number==2:
        class_str = 'DD'    
    if class_number==3:
        class_str = 'CD'    
    output_folder += '_'+class_str
    

        
    y_tr_binary, y_score_binary = reduce_y_dimension(y_truth, y_score, class_number)

    y_score_truePos = y_score_binary[np.array(y_tr_binary==1.)]
    y_score_trueNeg = y_score_binary[np.array(y_tr_binary==0.)]

    plt.figure()
    n_total, bins_total, patches_total = plt.hist( y_score_binary,
                                                    bins=nbins,
                                                    alpha=0.5,
                                                    color='black',
                                                    label='MVA output' )
    n_trueNeg, bins_trueNeg, patches_trueNeg = plt.hist(y_score_trueNeg,
                                                        bins=nbins,
                                                        alpha=0.5,
                                                        color='#dd0000',
                                                        label='true negative')

    n_truePos, bins_truePos, patches_truePos = plt.hist(y_score_truePos,
                                                        bins=nbins,
                                                        alpha=0.5,
                                                        color='green',
                                                        label='true positive')

    plt.title('MVA output distribution (positive class): '+titleStr)
    plt.xlim(-0.05, 1.05)
    plt.xlabel('MVA output')
    plt.ylabel('Entries')
    plt.grid(True)
    plt.yscale('log')
    plt.legend()
    
    plt.savefig(output_folder+'_MVA_output_distr.pdf')

    return n_truePos, n_trueNeg


def plot_eff_significance( n_truePos, n_trueNeg, class_str, nbins=100, output_folder=None ):
    """
    plot efficiency of signal and background and also the 
    significance = S(MVA>x)/sqrt(S(MVA>x)+B(MVA>x))
    """

    if output_folder is None:
        output_folder='/home/ratzenboe/Documents/imageClassification/plots/'    
    output_folder+=class_str

    MVAcut = np.empty((0))

    plt.figure()
    fig, ax1 = plt.subplots()
    signal_efficiency = np.empty((0))
    backgr_efficiency = np.empty((0))
    for i in range(nbins):
        signal_efficiency = np.append(signal_efficiency, \
                                      np.sum(n_truePos[i:n_truePos.shape[0]]) / np.sum(n_truePos))
        backgr_efficiency = np.append(backgr_efficiency, \
                                      np.sum(n_trueNeg[i:n_trueNeg.shape[0]]) / np.sum(n_trueNeg))
        MVAcut = np.append(MVAcut, i/(nbins*1.0))

    l1 = ax1.plot(MVAcut, signal_efficiency, label='signal efficiency', color='blue')
    l2 = ax1.plot(MVAcut, backgr_efficiency, label='background efficiency', color='red')
    ax1.set_xlabel('MVA cut')
    ax1.set_ylabel('Efficiency')

    ax2 = ax1.twinx()
    significance_per_MVAcut = np.empty((0))
    for i in range(nbins):
        significance_per_MVAcut = np.append(significance_per_MVAcut, \
                                            np.sum(n_truePos[i:n_truePos.shape[0]]) / \
                                            math.sqrt(np.sum(n_truePos[i:n_truePos.shape[0]] + \
                                                             n_trueNeg[i:n_trueNeg.shape[0]])))
    
    l3 = ax2.plot(MVAcut, significance_per_MVAcut,
                  label='significance',
                  color='green')
    pos_max = np.argmax(significance_per_MVAcut)
    MVAcut_opt = pos_max/(nbins*1.0)
    l4 = ax2.plot(pos_max/(nbins*1.0), significance_per_MVAcut[pos_max],
                  label='max. significance for cut at %.2f' % MVAcut_opt,
                  marker='o', markersize=10, fillstyle='none', mew=2, linestyle='none',
                  color='#005500')
    ax2.set_ylabel('Significance', color='green')
    ax2.tick_params('y', colors='green')

    plt.title('MVA cut efficiencies ('+class_str+')')
    lall = l1+l2+l3+l4
    labels = [l.get_label() for l in lall]
    ax2.legend(lall, labels, loc='lower left')
    plt.tight_layout()

    plt.savefig(output_folder+'_eff_significance.pdf')

    return

def plot_ROC_MVA_eff_sign_allDiff( y_test_true, y_test_score, 
         NN=None, eta_val=13, output_folder=None, nbins=100, dimension=1, onlyPos=False):
    """
    function that goes automaticlly through all 4 classes to produce the relevent plots:
    """
    if output_folder is None:
        output_folder='/home/ratzenboe/Documents/imageClassification/plots/'    
    titleStr = ''
    if NN is None:
        titleStr = ''
    elif NN:
        titleStr = 'NN'
    else:
        titleStr = 'Conv2D'

    output_folder+='_'+titleStr
    output_folder+='_Eta'+str(eta_val)
    output_folder+='_'+str(dimension)+'d'
    if onlyPos:
        output_folder+='_'+'onlyPos'

    plot_ROC(y_test_true, y_test_score, class_number=3, output_folder=output_folder)
    plot_ROC(y_test_true, y_test_score, class_number=2, output_folder=output_folder)
    plot_ROC(y_test_true, y_test_score, class_number=1, output_folder=output_folder)
    plot_ROC(y_test_true, y_test_score, class_number=0, output_folder=output_folder)
    n_TP_ND, n_TN_ND = plot_MVAoutput(y_test_true,y_test_score,nbins=nbins,
				      class_number=0 output_folder=output_folder)
    n_TP_SD, n_TN_SD = plot_MVAoutput(y_test_true,y_test_score,nbins=nbins,
				      class_number=1,NN=useNN, output_folder=output_folder)
    n_TP_DD, n_TN_DD = plot_MVAoutput(y_test_true,y_test_score,nbins=nbins,
				      class_number=2,NN=useNN, output_folder=output_folder)
    n_TP_CD, n_TN_CD = plot_MVAoutput(y_test_true,y_test_score,nbins=nbins,
				      class_number=3,NN=useNN, output_folder=output_folder)

    plot_eff_significance(n_TP_ND,n_TN_ND,'ND',nbins=nbins,NN=useNN,output_folder=output_folder)
    plot_eff_significance(n_TP_SD,n_TN_SD,'SD',nbins=nbins,NN=useNN,output_folder=output_folder)
    plot_eff_significance(n_TP_DD,n_TN_DD,'DD',nbins=nbins,NN=useNN,output_folder=output_folder)
    plot_eff_significance(n_TP_CD,n_TN_CD,'CD',nbins=nbins,NN=useNN,output_folder=output_folder)

    return


    




