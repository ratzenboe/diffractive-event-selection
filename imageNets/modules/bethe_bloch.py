#!/usr/bin/python

from math import *
import numpy as np
import matplotlib
import matplotlib.pyplot as plt

# we implement the bethe-bloch-sternheimer formula
#Define some natural constants

def ALEPH_bethe_bloch_ToF_beta(p, pdg, isInTPCTOF, par=None):
    """
    first output:
    calculate the bethe bloch approximation ALEPH with parameters 
    form the AliTRDdEdxBaseUtils class (url below)

    second output:
    the tof measurement provides a time measurement coupled with a length measurement of
    the passed volume -> get a velocity -> beta = v/c
    """
    # if isInTPCTOF < 0.:
    #     return 0., 0.

    # c = 299792458.              # m/s
    c = 1.                      # a.u.
    mp = 0.938272               # MeV
    me = 0.000510998               # MeV
    mpi = 0.13957018             # MeV
    mka = 0.493677               # MeV

    if abs(pdg) == 211:
        m0 = mpi
    elif abs(pdg) == 321:
        m0 = mka
    elif abs(pdg) == 2212:
        m0 = mp
    elif abs(pdg) == 11:
        m0 = me
    else:
        m0 = mpi
    # else:
    #     return 0., 0.
    
    beta = p/sqrt(c*c*m0*m0 + p*p)
    gamma = 1./sqrt(1-beta**2)
    # if pdgID is 211:
    #     print 'for p={}: beta={}, with gamma={}'.format(p, beta, gamma)
    
    if par is None:
        # pars for TPC, from:
        # http://portal.nersc.gov/project/alice/htmldoc/src/AliTRDdEdxBaseUtils.cxx.html#763

        p0 = 4.401269
        p1 = 9.725370
        p2 = 0.000178
        p3 = 1.904962
        p4 = 1.426576

        # p0 = 5.383
        # p1 = 8.8
        # p2 = 1.362
        # p3 = 2.274
        # p4 = 1.975

    else:
        p0 = abs(par[0]);
        p1 = abs(par[1]);
        p2 = abs(par[2]);
        p3 = abs(par[3]);
        p4 = abs(par[4]);

    aa = pow(beta, p3);
    bb = log( p2 + pow(1./(beta*gamma), p4) )

    return (p1-aa-bb)*p0/aa, beta

# the next funtion takes in an array of 
ALEPH_bethe_bloch_ToF_beta_vectorize = np.vectorize(ALEPH_bethe_bloch_ToF_beta)

def main():
    dEdx_p_arr = []
    dEdx_pion_arr = []
    dEdx_kaon_arr = []
    dEdx_e_arr = []

    tof_p = []
    tof_pion = []
    tof_kaon = []
    tof_e = []

    p_arr = []
    mult = 0.01
    c = 0.4
    for i in range(0,500):
        p_arr.append(i*mult+c)
        dEdx_p_arr.append(ALEPH_bethe_bloch_ToF_beta(i*mult+c, 2212)[0])
        dEdx_pion_arr.append(ALEPH_bethe_bloch_ToF_beta(i*mult+c, 211)[0])
        dEdx_kaon_arr.append(ALEPH_bethe_bloch_ToF_beta(i*mult+c, 321)[0])
        dEdx_e_arr.append(ALEPH_bethe_bloch_ToF_beta(i*mult+c, 11)[0])
        
        tof_p.append( ALEPH_bethe_bloch_ToF_beta(i*mult+c, 2212)[1] )
        tof_pion.append( ALEPH_bethe_bloch_ToF_beta(i*mult+c, 211)[1] )
        tof_kaon.append( ALEPH_bethe_bloch_ToF_beta(i*mult+c, 321)[1] )
        tof_e.append( ALEPH_bethe_bloch_ToF_beta(i*mult+c, 11)[1] )

    plt.figure(1)
    prot, = plt.plot( p_arr, dEdx_p_arr, 'g--', label='P' )
    kaon, = plt.plot( p_arr, dEdx_kaon_arr, 'r--', label='K' )
    pion, = plt.plot( p_arr, dEdx_pion_arr, 'b--', label='pi' )
    electr, = plt.plot( p_arr, dEdx_e_arr, 'y--', label='e-' )
    plt.legend(handles=[prot, kaon, pion, electr])
    plt.xscale('log')
    plt.xlabel('p (GeV)')
    plt.ylabel('dE/dx (au)')
    plt.grid(True)

    plt.figure(2)
    p_tof, = plt.plot( p_arr, tof_p, 'g--', label='P' )
    pion_tof, = plt.plot( p_arr, tof_pion, 'b--', label='pi' )
    k_tof, = plt.plot( p_arr, tof_kaon, 'r--', label='K' )
    e_tof, = plt.plot( p_arr, tof_e, 'y--', label='e-' )
    plt.xlabel('p (GeV)')
    plt.ylabel(r'$\beta$')
    plt.grid(True)

    plt.show()

    return

if __name__ == "__main__":
    main()


