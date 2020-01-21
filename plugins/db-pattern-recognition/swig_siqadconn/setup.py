#!/usr/bin/env python

'''
setup.py file for SWIG phys connector
'''

from distutils.core import setup, Extension
import os
os.environ["CC"] = "g++"
os.environ["CXX"] = "g++"
siqadconn_module = Extension('_siqadconn',
                                sources=['siqadconn_wrap.cxx', 'siqadconn.cc'],
                                )

setup (
        name    = 'siqadconn',
        version = '0.01',
        author  = 'Samuel Ng',
        description = '''Python wrapper for SiQAD connector''',
        ext_modules = [siqadconn_module],
        py_modules = [],
    )
