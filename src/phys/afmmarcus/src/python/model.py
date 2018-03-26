#!/usr/bin/env python
# encoding: utf-8

'''
Collection of different hopping models for calculating hopping rates
'''

__author__      = 'Jake Retallick'
__copyright__   = 'MIT License'
__version__     = '1.2'
__date__        = '2018-02-14'  # last update

import numpy as np

# Virtual base class for hopping models

class BaseModel(object):
    '''Virtual base class for all tunneling rate models'''

    # shared physical constants
    hbar    = 6.582e-16     # eV.s

    def __init__(self):
        '''Initialise a hopping rate model'''
        pass

    def setup(self, X, Y, kt):
        '''Setup the model for the given DB arrangement

        inputs:
            X  : matrix of x positions, angstroms
            Y  : matrix of y positions, angstroms
            kt : thermal energy of surface, eV
        '''
        raise NotImplementedError()

    def rates(self, dG, occ, nocc):
        '''Compute the hopping rates from the energy deltas

        inputs:
            dG      : matrix of energy deltas with dG[i,j] the change for a
                      hop from site occ[i] to site nocc[j].
            occ     : list of occupied sites, possible sources
            nocc    : list of unoccupied sites, possible targets
        '''
        raise NotImplementedError()

    def cohopping_rate(self, dG, i,j,k,l):
        '''Compute the cohopping rate from sites i,j to sites k,l with the given
        energy delta

        inputs:
            dG  : energy delta associated with the cohopping event
            i,j : electron source
            k,l : db targets
        '''
        raise NotImplementedError()

# derived models

class VRHModel(BaseModel):
    '''Variable-Range Hopping model for hopping rates'''

    # model-specific parameters
    alph    = 1e-2     # inverse attenuation length, 1/angstroms
    r0      = 1.e11   # scaling prefactor for rates
    lamb    = 0.01      # self-trapping energy, eV

    def __init__(self):
        super(VRHModel, self).__init__()

    def setup(self, X, Y, kt):
        self.beta = 1./kt
        dX, dY = X-X.reshape(-1,1), Y-Y.reshape(-1,1)
        R = np.sqrt(dX**2+dY**2)
        self.T0 = self.r0*np.exp(-2*self.alph*R)

    # TODO: problem with exp overflow here, decreases in energy beyond lamb
    #       cause essentially instantaneous hops
    def rates(self, dG, occ, nocc):
        return self.T0[occ,:][:,nocc]*np.exp(-self.beta*(dG+self.lamb))

    def cohopping_rate(self, dG, i, j, k, l):
        pass


class MarcusModel(BaseModel):
    '''Marcus Theory model for hopping rates'''

    # model-specific parameters
    lamb    = 0.04      # reorganization energy, eV

    # transfer integral parameters
    t0      = 1e-3      # prefactor
    alph    = 1e-2      # inverse attenuation length, 1/angstroms

    # cohopping parameters
    cohop_lamb = lamb   # cohopping reorganization energy, eV
    cohop_alph = 1e-2   # cohopping inverse attenuation length, 1/angstroms


    def __init__(self):
        super(MarcusModel, self).__init__()

    # inherited methods

    def setup(self, X, Y, kt):
        self.lbeta = 1./(self.lamb*kt)
        dX, dY = X-X.reshape(-1,1), Y-Y.reshape(-1,1)
        self.R = np.sqrt(dX**2+dY**2)
        self.Tp = np.abs(self.tint(self.R))**2*np.sqrt(self.lbeta*np.pi)/self.hbar

    def rates(self, dG, occ, nocc):
        return self.Tp[occ,:][:,nocc]*np.exp(-.25*self.lbeta*(dG+self.lamb)**2)

    def cohopping_rate(self, dG, i, j, k, l):
        return self.cohop_tint(i,j,k,l)*np.exp(-.25*self.lbeta*(dG+self.cohop_lamb)**2)


    # specific methods

    def cohop_tint(self, i,j,k,l):
        return np.sqrt(self.Tp[i,k]*self.Tp[j,l]+self.Tp[i,l]*self.Tp[j,k])*np.exp(-self.cohop_alph*self.R[i,j])

    def tint(self, R):
        tij = self.t0*np.exp(-self.alph*R)
        return tij

models = {  'marcus':   MarcusModel,
            'VRH':      VRHModel }
