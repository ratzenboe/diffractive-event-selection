import sys
import numpy as np

def load_grid(run_params, classifier_params):

    model_name = run_params['classifier_params_id']
    if 'GBR' in model_name:
        param_grid = {
            "loss": ("ls","lad","huber", "quantile"),
            "learning_rate": (0.1,0.2,0.3),
            "n_estimators": (300, 600, 1000),
            "max_depth": (3,6,15,50,None),
            "min_samples_split": (2,3,5),
            "criterion": ("friedman_mse", ),
            "min_weight_fraction_leaf": (0.0,),
            "min_impurity_decrease": (0.0,),
            "init": (None,),
            "max_features": (None,),
            "max_leaf_nodes": (None,),
            "warm_start": (False,),
            "presort": ("auto",),
            "alpha": (0.9,0.99)
        }

    elif 'SVC' in model_name:
        param_grid = {
            'C':          np.linspace(0., 10., 5),
            'epsilon':    (0.1,),
            'kernel':     ('rbf','linear', 'poly', 'sigmoid'),
            'degree':     (3,),
            'gamma':      ('auto',),
            'coef0':      np.linspace(-10.0,10.0,11),
            'shrinking':  (True,),
            'tol':        (1e-3,),
            'max_iter':   (-1,)
        }

    elif 'Bayes' in model_name:
        param_grid = {
            n_iter:         (300, 500),
            tol:            (1.e-3,),
            alpha_1:        (1.e-6,),
            alpha_2:        (1.e-6,),
            lambda_1:       (1.e-6,),
            lambda_2:       (1.e-6,),
            compute_score:  (False,),
            fit_intercept:  (True,),
            normalize:      (False,True),
            copy_X:         (True,)
        }

    elif model_name == 'KNR':
        param_grid = {
            'n_neighbors': (5,7,9,13),
            'weights': ('uniform','distance'),
            #'algorithm': 'auto',
            #'leaf_size': 30,
            'p': (2,3,4,5),
            'metric': ('minkowski', 'chebyshev'),
            #'metric_params': None,
        }

    else:
        print('Error: No gridsearch parameters '\
              'specified for model {}.'.format(model_name)
        )
        sys.exit(1)

        
    return param_grid
