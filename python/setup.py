#!/usr/bin/env python3

import sys
import os

if sys.version_info < (3, 6, 0):
    print('Tried to install with an unsupported version of Python. '
          'ytp requires Python 3.6.0 or greater')
    sys.exit(1)

import setuptools
from setuptools.command.build_ext import build_ext


class build_ext_custom(build_ext):
    def build_extensions(self):
        self.compiler.linker_so[0] = 'ld'
        super().build_extensions()


setuptools.setup (
    name = 'ytp',
    version = os.getenv('PACKAGE_VERSION'),
    author='Featuremine Corporation',
    author_email='support@featuremine.com',
    url='https://www.featuremine.com',
    description='Featuremine YTP packages',
    long_description='Featuremine YTP packages',
    classifiers=[
        'Programming Language :: Python :: 3 :: Only',
    ],
    license='COPYRIGHT (c) 2022 by Featuremine Corporation',
    ext_modules = [
        setuptools.Extension(
            'ytp.ytp',
            sources=[
                'ytp.cpp',
                'py_ytp.cpp'
            ],
            include_dirs = os.getenv('YTP_PACKAGE_INCLUDES').split(':'),
            extra_compile_args=[
                '-std=c++17',
                '-O3',
                '-fno-use-cxa-atexit',
                '-fvisibility-inlines-hidden',
                '-fvisibility=hidden',
            ],
            extra_link_args=[
                '-Wl,--exclude-libs,ALL',
                '-Wl,-Bdynamic',
                '-lgcc_s',
                '-Wl,-static',
                '-lstdc++',
            ] + os.getenv('YTP_PACKAGE_LIBS').split(':'),
            language='c',
        )
    ],
    packages=['ytp'],
    package_data={
        'ytp': [
            'ytp.pyi',
        ]
    },
    cmdclass = {
        'build_ext': build_ext_custom,
    }
)
