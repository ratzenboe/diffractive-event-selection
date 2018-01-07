#!/usr/bin/python

import math as mt
import numpy as np
import matplotlib
import matplotlib.pyplot as plt

# at some point make this a class, but now
# we only want a simple function that return the 4 momentum |p| of a particle
# if we plug in eta, phi, pt and the pdg value
# class FourVector(object):
#     def __init__(self,x0,x1,x2,x3):
#         self._x0 = x0
#         self._x1 = x1
#         self._x2 = x2
#         self._x3 = x3
                                                
#     def __add__(lhs,rhs):
#         return FourVector(*[sum(x) for x in zip(lhs.components(),rhs.components())])

def get_p_e( eta, phi, pt, pdg ):
    """
    calculate |p| and e from eta, phi, pt and pdg value
    """

    mp = 0.938272               # MeV
    me = 0.000510998            # MeV
    mpi = 0.13957018            # MeV
    mka = 0.493677              # MeV

    if abs(pdg) == 211:
        m0 = mpi
    elif abs(pdg) == 321:
        m0 = mka
    elif abs(pdg) == 2212:
        m0 = mp
    elif abs(pdg) == 11:
        m0 = me
    else:
        m0 = 0.
    # else:
    #     return 0., 0.

    pt = mt.fabs(pt)
    x = pt * mt.cos(phi)
    y = pt * mt.sin(phi)
    z = pt * mt.sinh(eta)

    e = mt.sqrt( x**2 + y**2 + z**2 + m0**2 )

    # if mt.fabs(eta) > 0.9:
    #     return mt.sqrt( x**2 + y**2 + z**2 ), -1.
    # else:
    return mt.sqrt( x**2 + y**2 + z**2 ), e

# def get_p_arr( data_pd ):
#     "read relevent data from panda dataframe and return array with p"""


