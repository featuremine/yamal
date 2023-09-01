"""
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.
        
        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
"""

import sys
from os import path

with open(path.join(path.dirname(__file__), '..', 'VERSION')) as file:
    yamal_version = list(file)[0].strip()

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
        'yamal': ['*.so', '*.dylib', '*.py', '*.pyi','include', 'include/*', 'include/*/*', 'include/*/*/*']
    },
    license='COPYRIGHT (c) 2019-2023 by Featuremine Corporation',
    packages=['yamal', 'yamal.tests'],
    scripts=['scripts/test-yamal-python', 'scripts/yamal-run.py'],
)
