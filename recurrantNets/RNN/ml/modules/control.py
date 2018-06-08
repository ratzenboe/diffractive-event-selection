import sys
import os
import ast
import argparse

try:
    import configparser
except ImportError:
    import ConfigParser as configparser


class fileparser(configparser.ConfigParser):
    """
    Extends the functionality of the Python configparser.
    """

    def as_dict(self):
        """
        Returns the entire config file as a dictionary.
        """

        d = dict(self._sections)
        for k in d:
            d[k] = dict(**d[k])
            d[k].pop('__name__', None)
            for j in d[k].keys():
                d[k][j] = ast.literal_eval(d[k][j])
        
        return d


def config_file_to_dict(filename):
    """
    Imports an entire config file and returns it as a dictionary.
    """
    if not isinstance(filename, str):
        raise ValueError('Passed argument is not a string but instead {}'.format(type(filename)))
 
    if not os.path.isfile(filename):
        raise IOError('File {} does not exist.'.format(filename))
    
    parser = fileparser(configparser.ConfigParser())
    parser.optionxform=str # make key parsing case-sensitive
    parser.read(filename)

    return parser.as_dict()


def check_data_config_requirements(data_params):
    """
    Check if the data_params dictionary contains all the required keys.
    """

    if not isinstance(data_params, dict):
        raise ValueError('Passed argument is not a dictionary but instead {}'.format(type(data_params)))
    
    required_keys = ['path',
                     'max_entries',
                     'target',
                     'evt_id']

    missing_keys = []

    for k in required_keys:
        if k not in data_params.keys():
            missing_keys.append(k)

    if missing_keys:
        raise AttributeError('Required data parameter keys missing: {}'.format(
            missing_keys))

    else:
        return True
