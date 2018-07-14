#!/usr/bin/env python
# encoding: utf-8

'''
Channel for time evolved clocking fields
'''

__author__      = 'Jake Retallick'
__copyright__   = 'MIT License'
__version__     = '1.2'
__date__        = '2018-04-09'  # last update

import numpy as np
from channel import Channel


class Clock(Channel):
    ''' '''

    name = 'clock'
    scale = 1.0

    active = False  # clocking fields are surface independent
    sdflag = False  # no hopping to/from clocking electrodes

    length = 2e3        # clocking signal spacing period, angstroms
    freq = 1e-1         # clock frequency, Hz

    flat = False

    # default waveform parameters
    wf_A    = .1        # waveform ampitude, eV
    wf_0    = 0.        # waveform offset, eV

    # time stepping
    dp = 1e-2           # fraction of clocking period between samples

    def __init__(self, fname=None):
        '''Initialise clock'''
        super(Clock, self).__init__()

        self.t = 0.         # internal clock time
        self.fname = fname  # placeholder for externally defined clocking fields

    # inherited methods

    def setup(self, X, Y, kt):
        super(Clock, self).setup(X, Y, kt)

        self.fgen = self._prepareFields()
        self.bias = self.fgen(self.t)

    def tick(self):
        ''' '''
        return self.dp/self.freq

    def run(self, dt):
        '''Advance the clocking fields by the given amount'''
        if self.enabled:
            super(Clock, self).run(dt)
            self.t += dt
            self.bias = self.fgen(self.t)

    def rates(self):
        return np.zeros(len(self.occ), dtype=float)

    def biases(self, occ):
        return self.bias if self.enabled else np.zeros(len(self.occ))

    def update(self, occ, nocc, beff):
        self.occ, self.nocc = occ, nocc

    # internal methods

    def _prepareFields(self):
        '''Precompute generator for time dependent fields'''

        if self.fname is None:
            return lambda t: self.waveform(self.X, t)

        raise NotImplementedError('External field generation not implemented')

    def waveform(self, x, t):
        '''Travelling wave approximation of clocking fields'''
        xx = x/self.length if not self.flat else 0
        phase = 2*np.pi*(x/self.length - self.freq*t)
        return self.wf_0 + self.wf_A*self._sinus(phase)

    def _sinus(self, x):
        '''periodic function bounded by -1 and 1 with a period of 2*pi'''
        b = 0
        return np.sqrt((1+b*b)/(1+(b*np.sin(x))**2))*np.sin(x)
