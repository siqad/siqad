#!/usr/bin/env python
# encoding: utf-8

'''
Simulator for the non-equilibrium surface dynamics of charges in QSi's DB
arrangements
'''

from __future__ import print_function

__author__      = 'Jake Retallick'
__copyright__   = 'MIT License'
__version__     = '1.2'
__date__        = '2018-04-10'  # last update

import numpy as np
from scipy.special import erf


from model import models as Models
from channel import Channel, channels as Channels
from tip_model import TipModel
from clocking import Clock

from itertools import combinations, chain, product
from collections import defaultdict

import sys, os

from timeit import default_timer as timer

# add non-standard channels
Channels['tip'] = TipModel
Channels['clock'] = Clock

class HoppingModel:
    '''Time dependent surface hopping model for charge transfer in DBs'''

    # machine precision
    MTR     = 1e-16         # for tickrates division
    verbose = False

    # energy parameters
    debye   = 50.           # debye screening length, angstroms
    erfdb   = 5.            # erf based screening length
    eps0    = 8.854e-12     # F/m
    q0      = 1.602e-19     # C
    kb      = 8.617e-05     # eV/K
    T       = 4.0           # system temperature, K
    epsr    = 6.35          # relative permittivity

    Kc = 1e10*q0/(4*np.pi*epsr*eps0)    # Coulomb strength, eV.angstrom

    # lattice parameters
    a = 3.84    # lattice vector in x, angstroms    (intra dimer row)
    b = 7.68    # lattice vector in y, angstroms    (inter dimer row)
    c = 2.25    # dimer pair separation, angstroms

    # general settings
    fixed_pop = False    # fixed number of electrons
    free_rho = 0.5       # filling density if not fixed_pop (Nel = round(N*free_rho))
    burn_count = 0      # number of burns hops per db

    enable_cohop = True     # enable cohopping
    enable_FRH = True       # enable finite range hopping

    hop_range = 20      # maximum range for hopping, angstroms
    cohop_range = 20    # coherance range for cohopping pairs, angstroms

    # useful lambdas
    rebirth = np.random.exponential     # reset for hopping lifetimes

    debye_factor = lambda self, R: np.exp(-R/self.debye)
    debye_factor = lambda self, R: erf(R/self.erfdb)*np.exp(-R/self.debye)

    coulomb = lambda self, R: (self.Kc/R)*self.debye_factor(R)

    def __init__(self, pos, model='marcus', **kwargs):
        '''Construct a HoppingModel for a DB arrangement with the given x and
        optional y coordinates in unit of the lattice vectors. For now, assume
        only the top site of each dimer pair can be a DB.

        inputs:
            pos     : Iterable of DB locations. Each elements of pos should be a
                      3-tuple (x,y,b) with x and y the dimer column and row and
                      b true if the DB is at the bottom of the dimer pair. If
                      pos[i] is an integer x, it gets mapped to (x,0,0).
            model   : Type of hopping rate model

        optional key-val arguments:

        '''

        # format and store db locations and number
        self._parseX(pos)

        self.charge = np.zeros([self.N,], dtype=int)    # charges at each db

        self.bias = np.zeros([self.N,])         # bias energy at each site
        self.dbias = np.zeros([self.N,])        # temporary additional bias

        # distance matrix
        dX = self.a*(self.X-self.X.reshape(-1,1))
        dY = self.b*(self.Y-self.Y.reshape(-1,1))
        self.R = np.sqrt(dX**2+dY**2)

        # prepare FRH parameters
        self._prepareFRH()

        # electrostatic couplings
        self.V = self.coulomb(np.eye(self.N)+self.R)
        np.fill_diagonal(self.V,0)

        # by default, use
        self.Nel = int(round(self.N*self.free_rho))

        self.vprint = print if self.verbose else lambda *a,**k: None

        # create model, setup on initialisation
        if model not in Models:
            raise KeyError('Invalid model type. Choose from [{0}]'.format(
                                ', '.join(Models.keys())))
        self.model = Models[model]()
        self.channels = []
        self.initialised = False

        # logging parameters
        self.fplog, self.logging = None, False

    def cleanup(self):
        print('Closing Hopping Model')
        if self.fplog is not None:
            self.fplog.close()

    def fixElectronCount(self, n):
        '''Fix the number of electrons in the system. Use n<0 to re-enable
        automatic population mechanism.'''

        self.vprint('setting electron count to {0}'.format(n))
        if n<0:
            self.Nel = int(round(self.N*self.free_rho))
            self.fixed_pop = False
        else:
            assert n <= self.N, 'Invalid number of electrons'
            self.Nel = n
            self.fixed_pop = True

        if self.initialised:
            self.charge[self.state[:self.Nel]]=1
            self.charge[self.state[self.Nel:]]=0
            self.update()

    def addBiasGradient(self, F, v=[1,0]):
        '''Apply a bias gradient of strength F (eV/angstrom) along the
        direction v'''

        self.bias += F*(v[0]*self.a*self.X+v[1]*self.b*self.Y)


    def getChannel(self, name):
        '''Get a handle for the Channel with the given name. If it doesn't
        exists, return None'''

        for channel in self.channels:
            if channel.name == name:
                return channel
        return None

    def updateFRHPars(self, hop=None, cohop=None):
        self.hop_range = self.hop_range if hop is None else hop
        self.cohop_range = self.cohop_range if cohop is None else cohop
        self._prepareFRH()
        if self.enable_cohop:
            self.ch_lifetimes = {ij: self.rebirth() for ij in self.ch_targets}
        self.update()



    def addChannel(self, channel, enable=True):
        '''Add a Channel instance to the HoppingModel.

        inputs:
            channel : Channel to include. Must either be a Channel instance or
                     a string indicating an accepted channel type in Channels.
            enable  : Enable the channel immediately
        return:
            handle for the added Channel
        '''

        if isinstance(channel, str):
            if channel not in Channels:
                raise KeyError('Invalid channel type. Choose from [{0}]'.format(
                    ', '.join(k for k in Channels if k != 'base')))
            self.channels.append(Channels[channel]())
        elif isinstance(channel, Channel):
            self.channels.append(channel)
        else:
            raise KeyError('Unrecognized Channel format: must be either a str \
                                or Channel derived class')

        self.channels[-1].setEnabled(enable)
        return self.channels[-1]


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
            assert Nel == self.Nel, 'Electron count mismatch'
            assert self.N == len(occ+nocc), 'DB count mismatch'
            self.state = np.array(occ+nocc)

        self.charge[self.state[:self.Nel]]=1
        self.charge[self.state[self.Nel:]]=0

        # setup model and channels
        X, Y, kt  = self.a*self.X, self.b*self.Y, self.kb*self.T
        self.model.setup(X, Y, kt)
        for channel in self.channels:
            channel.setup(X, Y, kt)

        # lifetimes[i] is the lifetime of an electron at the i^th DB
        self.lifetimes = np.array([self.rebirth() for _ in range(self.N)])

        # ch_lifetimes_lt[ij] is the lifetime of the electrons at DBs i,j
        if self.enable_cohop:
            self.ch_lifetimes = {ij: self.rebirth() for ij in self.ch_targets}

        self.update()
        self.energy = self.computeEnergy()
        self.initialised = True

        if charges is None:
            # burn off random initial state
            self.burn(self.burn_count*self.N)

        self.clock = 0.    # time between events for logging


    def computeBeff(self, occ, wch=False):
        '''Compute the effective bias at each site for the given occupation.
        Optional include channel contributions'''

        beff = self.bias - np.sum(self.V[:,occ], axis=1)
        if wch:
            beff += sum(ch.scale*ch.biases(occ) for ch in self.channels if ch.enabled)
        return beff


    def update(self):
        '''Update the energy deltas, tunneling rates, and tick rates'''

        occ, nocc = self.state[:self.Nel], self.state[self.Nel:]

        #t = tick()

        # TODO: include channel contributions to beff
        # effective bias at each location
        beff = self.bias-np.dot(self.V, self.charge)

        # add channel biases to beff
        beff += sum(ch.scale*ch.biases(occ) for ch in self.channels if ch.enabled)
        for channel in self.channels:
            channel.update(occ, nocc, beff)

        self.beff = beff

        # energy deltas associated with single charge hops
        self.dG = defaultdict(dict)
        for ind, src in enumerate(occ):
            tbeff = self.computeBeff(np.delete(occ, ind))
            for trg in self.h_targets[src]:
                if not self.charge[trg]:
                    self.dG[src][trg] = tbeff[src]-tbeff[trg]
                    self.dG[src][trg] += sum(ch.scale*ch.computeDelta(src, trg)
                                for ch in self.channels if ch.enabled)

        # single charge hopping rates
        self.trates = defaultdict(dict)
        for src, D in self.dG.items():
            for trg, dg in D.items():
                self.trates[src][trg] = self.model.rate(dg, src, trg)

        # tickrates for each occupying charge
        self.tickrates = np.zeros(self.Nel, dtype=float)
        for i, src in enumerate(occ):
            if src in self.trates:
                self.tickrates[i] = sum(self.trates[src].values())

        if self.channels and not self.fixed_pop:
            self.crates = np.array([ch.rates() for ch in self.channels]).T
            self.tickrates += np.sum(self.crates, axis=1)

        # cohopping
        if self.enable_cohop:

            self.ch_trates = defaultdict(dict)  # hopping rate for each cohop
            self.ch_dG = defaultdict(dict)      # energy delta for each cohop

            for s1 in self.trates:              # src with available targets
                for s2 in self.ch_pairs[s1]:    # cohopping partner: s2>s1
                    if not s2 in self.trates: continue  # partner has a target
                    for t1, t2 in self.ch_targets[(s1,s2)]:
                        if self.charge[t1] or self.charge[t2]: continue
                        dg = self._compute_cohop_dG(s1,s2,t1,t2)
                        ij, kl = (s1,s2), (t1,t2)
                        self.ch_dG[ij][kl] = dg
                        self.ch_trates[ij][kl] = \
                                self.model.cohopping_rate(dg, ij, kl)
            # tickrates for each cohopping
            self.ch_tickrates = {}
            for ij, D in self.ch_trates.items():
                self.ch_tickrates[ij] = sum(D.values())

    def peek(self):
        '''Return the time before the next tunneling event and information
        about theevent. Does not account for time evolution of channel bias
        contributions: i.e. all rates assumed fixed

        output:
            dt  : time to next hopping event, s
            T   : type of event:
            ind : index of event
        '''

        occ = self.state[:self.Nel]

        times = []  # list of ((T,ind),dt) generators/iterables

        # hopping events
        h_times = ([('hop',occ[i]),t] for i,t in enumerate(
                    self.lifetimes[occ]/(self.tickrates+self.MTR)))
        times.append(h_times)

        # optional cohopping
        if self.enable_cohop:
            ch_tmap = lambda ij: self.ch_lifetimes[ij]/(self.ch_tickrates[ij]+self.MTR)
            ch_times = {('chop',ij): ch_tmap(ij) for ij in self.ch_tickrates}
            times.append(ch_times.items())

        # optional channel hopping
        if self.channels and not self.fixed_pop:
            chan_times = ([('chan', i),ch.peek()] for i,ch in enumerate(
                            self.channels) if ch.enabled)
            times.append(chan_times)

        (T, ind), dt = min(chain(*times), key=lambda x:x[1])

        return dt, T, ind

    def burn(self, nhops, per=False):
        '''Burns through the given number of hopping events. If per is True,
        performs nhops*self.N hops'''

        # supress printing for burn
        self.vprint, tprint = lambda *a, **k: None, self.vprint

        if per:
            nhops *= self.N

        for n in range(nhops):
            sys.stdout.write("\rBurning: {0:3.1f}%".format((n+1)*100./nhops))
            sys.stdout.flush()
            self.run(self.peek()[0])
        print('\n')

        self.vprint = tprint


    def step(self, dt=np.inf):
        '''Advance the HoppingModel by the smaller of dt or its internal
        time step.

        returns:
            tick    : time advancement
        '''

        #import pdb; pdb.set_trace()

        # figure out the time step
        dt_hop, T, ind = self.peek()   # hopping events
        dt_ch = min(ch.tick() for ch in self.channels if ch.enabled)

        # advance lifetimes and channel states
        tick = max(min(dt, dt_hop, dt_ch), self.MTR)
        self._advance(tick)

        # handle hops
        if dt_hop <= min(dt, dt_ch)+self.MTR:
            self._hop_handler(T, ind)
        self.update()

        return tick


    def run(self, dt):
        '''Run the inherent dynamics for the given number of seconds.'''

        while dt>0:
            dt -= self.step(dt=dt)

    def measure(self, ind, dt=0.):
        '''Make a measurement of the indicated db after the given number of
        seconds.'''

        # keep hopping until the measurement event
        self.run(dt)

        # return the charge state of the requested db
        return self.charge[ind]


    def getLifetime(self, db):
        '''Get the expected lifetime, in seconds, of the given DB'''
        try:
            ind = self._index(db)
        except:
            print('Invalid DB index')
            return None

        if ind < self.Nel:
            return self.lifetimes[db]/(self.MTR+self.tickrates[ind])
        return 0.


    def computeEnergy(self, occ=None):
        '''Direct energy computation for the current charge configurations'''

        inds = self.state[:self.Nel] if occ is None else occ
        beff = self.bias - .5*np.sum(self.V[:,inds], axis=1)
        beff += sum(ch.scale*ch.biases(inds) for ch in self.channels if ch.enabled)
        return -np.sum(beff[inds])


    def addCharge(self, x, y, pos=True):
        '''Add the potential contribution from a charge at location (x,y). If pos
        is False, removes the influence of that charge'''

        dX, dY = self.a*self.X - x, self.b*self.Y - y
        R = np.sqrt(dX**2+dY**2)
        V = self.coulomb(R)
        self.bias -= V if pos else -V
        self.update()


    def getLevels(self, src):
        '''Get the effective relative level shifts for the occupied src DB to
        possible targets'''

        if src not in self.dG:
            return {}

        beff = self.beff[src]
        return {trg: beff - dg for trg, dg in self.dG[src].items()}

    def startLog(self, fname):
        '''Open/Construct the log file and begin logging'''

        self.endLog()   # close any existing log, logging = False

        direc = os.path.dirname(fname)
        if not os.path.isdir(direc):
            print('Creating directory: {0}'.format(direc))
            os.makedirs(direc)

        try:
            self.fplog = open(fname, 'w')
            self.logging = True
        except:
            print('Failed to open log file: {0}'.format(fname))
            self.fplog = None

    def endLog(self):
        '''Close the log file and stop logging'''

        if self.fplog is not None:
            print('Closing previous log...')
            self.fplog.close()
        self.logging = False



    # internal methods

    def _index(self, db):
        '''Get the index of the db in self.state'''
        return int(np.where(self.state==db)[0])


    def _parseX(self, X):
        '''Parse the DB location information'''

        f = self.c/self.b   # dimer pair relative separation factor

        X, Y, B = zip(*map(lambda x: (x,0,0) if isinstance(x,int) else x, X))
        self.X = np.array(X).reshape([-1,])
        self.Y = np.array([y+f*bool(b) for y,b in zip(Y,B)]).reshape([-1,])
        self.N = len(self.X)


    def _prepareFRH(self):
        '''Prepare necessaty parameters for finite range hopping'''

        # identify possible hopping pairs
        self.h_targets = {i: set() for i in range(self.N)}
        for i,j in zip(*np.where(self.R < self.hop_range)):
            if i==j: continue
            self.h_targets[i].add(j)

        # identify cohopping pairs
        if self.enable_cohop:
            self.ch_targets, self.ch_pairs = defaultdict(set), defaultdict(set)
            for i,j in zip(*np.where(self.R < self.cohop_range)):
                if i>=j: continue
                for k,l in product(self.h_targets[i], self.h_targets[j]):
                    if k==j or l==i or k==l: continue
                    self.ch_targets[(i,j)].add((k,l) if k<l else (l,k))
                if (i,j) in self.ch_targets:
                    self.ch_pairs[i].add(j)


    def _advance(self, dt):
        '''Advance the lifetimes and channels by the given time'''

        # lifetime updates
        self.lifetimes[self.state[:self.Nel]] -= dt*self.tickrates
        if self.channels:
            for channel in self.channels:
                if channel.enabled:
                    channel.run(dt)
        if self.enable_cohop:
            for ij, tickrate in self.ch_tickrates.items():
                self.ch_lifetimes[ij] -= dt*tickrate

        self.clock += dt


    def _hop_handler(self, T, ind):
        '''Handle all the possible hopping cases.'''

        self.vprint('Hopping type: {0} :: {1}'.format(T, ind))

        # could be made more concise with a static function map
        if T == 'hop':
            self._hop(ind)
        elif T == 'chop':
            self._cohop(ind)
        elif T == 'chan':
            self._channel_pop(ind)
        else:
            raise KeyError('Unrecognized hopping type')
        self.energy = self.computeEnergy()
        if self.logging and self.fplog is not None:
            self._logState()

        self.clock = 0.

    def _cohop(self, ij):
        '''Perform a cohop with the given pair i,j'''

        # determine targets
        targets, P = zip(*self.ch_trates[ij].items())
        ind = np.random.choice(range(len(targets)), p=np.array(P)/sum(P))
        (i,j), (k,l) = ij, targets[ind]

        # modify charge state
        self._surface_hop(i,k)
        self._surface_hop(j,l)

    def _hop(self, src):
        '''Perform a hop from the given DB'''

        # determine target
        if src in self.trates:
            targets, P = zip(*self.trates[src].items())
        else:
            targets, P = [], []
        targets, P = list(targets), np.array(P)

        ind = self._index(src)
        if self.channels and not self.fixed_pop:
            P = np.hstack([P, [np.sum(self.crates[ind])]])
            targets.append(-1)
        trg = np.random.choice(targets,p=P/P.sum())

        self.vprint('Hopping db from src {0} to trg {1}'.format(src, trg))

        # modify the charge state
        if trg < 0:
            self._channel_hop(src)
        else:
            self._surface_hop(src, trg)

    def _channel_hop(self, src):
        '''Hop the electron given off the surface to some channel'''

        ind = self._index(src)
        self.charge[src] = 0
        self.state[ind], self.state[self.Nel-1] = self.state[self.Nel-1], self.state[ind]

        self.Nel -= 1

        if self.enable_cohop:
            self._ch_rebirth(src)


    def _channel_pop(self, cind):
        '''Hop an electron from the give channel onto the surface'''

        ind = self.channels[cind].pop()+self.Nel
        target = self.state[ind]
        self.charge[target] = 1
        self.lifetimes[target] = self.rebirth()
        self.state[self.Nel], self.state[ind] = self.state[ind], self.state[self.Nel]

        self.Nel += 1

    def _surface_hop(self, src, trg):
        '''Hop a charge from the source db to the given target'''

        s_ind = self._index(src)
        t_ind = self._index(trg)
        self.charge[src], self.charge[trg] = 0, 1
        self.state[s_ind], self.state[t_ind] = trg, src
        self.lifetimes[trg] = self.rebirth()

        # reset all cohopping times involving src
        if self.enable_cohop:
            self._ch_rebirth(src)

    def _ch_rebirth(self, db):
        '''reset tall the cohopping lifetimes related to the given DB'''

        # n < db
        for n in range(db):
            if db in self.ch_pairs[n]:
                self.ch_lifetimes[(n,db)] = self.rebirth()

        # db < n
        for n in self.ch_pairs[db]:
                self.ch_lifetimes[(db,n)] = self.rebirth()


    def _compute_cohop_dG(self, i, j, k, l):

        if k not in self.dG[i] or l not in self.dG[j]:
            k,l = l,k

        return self.dG[i][k]+self.dG[j][l] \
            + (self.V[k,l]+self.V[i,j]) \
            - (self.V[i,l]+self.V[j,k])

    def _logState(self):
        '''Log the time since the last event and the current charge state'''
        state = int(''.join(str(c) for c in self.charge), base=2)
        hx = format(state, '0{0}x'.format(1+(self.N-1)//4))
        s = '{0:.4e} :: '.format(self.clock) + '0x{0}\n'.format(hx)
        self.fplog.write(s)


    def _setupLogging(self, fname):

        try:
            self.fplog = open(fname, 'w')
        except:
            print('Failed to open log file: {0}'.format(fname))
            self.fplog = None
