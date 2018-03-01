from __future__ import division

import os
import sys
import math
import itertools

import matplotlib
matplotlib.use('agg')
import matplotlib.pyplot as plt
import seaborn as sns
sns.set()

import numpy as np
import pandas as pd

from sklearn.metrics import roc_curve, roc_auc_score, accuracy_score, \
    confusion_matrix, classification_report, precision_recall_curve, \
    average_precision_score

from scipy import stats


def plot_model_loss(history, out_path):
    """
    Plot the model training and validation loss over the training epochs
    """
    plt.figure()
    plt.plot(history['loss'])
    plt.plot(history['val_loss'])
    plt.title('model loss')
    plt.ylabel('loss', fontsize=18)
    plt.xlabel('epoch', fontsize=18)
    plt.legend(['train', 'test'], loc='upper right', fontsize=15)
    plt.tight_layout()

    plt.savefig(out_path + 'model_loss' + '.png')
    plt.savefig(out_path + 'model_loss' + '.pdf')



def plot_all_features(evt_dic, branches_dic, outpath, post_fix='', real_bg=False):
    """
    Plot all features of the event dictionary comparing fully reconstructed events
    with background (feed-down) events
    """
    target_array = evt_dic['target']
    sig_value = 1
    bg_value  = 0
    sig_label = 'signal'
    bg_label  = 'background'
    combined_label  = 'signal+backgr.'
    if real_bg:
        sig_value = 0
        bg_value  = 99
        sig_label = 'real BG'
        bg_label  = '3+ tracks BG'
        combined_label  = 'Backgrounds combined'
    index_sig = np.where(target_array == sig_value)
    index_bg  = np.where(target_array == bg_value)

    for key in branches_dic.keys():
        if key == 'target':
            continue
        for list_val in branches_dic[key]:
            x_sig = np.array(evt_dic[key][index_sig][list_val].ravel())
            x_bg  = np.array(evt_dic[key][index_bg][list_val].ravel())
            print('::   Creating the feature plot for {} in {}.'.format(list_val, key))
            plot_feature(x_sig, x_bg, outpath, title=key, xlabel=list_val+post_fix, 
                                               sig_label=sig_label, bg_label=bg_label,
                                               combined_label=combined_label)




def plot_feature(x_sig, x_bg, out_path, **kwargs):
    """
    Plots the features comparing signal and background as histogram 
    """
    x_total = np.concatenate([x_sig, x_bg])
    n_unique = np.unique(x_total).shape[0]

    if n_unique < 40:
        nbins = n_unique*2
    else:
        nbins = 100
    
    plt.figure()

    title = kwargs.pop('title', None)
    xlabel = kwargs.pop('xlabel', 'x')
    sig_label = kwargs.pop('sig_label', 'signal')
    bg_label = kwargs.pop('bg_label', 'background')
    combined_label = kwargs.pop('combined_label', 'signal+backgr.')

    if kwargs:
        raise TypeError('Invalid kwargs passed: {}'.format(kwargs))

    # #### Kolmogorov-Smirnov test seems not to work at the moment ####
    # h_sig_norm = np.histogram(x_sig, nbins, density=True)[0]
    # h_bg_norm  = np.histogram(x_bg, nbins, density=True)[0] 

    # ks_p_val = stats.ks_2samp(h_sig_norm, h_bg_norm)[1]
    # plt.plot([], [], ' ', label='KS p-value: {:.3f}'.format(ks_p_val))

    n_total, bins_total, patches_total = \
        plt.hist(x_total,
                 bins=nbins,
                 range=(x_total.min(), x_total.max()),
                 alpha=.25,
                 color='black',
                 label=combined_label)
    
    n_trueNeg, bins_trueNeg, patches_trueNeg = \
        plt.hist(x_bg,
                 bins=nbins,
                 range=(x_total.min(), x_total.max()),
                 alpha=0.5,
                 color='#dd0000',
                 label=bg_label)
    
    n_truePos, bins_truePos, patches_truePos = \
        plt.hist(x_sig,
                 bins=nbins,
                 range=(x_total.min(), x_total.max()),
                 alpha=0.5,
                 color='green',
                 label=sig_label)

    
    plt.title(title, fontsize=18)
    # plt.xlim(-0.05, 1.05)
    plt.xlabel(xlabel, fontsize=17)
    plt.ylabel('Entries', fontsize=17)
    plt.legend(fontsize=14)
    plt.tight_layout()
    plt.savefig(out_path + 'feature_' + title + '_' + xlabel + '.png')
    plt.savefig(out_path + 'feature_' + title + '_' + xlabel + '.pdf')

    plt.yscale('log')
    plt.savefig(out_path + 'feature_' + title + '_' + xlabel + '_log.png')
    plt.savefig(out_path + 'feature_' + title + '_' + xlabel + '_log.pdf')
    
    return n_truePos, n_trueNeg


def plot_autoencoder_output(y_predictions, y_target, y_true, out_path, label=''):
    """
    Plots the distance to the expected output in a histogram
    """
    mse = np.mean(np.power(y_target - y_predictions, 2), axis=1)

    error_df = pd.DataFrame({'reconstruction_error': mse,
                             'true_class': y_true})

    x_sig = error_df['reconstruction_error'][error_df['true_class']== 1].as_matrix()
    x_bg  = error_df['reconstruction_error'][error_df['true_class']== 0].as_matrix()
    x_total = np.concatenate([x_sig, x_bg])

    n_unique = np.unique(x_total).shape[0]

    if n_unique < 30:
        nbins = n_unique*3
    else:
        nbins = 100
    
    plt.figure()

    n_total, bins_total, patches_total = \
        plt.hist(x_total,
                 bins=nbins,
                 range=(x_total.min(), x_total.max()),
                 alpha=.25,
                 color='black',
                 label='signal+backgr.')
    
    n_trueNeg, bins_trueNeg, patches_trueNeg = \
        plt.hist(x_bg,
                 bins=nbins,
                 range=(x_total.min(), x_total.max()),
                 alpha=0.5,
                 color='#dd0000',
                 label='background')
    
    n_truePos, bins_truePos, patches_truePos = \
        plt.hist(x_sig,
                 bins=nbins,
                 range=(x_total.min(), x_total.max()),
                 alpha=0.5,
                 color='green',
                 label='signal')
    
    # plt.title('Put title here')
    # plt.xlim(-0.05, 1.05)
    plt.xlabel('Reconstruction error ' + label, fontsize=18)
    plt.ylabel('Entries', fontsize=18)
    plt.legend(fontsize=15)
    plt.tight_layout()
    plt.savefig(out_path + 'reconstruction_error_' + label + '.png')
    plt.savefig(out_path + 'reconstruction_error_' + label + '.pdf')

    plt.yscale('log')
    plt.savefig(out_path + 'reconstruction_error_' + label + '_log.png')
    plt.savefig(out_path + 'reconstruction_error_' + label + '_log.pdf')    
    
    return error_df.true_class, error_df.reconstruction_error



def plot_MVAoutput(y_truth, y_score, out_path, label='', nbins=100):
    """
    Plots the MVA output as histogram and returns the underlying
    distributions of the positive and the negative class.
    """

    print('::   Creating MVA output plot...')
    
    y_score_truePos = y_score[np.array(y_truth==1)]
    y_score_trueNeg = y_score[np.array(y_truth==0)]
    
    plt.figure()

    n_total, bins_total, patches_total = \
        plt.hist(y_score,
                 bins=nbins,
                 alpha=.25,
                 color='black',
                 label='signal+backgr.')
    
    n_trueNeg, bins_trueNeg, patches_trueNeg = \
        plt.hist(y_score_trueNeg,
                 bins=nbins,
                 alpha=0.5,
                 color='#dd0000',
                 label='background')
    
    n_truePos, bins_truePos, patches_truePos = \
        plt.hist(y_score_truePos,
                 bins=nbins,
                 alpha=0.5,
                 color='green',
                 label='signal')
    
    #plt.title('MVA output distribution (positive class)')
    plt.xlim(-0.05, 1.05)
    plt.xlabel('MVA output', fontsize=18)
    plt.ylabel('Entries', fontsize=18)
    plt.legend(fontsize=15)
    plt.tight_layout()
    plt.savefig(out_path + 'MVAoutput_distr_' + label + '.png')
    plt.savefig(out_path + 'MVAoutput_distr_' + label + '.pdf')

    plt.yscale('log')
    plt.savefig(out_path + 'MVAoutput_distr_' + label + '_log.png')
    plt.savefig(out_path + 'MVAoutput_distr_' + label + '_log.pdf')    
    
    return n_truePos, n_trueNeg


def plot_cut_efficiencies(num_signal, num_background, out_path):
    """
    Calculates signal and background efficiencies as well as significance for each value
    of MVA output and determines the optimal MVA cut as the maximum of the significance
    distribution. Returns the optimized MVA cut value.
    """

    nbins = num_signal.shape[0]
    
    print('Creating cut efficiencies plot...')

    plt.figure()

    MVAcut = np.empty((0))

    fig, ax1 = plt.subplots()
    signal_efficiency = np.empty((0))
    backgr_efficiency = np.empty((0))

    for i in range(nbins):
        signal_efficiency = np.append(signal_efficiency, \
                                      np.sum(num_signal[i:num_signal.shape[0]]) / \
                                      (np.sum(num_signal)*1.0))
        backgr_efficiency = np.append(backgr_efficiency, \
                                      np.sum(num_background[i:num_background.shape[0]]) / \
                                      (np.sum(num_background)*1.0))
        MVAcut = np.append(MVAcut, i/(nbins*1.0))

    l1 = ax1.plot(MVAcut, signal_efficiency, label='signal efficiency', color='green')
    l2 = ax1.plot(MVAcut, backgr_efficiency, label='background efficiency', color='red')
    ax1.set_xlabel('MVA cut', fontsize=18)
    ax1.set_ylabel('Efficiency', fontsize=18)

    ax2 = ax1.twinx()
    significance_per_MVAcut = np.empty((0))

    for i in range(nbins):
        significance_per_MVAcut = np.append(significance_per_MVAcut, \
                                            np.sum(num_signal[i:num_signal.shape[0]]) / \
                                            math.sqrt(np.sum(num_signal[i:num_signal.shape[0]] + \
                                                             num_background[i:num_background.shape[0]])))

    l3 = ax2.plot(MVAcut, significance_per_MVAcut, label='significance', color='blue', alpha=.65)

    pos_max = np.argmax(significance_per_MVAcut)
    MVAcut_opt = pos_max/(nbins*1.0)
    print('Optimal MVA cut value determined as %.2f' % MVAcut_opt)

    l4 = ax2.plot(MVAcut_opt, significance_per_MVAcut[pos_max],
                  label='max. significance for cut at %.2f' % MVAcut_opt,
                  marker='o', markersize=10, fillstyle='none', mew=2, linestyle='none',
                  color='#0000aa', alpha=.8)
    ax2.set_ylabel('Significance', color='blue', fontsize=18)
    ax2.tick_params('y', colors='blue')

    #plt.title('MVA cut efficiencies')

    lall = l1+l2+l3+l4
    labels = [l.get_label() for l in lall]
    ax2.legend(lall, labels, fontsize=15)

    plt.tight_layout()

    plt.savefig(out_path + 'significance_vs_MVAcut.png')
    plt.savefig(out_path + 'significance_vs_MVAcut.pdf')

    return MVAcut_opt


def plot_ROCcurve(y_truth, y_score, out_path, sample_weight=None, label='', workingpoint=-1, pos_label=1):
    """
    Plots the ROC curve and (if specified) the chosen working point.
    """

    print('Creating ROC curve plot...')

    if sample_weight is None:
        print('  No sample weights are used for ROC calculation...')
        sample_weight = np.ones(y_truth.shape[0])
    
    fpr, tpr, thresholds = roc_curve(y_truth, y_score, sample_weight=sample_weight,
                                     pos_label=pos_label)
    roc_auc = roc_auc_score(y_truth, y_score, sample_weight=sample_weight)
    print('ROC AUC: %.3f' % roc_auc)
    
    plt.figure()
    plt.plot(fpr, tpr, label='ROC curve (AUC = %0.3f)' % roc_auc)
    plt.plot([0, 1], [0, 1], 'k--')
    plt.xlim([-0.05, 1.05])
    plt.ylim([-0.05, 1.05])
    plt.xlabel('False Positive Rate', fontsize=18)
    plt.ylabel('True Positive Rate', fontsize=18)
    plt.title('Receiver operating characteristic curve')
    
    if workingpoint != -1:
        # find and plot threshold closest to the chosen working point
        close_MVAcut_opt = np.argmin(np.abs(thresholds-workingpoint))
    
        plt.plot(fpr[close_MVAcut_opt], tpr[close_MVAcut_opt], 'o', markersize=10,
                 label="threshold at %.2f" % workingpoint, fillstyle="none",
                 mew=2)
    
    plt.legend(loc=4, fontsize=15)
    plt.savefig(out_path + 'roc_curve_' + label + '.png')
    plt.savefig(out_path + 'roc_curve_' + label + '.pdf')

    return

    
def plot_precision_recall_curve(y_truth, y_score, out_path, sample_weight=None, label='', workingpoint=-1):
    """
    Plots the precision-recall curve.
    """

    print('Creating precision-recall curve plot...')

    # if not defined, do not use sample weights
    if(sample_weight==None):
        sample_weight = np.ones(y_truth.shape[0])
    
    precision = dict()
    recall = dict()
    average_precision = dict()
    
    precision, recall, thresholds_PRC = \
        precision_recall_curve(y_truth,
                               y_score,
                               sample_weight=sample_weight)
    
    average_precision = average_precision_score(y_truth, y_score, sample_weight=sample_weight)
    
    plt.figure()
    plt.plot(recall, precision, lw=2,
             label='Precision-recall curve of signal class (area = {1:0.2f})'
                    ''.format(1, average_precision))
    
    if workingpoint != -1:
        # find threshold closest to the chosen working point
        close_optimum = np.argmin(np.abs(thresholds_PRC-workingpoint))
        
        plt.plot(recall[close_optimum], precision[close_optimum],
                 'o',
                 markersize=10,
                 label="threshold at %.2f" % workingpoint,
                 fillstyle="none",
                 mew=2)
    
    plt.xlim([-0.05, 1.05])
    plt.ylim([-0.05, 1.05])
    plt.xlabel('Recall', fontsize=18)
    plt.ylabel('Precision', fontsize=18)
    #plt.title('Precision-Recall Curve')
    plt.legend(loc="lower right", fontsize=15)
    plt.savefig(out_path + 'precision_recall_' + label + '.png')
    plt.savefig(out_path + 'precision_recall_' + label + '.pdf')


def plot_confusion_matrix(cm, classes,
                          out_path,
                          normalize=False,
                          title='Confusion matrix',
                          cmap=plt.cm.Blues,
                          label=''):
    """
    Prints and plots the confusion matrix.
    """
    
    print('Generating confusion matrix...')

    np.set_printoptions(precision=2)

    plt.figure()
    plt.imshow(cm, interpolation='nearest', cmap=cmap)
    plt.title(title)
    plt.colorbar()
    tick_marks = np.arange(len(classes))
    plt.xticks(tick_marks, classes, rotation=45)
    plt.yticks(tick_marks, classes, rotation=45)

    if normalize:
        cm = cm.astype('float') / cm.sum(axis=1)[:, np.newaxis]
        print("Normalized confusion matrix")
    else:
        print('Confusion matrix, without normalization')

    print(cm)

    thresh = cm.max() / 2.
    for i, j in itertools.product(range(cm.shape[0]), range(cm.shape[1])):
        plt.text(j, i, cm[i, j],
                 horizontalalignment="center",
                 color="white" if cm[i, j] > thresh else "black")

    plt.tight_layout()
    plt.ylabel('True label')
    plt.xlabel('Predicted label')
    plt.tight_layout()
    
    if normalize:
        outfile_name = out_path + 'confusion_matrix_normalized_' + label
        plt.savefig(outfile_name + '.png')
        plt.savefig(outfile_name + '.pdf')
    else:
        outfile_name = out_path + 'confusion_matrix_' + label
        plt.savefig(outfile_name + '.png')
        plt.savefig(outfile_name + '.pdf')

        
def plot_metrics_history(hist, out_path):
    """
    Plots the learning curves for all compiled metrics.
    """

    print('Create plots for metrics history...')
    
    plt.figure()
    plt.xlabel('epochs')
    
    for metric in hist.history:
        plt.plot(hist.history[metric], label=metric)

    plt.legend()
    plt.savefig(out_path + 'metrics_history.png')
    plt.savefig(out_path + 'metrics_history.pdf')
