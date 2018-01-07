#!/usr/bin/env python

import os
import operator
import pickle
import pandas as pd
import numpy as np
import sys
import mca
from sklearn.ensemble import ExtraTreesClassifier
from sklearn.externals import joblib
from sklearn.preprocessing import StandardScaler

from modules.utils import get_output_paths

# some ideas which may come of value later on
# Nom_list = [x for x in df.columns if df[x].dtype == np.dtype('O') and \
#                                       sum(df[x].isnull()) < 0.5*df.shape[0]]  
# nan_Cols = [x for x in df.columns if sum(df[x].isnull()) > 0.5*df.shape[0]]
# df[nan_Cols] = df[nan_Cols].fillna(0)
# Drop_list = [x for x in nan_Cols if df[x].nunique() >  4]
# df.AGE = np.array(pd.cut(np.array(df['AGE']), bins=5, labels=range(1,6)))

def getTrueIntegerCols(df):
    """
    if a integer column has some NaN values inside the column automatically is percieved as
    a float dtype. But we can iterate trough the column and check each individual element.
    If this gives us only ints and NaNs then we have a true integer column
    NaNs get converted into -9999 which can later be interpreted as a NaN
    """
    intColList = [x for x in df.columns if df[x].dtype == np.dtype('int')]
    for col in df.columns:
        if col in intColList:
            continue
        count = 0
        for x in df[col]:
            # we inccrement the count if we have a nan or an integer
            if np.isnan(x):
                count += 1
            if x.is_integer():
                count += 1
        # after looping through column we check if the lenght 
        # and number of ints+nans are the same
        if count == len(df[col]):
            # we have to fill the NaN values with a int in order to convert
            # the whole column into integer form
            df[col] = df[col].fillna(-9999) 
            df[col] = df[col].astype(int)
            intColList.append(col)

    return df, intColList


def parseObjectColums(df, run_params, keepNANs=False):
    """
    Parse all object-columns in df to integer 
    """
    output_prefix = get_output_paths(run_params)[0]
    outfile = output_prefix+'parser_mapping.pkl'
    print 'Parse all object type columns...'
    if run_params['predict']:
        # if the unknown test sample has values that have previously not parsed
        # and saved in parser_mapping then we have to look for a new mapping (next step)
        if not os.path.exists(outfile):
            print'No objects columns to parse'
            return df
        with open(outfile, 'r') as openfile:
            openZipList = pickle.load(openfile)
        for x, dic in openZipList:
            df[x] = df[x].astype('category')
            df[x] = pd.DataFrame(map(dic.get, df[x].tolist(), df[x].tolist() ))
    else:
        fix_list = [x for x in df.columns if df[x].dtype == np.dtype('O')]
        if not fix_list:
           print'No object columns to parse'
           return df
        print fix_list
        # replaces each NaN with -1
        dicList = []
        for x in fix_list:
            df[x] = df[x].astype('category')
            # we have to save the mapping to the new values
            # and redo it the same way with the unlabled data
            # dictionary is as such: {0:'a', 1:'c',...}
            # therefore we have to reverse key and item
            dic = dict(enumerate(df[x].cat.categories))
            rev_dic = { v:k for k,v in dic.iteritems() }
            dicList.append(rev_dic)
            df[x] = df[x].cat.codes
        with open(outfile, 'w') as f:
            zipList = zip(fix_list, dicList)
            pickle.dump(zipList, f)

    if keepNANs:
        df[fix_list][df[fix_list]==-1] = np.nan;
    return df


def fillNumerical(df):
    '''
    Fill missing NUMERICAL values using average
    '''
    #Fill average data in numerical column
    print 'Filling in using averages...'
    NumericalData = [x for x in df.columns if df[x].dtype == np.dtype('float') and \
                                              sum(df[x].isnull())>0]
    for col in NumericalData:
        df[col] = df[col].astype(float)
        Mean = np.mean(df[col])
        df[col] = df[col].fillna(Mean)

    return df
 
def learn_vals(df,label,features):
    '''
    learn_vals(df,label,features)
    where:
    df = data frame
    label = column to learn
    features = columns to learn from
    '''
    #Check that all columns are in data frame
    for item in features:
        if item not in list(df.columns):
            features.remove(item)

    if label not in list(df.columns) or sum(df[label].isnull())==0:
        return df
  
    print'Filling in {} values...'.format(label)
    for x in df[label]:
        if x == 'P':
            print x
  
    et = ExtraTreesClassifier(n_estimators=100, 
                              max_depth=None, 
                              min_samples_split=2, 
                              random_state=0,
                              verbose=True,
                              n_jobs=-1)
 
    # the labels ought not contain NaNs!
    labels_train = df[label][df[label].isnull() == 0].values

    features_train = df[features][df[label].isnull() == 0].values
    features_test  = df[features][df[label].isnull()].values

    et.fit(features_train,labels_train)
    labels_test = pd.Series(et.predict(features_test),index = df.index[df[label].isnull()])
    NewCol = pd.concat([ df[label][df[label].isnull()==0] , labels_test] )
  
    df = df.combine_first( pd.DataFrame({label:NewCol}) )

    return df


def fillCategorical(df, run_params):
    """
    Machine learning to fill categorical values in:
    """
    print 'Machine learining to Fill in nominal values...'  
    Labels_list = []
    # before we fill in the nominal values we want to make sure that all categorical values
    # are parsed to integer values
    df = parseObjectColums(df, run_params, keepNANs=True)
    df = getTrueIntegerCols(df)
    Nom_list = [x for x in df.columns if df[x].dtype == np.dtype('int')]
    # now we could transform all -9999 values back
    df[df==-9999] = np.nan

    for item in Nom_list:
        if item in df.columns:
            if sum(df[item].isnull()) > 0:
                Labels_list.append(item)
  
    for item in Labels_list:
        feature_list = [x for x in df.columns if x not in Labels_list]
        df = learn_vals(df,item,feature_list)
    # fix the type of the column to int as we may perform MCA on it later
    df[Nom_list] = df[Nom_list].astype(int)

    return df

    
def containsNAN(df):
    """
    checks if column contrains NAN and prints it out
    """
    count = 0
    for col in df.columns:
        if sum(df[col].isnull()) > 0:
            print 'NaN values remaining in column',col
            count += 1
    return count


def rmCorrelated(df, run_params):
    """
    Check for correlation between columns, and remove duplicate information
    """
    print 'Dropping highly correlated columns (>{})'.format(correlation)
    output_prefix = get_output_paths(run_params)[0]
    # when predicting we drop columns from a file that contains previously dropped cols
    drop_columns_from_file = run_params['predict']
    if not drop_columns_from_file:
        Drop_list = []
        Corr_Mat = df.corr()
        cols = list(Corr_Mat.columns)
        for n in range(Corr_Mat.shape[0]):
            for m in range(n+1,Corr_Mat.shape[0]):
                if abs(Corr_Mat.loc[cols[n],cols[m]]) > run_params['correlation']:
                    Drop_list.append(cols[m])

        with open(output_prefix+'correlation_cols.txt', 'w') as f:
            for item in Drop_list:
                f.write("%s\n" % item)

        f.close()
    else:
        with open(output_prefix+'correlation_cols.txt', 'r') as f:
            Drop_list = [line.rstrip('\n') for line in f]
        print 'drop list read from file...'

    print'number of highly correlated columns: {}'.format(len(list(set(Drop_list))))
    df.drop(Drop_list,axis = 1,inplace = True)
 
    return df


def rmLowVariance(df, run_params):
    """
    dropping colmns that have a too small variance
    """
    print'Dropping columns with nearly no variance < {}%...'.format(100*variance)
    output_prefix = get_output_paths(run_params)[0]
    # when predicting we drop columns from a file that contains previously dropped cols
    drop_columns_from_file = run_params['predict']
    if not drop_columns_from_file:
        Drop_list = []
        dat_var = df.var()
        Drop_list.extend(dat_var.index[dat_var < run_params['variance']])

        print'number of low variance columns: {}'.format(len(list(set(Drop_list))))
        df.drop(Drop_list,axis = 1,inplace = True)

        print'saving dropped columns...'
        with open(output_prefix+'variance_cols.txt', 'w') as f:
            for item in Drop_list:
                f.write("%s\n" % item)

        f.close()
    else:
        with open(output_prefix+'variance_cols.txt', 'r') as f:
            Drop_list = [line.rstrip('\n') for line in f]
        print 'drop list read from file...'

    print'number of highly correlated columns: {}'.format(len(list(set(Drop_list))))
    df.drop(Drop_list,axis = 1,inplace = True)
 
    return df
   

def doMCA(df):
    """
    perform MCA on integer columns
    """
    int_cols = [iCol for iCol in df.columns if df[iCol].dtype is np.dtype('int')]

    print'converting these {} integer columns:\n{}'.format(len(int_cols),int_cols)
    mca_out = mca.MCA(df[int_cols])
    mca_ben = pd.DataFrame(mca_out.fs_r(1))    
    print'mca-shape: {}'.format(mca_ben.shape)

    df.drop(int_cols, axis=1, inplace=True)
    print'df-shape after int drop: {}'.format(df.shape)

    df = pd.concat([df, mca_ben], axis=1, ignore_index=True)
    print'output shape: {}'.format(df.shape)

    return df


def standardScale(df, 
                  run_params, 
                  load_fitted_attributes=False,
                  load_att_from_filename=None):
    """
    standard scaling to 0-mean and std-derivation of 1
    returns pandas dataframe
    """
    output_pfx, model_pfx = get_output_paths(run_params)

    scaler_attributes_filename = output_pfx + model_pfx + 'StandardScaler_attributes.pkl'
    if load_att_from_filename != None:
        print'load from {}'.format(load_att_from_filename)
        scaler_attributes_filename = load_att_from_filename

    if load_fitted_attributes:
        print('Loading previously determined scaler attributes...')
        scaler_attributes = joblib.load(scaler_attributes_filename)
        scaler_mean = scaler_attributes[0,:]
        scaler_scale = scaler_attributes[1,:]
        df.as_matrix()
        df.sub(pd.DataFrame(scaler_mean), axis=0)
        df.divide(pd.DataFrame(scaler_scale), axis=0)

    else:
        scaler = StandardScaler()
        df = scaler.fit_transform(df)
        Xfeats_mean = scaler.mean_
        Xfeats_scale = scaler.scale_
        Xfeats_var = scaler.var_

        joblib.dump(np.array([Xfeats_mean, Xfeats_scale, Xfeats_var], dtype=np.float32), 
                    scaler_attributes_filename)

    return df


