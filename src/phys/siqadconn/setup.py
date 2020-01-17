#!/usr/bin/env python

from skbuild import setup

setup (
        name    = 'siqadtools',
        version = '0.2',
        cmake_with_sdist = True,
        packages = ['siqadtools'],
        zip_safe = False
        )
