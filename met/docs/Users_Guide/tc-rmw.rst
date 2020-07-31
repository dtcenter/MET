.. _tc-rmw:

TC-RMW Tool
===========

Introduction
____________

The TC-RMW tool regrids tropical cyclone model data onto a moving range-azimuth grid centered on points along the storm track provided in ATCF format, most likely the adeck generated from the file. The radial grid spacing may be set as a factor of the radius of maximum winds (RMW). If wind fields are specified in the configuration file the radial and tangential wind components will be computed. Any regridding method available in MET can be used to interpolate data on the model output grid to the specified range-azimuth grid. The regridding will be done separately on each vertical level. The model data files must coincide with track points in a user provided ATCF formatted track file.

Practical information
_____________________

tc_rmw usage
~~~~~~~~~~~~

The following sections describe the usage statement, required arguments, and optional arguments for **tc_rmw**.

.. code-block:: none

  Usage: tc_rmw
         -data file_1 ... file_n | data_file_list
         -deck file
         -config file
         -out file
         [-log file]
         [-v level]

tc_rmw has required arguments and can accept several optional arguments.

Required arguments for tc_rmw
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. The **-data file_1 ... file_n | data_file_list** options specify the gridded data files or an ASCII file containing a list of files to be used.

2. The **-deck source** argument is the ATCF format data source.

3. The **-config file** argument is the configuration file to be used. The contents of the configuration file are discussed below.

4. The **-out** argument is the NetCDF output file to be written.

Optional arguments for tc_rmw
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

5. The **-log file** option directs output and errors to the specified log file. All messages will be written to that file as well as standard out and error. Thus, users can save the messages without having to redirect the output on the command line. The default behavior is no logfile.

6. The **-v level** option indicates the desired level of verbosity. The contents of “level” will override the default setting of 2. Setting the verbosity to 0 will make the tool run with no log messages, while increasing the verbosity above 1 will increase the amount of logging.

tc_rmw configuration file
~~~~~~~~~~~~~~~~~~~~~~~~~

The default configuration file for the TC-RMW tool named 'TCRMWConfig_default' can be found in the installed **share/met/config/** directory. It is encouraged for users to copy these default files before modifying their contents. The contents of the configuration file are described in the subsections below.

_______________________

.. code-block:: none

  model =       "GFS";
  censor_thresh = [];
  censor_val    = [];
  data  = {
     field = [
          {
             name = "PRMSL";
             level = ["L0"];
          },
          {
             name = "TMP";
             level = ["P1000", "P500"];
          },
          {
             name = "UGRD";
             level = ["P1000", "P500"];
          },
          {
             name = "VGRD";
             level = ["P1000", "P500"];
          }
      ];
  }
  regrid = { ... }

The configuration options listed above are common to many MET tools and are described in :numref:`Data IO MET Configuration File Options`. The name and level entries in the data dictionary define the data to be processed.  The regrid dictionary defines if and how regridding will be performed.

_______________________

.. code-block:: none

  n_range = 100;

The **n_range** parameter is the number of equally spaced range intervals in the range-azimuth grid.

_______________________

.. code-block:: none

  n_azimuth = 180;

The **n_azimuth** parameter is the number of equally spaced azimuth intervals in the range-azimuth grid. The azimuthal grid spacing is 360 / **n_azimuth** degrees.

_______________________

.. code-block:: none

  max_range_km = 100.0;

The **max_range_km** parameter specifies the maximum range of the range-azimuth grid, in kilometers. If this parameter is specified and not **rmw_scale**, the radial grid spacing will be **max_range_km / n_range**.

_______________________

.. code-block:: none

  delta_range_km = 10.0;

The **delta_range_km** parameter specifies the spacing of the range rings, in kilometers.

_______________________

.. code-block:: none

  rmw_scale = 0.2;

The **rmw_scale** parameter overrides the **max_range_km** parameter. When this is set the radial grid spacing will be **rmw_scale** in units of the RMW, which varies along the storm track.

tc_rmw output file
~~~~~~~~~~~~~~~~~~

The NetCDF output file contains the following dimensions:

1. *range* - the radial dimension of the range-azimuth grid

2. *azimuth* - the azimuthal dimension of the range-azimuth grid

3. *pressure* - if any pressure levels are specified in the data variable list, they will be sorted and combined into a 3D NetCDF variable, which pressure as the vertical dimension and range and azimuth as the horizontal dimensions

4. *track_point* - the track points corresponding to the model output valid times

For each data variable specified in the data variable list, a corresponding NetCDF variable will be created with the same name and units.
