#!/usr/bin/env python
# encoding: utf-8

'''
Simulator for the non-equilibrium surface dynamics of charges in QSi's DB
arrangements
'''

__author__      = 'Jake Retallick'
__copyright__   = 'MIT License'
__version__     = '1.2'
__date__        = '2018-02-13'  # last updates

raise DeprecationWarning('Use the more generalized HoppingModel in hopper.py')

import numpy as np

class MarcusModel:
    '''Time dependent Marcus Theory based model for charge transfer in surface
    DBs'''

    # machine precision
    MTR     = 1e-16         # for tickrates division

    # physical constants
    hbar    = 6.582e-16     # eV.s
    eps0    = 8.854e-12     # F/m
    q0      = 1.602e-19     # C
    kb      = 8.617e-05     # eV/K
    T       = 4.5           # K
    epsr    = 11.7          # relative permittivity

    # somewhat magic numbers
    debye   = 50            # debye screening length, angstroms
    lamb    = 0.04          # reorganization energy, eV
    t0      = 1e-2          # staying integral, eV
    t1      = 5e-4          # tunneling integral at lattice vector a, eV
    tpow    = 1             # tunneling integral distance fall-off power

    # lattice parameters
    a = 3.84    # lattice vector in x, angstroms    (intra dimer row)
    b = 7.68    # lattice vector in y, angstroms    (inter dimer row)

    # general settings
    fixed_pop = True        # fixed number of electrons
    fixed_rho = 0.5         # filling density if fixed_pop (Nel = round(N*fixed_rho))
    burn_time = 500         # initial burn time, seconds

    # useful constants
    lkt = lamb*kb*T                     # lambda * thermal energy, eV^2
    Kc  = 1e10*q0/(4*np.pi*epsr*eps0)   # Coulomb strength, eV.angstrom

    # useful lambda functions
    rebirth = np.random.exponential     # reset for tunneling lifetime

    def __init__(self, X, Y=None, **kwarg):
        '''Construct a MarcusModel for a DB arrangement with the given x and
        optional y coordinates in units of the lattice vectors. For now, assumes
        only the top site of each dimer pair can be a db.'''

        # format and store db locations and number
        self.X = np.array(X).reshape([-1,])
        self.N = len(self.X)
        self.Y = np.zeros(self.N) if Y is None else np.array(Y).reshape([-1,])

        assert self.X.shape == self.Y.shape, 'X,Y shape mismatch'

        self.charge = np.zeros([self.N,], dtype=int)    # charges at each db

        self.bias = np.zeros([self.N,])         # bias energy at each site
        self.dbias = np.zeros([self.N,])        # temporary additional bias

        # distance matrix
        dX = self.a*(self.X-self.X.reshape(-1,1))
        dY = self.b*(self.Y-self.Y.reshape(-1,1))
        self.R = np.sqrt(dX**2+dY**2)

        # prefactors for the tunneling rates
        self.Tp = np.abs(self.tint())**2/self.hbar*np.sqrt(np.pi/self.lkt)
        self.Tp0 = self.Tp[0,0]*np.exp(-self.lamb**2/(4*self.lkt))

        # electrostatic couplings
        self.V = self.Kc/(np.eye(self.N)+self.R)*np.exp(-self.R/self.debye)
        np.fill_diagonal(self.V,0)

        # by default, use
        self.Nel = int(round(self.N*self.fixed_rho))


    def setElectronCount(self, n):
        '''Set the number of electrons in the system'''

        if n<0:
            self.fixed_pop = False
        else:
            assert n <= self.N, 'Invalid number of electrons'
            self.Nel = n
            self.fixed_pop = True


    def writeBias(self, bias, ind, dt, sigma):
        '''Influence of the tip on system when over the given db. Applies a square
        wave potential bias to the indicated db of length sigma seconds and
        ending at dt'''

        self.run(max(0,dt-sigma))
        self.dbias[ind] = bias
        self.update()
        self.run(sigma)
        self.dbias[ind]=0
        self.update()


    def setBiasGradient(self, F, v=[1,0]):
        '''Apply a bias gradient of strength F (eV/angstrom) along the
        direction v'''

        self.bias = F*(v[0]*self.a*self.X+v[1]*self.b*self.Y)

    # FUNCTIONAL METHODS

    def initialise(self, charges=None):
        '''Initialise all necessary system parameters'''

        # surface state described by a single array with the first Nel values
        # occupied sites and the remaining N-Nel unoccupied

        if charges is None: # random
            self.state = np.random.permutation(range(self.N))
        else:
            occ, nocc = [], []
            for i,c in enumerate(charges):
                (occ if c==1 else nocc).append(i)
            Nel = len(occ)
            assert Nel==self.Nel, 'Electron count mismatch'
            self.state = np.array(occ+nocc)

        self.charge[self.state[:self.Nel]]=1
        self.charge[self.state[self.Nel:]]=0

        self.update()

        self.lifetimes = [self.rebirth() for _ in range(self.Nel)]
        self.energy = self.computeEnergy()

        if charges is None:
            # burn off random initial state
            self.run(self.burn_time)

    def update(self):
        '''Update the energy deltas, tunneling rates, and tick rates'''

        occ, nocc = self.state[:self.Nel], self.state[self.Nel:]

        # effective bias at each location
        beff = self.bias+self.dbias-np.dot(self.V, self.charge)

        # update parameters.... magic math
        self.dG = beff[occ].reshape(-1,1) - beff[nocc] - self.V[occ,:][:,nocc]

        self.trates = self.Tp[occ,:][:,nocc]*np.exp(-(self.dG+self.lamb)**2/(4*self.lkt))
        self.tickrates = np.sum(self.trates, axis=1)

    def peek(self):
        '''Return the time before the next tunneling event and the index of the
        event'''

        tdeltas = self.lifetimes/(self.tickrates+self.MTR)
        ind = np.argmin(tdeltas)

        return tdeltas[ind], ind

    def hop(self, ind):
        '''Perform a hop with the given electron'''

        src = self.state[ind]

        # determine target
        P = np.copy(self.trates[ind])
        t_ind = np.random.choice(range(self.Nel, self.N),p=P/P.sum())
        target = self.state[t_ind]

        # update
        self.energy += self.dG[ind, t_ind-self.Nel]
        self.charge[src], self.charge[target] = 0, 1
        self.state[ind], self.state[t_ind] = target, src
        self.lifetimes[ind] = self.rebirth()
        self.update()

    def run(self, dt):
        '''Run the inherent dynamics for the given number of seconds'''

        while dt>0:
            tick, ind = self.peek()
            mtick = min(dt, tick)
            self.lifetimes -= mtick*self.tickrates
            if tick<=dt:
                self.hop(ind)
            dt -= mtick


    def measure(self, ind, dt):
        '''Make a measurement of the indicated db after the given number of
        seconds.'''

        # keep hopping until the measurement event
        self.run(dt)

        # return the charge state of the requested db
        return self.charge[ind]

    def computeEnergy(self):
        '''Direct energy computation for the current charge configurations'''

        inds = self.state[:self.Nel]
        return -np.sum((self.bias+self.dbias)[inds]) + .5*np.sum(self.V[inds,:][:,inds])

    def energyDelta(self, ind, tind):
        '''Manually compute the energy delta'''

        E0 = self.computeEnergy()

        self.state[ind], self.state[tind] = self.state[tind],self.state[ind]
        E1 = self.computeEnergy()
        self.state[ind], self.state[tind] = self.state[tind],self.state[ind]

        return E1-E0


    # helper functions

    def tint(self):
        '''Compute the tunneling integrals for the distance matrix'''

        tij = self.t1/np.power(np.eye(self.N)+self.R/self.a, self.tpow)
        np.fill_diagonal(tij, self.t0)
        return tij


if __name__ == '__main__':

    X = [1, 5, 7, 10, 12, 16]

    model = MarcusModel(X)
    model.setElectronCount(4)
    model.initialise()
