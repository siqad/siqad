#!/usr/bin/env python
# encoding: utf-8

'''
Channels for charges hopping on and off the surface
'''

__author__      = 'Jake Retallick'
__copyright__   = 'MIT License'
__version__     = '1.2'
__date__        = '2018-02-16'  # last update

import numpy as np
from scipy.special import expit
from itertools import product


class Channel(object):
    '''Virtual base class for all channels of charge on and off the surface'''

    name = None         # channel name for search
    scale = 1.0         # influence scale, attenuates energy/bias contributions

    # machine precision
    MTR     = 1e-16     # minimum tickrate, avoid zero division

    # flags
    active = False      # set True if the channel induces a bias that is state
                        # dependent
    enabled = True      # include channel in simulation

    sdflag = True       # set True if the channel is a charge source/drain

    # useful lambdas
    rebirth = np.random.exponential # reset for hop-on lifetime

    def __init__(self):
        '''Initialise a Channel'''
        pass

    def setup(self, X, Y, kt):
        '''Set up channel for the given DB positions and thermal energy'''
        self.kt = kt
        self.X, self.Y = X, Y
        self.lifetime = self.rebirth() if self.sdflag else np.inf
        self.tickrate = 1.

    def toggle(self):
        '''Toggle whether the channel is included in the simulation'''
        self.enabled = not self.enabled

    def setEnabled(self, enable):
        self.enabled = bool(enable)

    def tick(self):
        '''Time until channel state changes'''
        return np.inf

    def peek(self):
        '''Time until next charge hops onto the surface'''
        if self.sdflag and self.enabled:
            return self.lifetime/(self.tickrate+self.MTR)
        else:
            return np.inf

    def run(self, dt):
        '''Advance the channel lifetime by the given amount'''
        if self.sdflag and self.enabled:
            self.lifetime -= dt*self.tickrate

    def pop(self):
        '''Pop a charge off the channel. The HoppingModel should update its
        charge state and immediately pass new occ/nocc back'''

        P = self.offrates
        ind = np.random.choice(range(len(P)), p=P/P.sum())
        self.lifetime = self.rebirth()
        return ind

    def rates(self):
        '''Get the hopping rates onto the channel for the given occupied DBs'''
        return self.onrates if self.enabled else np.zeros(len(self.occ))

    def biases(self, occ):
        '''Get the induced local potentials for each DB as a result of the
        presence of the channel for the given occupation state'''
        return np.zeros(len(self.X))

    def computeDelta(self, n, m):
        '''Compute the energy delta for a single hop between an occupied db, n,
        and an unoccupied db, m.'''
        if not self.active:
            return 0
        occ = self.occ.tolist()
        occ[occ.index(n)]=m
        return self.biases(self.occ)[n]-self.biases(occ)[m]

    def computeDeltas(self):
        '''Compute the matrix of energy deltas for single hopping events'''
        if not self.active:
            return 0
        occ, nocc = self.occ, self.nocc
        dG = np.zeros([len(occ), len(nocc)], dtype=float)

        bias0 = self.biases(occ)[occ]
        def dE(n, m):
            '''Change in energy for the charge at occ[n] moving to nocc[m]'''
            occ[n], temp = nocc[m], occ[n]
            delta = bias0[n] - self.biases(occ)[nocc[m]]
            occ[n] = temp
            return delta

        for n, m in product(range(len(occ)), range(len(nocc))):
            dG[n,m] = dE(n,m)
        return dG

    def update(self, occ, nocc, beff):
        '''Tell the channel which DBs are occupied and what the local levels
        of each DB are.

        inputs:
            occ     : list of occupied DBs
            nocc    : list of unoccupied DBs
            beff    : array of local levels of each DB
        '''
        raise NotImplementedError()

class Bulk(Channel):
    '''Passive charge transfer between the surface and the bulk'''

    name = 'bulk'

    # prefactors
    nu = 1e3    # maximum rate of hops between bulk and surface
    mu = .25     # local energy at which electrons start to hop
    lamb = 0.   # self-trapping energy

    alpha   = 1.e0  # damping factor for kt, higher means sharper transition

    # lambdas
    sigmoid = lambda self, x: expit(-self.alpha*x/self.kt)

    # inherited methods

    def update(self, occ, nocc, beff):
        self.occ, self.nocc = occ, nocc
        self.offrates = self._compute_offrates(beff)
        self.onrates = self._compute_onrates(beff)
        self.tickrate = self.offrates.sum()

    # channel specific methods

    def _compute_onrates(self, beff):
        '''Compute the hopping rates from occupied DBs to the Bulk'''
        return self.nu*self.sigmoid(beff[self.occ]+self.mu+self.lamb)

    def _compute_offrates(self, beff):
        '''Compute the hopping rates from the Bulk onto unoccupied DBs'''
        return self.nu*self.sigmoid(-(beff[self.nocc]+self.mu))

channels = {'bulk': Bulk}
