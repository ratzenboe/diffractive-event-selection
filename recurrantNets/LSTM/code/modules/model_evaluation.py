import sys
import os
import time

import matplotlib
matplotlib.use('agg')
import matplotlib.pyplot as plt
import seaborn as sns
sns.set()

import numpy as np
import pandas as pd

from sklearn.metrics                import mean_squared_error, mean_absolute_error, \
                                           explained_variance_score, r2_score
from sklearn.externals              import joblib

def model_evaluation(X, y_true, model, label='test'):

    print'\n{} Evaluating the model on the {} sample... {}'.format(10*'-', label, 10*'-')
    start_time_eval_train = time.time()
    
    # for multiclassification we dont plot a roc-curve, just the ROC-AUC measure
    print('Predicting class labels...')
    y_score = model.predict(X)
    print'\nExplained variance score (best 1, worst 0):  {:.4f}'.format(
            explained_variance_score(y_true, y_score))
    print'Mean absolute error:  {:.4f}'.format(
            mean_absolute_error(y_true, y_score))
    print'Mean squared error:  {:.4f}'.format(
            mean_squared_error(y_true, y_score))    
    print'r^2 score (best 1, can get negative=arbitraryliy bad):  {:.4f}'.format(
            r2_score(y_true, y_score))    
     
    del y_score

    end_time_eval_train = time.time()

    print'\nEvaluation time {} sample: {} seconds'.format(label, 
                                                         end_time_eval_train-start_time_eval_train)
