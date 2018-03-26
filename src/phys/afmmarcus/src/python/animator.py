#!/usr/bin/env python
# encoding: utf-8

'''
Real-time animation of DB arrangements
'''

__author__      = 'Jake Retallick'
__copyright__   = 'MIT License'
__version__     = '1.2'
__date__        = '2018-02-15'  # last update

import shutil, os
import numpy as np
from itertools import product

from PyQt5.QtCore import (Qt, QTimer, QThread, pyqtSignal, QDateTime)
from PyQt5.QtGui import (QPen, QBrush, QColor, QPainter, QImage)
from PyQt5.QtWidgets import *

from hopper import HoppingModel


_SF = 50     # scale factor

class Thread(QThread):
    def __init__(self, func):
        super(Thread, self).__init__()
        self.func = func

    def run(self):
        ''' '''
        self.func()
        self.exec_()

class DB(QGraphicsEllipseItem):

    pen     = QPen(QColor("white"), .2*_SF)     # DB edge pen
    bgpen   = QPen(Qt.darkGray, .1*_SF, Qt.DotLine)

    pfill   = QBrush(QColor("orange"))     # charged DB for fixed perturbers
    fill    = QBrush(Qt.green)      # charged DB fill color
    nofill  = QBrush(Qt.NoBrush)    # uncharged DB fill color

    D = 1.8*_SF              # dot diameter

    def __init__(self, x, y, n=-1, bg=False, parent=None):
        super(DB, self).__init__(_SF*x, _SF*y, self.D, self.D, parent=parent)
        self.xx, self.yy, self.n = x, y, n
        self.setPen(self.bgpen if bg else self.pen)
        self.setCharge(False)
        self.bg = bg
        if not bg:
            self.setZValue(2)

    def setCharge(self, charged):
        '''Set the charge state of the DB'''
        self.charged = charged
        if charged:
            brush = self.pfill if self.bg else self.fill
        else:
            brush = self.nofill

        self.setBrush(brush)

    def mousePressEvent(self, e):
        if self.bg and e.button() == Qt.LeftButton:
            self.setCharge(not self.charged)



class Tracker(QGraphicsEllipseItem):

    pen = QPen(Qt.red, .2*_SF, Qt.DotLine)
    D = DB.D*1.5
    dd = .5*(D-DB.D)

    def __init__(self, parent=None):
        super(Tracker, self).__init__(0, 0, self.D, self.D, parent=parent)
        self.setZValue(1)
        self.setPen(self.pen)
        self.setBrush(QBrush(Qt.NoBrush))
        self.hide()

    def track(self, db):
        self.setPos(_SF*db.xx - self.dd , _SF*db.yy - self.dd)
        self.show()



class HoppingAnimator(QGraphicsView):
    ''' '''

    # lattice parameters
    a = 3.84    # lattice vector in x, angstroms    (intra dimer row)
    b = 7.68    # lattice vector in y, angstroms    (inter dimer row)
    c = 2.25    # dimer pair separation, angstroms

    rate = 10   # speed-up factor

    xpad, ypad = 6, 3

    bgcol = QColor(29, 35, 56)  # background color
    record_dir = './.temp_rec/'

    signal_tick = pyqtSignal()
    signal_dbtrack = pyqtSignal(int)

    def __init__(self, model, record=False, fps=30):
        '''Initialise the HoppingAnimator instance for the given DB positions.
        X should be formatted as for HoppingModel'''

        super(HoppingAnimator, self).__init__()

        assert isinstance(model, HoppingModel), 'Invalid model type'

        self.model = model
        self.X, self.Y = self.model.X, self.model.Y

        self._initGUI()

        self.model.initialise()

        # setup threads
        self.threads = []
        self.threads.append(Thread(self.tick))

        # setup recording
        self.recording = record
        if record:
            # force clean directory
            if os.path.exists(self.record_dir):
                shutil.rmtree(self.record_dir)
            os.makedirs(self.record_dir)
            self.rind = 0   # record index
            self.fps = fps

            # setup threads
            self.threads.append(Thread(self.record))

    def _initGUI(self):
        '''Initialise the animator window'''

        self.scene = QGraphicsScene(self)
        self.setScene(self.scene)

        self._drawDBs()
        self.tracker = Tracker()
        self.scene.addItem(self.tracker)

        self.setBackgroundBrush(QBrush(self.bgcol, Qt.SolidPattern))
        self.setWindowTitle('Hopping Animator')

        # Set Anchors
        self.setTransformationAnchor(QGraphicsView.NoAnchor)
        self.setResizeAnchor(QGraphicsView.NoAnchor)

    def _drawDBs(self):
        '''Draw all the DBs for the animator'''

        # background
        X = np.arange(np.min(self.X)-self.xpad, np.max(self.X)+self.xpad+1)
        Y = np.arange(round(np.min(self.Y))-self.ypad, round(np.max(self.Y))+self.ypad+1)

        f = self.c/self.b
        for x,y in product(X,Y):
            self.scene.addItem(DB(self.a*x, self.b*y, bg=True))
            self.scene.addItem(DB(self.a*x, self.b*(y+f), bg=True))

        # foreground
        self.dbs = []
        for n, (x,y) in enumerate(zip(self.X, self.Y)):
            self.dbs.append(DB(self.a*x,self.b*y, n=n))
            self.scene.addItem(self.dbs[-1])


    def screencapture(self, fname):
        '''Save a screenshot of the QGraphicsScene and save it to the given
        filename'''

        self.scene.setSceneRect(self.scene.itemsBoundingRect())
        image = QImage(self.scene.sceneRect().size().toSize(), QImage.Format_ARGB32)
        image.fill(self.bgcol)

        painter = QPainter(image)
        self.scene.render(painter)
        image.save(fname)
        painter.end()


    def record(self):
        '''Record the QGraphicsScene at the given fps'''

        assert self.fps>0 and self.fps<=1000, 'Invalid fps'

        fname = os.path.join(self.record_dir, 'grab{0:06d}.png'.format(self.rind))
        self.screencapture(fname)

        self.rind += 1

        self.rec_timer = QTimer()
        self.rec_timer.timeout.connect(self.record)
        self.rec_timer.start(int(1000./self.fps))

    def compile(self):
        '''compile the recording directory into a video'''

        os.chdir(self.record_dir)
        os.system("ffmpeg -r {0} -f image2 -i grab%06d.png -vcodec libx264 -crf 25 -pix_fmt yuv420p ../rec.mp4".format(int(self.fps)))
        os.chdir('..')
        shutil.rmtree(self.record_dir)

    def tick(self):
        ''' '''

        for i,c in enumerate(self.model.charge):
            self.dbs[i].setCharge(c)


        dt = self.model.peek()[0]
        self.model.run(dt)

        self.signal_tick.emit()
        millis = int(dt*1000./self.rate)

        #print(dt, millis)
        if millis>=1:
            self.timer = QTimer()
            self.timer.timeout.connect(self.tick)
            self.timer.start(min(millis, 10000))
        else:
            self.tick()


    def zoomExtents(self):
        '''Scale view to contain all items in the scene'''
        rect = self.scene.itemsBoundingRect()
        self.fitInView(rect, Qt.KeepAspectRatio)
        self.scale(2,2)

    def mousePressEvent(self, e):
        super(HoppingAnimator, self).mousePressEvent(e)
        item = self.itemAt(e.pos())
        if e.button() == Qt.LeftButton:
            if isinstance(item, DB) and item.bg:
                self.model.addCharge(item.xx, item.yy, pos=item.charged)
        elif e.button() == Qt.RightButton:
            if isinstance(item, DB):
                self.signal_dbtrack.emit(item.n)

    def mouseDoubleClickEvent(self, e):
        self.mousePressEvent(e)




class FieldSlider(QHBoxLayout):
    '''Container for parameter selected by a QSlider'''

    def __init__(self, txt, parent=None):
        super(FieldSlider, self).__init__(parent)

        self.txt = QLabel(txt)
        self.out = QLabel()
        self.fval = lambda n: n
        self.func = lambda x: None

        self.initGUI()

    def initGUI(self):

        self.slider = QSlider(Qt.Horizontal)
        self.slider.setTickInterval(1)
        self.slider.valueChanged.connect(self.valueChanged)
        self.slider.sliderReleased.connect(self.sliderReleased)

        self.addWidget(self.txt, stretch=4)
        self.addWidget(self.slider, stretch=40)
        self.addWidget(self.out, stretch=4)

    def setBounds(self, lo, hi, inc, val):

        self.lo, self.hi, self.inc = lo, hi, inc
        self.fval = lambda n: lo+n*self.inc
        self.val = val

        self.slider.setMinimum(0)
        self.slider.setMaximum(round((hi-lo)*1./inc))
        self.slider.setValue(round((val-lo)/inc))

    def setValue(self, val):
        self.val = val
        self.setValue(round((val-self.lo)/self.inc))

    # event handling
    def valueChanged(self):
        self.val = self.fval(self.slider.value())
        self.out.setText('{0:.3f}'.format(self.val))

    def sliderReleased(self):
        self.func(self.val)




class FieldEdit(QHBoxLayout):
    '''Container for parameter selected by a QLineEdit'''

    def __init__(self, parent=None):
        super(FieldEdit, self).__init__(parent)


class DockWidget(QDockWidget):
    ''' '''

    WIDTH = 200

    def __init__(self, parent=None):
        super(DockWidget, self).__init__(parent)

        self.initGUI()

    def initGUI(self):

        self.setMinimumWidth(self.WIDTH)
        self.setAllowedAreas(Qt.LeftDockWidgetArea | Qt.RightDockWidgetArea)

        widget = QWidget(self)
        self.vbox = QVBoxLayout(widget)
        self.vbox.setAlignment(Qt.AlignTop)
        self.setWidget(widget)

        self.hide()

    def addSlider(self, txt, lo, hi, inc, val, func):
        '''Add a slider controlled parameter to the Dock Widget

        inputs:
            txt     : Label of the slider
            lo      : Lowest value of the slider
            hi      : Highest value of the slider
            inc     : Increment between slider ticks
            func    : When slider is updated to x, func(x) called
        '''

        slider = FieldSlider(txt)
        slider.setBounds(lo, hi, inc, val)
        slider.func = func

        self.vbox.addLayout(slider)
        return slider

    def addWidget(self, widget, stretch=-1):
        self.vbox.addWidget(widget, stretch=stretch)



class MainWindow(QMainWindow):
    ''' '''

    WINX = 1400     # window width
    WINY = 1000      # window height

    ZOOM = .1

    def __init__(self, model, record=False, fps=30):
        ''' '''
        super(MainWindow, self).__init__()

        self.record = record
        self.fps = fps

        self.dbn = -1
        self.model = model
        self.bulk = self.model.addChannel('bulk')
        self.animator = HoppingAnimator(model, record=record, fps=fps)
        self.animator.signal_tick.connect(self.tickSlot)
        self.animator.signal_dbtrack.connect(self.trackDB)

        self.initGUI()
        self.createDock()

        for thread in self.animator.threads:
            thread.start()

    def initGUI(self):
        ''' '''

        self.setGeometry(100, 100, self.WINX, self.WINY)
        self.setCentralWidget(self.animator)
        self.animator.zoomExtents()

    def createDock(self):
        '''Create the dock widget for simulation options'''

        self.dock = DockWidget(self)

        self.beff = QLabel()
        self.dock.addWidget(self.beff)

        self.ecount = QLabel()
        self.dock.addWidget(self.ecount)

        val, func = self.bulk.mu_on, lambda v: self.setBulkMu(v)
        self.dock.addSlider("mu", .01, .3, .01, val, func)

        val, func = np.log10(self.animator.rate), lambda r: self.setRate(10**r)
        self.dock.addSlider("log(rate)",-3., 3., .5, val, func)

        self.addDockWidget(Qt.RightDockWidgetArea, self.dock)

    def setBulkMu(self, v):
        self.bulk.mu_on = v
        self.bulk.mu_off = v
        self.animator.tick()

    def setRate(self, v):
        self.animator.rate = v
        self.animator.tick()
        self.animator.tick()

    def tickSlot(self):
        self.ecount.setText('Number of Electrons: {0}'.format(self.model.Nel))
        self.echoDB()

    def trackDB(self, n):
        if self.dbn == n:
            n = -1
        self.dbn = n
        if n<0:
            self.animator.tracker.hide()
        else:
            self.animator.tracker.track(self.animator.dbs[n])
        self.echoDB()

    def echoDB(self):
        if self.dbn < 0:
            self.beff.setText('')
        else:
            self.beff.setText('DB-Beff: {0:.3f}'.format(self.model.beff[self.dbn]))


    def keyPressEvent(self, e):

        if e.key() == Qt.Key_Q:
            if self.record:
                self.animator.compile()
            self.close()
        elif e.key() == Qt.Key_O:
            self.dock.setVisible(not self.dock.isVisible())
        elif e.key() == Qt.Key_Space:
            self.animator.tick()
        elif e.key() in [Qt.Key_Plus, Qt.Key_Equal]:
            zfact = 1+self.ZOOM
            self.animator.scale(zfact,zfact)
        elif e.key() == Qt.Key_Minus:
            zfact = 1-self.ZOOM
            self.animator.scale(zfact, zfact)
        elif e.key() == Qt.Key_S:
            fname = QDateTime.currentDateTime().toString('yyyyMMdd-hhmmss.png')
            fname = os.path.join('.', fname)
            print('Screenshot saved to: {0}'.format(os.path.normpath(fname)))
            self.animator.screencapture(fname)




if __name__ == '__main__':

    import sys
    sys.setrecursionlimit(50)

    line = [8, 10, 15, 17]
    line.insert(0, line[0]-7)
    line.append(line[-1]+7)

    pair = lambda n: [0, n]

    _or = [(0,0,0),(2,1,0),(6,1,0),(8,0,0),(4,3,0),(4,4,1)]
    #_or.append((-2,-1,0))
    #_or.append((10,-1,0))

    def QCA(N):
        qca = []
        for n in range(N):
            x0 = 10*n
            qca += [(x0,0,1), (x0+3,0,1), (x0,2,0), (x0+3,2,0)]
        #qca.append((-4,0,1))
        return qca

    device = QCA(5)

    # NOTE: recording starts immediately if record==True. Press 'Q' to quit and
    #       compile temp files into an animation ::'./rec.mp4'
    # model = HoppingModel(device, model='marcus', record=True)
    model = HoppingModel(device, model='marcus')
    # model.fixElectronCount(5)
    #model.addChannel('bulk')

    app = QApplication(sys.argv)
    mw = MainWindow(model)

    mw.show()
    sys.exit(app.exec_())
