#!/usr/bin/env python
# encoding: utf-8

'''
Collection of different hopping models for calculating hopping rates
'''

__author__      = 'Jake Retallick'
__copyright__   = 'MIT License'
__version__     = '1.2'
__date__        = '2018-04-05'  # last update

import numpy as np

# Virtual base class for hopping models

class BaseModel(object):
    '''Virtual base class for all tunneling rate models'''

    # shared physical constants

    hop_alph   = 10.    # hopping attenuation length, angstroms
    cohop_alph = 100.   # cohopping attenuation length, angstroms

    # calibration values
    nu0     = .02   # calibration pair hopping frequency, Hz
    r0      = 19.2  # calibration pair distance, angstroms
    lamb    = 0.04  # reorganization energy, eV

    dlamb   = 0.00  # offset for lamb to account for tip
    fact    = 1.0   # additional prefactor for hopping

    expmax = 1e2

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
        self.beta = 1./kt
        dX, dY = X-X.reshape(-1,1), Y-Y.reshape(-1,1)
        self.R = np.sqrt(dX**2+dY**2)
        self._spatial_rates()

    def rate(self, dg, n, m):
        '''Compute the hopping rate from the energy delta, source, and target'''
        return self._exp(self.T0[n,m] + self._energy_rate(dg+self.dlamb))

    def rates(self, dG, occ, nocc):
        '''Compute the hopping rates from the energy deltas

        inputs:
            dG      : matrix of energy deltas with dG[i,j] the change for a
                      hop from site occ[i] to site nocc[j].
            occ     : list of occupied sites, possible sources
            nocc    : list of unoccupied sites, possible targets
        '''
        return self._exp(self.T0[occ,:][:,nocc] + self._energy_rate(dG+self.dlamb))

    def cohopping_rate(self, dG, ij, kl):
        '''Compute the cohopping rate from sites i,j to sites k,l with the given
        energy delta

        inputs:
            dG  : energy delta associated with the cohopping event
            i,j : electron source
            k,l : db targets
        '''
        (i,j), (k,l) = ij, kl
        return self._exp(self._ch_spatial_rate(i,j,k,l) + self._energy_rate(dG+self.dlamb))

    def setAttenuation(self, alph):
        self.alph = alph
        self._spatial_rates()

    def setPrefactor(self, fact):
        self.fact = fact
        self._spatial_rates()

    def setLambda(self, lamb):
        self.lamb = lamb

    # internal methods

    # main function to reimplement in derived classes
    def _energy_rate(self, dG):
        return 0.

    def _spatial_rates(self):
        '''compute the spatial decay components'''
        self.T0 = np.log(self.fact*self.nu0)-2*(self.R-self.r0)/self.hop_alph

    def _ch_spatial_rate(self, i,j,k,l):

        T = self.T0
        A = .5*(-np.log(2)+np.log(np.exp(T[i,k]+T[j,l])+np.exp(T[i,l]+T[j,k])))
        return A - 2*self.R[i,j]/self.cohop_alph

    def _exp(self, arg):
        '''Thresholded version of exp'''
        mask = arg <= self.expmax
        arg = arg*mask + self.expmax*(1-mask)
        return np.exp(arg)



# derived models

class VRHModel(BaseModel):
    '''Variable-Range Hopping model for hopping rates'''

    # model-specific parameters

    def __init__(self):
        super(VRHModel, self).__init__()

    def setup(self, X, Y, kt):
        super(VRHModel, self).setup(X, Y, kt)

    def _energy_rate(self, dG):
        return -dG*self.beta



class MarcusModel(BaseModel):
    '''Marcus Theory model for hopping rates'''

    def __init__(self):
        super(MarcusModel, self).__init__()

    # inherited methods

    def setup(self, X, Y, kt):
        super(MarcusModel, self).setup(X, Y, kt)
        self.setLambda(self.lamb)

    def setLambda(self, lamb):
        super(MarcusModel,self).setLambda(lamb)
        self.lbeta = np.inf if lamb == 0 else .25*self.beta/lamb

    def _energy_rate(self, dG):
        return -dG*(dG+2*self.lamb)*self.lbeta


models = {  'marcus':   MarcusModel,
            'VRH':      VRHModel }
