.. _tmp_file_use:

Use of Temporary Files
======================

The MET application and library code uses temporary files in several
places. Each specific use of temporary files is described below. The
directory in which temporary files are stored is configurable as
described in the User's Guide :numref:`config_tmp_dir`.

Whenever a MET application is run, the operating system assigns it a
process identification number (PID). All temporary files created by
MET include the PID in the file name so that multiple instances can
run concurrently without conflict. In addition, when creating a
temporary file name, the :code:`make_temp_file_name(...)` utility
function appends :code:`_0` to the PID, checks to see if the
corresponding file name is already in use, and if so, tries
:code:`_1`, :code:`_2` and so on, until an available file name is
found.

Note that creating, reading, and deleting temporary files from the
local filesystem is much more efficient than performing these
operations across a network filesystem. Using the default
:code:`/tmp` directory is recommended, unless prohibited by policies
on your system.

In general, MET applications delete any temporary files they create
when they are no longer needed. However, if the application exits
abnormally, the temporary files may remain.

PB2NC Tool
^^^^^^^^^^

The PB2NC tool reads input binary files in the BUFR or PrepBUFR
format, extracts and/or derives observations from them, filters
those observations, and writes the result to a NetCDF output file.

PB2NC creates the following temporary files when running:

* :code:`tmp_pb2nc_blk_{PID}`, :code:`tmp_pb2nc_meta_blk_{PID}`,
  :code:`tmp_pb2nc_tbl_blk_{PID}`

  PB2NC assumes that each input binary file requires Fortran'
  blocking prior to being read by the BUFRLIB library. It applies
  Fortran blocking, writes the result to this temporary file, and
  uses BUFRLIB to read its contents.

* :code:`tmp_pb2nc_bufr_{PID}_tbl`: PB2NC extracts Bufr table data
  that is embedded in input files, applies Fortran blocking, and
  writes it to this temporary file for later use.

.. note::
   The first 3 files listed below are identical. They are all
   blocked versions of the same input file. Recommend modifying the
   logic to only block the input file once.

Point2Grid Tool
^^^^^^^^^^^^^^^

The Point2Grid tool reads point observations from a variety of
inputs and summarizes them on a grid. When processing GOES input
files, a temporary NetCDF file is created to store the mapping of
input pixel locations to output grid cells.

If that geostationary grid mapping file already exists, it is used
directly and not recreated. If not, it is created as needed.

Note that this temporary file is *not* deleted by the Point2Grid
tool. Once created, it is intended to be reused in future runs.

.. note::
   Should this grid navigation file actually be written to the
   temporary directory or should it be written to an output
   directory instead since its intended to be reused across multiple
   runs?

Bootstrap Confidence Intervals
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Several MET tools support the computation of bootstrap confidence
intervals as described in the User's Guide :numref:`config_boot`
and :numref:`Appendix D, Section %s <App_D-Confidence-Intervals>`.
When bootstrap confidence intervals are requested, up to two
temporary files are created for each CNT, CTS, MCTS, NBRCNT, or
NBRCTS line type written to the output.

* :code:`tmp_{LINE_TYPE}_i_{PID}`: When the BCA bootstrapping method
  is requested, jackknife resampling is applied to the input matched
  pairs. Statistics are computed for each jackknife resample and
  written to this temporary file.

* :code:`tmp_{LINE_TYPE}_r_{PID}`: For each bootstrap replicate
  computed from the input matched pairs, statistics are computed
  and written to this temporary file.

Where {LINE_TYPE} is :code:`cnt`, :code:`cts`, :code:`mcts`,
:code:`nbrcnt`, or :code:`nbrcts`.

.. note::
   Consider whether or not its realistic to hold the resampled
   statistics all in memory. If so, that'd save a lot of time in
   I/O.

Stat-Analysis Tool
^^^^^^^^^^^^^^^^^^

The Stat-Analysis tool reads ASCII output created by the MET
statistics tools. A single job can be specified on the command line
or one or more jobs can be specified in an optional configuration
file. When a configuration file is provided, any filtering options
specified are applied to all entries in the :code:`jobs` array.

Rather than reading all of the input data for each job, Stat-Analysis
reads all the input data once, applies any common filtering options,
and writes the result to a temporary file.

* :code:`tmp_stat_analysis_{PID}`: Stat-Analysis reads all of the
  input data, applies common filtering logic, and writes the result
  to this temporary file. All of the specified jobs read data from
  this temporary file, apply any additional job-specific filtering
  criteria, and perform the requested operation.

.. note::
   Consider revising the logic to only use a temp file when actually
   necessary, when multiple jobs are specified along with non-empty
   common filtering logic.

Python Embedding
^^^^^^^^^^^^^^^^

As described in the User's Guide
:numref:`Appendix F, Section %s <appendixF>`, when the
:code:`MET_PYTHON_EXE` environment variable is set, the MET tools run
any Python embedding commands using the specified Python executable.

* :code:`tmp_mpr_{PID}`: When Python embedding of matched pair data
  is performed, a Python wrapper is run to execute the user-specified
  Python script and write the result to this temporary ASCII file.

* :code:`tmp_met_nc_{PID}`: When Python embedding of gridded data or
  point observations is performed, a Python wrapper is run to
  execute the user-specified Python script and write the result to
  this temporary NetCDF file.

The compile-time Python instance is run to read data from these
temporary files.

TC-Diag Tool
^^^^^^^^^^^^

The TC-Diag tool requires the use of Python embedding. It processes
one or more ATCF tracks and computes model diagnostics. For each
track point, it converts gridded model data to cyclindrical
coordinates centered at that point, writes it to a temporary NetCDF
file, and passes it to Python scripts to compute the model
diagnostics.

* :code:`tmp_met_nc_{PID}`: Cylindrical coordinate model data is
  written to this temporary NetCDF file for each track point
  and passed to Python scripts to compute diagnostics. If requested,
  the temporary NetCDF files for each track point are combined into
  a single output NetCDF cylindrical coordinates file for each track.
