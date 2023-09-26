.. _tmp_file_use:

Use of Temporary Files
======================

The MET application and library code uses temporary files in several
places. Each specific use of temporary files is described below. The
directory in which temporary files are stored is configurable as
described in the User's Guide section :numref:`config_tmp_dir`.

Note that creating, reading, and deleting temporary files from the
local filesystem is much more efficient than performing these
operations across a network filesystem. Using the default
:code:`/tmp` directory is recommended, unless prohibited by High
Performance Computing policies.

In general, MET applications delete any temporary files they create
when they are no longer needed. However, if the application exits
abnormally, the temporary files may remain.

JHG, work here.

PB2NC Tool
^^^^^^^^^^

Point2Grid Tool
^^^^^^^^^^^^^^^

TC-Diag Tool
^^^^^^^^^^^^

Bootstrap Confidence Intervals
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Stat-Analysis Tool
^^^^^^^^^^^^^^^^^^

Python Embedding
^^^^^^^^^^^^^^^^
