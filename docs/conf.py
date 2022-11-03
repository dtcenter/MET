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
sys.path.insert(0, os.path.abspath('.'))
print(sys.path)

# -- Project information -----------------------------------------------------

project = 'MET'
author = 'UCAR/NCAR, NOAA, CSU/CIRA, and CU/CIRES'
author_list = 'Opatz, J., T. Jensen, J. Prestopnik, H. Soh, L. Goodrich, B. Brown, R. Bullock, J. Halley Gotway, K. Newman'
version = '11.0.0-beta4'
verinfo = version
release = f'{version}'
release_year = '2022'
release_date = f'{release_year}-11-03'
copyright = f'{release_year}, {author}'

# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
# Adding 'sphinx_panels' to use drop-down menus in appendixA. 
extensions = ['sphinx.ext.autodoc','sphinx.ext.intersphinx','sphinx_panels',]

# settings for ReadTheDocs PDF creation
latex_engine = 'pdflatex'
latex_theme = 'manual'
latex_logo = os.path.join('_static','met_logo_2019_09.png')
latex_show_pagerefs = True
latex_master_doc = 'Users_Guide/index'

latex_elements = {
   # The paper size ('letterpaper' or 'a4paper').
   #
   'papersize': 'letterpaper',
   'releasename':"{version}",
   'fncychap': '\\usepackage{fncychap}',
   'fontpkg': '\\usepackage{amsmath,amsfonts,amssymb,amsthm,float}',
   'inputenc': '\\usepackage[utf8]{inputenc}',
   'fontenc': '\\usepackage[LGR,T1]{fontenc}',
                                                     
   'figure_align':'H',
   'pointsize': '11pt',
                                        
   'preamble': r'''
       \usepackage{charter}
       \usepackage[defaultsans]{lato}
       \usepackage{inconsolata}
       \setcounter{secnumdepth}{4}
       \setcounter{tocdepth}{4}
    ''',
                                                                            
    'sphinxsetup': \
        'hmargin={0.7in,0.7in}, vmargin={1in,1in}, \
        verbatimwithframe=true, \
        TitleColor={rgb}{0,0,0}, \
        HeaderFamily=\\rmfamily\\bfseries, \
        InnerLinkColor={rgb}{0,0,1}, \
        OuterLinkColor={rgb}{0,0,1}',
        'maketitle': '\\sphinxmaketitle',  
#        'tableofcontents': ' ',
        'printindex': ' '
}

# Grouping the document tree into LaTeX files. List of tuples
# (source start file, target name, title,
#  author, documentclass [howto, manual, or own class]).
latex_documents = [
    (latex_master_doc, 
     'users_guide.tex', 
     'MET User\'s Guide',
     ' ', 
     'manual')
]
    
# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store', 'Flowchart' ]

# Suppress certain warning messages
suppress_warnings = ['ref.citation']

# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'sphinx_rtd_theme'
html_theme_path = ["_themes", ]
html_js_files = ['pop_ver.js']
html_css_files = ['theme_override.css']

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']

# The name of an image file (relative to this directory) to place at the top
# of the sidebar.
html_logo = os.path.join('_static','met_logo_2019_09.png')

# -- Intersphinx control -----------------------------------------------------
intersphinx_mapping = {'numpy':("https://docs.scipy.org/doc/numpy/", None)}

numfig = True

numfig_format = {
    'figure': 'Figure %s',
}

# -- Export variables --------------------------------------------------------

rst_epilog = f"""
.. |copyright|    replace:: {copyright}
.. |author_list|  replace:: {author_list}
.. |release_date| replace:: {release_date}
.. |release_year| replace:: {release_year}
"""
