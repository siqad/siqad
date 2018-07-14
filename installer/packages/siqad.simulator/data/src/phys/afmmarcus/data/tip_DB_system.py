from numpy import *
import  matplotlib
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, Wedge
from matplotlib.ticker import MultipleLocator
from matplotlib.backends.backend_pdf import PdfPages
import scipy
import scipy.constants as const
from scipy.signal import cspline1d, cspline1d_eval
import  os, sys #This package is necessary to handle saving our data.
import  os.path
import pylab
# Below are modules needed for matrix diagonalization
# from householder import *
# from eigenvals3 import *
# from inversePower3 import *

matplotlib.rcParams['xtick.labelsize'] = 9
matplotlib.rcParams['ytick.labelsize'] = 9
matplotlib.rcParams['axes.labelsize']  = 11
font = {'family' : 'serif', 'weight' : 'normal', 'size'   : 9}
matplotlib.rc('font', **font)

# by Lucian Livadaru, Jan9, 2018

def nf(x):  # nice format for printing
  # return '$%*.3f' %x
  return '%1.5e' %x

def ns(x):  # nice format for printing
  # return '$%*.3f' %x
  return '%1.3e' %x

def NF(x):  # nice format for printing
  # return '$%*.3f' %x
  return '%1.5g' %x

def fl(x):  # nice format for printing
  # return '$%*.3f' %x
  return '%1.5f' %x

def fs(x):  # nice format for printing
  # return '$%*.3f' %x
  return '%1.2f' %x

def erf(x):
    '''
    Assumes positive argument only !!!
    '''
    ## save the sign of x
    # sign = 1 if x >= 0 else -1
    # x = abs(x)

    # constants
    a1 =  0.254829592
    a2 = -0.284496736
    a3 =  1.421413741
    a4 = -1.453152027
    a5 =  1.061405429
    p  =  0.3275911

    # A&S formula 7.1.26
    t = 1.0/(1.0 + p*x)
    y = 1.0 - (((((a5*t + a4)*t) + a3)*t + a2)*t + a1)*t* exp(-x*x)
    return  y # sign*y # erf(-x) = -erf(x)
    
def smooth(x,window_len= 5,window='flat'):
    '''smooth the data using a window with requested size.

    This method is based on the convolution of a scaled window with the signal.
    The signal is prepared by introducing reflected copies of the signal
    (with the window size) in both ends so that transient parts are minimized
    in the begining and end part of the output signal.
    input:
        x: the input signal
        window_len: the dimension of the smoothing window; should be an odd integer
        window: the type of window from 'flat', 'hanning', 'hamming', 'bartlett', 'blackman'
            flat window will produce a moving average smoothing.
    output:
        the smoothed signal
    example:
    t=linspace(-2,2,0.1)
    x=sin(t)+randn(len(t))*0.1
    y=smooth(x)
    '''

    if x.ndim != 1:
        raise ValueError, "smooth only accepts 1 dimension arrays."
    if x.size < window_len:
        raise ValueError, "Input vector needs to be bigger than window size."
    if window_len<3:
        return x
    if not window in ['flat', 'hanning', 'hamming', 'bartlett', 'blackman']:
        raise ValueError, "Window is on of 'flat', 'hanning', 'hamming', 'bartlett', 'blackman'"
    s= r_[2*x[0]-x[window_len-1::-1],x,2*x[-1]-x[-1:-window_len:-1]]
    #print(len(s))
    if window == 'flat': #moving average
        w=ones(window_len,'d')
    else:
        w= eval(window+'(window_len)')

    y= convolve(w/w.sum(),s,mode='same')
    return y[window_len:-window_len+1]
  
  
def diagonalize2x2(A):
  '''
  This function diagonalizes a 2x2 matrix A of the form
  [a11  a12]
  [a21  a22]
  Output: E1, E2 = eigenvalues
          V1, V2 = eigenvectors (as numpy arrays [a1, b1], [a2, b2])
        Do I need to make them into columns? No.
  '''
  a11 = A[0,0]
  a12 = A[0,1]
  a21 = A[1,0]
  a22 = A[1,1]
  
  trA = a11 + a22
  detA = a11 * a22 - a12 * a21
  discA = trA **2 - 4.* detA
  if discA == 0. : exit('Matrix Discriminant is Zero! stopped.')
  eigenvals = roots([1., -trA, detA])
  # sorting the eigenvals
  E0 = amin(eigenvals)
  E1 = amax(eigenvals)
  # un-normalized eigenvectors
  V0 = array([a12, E0 - a11])
  V1 = array([a12, E1 - a11])
  # normalizing eigenvectors
  # norm0 = sqrt(abs(a12)**2 + abs(E0 - a11)**2)
  # norm1 = sqrt(abs(a12)**2 + abs(E1 - a11)**2)
  # if norm0 != 0.: V0 /= norm0
  # else: exit('Null norm of eigenvector V0')
  # if norm1 != 0.: V1 /= norm1
  # else: exit('Null norm of eigenvector V1')
  return E0, E1, V0, V1
  
def diagonalize2x2realSymm(A):
  '''
  This function diagonalizes a 2x2 matrix A of the form
  [a11  a12]
  [a21  a22]
  Output: E1, E2 = eigenvalues
          V1, V2 = eigenvectors (as numpy arrays [a1, b1], [a2, b2])
        Do I need to make them into columns? No.
  '''
  if A[1,0] != A[0,1]: exit('The input matrix is not symmetric ')
  
  a11 = A[0,0]
  b = A[0,1]
  a22 = A[1,1]
    
  eigenvals = zeros((2))
  eigenvals[0] = ( a11 + a22 + sqrt((a11 - a22)**2 + 4.*b **2) ) /2.
  eigenvals[1] = ( a11 + a22 - sqrt((a11 - a22)**2 + 4.*b **2) ) /2.
  # sorting the eigenvals
  E0 = amin(eigenvals)
  E1 = amax(eigenvals)
  # normalized eigenvectors
  norm0 = sqrt(b**2 + (E0 - a11)**2)
  V0 = array([b / norm0 , (E0 - a11) / norm0])
  norm1 = sqrt(b**2 + (E1 - a11)**2)
  V1 = array([b / norm1, (E1 - a11) / norm1])
  # normalizing eigenvectors
  # norm1 = sqrt(abs(a12)**2 + abs(E1 - a11)**2)
  # if norm0 != 0.: V0 /= norm0
  # else: exit('Null norm of eigenvector V0')
  # if norm1 != 0.: V1 /= norm1
  # else: exit('Null norm of eigenvector V1')
  return E0, E1, V0, V1
  
def quad_eq(a, b, c):  
    '''
    Solves the quadratic eq. ax^2 + bx + c = 0
    '''
    d = b **2 - 4.* a * c # discriminant

    if d < 0:
        print ("This equation has no real solution")
    elif d == 0:
        x = - b / (2.* a)
        # print ("This equation has one solutions: "), x
        return x, x
        
    else:
        x1 = (- b + sqrt(d))/ (2.* a)
        x2 = (- b - sqrt(d))/ (2.* a)
        # print ("This equation has two solutions: ", x1, " or", x2) 
        return x1, x2        
        

def PsiCB_dimmless(x, z, E, costh, A, Tx):  
  '''
  # dimensionless wavefunction of the CB in Si
  # Input: x, z = 2 cartesian coord [Ang], 
           E = energy [J] from surface CBM, 
           costh = cos of angle of Kz with surface normal, 
           A = surface corrugation Amplitude [Ang], 
           Tx = its Period [Ang]
    return: psi       
  globals: Eg_J, Xaff_eV, h_bar, me, qe, M2h2, sqrt2mh2
  '''
 
  Evac = (Eg_eV + Xaff_eV) * qe         # vacuum level [J] from bulk VBM
  K2 = E * M2h2    
  K = sqrt(K2)                      # total wavevector length [m-1]
  Kz = K * costh                    # surface-normal component of K [m-1]
  Kxy2 = K2 * (1. - costh **2)
  Kxy = sqrt(Kxy2)                  # surface-parallel component of K [m-1]
  zeta_Si2 = M2h2 * (Evac - E)      # square of decay length [m-1]
  zetaKxy = sqrt(zeta_Si2 + Kxy2)   # in [m-1]
  fact1 = 1. # 1j * cos(Kxy * x) - sin(Kxy * x)  # = i exp(iK.x) in my paper
  H = A * cos(pi * x / Tx) **2
  fact2 = Kz * exp(- zetaKxy * (z - H) *1e-10) / (zetaKxy - 1j * Kz)
  psi = fact1 * fact2
  return psi

def DBionizPoten_J (z, DBsign): 
  '''
  # argument can be either float, or numpy, or escript data object
  # z is the cos of the theta angle, which is zero toward vacuum
  '''
  # globals DB_STO_Norm, W_DBm_vac, W_DBm_CB, W_DB0_CB, W_DB0_vac # these are in [eV]
  
  z0 = 0. #-1.3
  Dz = 5.
  Wiform = 1
  if Wiform == 0:  #  constant; DB has 2 equal lobes in vac and Si
    Wi = W_DBm_vac
  elif Wiform == 1:
    if array(z).size > 1: # is numpy array with more than 1 element
      if DBsign == -1:
        Wi = W_DBm_CB + 0.5* (W_DBm_vac - W_DBm_CB) * (tanh((z - z0) / Dz) + 1.)
      elif DBsign == 0:
        Wi = W_DB0_CB + 0.5* (W_DB0_vac - W_DB0_CB) * (tanh((z - z0) / Dz) + 1.)
      else:
        exit('DB sign should be -1 or 0, please revise call to DBionizPoten_J')
    else: #  x is a scalar or escript data object
    # elif isinstance(z, float) : #  x is a scalar
      if DBsign == -1:
        Wi = W_DBm_CB + 0.5* (W_DBm_vac - W_DBm_CB) * (tanh((z - z0) / Dz) + 1.)
      elif DBsign == 0:
        Wi = W_DB0_CB + 0.5* (W_DB0_vac - W_DB0_CB) * (tanh((z - z0) / Dz) + 1.)
      else:
        exit('DB sign should be -1 or 0, please revise call to DBionizPoten_J')
    
  return Wi * qe  # in [J]
    
def DBchargeDens(X, Z, x0 = 0., z0 = 0.):
  '''
  # Calculates the charge density of the DB orbital (properly normalized). 
  # Input: 2D escript coordinates (x[0],x[1]) or FLOATS; in units of [Ang]
          x0 = 0., z0 = 0.  are the center coords.
  # globals: DB_STO_Norm, W_DBm_vac, W_DBm_CB, h_bar, me, LD, sqrt2mh2  # LD in [nm]
  '''
  
  x = X - x0
  z = Z - z0
  r = (x**2 + z**2)**(0.5)            # the radial distance [nm]
  r_m = r * 1e-10                   # convert r to [m]
  # costht = z / r                    # cos of theta angle
  zeta = sqrt(2* me)/ h_bar * (DBionizPoten_J(z, -1))**(0.5) # in [m-1]
  ChargeDens = (z * exp(- r_m * zeta) * DB_STO_Norm)**2
  return ChargeDens
      
def DB_STO_2p (X , Z, x0 = 0., y0 = 0., z0 = 0.): # z is perpendicular to the surface, x parallel; in [Ang]
  '''
  # Calculates the charge density of the DB orbital (properly normalized). 
  # Input: 2D numpy array or floats x,z coordinates (x[0],x[1]) or FLOATS; in units of [Ang]
          x0 = 0., z0 = 0.  are the center coords. in [Ang]
    Output: psi = an array with the wavefunction values at each point 
  
  '''
  global DB_STO_Norm
  x = X - x0
  z = Z - z0

  # print W_DBm_vac / qe, W_DBm_CB / qe, h_bar, me
  if DB_STO_Norm == 0.:
    DB_STO_Norm = 1.

  r = (x * x + z * z)**(0.5)    # calc. the radial distance (as an array); DOES THIS work if z is a float?
  r_m = r * 1e-10                 # convert to [m]
  # costht = z / r                  # cos of theta angle, also a 2D array
  # tht= arccos(z / r)
  # zeta= 0.512 * sqrt(0.32+ 2.025*(costht + 1.))  # this is zeta in 1/ang.; the first factor here is sqrt(2me)/hbar in 1/ang
  # costh = where(costht > 0., costht, 0.)     # this should work, but it does not produce desired results; why?
  # zeta = sqrt(2.* me)/ h_bar * sqrt(W_DBm_vac + (W_DBm_vac - W_DBm_CB)*(costh - 1.))
  zeta = sqrt(2.* me)/ h_bar * (DBionizPoten_J(z, -1))**(0.5)
  psi = z * exp(- r_m * zeta) * DB_STO_Norm    # should z here be in [m]?
  return psi
 
# Normalization of STO orbital:
def DetermineNormalizationForDB_STO_ptype2():
  dX = 0.1e-10  # units of [m] for all lengths here
  dZ = 0.1e-10 
  X1d = arange(0., 15e-10, dX)  # range limits in [m] [my notes on March 3,2010]
  Z1d = arange(-30e-10, 10e-10, dZ)
  X, Z = meshgrid(X1d, Z1d)
  psi = DB_STO_2p(X *1e10, Z *1e10)  # arguments converted in [Ang.]
  dV = X * dX * dZ * 2 * pi
  Integral = sum(dV * psi**2)
  return sqrt(1./ Integral)

def DB_volume():
  '''
  # The volume of STO orbital:
  '''
  dX = 0.1e-10  # units of [m] for all lengths here
  dZ = 0.1e-10 
  X1d = arange(0., 15e-10, dX)  # range limits in [m] [my notes on March 3,2010]
  Z1d = arange(-30e-10, 10e-10, dZ)
  X, Z = meshgrid(X1d, Z1d)
  psi = DB_STO_2p(X*1e10, Z*1e10)  # arguments converted in [Ang.]
  psiMax = amax(abs(psi))
  dV = X * dX * dZ * 2 * pi
  dV0 = where(abs(psi)**2 > 0.01* psiMax**2, dV, 0.) # where |psi|^2 is over 1% of its max. 
  Volume = sum(dV0)
  return Volume
  
# Dimensionless DB orbital function
def DB_STO_dimmless(x,z): 
  # input: cartesian coords [Ang]
  global OmegaDB
  psi = DB_STO_2p(x,z) * sqrt(OmegaDB)
  return psi
     

def P_Teller_potential(X, Xwells, lam3, alf, x0t= 0.384, U0t = 0., XpList = [-1.536], YpList= [0.], QpList = [0.], epsRelPlist=[6.3]):
  '''
  # @@@ a Poschl - Teller potential on a mesh X with multiple minima
  at points in Xwells array.
  Input: 
    X = an array with mesh point locations [nm], e.g. by linspace
    Xwells = an array with minima locations [nm] of the potential (wells)
    lam = the lambda factor [dimensionless] giving the depth of the potential wells
        OR lam2 = the factor lam*(lam -1.) [dimensionless] giving the depth of the potential wells
    alf = the alpha factor [nm] giving the width of the potential wells
    x0t= 0.384, [nm] a width over the which linear potential tilt is U0t / 2 (below)
    U0t = 0.,  [V] the potential tilt
    XpList = 1.536 [nm], x-distance from (Left-positioned) pertuber to the leftmost DB in the chain.
    YpList = 0.384 [nm], y-distance from (Left-positioned) pertuber to the leftmost DB in the chain.
    QpList = 1.   # a dimensionless factor for scaling the perturber potential 
  NB: all wells have the same depth and width 
  Output:
    U_tot = total potential in atomic units
  '''
  global xP, yP
  xP = zeros((len(XpList)))  # initialize xP
  yP = zeros((len(XpList)))  # initialize yP
  # convert X, etc into atomic units of length
  X_au = X /(a0B * 1e9)
  Xwells_au = Xwells /(a0B * 1e9)
  alf_au = alf / (a0B * 1e9)
  # print 'alf_au =',  alf_au, 'AU'
  Nw = Xwells.size  # number of wells
  Uw = zeros((Nw, X.size))
  U0t_au = U0t / har_eV  # in A.U.
  Ut = 0.5* U0t_au * X / x0t  # potential tilt, linear.
  Upert_hart = zeros((len(XpList), X.size))
  U_tot = zeros((X.size))
  U_tot[:] = Ut[:]
  # ADD perturber(s) potential 
  for ip, x0pi  in enumerate(XpList):
      # xP[ip] =  Xwells[0] - x0pi     # abs. coordinate of the pertuber [nm] 
      xP[ip] = x0pi     # abs. coordinate of the pertuber [nm] 
      yP[ip] = YpList[ip]     # abs. coordinate of the pertuber [nm] 
      X_to_P = 10. * (X - xP[ip])   # x-distances from the pertuber [Ang]
      Y_to_P = 10. * (0. - yP[ip])   # y-distances from the pertuber [Ang]; 0. is the y-coord of my double well line
      r_to_P = (X_to_P **2 + Y_to_P **2)**0.5  # r-distances from the pertuber [Ang]
      ''' TO get perturbed along x only '''
      # Upert_eV = (-1.)* QpList * interp(abs(X_to_P), r_DB_interp, DB_param[:,2]) # (-1) here is for -1e charge
      ''' TO get pertubed along r '''
      # Upert_eV = (-1.)* QpList * interp(abs(r_to_P), r_DB_interp, DB_param[:, 2]) # (-1) here is for -1e charge
      Upert_eV = (-1.)* QpList[ip] * qe /((4* pi * eps0 * epsRelPlist[ip]) * (abs(r_to_P)* 1e-10))  # perturbation energy [eV]      
      Upert_hart[ip, :] = Upert_eV / har_eV # in A.U.
      # plt.plot (X, Upert_eV) 
      # plt.show()
      # print r_DB_interp
      # print DB_param[:,2]
      # exit()
      ''' 
      ## TO get same electrode effect on both sides use
      '''
      U_tot[:] += Upert_hart[ip, :]            #  zeros((X.size))
      ####
      ''' 
      ## TO get tilt, use below if
      if ip == 0: 
         U_tot[:] += Upert_hart[ip, :]            #  zeros((X.size))
      else:
         U_tot[:] -= Upert_hart[ip, :]            #  zeros((X.size))
      '''
  for iw in range(Nw):
    # create individual P-T wells. use atomic units
    # Uw[iw, :] = - lam * (lam -1.)/2./ alf_au **2 / (cosh((X_au - Xwells_au[iw])/ alf_au)) **2   # as in the Schofield paper
    # Uw[iw, :] = - lam2 /2./ alf_au **2 / (cosh((X_au - Xwells_au[iw])/ alf_au)) **2  # Has alf in the well depth
    Uw[iw, :] = - lam3 / (cosh((X_au - Xwells_au[iw])/ alf_au)) **2  # Has alf only in well width.
    U_tot[:] += Uw[iw, :]
    
  '''  
  plt.plot(X, U_tot * har_eV, linewidth = 2.)
  # print "\nEigenvectors:\n", psi_well
  plt.xlim(-2, 2)
  plt.ylim(-2.0, 0.2)
  plt.show()
  '''
  return U_tot
  
def Solve_discrete_Schrod_1D(X, well_loc, subplotstring1 = '111', Qp1 = 77e-7, Ut_lin = 0.):
  '''
  @@@ - this means "Work in progress!!!"
  (initiated: 30.01.17)
  solves Schroedinger equation for 1D HO in discrete form on a 1D spatial mesh, x
  Uses atomic units for S.Eq.
  Inputs: Xmin, Xmax, Nx  = x limits [nm] and number of mesh INTERVALS (not points)
          well_loc = a numpy array with the minima of the potential well [nm] 
          Qp1 = charge of perturber [e]
          Ut = 0. # 0.05   # potential tilt in V
  Returns Elevels_eV =  a 1D array with eigenenergies in [eV]
          psi_well[:, i] = the eigenfunctions i on the specified grid :
  
  '''
  global lam3, alf1, Ut, PSI1, PSI2, WKB_barrZeroGate, WKB_barrNegGate
  Xmin = X[0]
  Xmax = X[-1]
  Nx = X.size
  # convert length in atomic units
  Xmin_au = Xmin / (a0B *1e9)
  Xmax_au = Xmax / (a0B *1e9)
  d = (Xmax_au - Xmin_au) / (Nx + 1)  # mesh spacing [atomic u.]
  # construct mesh [a.u.]
  X_au = linspace(Xmin_au, Xmax_au, Nx)
  # construct V
  x0 = (Xmax_au + Xmin_au) / 2.  # center of the mesh 
  # V_au = P_Teller_potential(X, array([-0.4, 0.4]), 2.2, 0.24)  # as in UCL
  # lam2 = .35  # 0.3 well depth 
  ''' CHOOSE THE NUMBER OF e IN THE SYSTEM BY CHOOSING WELL DEPTH @@@!!!'''
  # lam3 = 0.055    # this is chosen for 3e- in the double well. for QSi-QC 
  # lam3 = 0.105   # this is chosen for a 1e- system in the double well for QSi-QC
  # alf1 = 0.09  #  0.09  # 0.1 single well width 
  
  ''' # Perturber positions for TILT GATE 
  '''
  xp1 = -6.* da * 0.1   # perturber location [nm]; absolute
  xp2 = - xp1           # perturber location [nm] 
  # Qp1 = -1.    # scaling factor representing the perturber charge, equal to the charge in qe units
  Qp2 = - Qp1    # scaling factor representing the perturber charge, equal to the charge in qe units
  yp1 = 0.
  yp2 = 0.
  ###  Choose between above and below options
  ''' # Perturber positions for BARRIER GATE 
  '''
  xp1 = - 0.5 * di * 0.1   # perturber location [nm]; absolute
  xp2 = - xp1              # perturber location [nm]     
  # Qp1 = -1.    # scaling factor representing the perturber charge, equal to the charge in qe units
  Qp2 = Qp1    # scaling factor representing the perturber charge, equal to the charge in qe units
  yp1 = 1.* da * 0.1   # perturber location [nm] 
  yp2 = yp1
  
   ###  Choose between above and below options
   
  ''' # Perturber positions for CNOT gate
  xp1 = 0.   # perturber location [nm]; absolute 
  # yp1 = 4.* da * 0.1   # CLOSE perturber location [nm] for CNOT gate
  # yp1 = 6.* da * 0.1   # FAR perturber location [nm] for CNOT gate
  '''
  
  
  if well_loc.size == 1:
    if well_loc[0]==0.: xp1 =  0.1* 3.* da
    if well_loc[0]> 0.: xp1 =  0.1* 6.* da
  # V_au = P_Teller_potential(X, array([-.192, .192]), lam2, alf1)  # with well width = .384 nm 
  # V_au = P_Teller_potential(X, well_loc, lam3, alf1, x0p = [xp], U0p = Qp1)  #  
  # V_au = P_Teller_potential(X, well_loc, lam3, alf1, XpList = [xp1, well_loc[1], well_loc[0]], YpList = [0., 1.2, 1.2], QpList = [Qp1, Qp2, Qp3], epsRelPlist=[6.3, 1., 1.])  #  with image charges
  # V_au = P_Teller_potential(X, well_loc, lam3, alf1, XpList = [xp1, xp2], YpList = [yp1, yp2], QpList = [Qp1, Qp2], epsRelPlist=[6.3, 6.3])  #  FOR BARRIER GATE
  # V_au = P_Teller_potential(X, well_loc, lam3, alf1, XpList = [xp1, xp2], YpList = [yp1, yp2], QpList = [Qp1, Qp2], epsRelPlist=[6.3, 6.3])  #  FOR TILT GATE
  V_au = P_Teller_potential(X, well_loc, lam3, alf1, x0t= well_loc[0], U0t = Ut_lin)   #  FOR LINEAR TILT GATE
  V_au = P_Teller_potential(X, well_loc, lam3, alf1, x0t= well_loc[0], XpList = [xp1, xp2], YpList = [yp1, yp2], QpList = [Qp1, Qp2], epsRelPlist=[6.3, 6.3], U0t = Ut_lin)   #  FOR LINEAR TILT GATE + BARRIER GATE !
  # V_au = P_Teller_potential(X, well_loc, lam3, alf1, XpList = [xp1], YpList = [yp1], QpList = [Qp1], epsRelPlist=[6.3])  #  for CNOT gate, FOR BARRIER GATE with a Control Qubit
  sigm1 = 0.16
  # V_au = El_poten_gaussian(X, well_loc, sigm = sigm1)  # with well width = .384 nm  Xwells, sigm
  Vmin_au = min(V_au)   # min of the potential
  # plt.show()
  
  # calculate the coeffs for the SEq. system of linear eqs. 
  Adiag = zeros(shape(X))
  Ai_m1 = zeros((Nx - 1))  # A[i, i-1]
  Ai_p1 = zeros((Nx - 1))  # A[i, i+1]
  # assign values
  Ai_m1[:] = -1./ (2.* d **2)
  Ai_p1[:] = -1./ (2.* d **2)
  Adiag = 1./ (d **2) + V_au
  Neig = 2
  # if 2 < well_loc.size < 6  : Neig = 3  # we need to look at more states
  squareA = zeros((Nx, Nx))
  # assign proper values
  for i in range(Nx):
    squareA[i,i]= Adiag[i] 
    if (i-1) >=0 : squareA[i-1,i]= Ai_m1[i-1] 
    if (i+1) < Nx : squareA[i+1,i]= Ai_p1[i]
  ''' # solution by a different method:  
  vals, vecs = sparse.linalg.eigs(squareA, k = Neig, which ='SM')
  # some tests:
  # vals = eigenvals3(ones(Nx), zeros(Nx-1) , 3)   # should give 1,1,1...
  # vals = eigenvals3(2.* ones(Nx), -1.* ones(Nx-1) , 3)  # should give [ 0.00096744  0.00386881  0.0087013 ]
  print vals.real * har_eV
  # print vecs[:, 0].real
  '''
  Evals = eigenvals3(Adiag, Ai_p1 , Neig)  # calculate the eigenenergies 
  # print 'Computed eigenenergies are: (in atomic units)'
  Elevels_eV = Evals* har_eV
  # print 'Computed eigenenergies are: (in eV) with eigenvals3'
  # print Evals * har_eV
  t_eV = (Evals[1] - Evals[0]) * har_eV
  print ''
  print 'DB-DB distance=', 0.1* x_DB2 , ' nm'
  print 'Gate charge: ', Qp1
  sys.stdout.flush()
  # print 'Computed eigenenergies are: (in eV) with scipy.sparse'
  # exit()
  # Plot potential:
  plt.subplot(subplotstring1)
  if Qp1 == 0. or Ut_lin ==0.:
    plt.plot(X, V_au * har_eV, 'k--', linewidth = 2.,  label = r'$V_{well}$')
  elif Qp1 == 77e-7 and Ut_lin != 0.:
    plt.plot(X, V_au * har_eV, 'm--', linewidth = 2.,  label = r'biased $V_{well}$')    
  elif Qp1 < 0.:  
    plt.plot(X, V_au * har_eV, 'm--', linewidth = 2.,  label = r'gated $V_{well}$')
  elif Qp1 > 0.:
    plt.plot(X, V_au * har_eV, 'g--', linewidth = 2.,  label = r'gated $V_{well}$')
    
  plt.xlabel(r'Distance, $x$ [nm]')
  plt.ylabel(r'Energy, $E$ [eV]')
  
  # Computing the eigenfunctions
  psi_well = zeros((Nx, Neig))
  E_plusU = zeros((Evals.size)) 
  dX = X[1] - X[0]  # in nm.
  fc0 = 0.1
  for i in range(Neig):
    s = Evals[i] * 1.0000001 # Shift very close to eigenvalue; does it impact convergence ??
    lam, Psi = inversePower3(Adiag, Ai_p1, s) # Compute eigenvector [x]
    psi_well[:,i] = Psi # Place [v] in array 
    int2 = sum(psi_well[:,i] **2)* dX
    psi_well[:,i] /= sqrt(int2)
    # plt.plot(X, Psi + Evals[i]* har_eV, linewidth = 2., label = 'psi'+str(i))
    ## THIS IS FOR THE CASE WHEN THE SINGLE-WELL WAS FITTED FOR DB- STATE
    if well_loc.size == 1 : E_plusU[i] = Evals[i]* har_eV  
    if well_loc.size == 2 : E_plusU[i] = Evals[i]* har_eV + i * U_intra2
    '''
    ## THIS IS FOR THE CASE WHEN THE SINGLE-WELL WAS FITTED FOR DB0 STATE
    # if well_loc.size == 1: E_plusU[i] = Evals[i]* har_eV + U_intra1 /2. 
    # if well_loc.size == 2: E_plusU[i] = Evals[i]* har_eV + (i+1) * U_intra2 /2.
    '''

    ## plt.plot(X, 5* Psi **2 + E_plusU[i], linewidth = 2., label = r'$\psi^2$('+str(i)+')')
    ## plt.plot([X[0], X[-1]], [E_plusU[i], E_plusU[i]], '--', linewidth = 2., label = 'E'+str(i))
    ## Plot levels and Wavefunctions:
    if Ut_lin != 0.:
        if i == 0: col = 'blue'
        else: col = 'red'
        plt.plot(X, fc0* psi_well[:,i]**2 + Evals[i]* har_eV, linewidth = 2., label = r'$\psi^2$('+str(i)+')', color = col)
        plt.plot([X[0], X[-1]], [Evals[i]* har_eV, Evals[i]* har_eV], ':', linewidth = 2., color = col)
  # Plot e- at just the upper level  
  if Qp1 == Qp2 and Ut_lin == 0.:  
    if well_loc.size == 2 :
      if Qp1 == 0.:
        plt.plot([well_loc[0], well_loc[1]], [Evals[1]* har_eV, Evals[1]* har_eV], 'o:', color='black', linewidth = 1.)#, label = 'E'+str(i))
      elif Qp1 < 0.:                                                                                                     #
        plt.plot([well_loc[0], well_loc[1]], [Evals[1]* har_eV, Evals[1]* har_eV], 'o:', color='m',     linewidth = 1.)#, label = 'E'+str(i))
      elif Qp1 > 0.:                                                                                                     #
        plt.plot([well_loc[0], well_loc[1]], [Evals[1]* har_eV, Evals[1]* har_eV], 'o:', color='black', linewidth = 1.)#, label = 'E'+str(i))

  '''
  # plot yet another 2nd level with higher occupation = 4e-
  E_4e = Evals[1]* har_eV + 3.* U_intra2 /2.
  plt.plot([X[0], X[-1]], [E_4e, E_4e], '--', linewidth = 2., label = 'E_4e')
  '''
  Yo = -0.   # y locations of DB symbols in the plot below
  # plot Ef; @@@!!! CAUTION HERE!!! work in progress for p-type
  Ef_plotted = Ef_eV_CBM
  if lam3 < 0.1:
    # this is for n-type
    if Qp1 ==0 : plt.plot([X[0], X[-1]], [Ef_plotted, Ef_plotted], 'r-.', linewidth = 2., label = r'Fermi level')
  else:  
    # this is for p-type
    Ef_plotted = -0.8
    if Qp1 ==0 and Ut_lin == 0.: plt.plot([X[0], X[-1]], [Ef_plotted, Ef_plotted], 'r-.', linewidth = 2., label = r'Fermi level')
  # Plot DB locations 
  # plt.plot([xP[0]], [Yo], 'o', color = 'r')  # perturber
  # plt.plot([well_loc[0]], [Yo], 'o', color = 'k')
  # if well_loc.size > 1:
    # plt.plot([well_loc[1]], [Yo], 'o', color = 'k')

  sys.stdout.flush()
  
  # print "\nEigenvectors:\n", psi_well
  plt.xlim(-2.5, 2.5)
  if Qp1 < 0.:
    plt.ylim(-1.2, 1.2)
  else:
    plt.ylim(-1.2, 0.2)
  plt.xlabel(r'Distance, $x$ [nm]')
  plt.ylabel(r'Energy, $E$ [eV]')
  # plt.title(r'$E_1 - E_0=$'+ fs(1e3*(E_plusU[1]- E_plusU[0]))+ 'meV ('+nf((qe *(E_plusU[1]- E_plusU[0]))/ h_bar)+' sec-1), for DB-Pair at ['+ fs(-well_loc[0])+','+fs(well_loc[0])\
  plt.title(r'$\Delta=$'+ nf(1e3*(E_plusU[1]- E_plusU[0]))+ 'meV (T='+nf(h_P / (qe * t_eV))+r' s), for DB-Pair at ['+ fs(-well_loc[0])+','+fs(well_loc[0])+ '] nm')
           # +',\n perturber DB at ='\
           # +fs(xP[0])+'nm, with charge '+str(Qp1)+'e')
           # +'nm \n lam3 ='+fs(lam3)+', alpha='+fs(alf1)+' nm')
  # plt.title(r'$E_1 - E_0=$'+ fs(t_eV)+ 'eV, for params: sigma='+ fs(sigm1)+'nm')
  plt.legend(loc='lower right')
  plt.minorticks_on()
  
  if well_loc.size == 1:
    if well_loc[0]==0.: PSI1 = psi_well[:,0]
    if well_loc[0]> 0.: PSI2 = psi_well[:,0]

  if Qp1 != 77e-7 and Ut_lin ==0.:  
      plt.subplot(212)
      str2 = 'Qp_total='+ str(2.*Qp1) + '\n' \
          +'Q.Double-Well Tunnel splitting='+ nf(t_eV * 1e3)+ 'meV\n'          \
          +'Q.Double-Well Tunnel rates = '+ nf((qe * t_eV)/ h_bar)+ ' rad/sec \n'   \
          +'Q.Double-Well Tunnel Period = '+ nf(h_P / (qe * t_eV))+ ' s\n'         \
          +'WKB Tunnel splitting='+ nf(t_WKB_eV * 1e3)+ 'meV\n'                    \
          +'WKB Tunnel rates = '+ nf((qe * t_WKB_eV)/ h_bar)+ ' rad/sec\n'         \
          +'WKB Tunnel Period = '+ nf(h_P / (qe * t_WKB_eV))+ ' s\n'
      
      if Qp1 == 0.:
        plt.text(0.1, 0.5, str2, fontsize = 8)  # data coords are controled by xlim and ylim
      elif Qp1 != 0.:
        plt.text(0.1, 0.0, str2, fontsize = 8)
      
      # horizontalalignment='center', verticalalignment='center', transform=plt.transAxes)

      if Qp1 == Qp2:
        str3 = 'Modified_barrier_DBP_separ_'+str(abs(2.*well_loc[0]))+'nm_QpTotal_'+str(2.*Qp1)+'.pdf'
      else:
        str3 = 'Modified_tilt_DBP_separ_'+str(abs(2.*well_loc[0]))+'nm_QpTotal_'+str(2.*Qp1)+'.pdf'
      plt.savefig(str3, dpi = 150)  
      return  Elevels_eV, psi_well
  ## @@@ !!!
  # plot total charge densities
  
  if Ut_lin == 0.: return  Elevels_eV, psi_well
  plt.subplot(212)
  fc1 = 1.
  # if well_loc.size > 1:  
  # if well_loc.size > 1 and Qp1 != 0. :  
  if well_loc.size > 1 :  
    totCharge1e = 1.*psi_well[:,0]**2
    totCharge2e = 2.*psi_well[:,0]**2
    totCharge3e = 2.*psi_well[:,0]**2 + psi_well[:,1]**2
    print 'Test the psi0^2 Normalization:', sum(psi_well[:, 0]**2 * dX)
    print 'Test the psi1^2 Normalization:', sum(psi_well[:, 1]**2 * dX)
    # Get partial charges on each well
    leftSide = where(X <= 0.)
    rightSide = where(X > 0.)
    leftCharge1e = sum(psi_well[leftSide, 0]**2 * dX)
    rightCharge1e = sum(psi_well[rightSide, 0]**2 * dX)
    leftCharge2e = sum(totCharge2e[leftSide] * dX)
    rightCharge2e = sum(totCharge2e[rightSide] * dX)
    leftCharge3e = sum(totCharge3e[leftSide] * dX)
    rightCharge3e = sum(totCharge3e[rightSide] * dX)
    
    print 'LeftCharge with 1e=', leftCharge1e
    print 'RightCharge with 1e=', rightCharge1e
    polariz1e = abs(rightCharge1e - leftCharge1e)
    print 'LeftCharge with 2e=', leftCharge2e
    print 'RightCharge with 2e=', rightCharge2e
    polariz2e = abs(rightCharge2e - leftCharge2e)
    print 'LeftCharge with 3e=', leftCharge3e
    print 'RightCharge with 3e=', rightCharge3e
    polariz3e = abs(rightCharge3e - leftCharge3e)
    
    plt.plot(X, fc1*(totCharge1e), color = 'k', linewidth = 2., label = 'Tot. Charge\n density with 1e\n L='+fs(leftCharge1e)+'e, R='+fs(rightCharge1e)+'e')
    plt.plot(X, fc1*(totCharge2e), color = 'b', linewidth = 2., label = 'Tot. Charge\n density with 2e\n L='+fs(leftCharge2e)+'e, R='+fs(rightCharge2e)+'e')
    plt.plot(X, fc1*(totCharge3e), color = 'r', linewidth = 2., label = 'Tot. Charge\n density with 3e\n L='+fs(leftCharge3e)+'e, R='+fs(rightCharge3e)+'e')
  
    # Plot a box of charge +2e
    plt.plot([-1,-1,1,1], [0,1,1,0], 'y--')
    # Plot DB locations 
    plt.plot([xP[0]], [0.], 'o', color = 'r')
    plt.plot([well_loc[0]], [0.], 'o', color = 'k')
    if well_loc.size > 1:
      plt.plot([well_loc[1]], [0.], 'o', color = 'k')
      
    plt.xlim(-2.5, 2.5)
    # plt.ylim(-0.1, 0.2)
    plt.xlabel('x [nm]')
    plt.ylabel('Charge density [au]')
    plt.legend(loc='upper right')
    plt.minorticks_on()
 
    st1= ('Total charge densities: \n Net polarization with 1e: '+nf(polariz1e)+'\n Net polarization with 2e: '+nf(polariz2e)\
              +'\n Net polarization with 3e: '+nf(polariz3e) )
    print st1               

    str3 = 'Modified_tilt_DBP_separ_'+str(abs(2.*well_loc[0]))+'nm_QpTotal_'+str(2.*Qp1)+'_Ut_lin='+str(Ut_lin)+'V.pdf'
    plt.savefig(str3, dpi = 150) 
  return Elevels_eV, psi_well
  # plt.show()


def PlotConfig(iconf, xylabels = False):
    '''
    this plots a config
    Input: iconf, xylabels = False
    Output: actual popup graphs on screen, and/or saved in folder
   
    '''
    # globals needed: xDBarray, yDBarray, iCellInFlexArr, QCAcellArr, xDBarrayVirt, yDBarrayVirt, Nflex, font1
    
    for icell in arange(QCAcellArr.size):
        for idb in arange(2):
            cir1 = pylab.Circle((xDBarray[icell, idb], yDBarray[icell, idb]), radius = 0.2* xDB2 /15, facecolor='k', fill= True)
            pylab.gca().add_patch(cir1)

    # draw extra electrons
    for icell in arange(QCAcellArr.size):
        if QCAcellArr[icell] == 1:
            for idb in [1]:
                cir1 = pylab.Circle((xDBarray[icell, idb], yDBarray[icell, idb]), radius = 1.3* xDB2 /15, facecolor='r', fill= True)
                pylab.gca().add_patch(cir1)
            plt.plot([xDBarray[icell, idb] for idb in [1,3]], [yDBarray[icell, idb] for idb in [1,3]], 'r')
            #plt.text(xDBarray[icell, 2]+ 1, yDBarray[icell, 2]+ 1, str(iCellInFlexArr[icell]), fontsize= font1, color='r')
        elif QCAcellArr[icell] == 0:
            for idb in [0]:
                cir1 = pylab.Circle((xDBarray[icell, idb], yDBarray[icell, idb]), radius = 1.3* xDB2 /15, facecolor='r', fill= True)
                pylab.gca().add_patch(cir1)
            plt.plot([xDBarray[icell, idb] for idb in [0,2]], [yDBarray[icell, idb] for idb in [0,2]], 'r')

        elif QCAcellArr[icell] == 2 and iconf == -1:  # -1 means "plot the user specified initial config"!!!
            for idb in [0,1,2,3]:
                cir1 = pylab.Circle((xDBarray[icell, idb], yDBarray[icell, idb]), radius = 0.8* xDB2 /15, facecolor='g', fill= True)
                pylab.gca().add_patch(cir1)
            plt.plot([xDBarray[icell, idb] for idb in [0,2]], [yDBarray[icell, idb] for idb in [0,2]], 'g')
            plt.plot([xDBarray[icell, idb] for idb in [1,3]], [yDBarray[icell, idb] for idb in [1,3]], 'g')
            #plt.text(xDBarray[icell, 2]+ 1, yDBarray[icell, 2]+ 1, str(iCellInFlexArr[icell]), fontsize= font1, color='green')
            
        plt.text(xDBarray[icell, 0]+1, yDBarray[icell, 0]+1, str(icell), fontsize= font1, color='gray')

    # ## Writing some distances by the first cell.
    # if iconf == 0:
      # plt.text(xDBarray[0, 0]-1, yDBarray[0, 0]-2, 'rx='+fs(rx)+' A', fontsize= font1, color='k')
      # plt.text(xDBarray[0, 0]-1, yDBarray[0, 0]-3, 'ry='+fs(ry), fontsize= font1, color='k')
      # plt.text(xDBarray[0, 0]-1, yDBarray[0, 0]-4, 'dCC='+fs(dCC), fontsize= font1, color='k')
      # plt.text(xDBarray[0, 0]-1, yDBarray[0, 0]-5, 'dLL='+fs(dLL), fontsize= font1, color='k')
      # plt.text(xDBarray[0, 0]-1, yDBarray[0, 0]-6, 'Lx='+fs(Lx), fontsize= font1, color='k')
      # plt.text(xDBarray[0, 0]-1, yDBarray[0, 0]-7, 'Hy='+fs(Hy), fontsize= font1, color='k')

      # plt.text(xDBarray[1, 0]-1, yDBarray[1, 0]-2, 'Vee(rx)='+ fl(Vee_point_screened(rx, L_debye))+' eV', fontsize= font1, color='k')
      # plt.text(xDBarray[1, 0]-1, yDBarray[1, 0]-3, 'Vee(ry)='+ fl(Vee_point_screened(ry, L_debye)), fontsize= font1, color='k')
      # plt.text(xDBarray[1, 0]-1, yDBarray[1, 0]-4, 'Vee(dCC)='+fl(Vee_point_screened(dCC, L_debye)), fontsize= font1, color='k')
      # plt.text(xDBarray[1, 0]-1, yDBarray[1, 0]-5, 'Vee(dLL)='+fl(Vee_point_screened(dLL, L_debye)), fontsize= font1, color='k')
      # plt.text(xDBarray[1, 0]-1, yDBarray[1, 0]-6, 'Vee(Lx)='+ fl(Vee_point_screened(Lx, L_debye)), fontsize= font1, color='k')
      # plt.text(xDBarray[1, 0]-1, yDBarray[1, 0]-7, 'Vee(Hy)='+ fl(Vee_point_screened(Hy, L_debye)), fontsize= font1, color='k')
      
                     
    if xylabels:
        plt.xlabel(r'x[$\AA$]', fontsize= font1)
        plt.ylabel(r'y[$\AA$]', fontsize= font1)
    # plt.legend(loc='upper left')
    plt.xlim(amin(xDBarrayVirt) -10, amax(xDBarrayVirt)+ 10)
    plt.ylim(amin(yDBarrayVirt) -10, amax(yDBarrayVirt)+ 10)    

def plotDBorbitals_XZplane(what = 'psi', coeffs =[0.7071067812, 0.7071067812] , x0_DB = [-10., 10.], z0_DB = [0., 0.]):
  '''
  ## Print the DB-system wavefunctions or their charge density 
  Inputs:
     what = 'psi', what to plot : 'psi' is the wavefunction, 'rho' is the charge density
     nDB = 2, the number of DBs involved
     coeffs =[0.7071067812, 0.7071067812], mixing coefficients for the extended wavefunction -- to be calculated by MO theory, etc
     x0_DB = [-10., 10.], x-coordinates for the DB centers
     z0_DB = [0., 0.], z-coordinates for the DB centers
  Output:
      no variable, 
        (just a Plot of the wavefunctions or charge density)
  '''
  global DB_STO_Norm 
  
  DB_STO_Norm = 1.
  DB_STO_Norm = DetermineNormalizationForDB_STO_ptype2()
  OmegaDB = DB_volume()   # DB volume [m^3]
  print 'DB_STO_Norm=', DB_STO_Norm
  print 'DB volume =', DB_volume() * 1e30, 'Ang^3'

  X1d = linspace(-20., 20., 400) *0.1        # range limits in nm [my notes on March 3,2010]
  Z1d = linspace(-15., 5., 400)  *0.1        # [nm]
  # Z1d = linspace(15., -5., 400)  *0.1      # [nm]
  
  surfaceLine = 0. * X1d 
  # 2D section plotted here
  # fig2 = plt.figure()
  X, Z = meshgrid(X1d, Z1d)         # Return coordinate [nm] 2D matrices from two coordinate vectors; see below
  # psi2plot = DB_STO_dimmless(X, Z)   # DB_STO_2p(X, Z) * 1e-15

  totalPsi = 0. * DB_STO_2p(X *10., Z *10.)  # as initialization 
  for i, c in enumerate(coeffs):
    psiDBi = coeffs[i] * DB_STO_2p(X *10., Z *10., x0 = x0_DB[i], z0 = z0_DB[i])
    totalPsi += psiDBi
  if what == 'psi':
    what2plot = totalPsi
  elif what == 'rho':
    what2plot = totalPsi **2
  
  Ncont = 40
  NcontLines = 40
  plt.contourf (X, Z, what2plot, Ncont)
  plt.colorbar(spacing='proportional', drawedges=True) #, ticks=tickList1)
  CS1 = plt.contour(X, Z, what2plot, NcontLines, linewidths=0., colors='k')
  # plt.clabel(CS1, inline=0,  fontsize=8)
  # plt.xlim(X1d[0], X1d[-1])
  plt.plot(X1d, surfaceLine, color='black', linewidth= 2)  # draw the surface. 
  x0_DB_nm = array(x0_DB) * 0.1
  z0_DB_nm = array(z0_DB) * 0.1
  plt.plot(x0_DB_nm, z0_DB_nm, 'o', color='black')    # draw DB centers

  plt.ylim(Z1d[0], Z1d[-1])
  plt.xlabel('x [nm]') #, fontsize = 14)
  plt.ylabel('z [nm]') #, fontsize = 14)
  plt.axis('equal')
  # plt.savefig("PsiDBcontours.pdf", dpi = 150)

def plotDBorbitals_XYplane(what = 'psi', Zslice = -5., coeffs =[0.7071067812, 0.7071067812] , x0_DB = [-10., 10.], y0_DB = [0., 0.]):
  '''
  ## Print the DB-system wavefunctions or their charge density 
  Inputs:
     what = 'psi', what to plot : 'psi' is the wavefunction, 'rho' is the charge density
     Zslice = z-coord. where the orbital slice is taken. in [Ang.]
     coeffs =[0.7071067812, 0.7071067812], mixing coefficients for the extended wavefunction -- to be calculated by MO theory, etc
     x0_DB = [-10., 10.], x-coordinates for the DB centers in [Ang.]
     y0_DB = [0., 0.], z-coordinates for the DB centers in [Ang.]
  Output:
      no variable, 
        (just a Plot of the wavefunctions or charge density)
  '''
  global DB_STO_Norm 
  
  DB_STO_Norm = 1.
  DB_STO_Norm = DetermineNormalizationForDB_STO_ptype2()
  OmegaDB = DB_volume()   # DB volume [m^3]
  print 'DB_STO_Norm=', DB_STO_Norm
  print 'DB volume =', DB_volume() * 1e30, 'Ang^3'

  X1d = linspace(-20., 20., 400)        # range limits in nm [my notes on March 3,2010]; 400 points is a good choice !
  Y1d = linspace(-20., 20., 400)        # [A]
  # Y1d = linspace(15., -5., 400)  *0.1      # [nm]
  
  surfaceLine = 0. * X1d 
  # 2D section plotted here
  # fig2 = plt.figure()
  X, Y = meshgrid(X1d, Y1d)         # Return coordinate [nm] 2D matrices from two coordinate vectors; see below
  Z = Zslice    # linspace(Zslice, Zslice, 1) *0.1     # [A]
  # print X
  # print 
  # print Y
  # exit()
  
  
  # psi2plot = DB_STO_dimmless(X, Z)   # DB_STO_2p(X, Z) * 1e-15
  totalPsi = 0. * DB_STO_2p(X, Z )  # as initialization 
  for i, c in enumerate(coeffs):
    Rxy = ((X - x0_DB[i]) **2 + (Y - y0_DB[i]) **2)**0.5   # radial distance in the x-y plane, a 2D array
    psiDBi = coeffs[i] * DB_STO_2p(Rxy , Z)   # this should work, as it feeds only 2D arrays of the same shapes 
    totalPsi += psiDBi
  if what == 'psi':
    what2plot = totalPsi
  elif what == 'rho':
    what2plot = totalPsi **2
  
  Ncont = 40
  NcontLines = 40
  X *= 0.1
  Y *= 0.1
  plt.contourf (X, Y, what2plot, Ncont)
  plt.colorbar(spacing='proportional', drawedges=True) #, ticks=tickList1)
  CS1 = plt.contour(X, Y, what2plot, NcontLines, linewidths=0., colors='k')
  # plt.clabel(CS1, inline=0,  fontsize=8)
  # plt.xlim(X1d[0], X1d[-1])
  # plt.plot(X1d, surfaceLine, color='black', linewidth= 2)  # draw the surface. 
  x0_DB_nm = array(x0_DB) * 0.1
  y0_DB_nm = array(y0_DB) * 0.1
  plt.plot(x0_DB_nm, y0_DB_nm, '+', color='black')    # draw DB centers

  plt.ylim(Y1d[0], Y1d[-1])
  plt.xlabel('x [nm]') #, fontsize = 14)
  plt.ylabel('y [nm]') #, fontsize = 14)
  plt.axis('equal')
  plt.title('Orbital slice at z= '+fs(Z*0.1)+' nm')
  # plt.savefig("PsiDBcontours.pdf", dpi = 150)

def db_energy_shifts(DBsXlist= [0.,1.], DBsYlist= [0., 0.], Xtip = 0., Ytip = 0., Htip = 0.5, QconfigList=[[1,1], [0,1]], skipPlots = True):

  '''
  INPUT:
  DBsXlist = [x0, x1, ...]  = a list of x-coordinates for DBs, in [nm]; NB: make x0 = 0. 
  DBsYlist = [y0, y1,....]  = a list of y-coordinates for DBs, in [nm]; NB: make y0 = 0. 
  Xtip = x-coordinate of tip apex, in [nm], wrt the first DB in the list
  Ytip = y-coordinate of tip apex, in [nm], wrt the first DB in the list
  Htip = z-coordinate of tip apex wrt to the surface, in [nm]; this is also referred to as "absolute tip height"
  QconfigList = a list of charge configurations of the DBs, i.e. a list of lists, e.g.  [[1, 1,0, 0,0, 1], [1, 0,1, 0,0, 1]]
  
  OUTPUT:
  Edb_total_wrtEf[iconf, :] = array of DB energies, wrt the sample Fermi level, for each configuration iconf in QconfigList; in units of eV.  If the DB is not doubly occupied, the return is 0.
  '''
  if len(DBsXlist) != len(DBsYlist):
    print 'There is a problem with the number of Xdb and Ydb coordinates in the arguments of db_energy_shifts'
    exit()
     
  ## I need a table of DB-DB distances and interactions here. 
  DBsXarray = array(DBsXlist)
  DBsYarray = array(DBsYlist)
  rx12 = zeros((DBsXarray.size, DBsXarray.size), dtype =float64)
  Uee12 = zeros((DBsXarray.size, DBsXarray.size), dtype =float64)
  # Edb_total_wrtEf = zeros((DBsXarray.size))
  Edb_total_wrtEf = zeros((len(QconfigList), DBsXarray.size))
  CPIBB = zeros((DBsXarray.size))
  Ximg = zeros((DBsXarray.size)) 
  Yimg = zeros((DBsXarray.size)) 
  Zimg = zeros((DBsXarray.size)) 
  Qimg = zeros((DBsXarray.size)) 
  TunRate = zeros((DBsXarray.size))
  for idb1, xdb1 in enumerate(DBsXlist):
    for idb2, xdb2 in enumerate(DBsXarray[:idb1]):
      # rx12[idb1, idb2] = abs(DBsXarray[idb1] - DBsXarray[idb2])  # in nm
      rx12[idb1, idb2] = r0 + ((DBsXarray[idb1] - DBsXarray[idb2])**2 + (DBsYarray[idb1] - DBsYarray[idb2])**2)**0.5 # in nm
      # Uee12[idb1, idb2] = KKc1 / rx12[idb1, idb2] # Kc1 * qe / (rx12[idb1, idb2]*1e-9)  # in eV
      Uee12[idb1, idb2] = KKc1* erf(rx12[idb1, idb2]/ sqrt2sig_DB)/ rx12[idb1, idb2] # Kc1 * qe / (rx12[idb1, idb2]*1e-9)  # in eV
      Uee12[idb2, idb1] = Uee12[idb1, idb2]
      # print idb1, idb2, rx12[idb1, idb2], Uee12[idb1, idb2] , KKc1* erf(rx12[idb1, idb2]/ sqrt2sig_DB)/ rx12[idb1, idb2]
   
  # xlGrid = linspace(DBsXarray[0] - 1., DBsXarray[-1] + 1., 400)  # this is the x-grid for the effective potential plot
  xlGrid = DBsXarray 
  ylGrid = 0.   # this can be made an array later
  # xlGrid = linspace(xl2 - 0.5, xl3 + 0.5, 400)   # only the first pair shown.

  # Xtip = 5.   # location of tip apex in nm
  DW_ts = 0.65  # actual difference in work functions of tip and sample
  Rtip = 7.      # actual tip radius in nm, to be fit to the experiment
  tipDwellTime = 1e-3   # [s], how long the tip spends at each pixel in the image
  ## Rescale the TIBB potential with the actual tip radius: 
  pw0 = 1.01    # small power law to be used heuristically below in TIBB
  pw1 = 1.1
  dcy0 = .5  # scaling for the gaussian decay to be used heuristically below in TIBB
  aR = 1.
  # TIBB_H0_r *= ((Rtip + Htip)/ (Rtip0 + Htip))*(1. + (((Rtip0 + Htip)/(Rtip + Htip))**pw0 -1.)* exp(- (dcy0 * rx / Rtip)**1.2))
  TIBB_H0_r = femTIBB_H0_r * ((Rtip + Htip)/ (Rtip0 + Htip))**pw0 * (1. + (((Rtip0 + Htip)/(Rtip + Htip)) -1.)* exp(- (dcy0 * rx / Rtip)**pw1))   # this form is OK, but only for ranges up to 10 nm laterally; after that, it doesn't decay properly,

  if Rtip >= 200.: 
    ## FLAT TIP APPROXIMATION
    TIBB_H0_r = TIBB_H0_r[0] + 0.* TIBB_H0_r 
    
  TIBBr0fit3 = coefs[0]* Htip**3 + coefs[1]* Htip**2 + coefs[2]* Htip + coefs[3]  # TIBB at r = 0, fit by 3-rd order poly.
  # print 'TIBB_r=0 (fit Or3)=', TIBBr0fit3
  # print 'TIBB_H0_[r=0]=', TIBB_H0_r[0]
  TIBB_H_r = TIBB_H0_r * (TIBBr0fit3 / TIBB_H0_r[0]) * (DW_ts / DW0_ts)   # scaling due to tip height and due to contact potential

  Xtip_grid = [Xtip]  # linspace(xlGrid[0], xlGrid[-1], 10)   # define the grid where AFM tip will be rastered 
  # DBsQlist = [1, 0.5,0.5, 0.5,0.5, 1]   # a list with charges on each DB in units of -qe.

  # print QconfigList, len(QconfigList)
    
  for iconf, Qconf in enumerate(QconfigList):
    Xtip_wStableConfig = []
    GrandPoten4StableConfig = []  # start two lists containing the stable configurations, to be plotted later. 
    # charge configuration of the DB system, as an array 
    DBsQarray = array(Qconf) 

    TunRate[...] = 0.
    Edb_total_wrtEf[iconf, :] = 0.
    for idb1, xdb1 in enumerate(DBsXlist):
      # this is the DB where I'm calculating the total energy.
      # if  DBsQarray[idb1] == 0: continue
      phi_fromAllDBm = 0.  # initializing the potential felt by CB due to all negative DBs
      phi_fromAllImg = 0.  # initializing the potential felt by CB due to all image charges
      Xfrom_apex = abs(xdb1 - Xtip)   # a scalar 
      Yfrom_apex = abs(DBsYarray[idb1] - Ytip)    # a scalar 
      RxyFromApex = (Xfrom_apex **2 + Yfrom_apex **2)**0.5
      TIBBr_shifted = interp(RxyFromApex, rx, TIBB_H_r)
      a = ((Rtip + Htip)**2 + (xdb1 - Xtip)**2 + (DBsYarray[idb1] - Ytip)**2)**0.5  # in nm
      b = Rtip **2 / a
      d = a - Rtip          # the shortest distance DB-tip  # nm
      TunRate[idb1] = TunRate0 * exp(- k_tip_DBm * d)   # tunneling rate DB-tip [Hz]      
    
      for idb, xdb in enumerate(DBsXlist):
        if  DBsQarray[idb] != 0:
          # Calc. image charge values and location for this DB
          a = ((Rtip + Htip)**2 + (xdb - Xtip)**2 + (DBsYarray[idb] - Ytip)**2)**0.5  # in nm
          b = Rtip **2 / a
          d = a - Rtip          # the shortest distance DB-tip  # nm
          # TunRate[idb] = TunRate0 * exp(- k_tip_DBm * d)   # tunneling rate DB-tip [Hz]      
          Ximg[idb] = xdb - (a - b)* (xdb - Xtip) / a  # in nm
          Yimg[idb] = DBsYarray[idb] - (a - b)* (DBsYarray[idb] - Ytip) / a  # in nm
          Zimg[idb] = (a - b) * (Rtip + Htip) / a   # in nm
          Qimg[idb] = + (Rtip / a)    # in units of qe, positive bc/ DBs are negative
          # if idb == 0 : print 'img charges:'
          # if  DBsQarray[idb] != 0: print idb, Ximg[idb], Zimg[idb], Qimg[idb]/ qe 
          RfromImg = ((xdb1 - Ximg[idb])**2 + (DBsYarray[idb1] - Yimg[idb])**2 + (Zimg[idb])**2) **0.5  # array, same as xlGrid; in nm
          phi_fromAllImg += KKci * Qimg[idb] / RfromImg * DBsQarray[idb] # this potential has positive sign because img. charges are positive
          if idb != idb1: 
            X12 = abs(xdb1 - xdb)   # in nm
            Y12 = abs(DBsYarray[idb1] - DBsYarray[idb])   # in nm
            Rxy12 = r0 + (X12 **2 + Y12 **2)**0.5   # this only works for a scalar ylGrid.
            PhiDB_gaussian = - KKc1 * erf(Rxy12 / sqrt2sig_DB)/ (Rxy12)  # potential of a DB assuming it has a Gaussian charge cloud 
            phi_fromAllDBm += PhiDB_gaussian * DBsQarray[idb] 
      # PEfeltByCBe_fromAllDBm = TIBBr_shifted - phi_fromAllDBm # - phi_fromAllImg # minus in the LHS here is bc/ I converted potential [V] into energy [eV]
      totalPEfeltByDB = TIBBr_shifted - phi_fromAllDBm - phi_fromAllImg # minus in the LHS here is bc/ I converted potential [V] into energy [eV]
      Edb_total_wrtEf[iconf, idb1] = E_DB_m_bulkCB_eV + totalPEfeltByDB - Ef_eV_CBM   # in eV
      Edb_total_wrtEf[iconf, idb1] *=  DBsQarray[idb1]   # account for the actual charge on the DB.
  return Edb_total_wrtEf  
  
###############################################  
## MAIN STARTS HERE ###########################
###############################################  
  
##  Physical constants  
h_P= const.h
h_bar = h_P / 2./ pi
h_bar2 = h_bar **2
me= const.m_e
qe= const.e
a0B = 5.2917720e-11   # Bohr radius [m]
har_eV = 27.211385   # hartree over eV
M2h2 = 2 * me / h_bar2
sqrt2mh2 = sqrt(M2h2)
eps0= const.epsilon_0
m_eff_n = 1.08   # effective mass for density of states [bart van Zeghbroeck - online book]
                 # from [green2] #   0.26  mass for conductivity calcl.
m_eff_p = 0.81   # effective mass for density of states
  # 1.15  # 0.57
Eg_eV = 1.17   # in [eV] at low T !!
Eg_J = Eg_eV * qe                          # band gap in [J] for Si at T=300 K from [green1]
W_tip = 4.5                                # work function of W in [eV]
Xaff_eV = 4.05                             # Electron affinity of our Semiconductor type [eV]
rho_Si_g_cm3 = 1.90                      # Si density in g/cm^3
rho_Si = 4.07e28                         # Si density in atoms/m^3
Evbm_J = 0.                                 # valence band top in [J]
Ecbm_J = Evbm_J + Eg_J                      # conduction band bottom in [J]
E_DB_0_bulkCB_J = - 0.77 * qe
E_DB_m_bulkCB_J = - 0.32 * qe
E_DB_0_bulkVB_J = Eg_J - E_DB_0_bulkCB_J  # 0.35 * qe                # neutral DB level [J] in bulk wrt VBM
E_DB_m_bulkVB_J = Eg_J - E_DB_m_bulkCB_J  # 0.82 * qe                # negative DB level [J] in bulk wrt VBM
# Ed_wrtCBM_J = - 0.045 * qe               # P donor energy in Si wrt Ecbm_J in  [J] 
Ed_wrtCBM_J = - 0.054  * qe               # As donor energy in Si wrt Ecbm_J in     [J] 
# Ed_wrtCBM_J = - 0.043  & qe              # Sb donor energy in Si wrt Ecbm_J in [J] 
  
W_DBm_vac = (1./ qe)*((Xaff_eV + Eg_eV)* qe - E_DB_m_bulkVB_J)  # in eV
W_DB0_vac = (1./ qe)*((Xaff_eV + Eg_eV)* qe - E_DB_0_bulkVB_J)
W_DBm_CB  = (1./ qe)*(Ecbm_J - E_DB_m_bulkVB_J)  # in J
W_DB0_CB  = (1./ qe)*(Ecbm_J - E_DB_0_bulkVB_J) # in J
  
# Fermi level at low T and for non-degenerate case ONLY !!
Ef = Ecbm_J + Ed_wrtCBM_J / 2.   # in J, wrt VBM  
Ef_eV_CBM = (Ef - Ecbm_J) / qe   # Ef in [eV]
  
## Adjustable model parameters:  
# Define some quantities for the Hubbard parameters.
# THIS IS THE RAW DATA COLLECTED BY VARIOUS WAYS.
#         DB-DB distance,     t,         e-e,      n-e,    point e-e
DB_param_raw= array([\
             [        0.,     0.,        0.49539, 0.43930,         0. ],
             [   3.84000, 0.3077,        0.37409, 0.34197,    0.59522 ], 
             [   7.68000, 0.0878,        0.23848, 0.24216,    0.29761 ], 
             [  11.52000, 0.0446,        0.16581, 0.18171,    0.19841 ], 
             [  15.36000, 0.0193,        0.12683, 0.14377,    0.14881 ], 
             [   9.93000, 0.0606,        0.18965, 0.20317,    0.23018 ], 
             [  11.73767, 0.0427,        0.16300, 0.17908,    0.19473 ], 
             [  15.83273, 0.0173,        0.12324, 0.14010,    0.14436 ], 
             [  17.17300, 0.0113,        0.11406, 0.13059,    0.13310 ], 
             [  19.20000, 0.0047,        0.10249, 0.11832,    0.11904 ], 
             [  19.58023, 0.0042,        0.10058, 0.11626,    0.11673 ], 
             [  20.67903, 0.0030,        0.09541, 0.11068,    0.11053 ],
             [  23.04000, 0.0011,        0.08592, 0.10026,    0.09920 ],
             [  12.55338, 0.0359,        0.15321, 0.16982,    0.18208 ],
             [  15.20905, 0.0199,        0.12802, 0.14498,    0.15028 ],
             [  10.86116, 0.0509,        0.17496, 0.19009,    0.21044 ],
             [  17.61000, 0.0094,        0.11136, 0.12775,    0.12979 ],
             [  26.43041, 0.0002,        0.07515, 0.08824,    0.08648 ],
             [  19.21183, 0.0047,        0.10243, 0.11826,    0.11897 ],
             [  25.29000, 0.0004,        0.07846, 0.09196,    0.09038 ],
             [  26.88000, 0.00025,       0.07392, 0.08686,    0.08503 ],
             [  38.01406, 0.0   ,        0.05255, 0.06237,    0.06013 ],
             [  30.72000, 0.0   ,        0.06484, 0.07654,    0.07440 ],
             [  43.44464, 0.0   ,        0.04604, 0.05478,    0.05261 ],
             [  34.56000, 0.0   ,        0.05773, 0.06838,    0.06614 ],
             [  48.87522, 0.0   ,        0.04096, 0.04882,    0.04677 ],
             [  38.40000, 0.0   ,        0.05202, 0.06177,    0.05952 ],
             [  54.30580, 0.0   ,        0.03689, 0.04403,    0.04209 ],
             [  42.24000, 0.0   ,        0.04734, 0.05631,    0.05411 ],
             [  59.73638, 0.0   ,        0.03355, 0.04008,    0.03826 ],
             [  46.08000, 0.0   ,        0.04343, 0.05172,    0.04960 ],
             [  65.16696, 0.0   ,        0.03077, 0.03678,    0.03507 ],
             [  49.92000, 0.0   ,        0.04011, 0.04782,    0.04579 ],
             [  70.59754, 0.0   ,        0.02841, 0.03398,    0.03238 ],
             [  53.76000, 0.0   ,        0.03726, 0.04447,    0.04252 ],
             [  76.02812, 0.0   ,        0.02638, 0.03157,    0.03006 ],
             [  57.60000, 0.0   ,        0.03479, 0.04155,    0.03968 ],
             [  81.45870, 0.0   ,        0.02463, 0.02948,    0.02806 ],
             [  61.44000, 0.0   ,        0.03262, 0.03898,    0.03720 ],
             [  86.88928, 0.0   ,        0.02309, 0.02765,    0.02631 ],
             [  100.0000, 0.0   ,        0.02007, 0.02405,    0.02286 ],
             [  150.0000, 0.0   ,        0.01339, 0.01605,    0.01524 ],
             [  200.0000, 0.0   ,        0.01004, 0.01205,    0.01143 ],
             [  76.80000, 0.0   ,        0.02612, 0.03126,    0.02976 ]]) 
             
## plt.plot(DB_param_raw[1:,0], DB_param_raw[1:,1], 'ro')
## plt.show()
## exit()             
## Define the coordinates of DBs in a cell 
# distances [Ang]
da= 3.84        # along a dimer
ds= 5.43        # gap between dimers perpendicular to dimer row
di= 2.25        # DB-DB spacing inside a dimer

x_DB1 = 0. 
y_DB1 = 0.
x_DB2 = 2.* da  # for TESTING
# x_DB2 = 2.* da + ds # Qubit geometry 1
# x_DB2 = 4.* da + di #  Qubit geometry 2
# x_DB2 = 6.* da + ds #  Qubit geometry 3
y_DB2 = 0.
xCC   = 5* da 
yCC   = 0.

x_DB3 = x_DB2 + xCC
y_DB3 = y_DB2 + yCC
x_DB4 = 2.*(x_DB2 - x_DB1) + xCC
y_DB4 = y_DB3

#v check cells
if sqrt(yCC **2 + xCC **2) < x_DB2:
  print 'Dot spacing inside a cell is chosen to be GREATER than cell-cell spacing!'
  print 'Please revise the values (x_DB2, yCC)'
  exit('Stopped.')


############ Model    parameters ###########################
Temp = 4                    # temperature in K
kT = const.k * Temp / qe             # kT in [eV]
# kT= 25.8e-3* Temp/ 300.       # kT in eV
U_intra1 = 0. # 0.495 # 0.42           # Hubbard-U for a SINGLE DB in eV
## THIS IS A CRITICAL PARAMTER TO ESTIMATE CORRECT FILLING. DO CALCULATE IT FROM MY ORBITALS AS CORRECTLY AS POSSIBLE.
U_intra2 =  U_intra1 / 3. # 0.42           # Hubbard-U for a SINGLE DB in eV
U_intra = [U_intra1, U_intra2]   # a list of these U
Eo = 0.36  # 0.35          # self energy in eV; ALL energies WRT Silicon VB at infin.
chempot = 1.05      # 0.95   # chemical potential in eV
Eo -= chempot     # henceforth, the site energy and the final Hubbard energies are 
                  # WRT the chemical potential (not the VBM !)
nsite = 3
Ncells = 200  # the number of the experimental configs. of DBs    
Npol = 400   # number of discretization Intervals for Cell Polarization
dP = 2./ Npol  # size of discrete step in P 
iPrange = arange(0, Npol+1)   # range for polarization indices
PolTable = -1. + dP * iPrange  # table of polarization values           
## print 'Self-energies, isolated DB= {0}'.format (E0[1:nsite+1])
# DB spacing in Ang.
long_output= False        # if you want all details of results
probab_cutoff= 0.01 #1e-2        # displays only configurations with probability higher than this
V_ne_accounted = True     # this is a switch for different modes of the model - accounting for 
                          # the full interactions, including V_ne or not.
screened_coul = False    # account for screening
surf_diel= 6.3
L_debye= 25.              # Debye screening length in Ang.      
tunnel_scaling= 1.       
V_BB_tip_uniform= 0.22    # uniform potential shift of DB level due to biased STM tip         
#############################################################
#############################################################
R_large= max(DB_param_raw[:,0])         # max(DB_param_raw[:,0])
# Sort and interpolate input data       
A= sort(DB_param_raw[:,0]) 
iA= argsort(DB_param_raw[:,0])      
# An array from an interpolation of parameters above        
DB_param_sorted= zeros((shape(DB_param_raw)[0], shape(DB_param_raw)[1]))          
for j in range(0, shape(DB_param_raw)[1]):
  DB_param_sorted[:, j]= DB_param_raw[iA, j]
## # test sorted array  
## for i in range(1, shape(DB_param_raw)[0]):
##   print str(DB_param_sorted[i,0]).ljust(14), \
##         str(DB_param_sorted[i,1]).ljust(14), \
##         str(DB_param_sorted[i,2]).ljust(14), \
##         str(DB_param_sorted[i,3]).ljust(14)

# prepare interpolation details  
r_DB_input= zeros((shape(DB_param_raw)[0])) 
r_DB_input= DB_param_sorted[:,0]
dr= 0.005 # distance interval in Angs. 
r_DB_interp= r_[min(DB_param_sorted[1:,0]): max(DB_param_sorted[1:,0]): dr]  # distance grid in [Ang.]
# print shape(DB_param_sorted) 
DB_param_interp= zeros((size(r_DB_interp), shape(DB_param_raw)[1]))
#cj= cspline1d (DB_param_sorted[:, 1])
# DB_param_interp[:,1]= cspline1d_eval(cj, r_DB_interp, dx= dr, x0= r_DB_interp[0])
#DB_param_interp[:,1]= cspline1d_eval(cj, r_DB_input, dx= dr, x0= r_DB_interp[0])

# Interpolate using numpy.interp 
for j in range(0, shape(DB_param_raw)[1]):
  DB_param_interp[:,j] = interp(r_DB_interp, r_DB_input, DB_param_sorted[:,j])  
  
# # settle the origin value
# if DB_param_sorted[0,0]== 0.:
  # DB_param_interp[0,:]= 0.
  
QCA_cells= [40]
# print 'R_large=', R_large, 'A'
UP_symb=   ['o', 'U']  # symbols used for spin UP electrons; first symb. is for occup. number=0, second for 1
DOWN_symb= ['o', 'D']  # symbols used for spin DOWN electrons

# DB_param=  DB_param_raw.copy()  # when you want to copy data you
DB_param=  DB_param_interp.copy()  # when you want to copy data you
                                # must explicitly ask for a copy by some means; 
                                # otherwise you'll have 2 handles for the same variable (memory address)
DB_param[:,1] *=  0.5   # CAUTION: BECAUSE  t is HALF THE TUNNEL SPLITTING
DB_param[:,1] *=  tunnel_scaling   # An arbitrary scaling of this parameter; justified to match experiment
# print DB_param[:,1]
# # DB_param[:, 2:5] *= 6.3/ surf_diel   # RESCALING THE COULOMB INTERACTIONS
                                    # BY THE DIELECTRIC CONSTANT OF THE SURFACE
if screened_coul:
  DB_param[:, 2] *= exp(- DB_param[:,0]/ L_debye)
  #print 'Screening by factors:', exp(- DB_param[:,0]/ L_debye)
  DB_param[:, 3] *= exp(- DB_param[:,0]/ L_debye)
  DB_param[:, 4] *= exp(- DB_param[:,0]/ L_debye)

# f_param= open ('Model_params.dat', 'w')   
# write out the parameters (interpolated)
# for i in range(0,size(r_DB_interp)):
  # s4= str(r_DB_interp[i]).ljust(18)+ str(DB_param[i,1]).ljust(18)+ \
        # str(DB_param[i,2]).ljust(18)+ str(DB_param[i,3]).ljust(18)+ '\n'
  # f_param.write(s4)  
# f_param.close() 

# Printing out some parameters              
DB_param_raw[:,2] *= exp(- DB_param_raw[:,0]/ L_debye)                     
DB_param_raw[:,3] *= exp(- DB_param_raw[:,0]/ L_debye)                     
DB_param_raw[:,4] *= exp(- DB_param_raw[:,0]/ L_debye)                     
ind2= argsort(DB_param[1:,0]) 
'''                                  
# print sort(DB_param_raw[1:, 0])[0:7]
# print sort(DB_param_raw[1:, 1])[-1:-8:-1]/2
# print sort(DB_param_raw[1:, 2])[-1:-8:-1]
## print sort(DB_param_raw[1:, 3])[-1:-8:-1]
## print sort(DB_param_raw[1:, 4])[-1:-8:-1]
print     
print 'T= ', Temp, 'K'
print 'kT= ', kT, 'eV'
print 'Fermi level= ', chempot, '[eV] w.r.t. VBM'
print 'Eo=', Eo + chempot, '[eV] w.r.t. VBM'
print 'Eo+ V_BB_tip=', Eo + chempot+ V_BB_tip_uniform, '[eV] w.r.t. VBM'
print 'U_intra1=', U_intra1, '[eV]'
print 'Surface dielectric constant=', surf_diel
print 'V_ne accounted=', V_ne_accounted
print 'Screening included=', screened_coul
print 'L_debye=', L_debye, 'AA'
print 'V_BB_tip=', V_BB_tip_uniform, 'eV'
if tunnel_scaling == 1.:
  print 'tunnel_scaling= Off'
else:
  print 'tunnel_scaling=', tunnel_scaling
print

print 'Cell Dimension =', fs(x_DB2), 'A'
print 'Cell-Cell smallest distance= ', fs(sqrt(xCC **2 + yCC **2)), 'A'
'''
nsite = 2      # the number of tunnel coupled DBs
x_DB = zeros((Ncells, nsite))
y_DB = zeros((Ncells, nsite))

## Define the parameters of the biasing electrode = sphere 
Rele = 10. # 3.* da # radius of spherical electrode [A]
#v center of 1st electrode
x_ele = - ( 2* da + Rele)
y_ele = - ( 2* da + Rele)
r1_ele = sqrt((x_ele - x_DB1)**2 + (y_ele - y_DB1)**2)
r2_ele = sqrt((x_ele - x_DB2)**2 + (y_ele - y_DB2)**2)

## NB: THE ENERGY REFERENCE IS TAKEN AS (2*E_isol_DB + eV_diag).
#V Look up t in the parameter tables
#v along x:
rx = abs(x_DB2 - x_DB1)
t1 = interp([rx], r_DB_interp, DB_param[:,1])[0]
# print 't=', t1, 'eV'
#v Get Udd = U_e1-e2 - U_e1-e3 CAUTION: ASSUMING SQUARE CELL SO THAT THE FORMULA IN lent3qubit applies
# Ue1e2 = interp([rx], r_DB_interp, DB_param[:,2])
# rxy = sqrt(rx **2 + ry **2)
# Ue1e3 = interp([rxy], r_DB_interp, DB_param[:,2])
# Udd = Ue1e2[0] - Ue1e3[0]
# print 'Udd=', Udd, 'eV'
#v Cacl. gamma 
gamma = t1   # [eV] CAUTION: MINUS IS NOT APPLIED HERE!!!
# print 'gamma=', gamma, 'eV'
# Ekink = 0.2  # kink energy [eV]

gIh = abs(gamma) * qe / h_bar  # gamma Over h_bar in 1/s.
  #^ NB: Tunneling period corresponds to Delta!
Tau = 2.* pi / (2* abs(gamma) * qe / h_bar)
# print 'Tunneling period=', Tau, 'sec.'

## Calculate the kink energy (consider cell "a" and cell "b")
#v for the kink= anti-parallel Polarizations
r23 = sqrt(xCC **2 + yCC **2) 
r13 = sqrt((x_DB1 - x_DB3)**2 + (y_DB1 - y_DB3)**2)
# print 'r3a4b=', r3a4b, 'A'
Ur23 = interp([r23], r_DB_interp, DB_param[:,2])[0]
Ur13 = interp([r13], r_DB_interp, DB_param[:,2])[0]
#v for the parallel Polarizations
r24 = sqrt((rx + xCC)**2 + yCC **2)
# print 'r3a1b=', r3a1b, 'A'
Ur24 = interp([r24], r_DB_interp, DB_param[:,2])[0]
# Ekink = (Ur23 - Ur24) # kink energy [eV]
Ekink = (Ur23 - Ur13) # kink energy [eV]
# print 'Ekink = ', Ekink, 'eV'

#v Another estimate of kink energy based on point-charges
Ekink1 = qe **2 /(4* pi * eps0 * surf_diel) * (1/ r23 - 1/ r24) * 1e+10 # kink energy [J]
Ekink1 /= qe  # convert to [eV]
# print 'Ekink (from point charge calcl.) = ', Ekink1, 'eV'

# for 2 wells - Taleana's case
x1_nm = (x_DB1 - 0.5* (x_DB1 + x_DB2))/ 10.
x2_nm = (x_DB2 - 0.5* (x_DB1 + x_DB2))/ 10.
# Xmsh = linspace(-3., 5, 280)   # in nm
# Xmsh = linspace(-2., 2., 220)   # in nm
# Xmsh = linspace(-3.7, 3.7, 290)   # in nm
Xmsh = linspace(-3.5, 3.5, 250)   # in nm
dX = abs(Xmsh[1] - Xmsh[0])  # in nm
## Get the resonance integral from the t-table above:
r12 = abs(x1_nm - x2_nm)* 10.  # in Ang.
beta = - interp([r12], r_DB_interp, DB_param[:,1])[0]    # MINUS HERE FOR BETA
# beta *= 2.  # BECAUSE  t is HALF THE TUNNEL SPLITTING
# print 'beta =', beta, 'eV, for separation of ', r12, 'Ang'

lam3 = 0.05   # this is chosen for a 1e- system in the double well for Write + Read project.
alf1 = 0.09  #  0.09  # 0.1 single well width 

## TIBB data as a function of tip height: 
## Htip/nm, TIBB / meV, ICIBB / meV
data0 = array([
   [0.600,	397.3,   -155.2890696 ],
   [0.550,  409.4,   -169.0287632 ],
   [0.500,	424.9,   -185.2759245 ],
   [0.450,	441.1,   -204.7832991 ],
   [0.400,	460.5,   -228.6383749 ],
   [0.350,	480.3,   -258.4727269 ],
   [0.300,	504.3,   -296.8509287 ],
   [0.250,	534.4,   -348.0487002 ],
   [0.200,  565.2,   -419.7657664 ],
   [0.150,	598.0,   -527.4112206 ],
   [0.100,  647.0,   -706.9739223 ]])
'''
# this is to fit the TIBB vs. tip height 
'''
Htip1 = data0[:-2,0]
TIBB1 = data0[:-2,1] * 1e-3   # in eV
Htip = data0[:,0]
TIBB = data0[:,1] * 1e-3    # in eV
ICIBB = data0[:,2] * 1e-3    # in eV
# print coef
# coefs= polyfit(Htip1, TIBB1, 2)
# TIBB2 = coefs[0]* Htip**2 + coefs[1]* Htip + coefs[2]
coefs= polyfit(Htip1, TIBB1, 3)
TIBB3 = coefs[0]* Htip**3 + coefs[1]* Htip**2 + coefs[2]* Htip + coefs[3]

TIBB[-2:] = TIBB3[-2:]
sumTIBB_ICIBB = TIBB + ICIBB
E_DB_m_bulkCB_eV = E_DB_m_bulkCB_J / qe 
Rtip0 = 5.      # tip radius in nm, ASSUMED IN FEM CALCULATIONS.
## READ TIBB vs R from a file. (for a given fixed tip height)  
TIBBvsRfile = open('TIBB_vs_R_d200pm.dat', 'r')
H0tip = 0.2    # nm, tip height for which this TIBB data was calculated 
DW0_ts = 5. - 4.1   # difference between tip and sample work function assumed in my FEM calculations.
lines = [line.strip() for line in TIBBvsRfile] # read line into a list of strings
TIBBvsRfile.close()
rx = zeros(len(lines))
femTIBB_H0_r = zeros(len(lines))
for j, line in enumerate(lines):
  # line.split()[1]
  rx[j]= line.split()[0]
  femTIBB_H0_r[j]= line.split()[1]
  
  
# place the DBs along the line
yl1 = 0.
xl1 = 0.   # this DB is at the ORIGIN of coordinates
xl2 = (7.* da) * 0.1   # in [nm]
xl3 = xl2 + (2.* da) * 0.1
xl4 = xl3 + (5.* da) * 0.1
xl5 = xl4 + (2.* da) * 0.1
xl6 = xl5 + (7.* da) * 0.1
Kc = 8.987551787e9   # Coulomb constant for vacuum 
epsr = 6.35   # dielectric constant for the surface
Kc1 = Kc / epsr
KKc1 =  Kc1 * qe / 1e-9  # a shorthand constant for potential calc.; assumes distances will be given in nm
epsrI = 9.   # dielectric constant for the image charge - charge interaction 
Kci = Kc / epsrI
KKci =  Kci * qe / 1e-9  # a faster-to-use constant for potential calc.; assumes distances will be given in nm
sqrt2sig_DB = 0.5  # nm; A measure of the (Gaussian) size of DB orbital used for erf(r)/r type potential
'''
## Tunneling rates tip-DB 
## Far-limit is from Marco Taucer's PRL 112, 256801 (2014)
## Close limit is from known STM currents, assumed 1 e/sec for tip-sample distance of 0.6 nm.
'''
TunRate0 = 43.3e9 # 2.1e5   # Hz, a constant factor for the lateral decay of the tunneling rate tip-DB.
k_tip_DBm  = 6.28 # 1.91   # [1/nm] spatial decay rate for the tunneling rate (from the Marco experiment)
## Potential at the center of a DB-, assuming erf(r)/ r form with above sigma
r0 = 1e-6 # nm, approximates zero for the needs below
Vee_gaussian = KKc1 * erf(r0 / sqrt2sig_DB)/ r0    # this is the max of the DB- potential (at the center of a DB) 
# print 'Vee_gaussian=', Vee_gaussian, ' Volt, for sqrt2sig_DB=', sqrt2sig_DB, 'nm'

QconfigList = [] 
## ADD ALL LIKELY CHARGE CONFIGS BELOW. ASSUME END DBs ARE NEGATIVE AS INDICATED BY EXPERIMENTS
'''
## Configs with four 0s:
QconfigList.append([1, 0,0, 0,0, 1])   # a list with charges on each DB in units of -qe.
QconfigList.append([1, 1,1, 1,1, 1])   # a list with charges on each DB in units of -qe.
##  Configs with a single 0: (end DBs are taken always to be 1)
QconfigList.append([1, 1,0, 1,1, 1])   # a list with charges on each DB in units of -qe.
QconfigList.append([1, 1,1, 0,1, 1])   # a list with charges on each DB in units of -qe.
QconfigList.append([1, 1,1, 1,0, 1])   # a list with charges on each DB in units of -qe.
QconfigList.append([1, 0,1, 1,1, 1])   # a list with charges on each DB in units of -qe.
## Configs with two 0s:
QconfigList.append([1, 0,1, 0,1, 1])   # a list with charges on each DB in units of -qe.
QconfigList.append([1, 1,0, 1,0, 1])   # a list with charges on each DB in units of -qe.
QconfigList.append([1, 0,0, 1,1, 1])   # a list with charges on each DB in units of -qe.
QconfigList.append([1, 1,1, 0,0, 1])   # a list with charges on each DB in units of -qe.
QconfigList.append([1, 1,0, 0,1, 1])   # a list with charges on each DB in units of -qe.
QconfigList.append([1, 0,1, 1,0, 1])   # a list with charges on each DB in units of -qe.
'''
## Configs with three 0s:
QconfigList.append([1, 1,0, 0,0, 1])   # a list with charges on each DB in units of -qe.
QconfigList.append([1, 0,1, 0,0, 1])   # a list with charges on each DB in units of -qe.
QconfigList.append([1, 0,0, 1,0, 1])   # a list with charges on each DB in units of -qe.
QconfigList.append([1, 0,0, 0,1, 1])   # a list with charges on each DB in units of -qe.

DBsXlist = [xl1, xl2, xl3, xl4, xl5, xl6]
DBsYlist = [yl1, 0., 0., 0., 0., 0.]  
Xtip = 0.  # tip coordinate in [nm] w.r.t. the DB numbered 1.
Ytip = 0.  # tip coordinate in [nm] w.r.t. the DB numbered 1.
Htip = 0.5  # actual tip height in experiment in nm

if __name__ == "__main__":
  print db_energy_shifts(DBsXlist= DBsXlist, DBsYlist= DBsYlist, Xtip = Xtip, Ytip = Ytip, Htip = Htip, QconfigList = QconfigList)