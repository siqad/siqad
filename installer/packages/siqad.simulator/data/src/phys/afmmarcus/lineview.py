#!/usr/bin/env python3
# encoding: utf-8

'''
Display latest log file from animation
'''

import os, re, json
from time import sleep

import matplotlib.pyplot as plt
import matplotlib.animation as anim
from matplotlib.patches import Circle, Ellipse

import numpy as np

class Viewer:
    ''' '''

    tick = 1e-3   # wait time before checking for new data, seconds
    pad = 6       # x-padding in angstroms
    dbrad = 2

    LW = 2

    # Poschl-Teller Parameters
    pt_0 = 3.81    # Poschl-Teller prefactor, eV.angstrom^2
    pt_alpha = 2.4  # well width, angstroms
    pt_lamb = 1.5   # well depth control

    pt_0 *= pt_lamb*(pt_lamb-1)/pt_alpha**2

    #wellfunc = lambda self, x: .32-.6*np.exp(-.5*np.abs(x)**3)
    wellfunc = lambda self, x: .16 - self.pt_0/np.cosh(x/self.pt_alpha)**2


    def __init__(self, start_file):
        '''Setup the Viewer to track the file given'''

        self.fn = start_file

    def update(self):

        try:
            self._show()
        except Exception as e:
            print(e)

    def run(self):

        self.hlist = []
        self.handles = {}

        def update(frame):
            print('update: {0}'.format(frame))
            self.update()
            return self.hlist

        fig = plt.figure()
        self.ax = fig.add_subplot(1,1,1)
        self.setup_check = False

        self.ani = anim.FuncAnimation(fig, update, repeat=False, interval=30)
        plt.show()

    def _show(self):
        '''Display the current log file and delete the old one'''

        data = self._loadLog()

        if not self.setup_check:
            self._setup(data)

        lamb = 0. if 'lamb' not in data else data['lamb']
        mu = 0. if 'mu' not in data else data['mu']

        # update all the artist parameters
        tflag = 'tbias' in data
        for n,(x,b,c) in enumerate(zip(self.X, data['bias'], data['charge'])):
            db = self.handles['db'][n]
            db['well'].set_data(self.dbx+x, self.dby-b-mu)
            db['lvl'].set_data(self.dbx+x, -b-mu)
            if tflag and c==0 and str(n) in data['tbias']:
                db['tlvl'].set_visible(True)
                db['tlvl'].set_data(self.dbx+x, -data['tbias'][str(n)]-mu)
            else:
                db['tlvl'].set_visible(False)
            db['dot'].center = (x, -b-(mu+lamb))
            db['dot'].set_visible(c==1)

        # update tip
        if 'tip' in self.handles:
            tip, dtip = self.handles['tip'], data['tip']

            vis = dtip['enabled']
            tip['C1'].width = 2*dtip['R1']
            tip['C1'].height = tip['C1'].width/self.ratio
            x, h = dtip['x'], .5*tip['C1'].height+dtip['H']/self.ratio
            tip['C1'].center = (x, h)
            tip['C1'].set_visible(vis)

            tip['C2'].width = 2*dtip['R2']
            tip['C2'].height = tip['C2'].width/self.ratio
            x, h = dtip['x'], .5*tip['C2'].height+dtip['H']/self.ratio
            tip['C2'].center = (x, h)
            tip['C2'].set_visible(vis)

            tip['apex'].set_data([x,x], [0,1])
            tip['apex'].set_visible(vis)


    def _setup(self, data):
        '''Setup everything using the first set of data'''

        self.X, self.Y = data['x'], data['y']

        self.ax.set_xlim(min(self.X)-self.pad, max(self.X)+self.pad)
        self.ax.set_ylim(-1,1)

        # fixed features
        self.hlist.append(self.ax.axhline(0, color='r',
                                linestyle='--', linewidth=self.LW))

        x0, y0 = self.ax.transAxes.transform((0,0))
        x1, y1 = self.ax.transAxes.transform((1,1))
        pix_ratio = (y1-y0)*1./(x1-x0)  # pixel ratio
        x0, x1 = self.ax.get_xlim()
        y0, y1 = self.ax.get_ylim()
        scl_ratio = (y1-y0)*1./(x1-x0)  # axis-scale ratio
        self.ratio = abs(pix_ratio/scl_ratio)

        # tip
        if 'tip' in data:
            tip = data['tip']
            h = {
                'apex': self.ax.axvline(0,color='b',
                                linestyle='dotted', linewidth=self.LW),
                'C1': self.ax.add_artist(Ellipse((0,0),
                                tip['R1'], tip['R1']/self.ratio, color='b')),
                'C2': self.ax.add_artist(Ellipse((0,0),
                                tip['R2'], tip['R2']/self.ratio, color='m',
                                alpha=.1, ls='dashed', lw=2))
                }
            self.handles['tip'] = h
            self.hlist += h.values()

        # dangling bonds
        self.handles['db'] = []
        for n,x in enumerate(self.X):
            h = {
                'well': self.ax.plot([], [], 'k-')[0],
                'lvl':  self.ax.plot([], [], 'g--')[0],
                'tlvl': self.ax.plot([], [], 'c--')[0],
                'dot':  self.ax.add_artist(Ellipse((0,0),
                                self.dbrad, self.dbrad/self.ratio, color='g'))}
            h['dot'].set_visible(False)
            self.handles['db'].append(h)
            self.hlist += h.values()

        self.dbx = np.linspace(-8,8,50)
        self.dby = self.wellfunc(self.dbx)

        self.setup_check=True


    def _loadLog(self):
        ''' '''

        with open(self.fn, 'r') as fp:
            data = json.load(fp)

        return data




if __name__ == '__main__':

    import sys

    try:
        fname = sys.argv[1]
    except:
        print('Problem')
        sys.exit()

    viewer = Viewer(fname)
    viewer.run()
