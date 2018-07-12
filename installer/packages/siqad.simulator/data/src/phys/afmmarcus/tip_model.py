#!/usr/bin/env python
# encoding: utf-8


'''
Model for computing the influence of the AFM tip on the DBs
'''

__author__      = 'Jake Retallick, Lucian Livadaru'
__copyright__   = 'MIT License'
__version__     = '1.1'
__date__        = '2018-03-09'  # last update

import numpy as np
import scipy.constants as const
from scipy.interpolate import interp1d
from scipy.special import expit     # overflow safe sigmoid function
from scipy.signal import savgol_filter

import os.path

from channel import Channel

class TipModel(Channel):
    '''Tip Model'''

    name = 'tip'
    scale = 0.
    active = True

    # physics constants
    q0   = const.e
    Kc   = 1e9*const.e/(4*np.pi*const.epsilon_0)   # Coulomb strength, eV.[nm]
    epsr = 9.0      # dielectric constant for surface-tip

    dH = 0.038      # height delta between DB0 and DB-, lattice relaxation, nm

    # experimental fit parameters
    tipR1   = 5.    # tip radius for ICIBB, nm
    tipR2   = 50    # tip radius for TIBB, nm
    tipH    = .2    # tip-surface separation, nm
    tipDW   = 0.65  # actual difference in tip-sample work-functions, eV

    # calibration data
    data_dir = os.path.join(os.path.dirname(__file__), 'data')
    TIBBvH_fname = os.path.join(data_dir, 'TIBB_vs_H.dat')
    TIBBvR_fname = os.path.join(data_dir, 'TIBB_vs_R_d200pm.dat')
    tipR0   = 5.0   # tip radius in FEM calculations, nm
    tipH0   = 0.2   # tip height in FEM calculations, nm
    tipDW0  = 0.9   # tip-sample work-function delta in FEM calculations, eV
    tibbPars = [1.01, .5, 1.1]  # tip model fit parameters, [pw0, dcy0, pw1]

    # empirical parameters
    TR0 = 43.3e9            # lateral decay of the tunneling rate tip-DB, Hz
    TR0 = 1e-9
    k_tip_DBm = 6.28        # spatial decay of the tunneling rate, 1/nm
    E_DB_m_bulkCB = -0.32   # negative DB level in bulk wrt CBM, eV

    mu = .1         # chemical potential
    lamb = .1       # self-trapping energy
    alpha = 1.e0    # damping factor for kt, higher means sharper transition

    # lambdas
    sigmoid = lambda self, x: expit(-self.alpha*x/self.kt)

    # scan parameters
    rate = 10.0    # default scan rate in nm/s
    ds = 1e-1      # travel distance between time steps, nm

    def __init__(self):
        '''Initialise the model parametrization'''

        self._TIBBvH_calib()
        self._TIBBvR_calib()
        self._updateTIBB()

        # tip position and path
        self.tipX, self.tipY = 0., 0.
        self.path = []
        self.rx, self.ry = 0., 0.   # movement rate in x and y
        self.rt = 0.               # time until next target

    def setup(self, X, Y, kt):
        super(TipModel, self).setup(.1*X, .1*Y, kt)
        self.setPos(self.tipX, self.tipY)

    def tick(self):
        '''Time until the tip position/state changes'''
        return self.ds/self.rate if self.path and self.enabled else np.inf

    def run(self, dt):
        '''Advance the tip by the given amount of time'''
        super(TipModel, self).run(dt)
        while dt > 0 and self.path and self.enabled:
            # advance doesn't reach next target
            if dt<self.rt:
                self.setPos(self.tipX+dt*self.rx, self.tipY+dt*self.ry)
                self.rt -= dt
                dt = 0
            # advance at least to next target
            else:
                x,y = self.path.pop(0)
                self.setPos(x, y)
                dt -= self.rt
                if self.path:
                    self._update_scanrates(*self.path[0])
                if self.loop:
                    self.path.append((x,y))


    def biases(self, occ):
        return self.level_shifts(occ) if self.enabled else np.zeros(len(self.X))

    def update(self, occ, nocc, beff):
        self.occ, self.nocc = occ, nocc
        A = np.sqrt(self.R**2 + (self.tipR1+self.tipH)**2)
        self.baserates = self.TR0*np.exp(-self.k_tip_DBm*(A-self.tipR1))
        self.offrates = self._compute_offrates(beff)
        self.onrates = self._compute_onrates(beff)
        self.tickrate = self.offrates.sum()

    # channel specific methods

    def setScan(self, path, rate=None, loop=False):
        '''Define the scan path for the tip.

        inputs:
            path    : iterable of (x,y) points that the tip moves between, nms
            rate    : scan rate for the tip in nm/s
            loop    : if True, the tip will loop over the path. Otherwise, the
                      tip stops as the final point.
        '''

        # TODO: add path check here
        self.path = [(x,y) for x,y in path]
        if rate is not None:
            self.rate = float(rate)
        self.loop = bool(loop) if len(self.path)>1 else False
        self._update_scanrates(*self.path[0])

    def setTarget(self, x, y, rate=None):
        self.path = [(x,y)]
        if rate is not None:
            self.rate = float(rate)
        self.loop = False
        self._update_scanrates(*self.path[0])

    def setPos(self, x, y):
        '''Update the tip position, make the appropriate pre-calcs'''
        self.tipX, self.tipY = x, y
        self.dX, self.dY = self.X-x, self.Y-y
        self.R = np.sqrt(self.dX**2 + self.dY**2)   # DBs to tip apex
        self.TIBB = None

    def setRadius(self, tibb=None, icibb=None):
        '''Set the tip radius in nm, make appropriate pre-calcs'''
        print(tibb, icibb)
        self.tipR2 = self.tipR2 if tibb is None else tibb
        self.tipR1 = self.tipR1 if icibb is None else icibb
        self._updateTIBB()

    def setHeight(self, H):
        '''Update the tip height, make appropriate pre-calcs'''
        self.tipH = H
        self._updateTIBB()

    # from Lucian's model

    def level_shifts(self, occ):
        '''Calculate the tip induced level shifts for each DB when the given
        DBs are occupied'''

        # TODO: add a store of the energy shifts for given occs, reset on
        #       any change to the tip

        if self.TIBB is None:   # only recompute this if pos, tipR, tipH changed
            self.TIBB = self.tibb_fit(self.R)

        # image charge locations
        facts = self.tipR1**2/(self.R[occ]**2 + (self.tipR1+self.tipH)**2)
        posIC = [self.X[occ]-(1-facts)*self.dX[occ],
                 self.Y[occ]-(1-facts)*self.dY[occ],
                 (1-facts)*(self.tipH+self.tipR1)]

        # image charge induced band bending
        dX = self.X - posIC[0].reshape(-1,1)
        dY = self.Y - posIC[1].reshape(-1,1)
        dZ = self.dH*np.ones(self.X.shape, dtype=float)
        dZ[occ] = 0.
        R, Q = np.sqrt(dX**2+dY**2+dZ**2+posIC[2].reshape(-1,1)**2), np.sqrt(facts)
        ICIBB = self.Kc*np.dot(Q, 1/R)/self.epsr

        return -(self.TIBB-ICIBB)

    # internal methods

    def _compute_onrates(self, beff):
        '''Compute the hopping rates from occupied DBs to the tip'''
        return self.baserates[self.occ]*self.sigmoid(beff[self.occ]+self.mu+self.lamb)

    def _compute_offrates(self, beff):
        '''Compute the hopping rates from the tip to unoccupied DBs'''
        return self.baserates[self.nocc]*self.sigmoid(-(beff[self.nocc]+self.mu))

    def _update_scanrates(self, x, y):
        '''Update the x and y scan rates as well as the time until the next
        target'''

        nx, ny = x-self.tipX, y-self.tipY
        self.rt = np.sqrt(nx**2+ny**2)/self.rate
        if self.rt==0:
            self.rx, self.ry = 0., 0.
        else:
            self.rx, self.ry = nx/self.rt, ny/self.rt

    def _updateTIBB(self):
        '''update the TIBB interpolator for new tip radius or height'''

        R, H, R0, femR = self.tipR2, self.tipH, self.tipR0, self.femR
        p0, p1, p2 = self.tibbPars
        factR = ((R+H)/(R0+H))**p0*(1.-(R-R0)/(R+H)*np.exp(-(p1*femR/R)**p2))
        factH = self.TIBBvH_fit(H)/self.femTIBB[0]
        if R<200:
            TIBB = self.femTIBB*factR
        else:       # flat tip approx.
            TIBB = self.femTIBB[0]*np.ones(len(self.femTIBB))
        TIBB *= factH*self.tipDW/self.tipDW0

        self.tibb_fit = interp1d(femR, TIBB, kind='quadratic',
                                    fill_value='extrapolate')
        self.TIBB = None

    def _TIBBvH_calib(self):
        '''Calibrate the H dependence of the TIBB'''

        data = np.genfromtxt(self.TIBBvH_fname, delimiter=None)
        assert data.shape[1]==3, 'TIBBvH calibration data must be 3-column'
        Htip, TIBB, ICIBB = data.T
        coefs = np.polyfit(Htip, TIBB*1e-3, 3)
        self.TIBBvH_fit = lambda H: np.polyval(coefs, H)

    def _TIBBvR_calib(self):
        '''Calibrate the lateral distance, R, dependence of the TIBB for a
        specific height and tip radius'''

        data = np.genfromtxt(self.TIBBvR_fname, delimiter=None)
        assert data.shape[1]==2, 'TIBBvR calibration data must be 2-column'
        self.femR, self.femTIBB = data.T
        self.femTIBB = savgol_filter(self.femTIBB, 51, 2, mode='interp')


if __name__ == '__main__':

    da = .384

    DBsXlist = da*np.array([0, 7, 9, 14, 16, 23])
    DBsYlist = [0, 0., 0., 0., 0., 0.]
    Xtip = 0.  # tip coordinate in [nm] w.r.t. the DB numbered 1.
    Ytip = 0.  # tip coordinate in [nm] w.r.t. the DB numbered 1.
    Htip = 0.5  # actual tip height in experiment in nm


    tip = TipModel()
    tip.setup(DBsXlist*10, DBsYlist*10, 1.)
    print(tip.level_shifts([0,1,5]))
    print(tip.level_shifts([0,2,5]))
    print(tip.level_shifts([0,3,5]))
    print(tip.level_shifts([0,4,5]))
