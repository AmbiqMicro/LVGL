#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# LVGL documentation build configuration file, created by
# sphinx-quickstart on Wed Jun 12 16:38:40 2019.
#
# This file is execfile()d with the current directory set to its
# containing dir.
#
# Note that not all possible configuration values are present in this
# autogenerated file.
#
# All configuration values have a default; values that are commented out
# serve to show the default.

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
import os
import sys

sys.path.insert(0, os.path.abspath('./_ext'))

from sphinx.builders.html import StandaloneHTMLBuilder

# -- General configuration ------------------------------------------------

# If your documentation needs a minimal Sphinx version, state it here.
#
# needs_sphinx = '1.0'

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    'sphinx_rtd_theme',
    'sphinx.ext.autodoc',
    'sphinx.ext.intersphinx',
    'sphinx.ext.todo',
    'breathe',
    'sphinx_sitemap',
    'lv_example',
    'sphinx_rtd_dark_mode',
    # "sphinx.ext.autosectionlabel"
]

# sphinx.ext.autosectionlabel
# adds unique references to section headers
# to view a list of all of the references use
# python -m sphinx.ext.intersphinx html_out/objects.inv
# autosectionlabel_prefix_document = True
# autosectionlabel_maxdepth = 2


default_dark_mode = False

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']


# breathe_debug_trace_directives = True
# breathe_debug_trace_doxygen_ids = True
# breathe_debug_trace_qualification = True


# Only add spelling extension if it is available. We don't know if it is installed as we don't want
# to put it in the setup.py file as a dependency as we don't want Breathe to be dependent on it as
# people should be able to use Breathe without 'spelling'. There might be a better way to handle
# this.
try:
    import sphinxcontrib.spelling

    extensions.append("sphinxcontrib.spelling")

    # Configuration for spelling extension
    spelling_word_list_filename = "spelling_wordlist.txt"
    spelling_lang = "en_US"

except ImportError:
    pass


# The default language to highlight source code in. The default is 'python'.
# The value should be a valid Pygments lexer name, see Showing code examples for more details.


highlight_language = 'c'

# The suffix(es) of source filenames.
# You can specify multiple suffix as a list of string:
#
source_suffix = ['.rst']


# The master toctree document.
master_doc = 'index'

# General information about the project.
project = 'LVGL'
copyright = '2024, LVGL Kft'
author = 'LVGL community'


# The version info for the project you're documenting, acts as replacement for
# |version| and |release|, also used in various other places throughout the
# built documents.
#
# The short X.Y version.
# embeddedt: extract using scripts/find_version.sh

version = ''

# The language for content autogenerated by Sphinx. Refer to documentation
# for a list of supported languages.
#
# This is also used if you do content translation via gettext catalogs.
# Usually you set "language" from the command line for these cases.
language = 'en'

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This patterns also effect to html_static_path and html_extra_path
exclude_patterns = ['_build', 'doxygen_html', 'Thumbs.db', '.DS_Store',
                    'README.md', 'lv_examples', 'out_html', 'env', '_ext', 'examples']

# The name of the Pygments (syntax highlighting) style to use.
pygments_style = 'sphinx'

# If true, `todo` and `todoList` produce output, else they produce nothing.
todo_include_todos = True


# -- Options for HTML output ----------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'sphinx_rtd_theme'

# Theme options are theme-specific and customize the look and feel of a theme
# further.  For a list of options available for each theme, see the
# documentation.
#


html_theme_options = {
    'display_version': True,
    'prev_next_buttons_location': 'both',
    'style_external_links': False,
    # 'vcs_pageview_mode': '',
    # 'style_nav_header_background': 'white',
    # Toc options
    'sticky_navigation': True,
    'navigation_depth': 4,
    'includehidden': True,
    'titles_only': False,

    'collapse_navigation': False,
    'logo_only': True,
}


# For site map generation
html_baseurl = f"https://docs.lvgl.io/{os.environ['LVGL_URLPATH']}/en/html/"

sitemap_url_scheme = "{link}"

#lvgl_github_url = f"https://github.com/lvgl/lvgl/blob/{os.environ['LVGL_GITCOMMIT']}/docs"

#extlinks = {'github_link_base': (github_url + '%s', github_url)}

html_context = {
    'github_version': os.environ['LVGL_GITCOMMIT'],
    'github_user': 'lvgl',
    'github_repo': 'lvgl',
    'display_github': True,
    'conf_py_path': '/docs/'
}


# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']

# Custom sidebar templates, must be a dictionary that maps document names
# to template names.
#
# This is required for the alabaster theme
# refs: http://alabaster.readthedocs.io/en/latest/installation.html#sidebars
html_sidebars = {
    '**': [
        'relations.html',  # needs 'show_related': True theme option to display
        'searchbox.html',
    ]
}

html_js_files = [
    'js/custom.js',
    'js/include_html.js'
]

html_favicon = 'favicon.png'
html_logo = 'logo_lvgl.png'

# -- Options for HTMLHelp output ------------------------------------------

# Output file base name for HTML help builder.
htmlhelp_basename = 'LVGLdoc'

html_last_updated_fmt = ''

# -- Options for LaTeX output ---------------------------------------------

latex_engine = 'xelatex'
latex_use_xindy = False
latex_elements = {
    # The paper size ('letterpaper' or 'a4paper').
    #
    # 'papersize': 'letterpaper',

    # The font size ('10pt', '11pt' or '12pt').
    #
    # 'pointsize': '10pt',

    # Additional stuff for the LaTeX preamble.
    #
    # 'preamble': '',

    # Latex figure (float) alignment
    #
    # 'figure_align': 'htbp',

    'inputenc': '',
    'utf8extra': '',
    'classoptions': ',openany,oneside',
    'babel': '\\usepackage{babel}',
    'passoptionstopackages': r'''
\PassOptionsToPackage{bookmarksdepth=5}{hyperref}% depth of pdf bookmarks
''',
    'preamble': r'''
\usepackage{fontspec}
\setmonofont{DejaVu Sans Mono}
\usepackage{silence}
\WarningsOff*
''',
}

# Grouping the document tree into LaTeX files. List of tuples
# (source start file, target name, title,
#  author, documentclass [howto, manual, or own class]).
latex_documents = [
    (master_doc, 'LVGL.tex', 'LVGL Documentation ' + version,
     'LVGL community', 'manual'),
]


# -- Options for manual page output ---------------------------------------

# One entry per manual page. List of tuples
# (source start file, name, description, authors, manual section).
man_pages = [
    (master_doc, 'lvgl', 'LVGL Documentation ' + version,
     [author], 1)
]


# -- Options for Texinfo output -------------------------------------------

# Grouping the document tree into Texinfo files. List of tuples
# (source start file, target name, title, author,
#  dir menu entry, description, category)
texinfo_documents = [
    (master_doc, 'LVGL', 'LVGL Documentation ' + version,
     author, 'Contributors of LVGL', 'One line description of project.',
     'Miscellaneous'),
]


breathe_projects = {
  "lvgl":"xml/",
}

StandaloneHTMLBuilder.supported_image_types = [
    'image/svg+xml',
    'image/gif',  #prefer gif over png
    'image/png',
    'image/jpeg'
]

smartquotes = False

repo_commit_hash = os.environ['LVGL_GITCOMMIT']


# Example configuration for intersphinx: refer to the Python standard library.

def setup(app):
    # app.add_config_value('recommonmark_config', {
    #         'enable_eval_rst': True,
    #         'enable_auto_toc_tree': 'True',
    #         }, True)
    # app.add_transform(AutoStructify)
    app.add_css_file('css/custom.css')
    app.add_css_file('css/fontawesome.min.css')
