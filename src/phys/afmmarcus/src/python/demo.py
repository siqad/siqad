#!/usr/bin/env python

from afm import AFMLine

# set the corrdinates of the dangling bonds in lattice units
#X = [1, 12, 15, 22, 25]         # 1-2-2
X = [1, 8, 10, 15, 17, 24]     # 1-2-2-1


afm = AFMLine(X)            # create AFM handler for the given DB arrangement

# set the scan type
afm.setScanType(0)          # read-read, set by default
#afm.setScanType(1, .01)     # write(right)-read(left) with a strength of 10meV
#afm.setScanType(-1, .01)    # write(left)-read(right) with a strength of 10meV

# set the bias
#afm.setBias(.00012)         # 120 ueV/angstrom bias gradient

# run the AFM and produce the AFM image, image will be saved to tempfig.pdf
# Nel:      number of electrons in the system, half filled by default
# nscans:   number of line scans, 2 per sweep with read-read else 1
# pad:      number of lattice sites included on the [left, right] edge of the scan
# srate:    tip speed in angstroms/second
afm.run(Nel=3, nscans=200, pad=[3,3], srate=82)
