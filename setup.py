"""
        COPYRIGHT (c) 2022 by Featuremine Corporation.
        This software has been provided pursuant to a License Agreement
        containing restrictions on its use.  This software contains
        valuable trade secrets and proprietary information of
        Featuremine Corporation and is protected by law.  It may not be
        copied or distributed in any form or medium, disclosed to third
        parties, reverse engineered or used in any manner not provided
        for in said License Agreement except with the prior written
        authorization from Featuremine Corporation.
"""

import sys
from os import path

yamal_version = list(open(path.join(path.dirname(__file__), 'VERSION')))[0].strip()

if sys.version_info < (3, 6, 0):
    print('Tried to install with an unsupported version of Python. '
          'yamal requires Python 3.6.0 or greater')
    sys.exit(1)

import setuptools

setuptools.setup (
    name = 'yamal',
    version = yamal_version,
    author='Featuremine Corporation',
    author_email='support@featuremine.com',
    url='https://www.featuremine.com',
    description='Featuremine YTP packages',
    long_description='Featuremine YTP packages',
    classifiers=[
        'Programming Language :: Python :: 3 :: Only',
    ],
    package_data={
        'yamal': ['*.so', '*.py', '*.pyi','include', 'include/*', 'include/*/*', 'include/*/*/*']
    },
    license='COPYRIGHT (c) 2022 by Featuremine Corporation',
    packages=['yamal', 'yamal.tests'],
    scripts=['scripts/test-yamal-python', 'scripts/yamal-run.py'],
)
