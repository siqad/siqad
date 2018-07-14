#!/usr/bin/env python
# encoding: utf-8

'''
Python connector that takes the output from C++ physics engine connector and
runs the simulation with the desired parameters
'''

__author__      = 'Samuel Ng'
__copyright__   = 'MIT License'
__version__     = '0.2'
__date__        = '2018-06-02'  # last update

from argparse import ArgumentParser
import os.path
import xml.etree.ElementTree as ET

from PyQt5.QtCore import (Qt, QTimer, QThread)
from PyQt5.QtGui import (QPen, QBrush, QColor, QPainter, QImage)
from PyQt5.QtWidgets import (QApplication, QGraphicsView, QGraphicsScene,
                             QGraphicsEllipseItem)

import siqadconn

from afm import AFMLine
from animator import (HoppingAnimator, MainWindow)
from hopper import HoppingModel

class DBSimConnector:
    '''This class serves as a connector between the C++ physics engine connector and
    the AFMMarcus Python classes'''

    dbs = []        # list of tuples containing all dbs, (x, y)
    afmnodes = []   # list of tuples containing all afmnodes, (x, y, z)

    def parseCmlArguments(self):
        parser = ArgumentParser(description="This script takes the problem file "
                "and runs the AFM tip simulation with the AFM path and DB locations "
                "given in that file.")
        #parser.add_argument("-i", "--input", dest="in_file", required=True,
        parser.add_argument(dest="in_file", type=self.fileMustExist,
                help="Path to the problem file.",
                metavar="IN_FILE")
        #parser.add_argument("-o", "--output", dest="out_file", required=True,
        parser.add_argument(dest="out_file", help="Path to the output file.",
                metavar="OUT_FILE")
        self.args = parser.parse_args()

    def fileMustExist(self, fpath):
        '''Check if input file exists for argument parser'''
        if not os.path.exists(fpath):
            raise argparse.ArgumentTypeError("{0} does not exist".format(fpath))
        return fpath


    # Import problem parameters and design from SiQAD Connector
    def initProblem(self):
        self.sqconn = siqadconn.SiQADConnector("AFMMarcus", self.args.in_file,
            self.args.out_file)

        # retrieve DBs and convert to a format that hopping model takes
        for db in self.sqconn.dbCollection():
            self.dbs.append((db.n, db.m, db.l))

    # Run simulation
    def runSimulation(self):
        '''Run the simulation'''

        # check simulation type ('animation' or 'line_scan')
        if (self.sqconn.getParameter('simulation_type') == 'line_scan'):
            self.runLineScan()
        else:
            self.runAnimation()

    def runLineScan(self):
        # for now, only 1D line scan is supported, all y values will be discarded
        # TODO 2D support
        X = []
        for dbloc in self.dbs:
            X.append(dbloc[0])
        X.sort()
        print(X)

        # call the AFM simulation
        self.afm = AFMLine(X)
        self.afm.setScanType(int(self.sqconn.getParameter('scan_type')),
                float(self.sqconn.getParameter('write_strength')))
        self.afm.setBias(float(self.sqconn.getParameter('bias')))
        self.afm.run(Nel=int(self.sqconn.getParameter('num_electrons')),
                nscans=int(self.sqconn.getParameter('num_scans')),
                pad=[int(self.sqconn.getParameter('lattice_padding_l')),
                    int(self.sqconn.getParameter('lattice_padding_r'))]
                )

    def runAnimation(self):
        import sys

        model = HoppingModel(self.dbs, self.sqconn.getParameter('hopping_model'))
        model.fixElectronCount(int(self.sqconn.getParameter('num_electrons')))

        model.addChannel('bulk')
        model.addChannel('clock', enable=False)
        model.addChannel('tip', enable=False)

        app = QApplication(sys.argv)
        mw = MainWindow(model)

        mw.show()
        mw.animator.start()
        sys.exit(app.exec_())

if __name__ == "__main__":
    # TODO maybe move this to animator.py
    connector = DBSimConnector()
    connector.parseCmlArguments()
    connector.initProblem()
    connector.runSimulation()
