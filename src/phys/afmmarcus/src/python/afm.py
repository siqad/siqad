import numpy as np
from matplotlib import use as mpl_use
mpl_use('TkAgg')
import matplotlib.pyplot as plt
from matplotlib.colors import LinearSegmentedColormap

#from marcus import MarcusModel
from hopper import HoppingModel

Model = lambda X: HoppingModel(X, model='marcus')

class AFMLine:
    '''AFM simulator for a line of DBs with sequential 1D scan'''

    # coloring for plots
    cdict = {'red':    ((0, .48, .48),
                        (.5, .14, .14),
                        (1., 0, 0)),
             'green':  ((0,.82,.82),
                        (.5,.55,.55),
                        (1., 0, 0)),
             'blue':   ((0,.89,.89),
                        (.5,.70,.70),
                        (1., 0, 0))
             }

    cm = LinearSegmentedColormap('cm', cdict)

    # physics stuff
    wbias   = .01      # writing bias, eV
    wsig    = 2.       # writing bias width, angstroms

    def __init__(self, X):
        '''Contruct an AFM for a wire with the given db sites in lattice units'''

        self.X = sorted([x-min(X) for x in X])
        self.N = len(self.X)
        self.model = Model(X)

        self.setScanType()


    def setScanType(self, stype=0, wbias=None):
        '''Change the method of scanning:
                0   :: read in both directions, starting from left
                1   :: write-right  ,  read-left
                -1  :: write-left   ,  read-right
        '''

        if stype==0:
            self.gen = self.rrGen
        elif stype==1:
            self.gen = self.wrGen
        else:
            self.gen = self.rwGen

        if wbias is not None:
            self.wbias = wbias


    def setBias(self, F):
        '''Apply a bias of F eV/angstrom across the wire'''

        self.model.setBiasGradient(F, v=[1,0])


    def run(self, nscans=100, srate=82, pad=[2,2], Nel=None):
        '''Run the AFM for the given number of line scans'''

        self.nscans = nscans
        self.srate = srate
        self.pad = pad

        if Nel is not None:
            self.model.fixElectronCount(Nel)

        # initialise model
        self.model.initialise()

        # read the data
        self.charges = np.zeros([self.nscans, self.N], dtype=int)
        for db, dt, nscan, write in self.gen(self.nscans):
            if write:
                self.model.writeBias(self.wbias, db, dt, self.wsig/self.srate)
            else:
                self.charges[nscan, db] = self.model.measure(db, dt)

        self.plotImage()


    # scantype generators

    def rrGen(self, nscans):
        '''Generate the db order and timings'''

        rate = self.srate/self.model.a
        direc, nscan = 1, 0
        next_db, curr_x = 0, self.X[0]-self.pad[0]

        while nscan<nscans:
            dt = abs(self.X[next_db]-curr_x)/rate
            yield(next_db, dt, nscan, False)
            curr_x = self.X[next_db]
            next_db += direc
            if next_db == -1:
                next_db, curr_x, direc = 0, self.X[0]-2*self.pad[0], 1
                nscan += 1
            elif next_db == self.N:
                next_db, curr_x, direc = self.N-1, self.X[-1]+2*self.pad[1], -1
                nscan += 1

    def wrGen(self, nscans):

        rate = self.srate/self.model.a
        direc, nscan = 1, 0
        next_db, curr_x = 0, self.X[0]-self.pad[0]

        while nscan<nscans:
            dt = abs(self.X[next_db]-curr_x)/rate
            yield(next_db, dt, nscan, direc==1)
            curr_x = self.X[next_db]
            next_db += direc
            if next_db == -1:
                next_db, curr_x, direc = 0, self.X[0]-2*self.pad[0], 1
                nscan += 1
            elif next_db == self.N:
                next_db, curr_x, direc = self.N-1, self.X[-1]+2*self.pad[1], -1

    def rwGen(self, nscans):

        rate = self.srate/self.model.a
        direc, nscan = -1, 0
        next_db, curr_x = self.N-1, self.X[-1]+2*self.pad[1]

        while nscan<nscans:
            dt = abs(self.X[next_db]-curr_x)/rate
            yield(next_db, dt, nscan, direc==-1)
            curr_x = self.X[next_db]
            next_db += direc
            if next_db == -1:
                next_db, curr_x, direc = 0, self.X[0]-2*self.pad[0], 1
            elif next_db == self.N:
                next_db, curr_x, direc = self.N-1, self.X[-1]+2*self.pad[1], -1
                nscan += 1


    # plotting

    def plotImage(self):
        '''Make an image showing the charge configuration for all line scans'''

        nx = 400
        x = np.linspace(np.min(self.X)-self.pad[0], np.max(self.X)+self.pad[1], nx)

        dx = (np.max(x)-np.min(x))/(nx-1)
        sig = 1.

        kernel = np.exp(-.2*np.linspace(-2,2.,1+int(sig/dx))**2)

        inds = np.round(nx*(self.X-np.min(x))/(np.max(x)-np.min(x))).astype(int)
        data = np.zeros([self.nscans, nx], dtype=float)
        data[:, inds+int(.25*len(kernel))] = .5*(self.charges+1)

        for line in data:
            line[:] = np.convolve(line,kernel,'same')


        plt.imshow(data, interpolation='None', aspect='auto', cmap=self.cm)
        plt.colorbar()

        ax = plt.gca()
        ax.set_xticks([])
        ax.set_yticks([])

        plt.savefig('tempfig.pdf', bbox_inches='tight')
        plt.show()




if __name__ == '__main__':

    #X = [1, 8, 10, 15, 17, 24]
    X = [1, 12, 15, 22, 25]

    afm = AFMLine(X)
    afm.setScanType(-1, -.01)
    afm.setBias(.00012)
    afm.run(Nel=3, nscans=200, pad=[3,3])
