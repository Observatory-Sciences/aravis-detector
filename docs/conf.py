# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

from pathlib import Path
from subprocess import check_output
import sys
import os
import requests

sys.path.append(os.path.abspath('../python/tools/arvcli'))

# -- General configuration ------------------------------------------------

# General information about the project.
project = "aravis-detector"

# The full version, including alpha/beta/rc tags.
release = '0.0.1'

# The short X.Y version.
if "+" in release:
    # Not on a tag, use branch name
    root = Path(__file__).absolute().parent.parent
    git_branch = check_output("git branch --show-current".split(), cwd=root)
    version = git_branch.decode().strip()
else:
    version = release

# Sphinx Extensions
extensions = [
    # Use this for generating API docs
    "sphinx.ext.autodoc",
    # This can parse google style docstrings
    "sphinx.ext.napoleon",
    # For linking to external sphinx documentation
    "sphinx.ext.intersphinx",
    # Add links to source code in API docs
    "sphinx.ext.viewcode",
    # Adds the inheritance-diagram generation directive
    "sphinx.ext.inheritance_diagram",
    # Add some useful widgets - Cards, Grids, Dropdowns, etc.
    # https://sphinx-design.readthedocs.io
    "sphinx_design",
    # Define full top down hierarchy in sphinx.yaml instead of throughout page tree
    # https://sphinx-external-toc.readthedocs.io
    "sphinx_external_toc",
    # Enable Markedly Structured Text
    # https://myst-parser.readthedocs.io
    "myst_parser",
    # Enable breathe for doxygen integration
    "breathe",
    # 'autoapi.sphinx'
]

# Sphinx External TOC Config
external_toc_path = "sphinx.yaml"

# MyST Extensions & Config
myst_enable_extensions = [
    "deflist",
    "colon_fence",
]

# Enable internal link generation for page headings up to this level
myst_heading_anchors = 4

# Breathe Config
breathe_projects = {"aravis-detector": "build/doxygen/xml/"}
breathe_default_project = "aravis-detector"
breathe_default_members = (
    "members",
    "undoc-members",
    "protected-members",
    "private-members",
)

# If true, Sphinx will warn about all references where the target cannot
# be found.
nitpicky = True

# A list of (type, target) tuples (by default empty) that should be ignored when
# generating warnings in "nitpicky mode". Note that type should include the
# domain name if present. Example entries would be ('py:func', 'int') or
# ('envvar', 'LD_LIBRARY_PATH').
nitpick_ignore = [("py:func", "int")]

# Both the class’ and the __init__ method’s docstring are concatenated and
# inserted into the main body of the autoclass directive
autoclass_content = "both"

# Order the members by the order they appear in the source code
autodoc_member_order = "bysource"

# Don't inherit docstrings from baseclasses
autodoc_inherit_docstrings = False

# Output graphviz directive produced images in a scalable format
graphviz_output_format = "svg"

# The name of a reST role (builtin or Sphinx extension) to use as the default
# role, that is, for text marked up `like this`
default_role = "any"

# The suffix of source filenames.
source_suffix = ".md"

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# These patterns also affect html_static_path and html_extra_path
exclude_patterns = ["_build"]

# The name of the Pygments (syntax highlighting) style to use.
pygments_style = "sphinx"

# This means you can link things like `str` and `asyncio` to the relevant
# docs in the python documentation.
intersphinx_mapping = dict(python=("https://docs.python.org/3/", None),
                           odin=("https://github.com/odin-detector/odin-data", None),
                           odin_control=('https://github.com/odin-detector/odin-control', None),
                           aravis=("https://github.com/AravisProject/aravis", None),
                           genicam=("https://www.genicam.org", None),
                           meson=('https://mesonbuild.com/', None))

# A dictionary of graphviz graph attributes for inheritance diagrams.
inheritance_graph_attrs = dict(rankdir="TB")

# Ignore localhost links for period check that links in docs are valid
linkcheck_ignore = [r"http://localhost:\d+/"]

# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.

html_theme = "pydata_sphinx_theme"
github_repo = project
github_user = "Observatory-Sciences"

switcher_json = f"https://{github_user}.github.io/{github_repo}/switcher.json"
switcher_exists = requests.get(switcher_json).ok
if not switcher_exists:
    print(
        "*** Can't read version switcher, is GitHub pages enabled? \n"
        "    Once Docs CI job has successfully run once, set the "
        "Github pages source branch to be 'gh-pages' at:\n"
        f"    https://github.com/{github_user}/{github_repo}/settings/pages",
        file=sys.stderr,
    )

# Options for theme
html_theme_options = dict(
    logo=dict(
        text=project,
    ),
    github_url=f"https://github.com/{github_user}/{github_repo}",
    secondary_sidebar_items=["page-toc", "edit-this-page", "sourcelink"],
    use_edit_page_button=True,
    switcher=dict(
        json_url=switcher_json,
        version_match=version,
    ),
    check_switcher=False,
    navbar_end=["theme-switcher", "icon-links", "version-switcher"],
    external_links=[
        dict(
            name="Release Notes",
            url=f"https://github.com/{github_user}/{github_repo}/releases",
        )
    ],

    # Number of navigation levels
    navigation_depth=3,
    # Default expanded levels
    show_nav_level=2,
    show_toc_level=2,
)


# A dictionary of values to pass into the template engine’s context for all pages
html_context = dict(
    github_user=github_user,
    github_repo=project,
    github_version=version,
    doc_path="docs",
)

# autoapi_modules = {'arvcli': {'output': 'developer/reference'}}
# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
# html_static_path = ["_static"]

# If true, "Created using Sphinx" is shown in the HTML footer. Default is True.
html_show_sphinx = False

# If true, "(C) Copyright ..." is shown in the HTML footer. Default is True.
html_show_copyright = False

# Logo
html_logo = "images/favicon.ico"
