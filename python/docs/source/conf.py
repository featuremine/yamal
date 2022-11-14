# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
import os
import sys
import shutil

# Workaround to generate docs from ytp.pyi, because sphinx does not like .pyi files
in_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'src') # Where the .pyi files are 
out_dir =  os.path.join(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'build'))
if not os.path.exists(out_dir):
    os.makedirs(out_dir)

pyi_files = []
for file in os.listdir(in_dir):
    if file.endswith('.pyi'):
        pyi_files.append(file)

for file in pyi_files:
    shutil.copyfile(os.path.join(in_dir, file), os.path.join(out_dir, os.path.splitext(file)[0] + '.py'))
sys.path.insert(0, os.path.abspath(out_dir))

# -- Project information -----------------------------------------------------

project = 'Yamal Python'
copyright = '2022, Featuremine'
author = 'Featuremine'

# The full version, including alpha/beta/rc tags
release = list(open(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', 'VERSION')))[0].strip()

# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [ 
    'sphinx.ext.autodoc', 
    'sphinx.ext.autodoc.typehints'
]

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']


# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'sphinx_rtd_theme'

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
# html_static_path = ['_static']

html_show_sourcelink = False
