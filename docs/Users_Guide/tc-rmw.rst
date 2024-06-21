.. _tc-rmw:

***********
TC-RMW Tool
***********

Introduction
============

The TC-RMW tool regrids tropical cyclone model data onto a moving range-azimuth grid centered on points along the storm track provided in ATCF format, most likely the adeck generated from the file. The radial grid spacing can be defined in kilometers or as a factor of the radius of maximum winds (RMW). The azimuthal grid spacing is defined in degrees counter-clockwise from due east. If wind fields are specified in the configuration file, the radial and tangential wind components will be computed. Any regridding method available in MET can be used to interpolate data on the model output grid to the specified range-azimuth grid. The regridding will be done separately on each vertical level. The model data files must coincide with track points in a user provided ATCF formatted track file.

Practical Information
=====================

tc_rmw Usage
------------

The following sections describe the usage statement, required arguments, and optional arguments for tc_rmw.

.. code-block:: none

  Usage: tc_rmw
         -data file_1 ... file_n | data_file_list
         -deck file
         -config file
         -out file
         [-log file]
         [-v level]

tc_rmw has required arguments and can accept several optional arguments.

Required Arguments for tc_rmw
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. The **-data file_1 ... file_n | data_file_list** options specify the gridded data files or an ASCII file containing a list of files to be used.

2. The **-deck source** argument is the ATCF format data source.

3. The **-config file** argument is the configuration file to be used. The contents of the configuration file are discussed below.

4. The **-out** argument is the NetCDF output file to be written.

Optional Arguments for tc_rmw
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

5. The **-log file** option directs output and errors to the specified log file. All messages will be written to that file as well as standard out and error. Thus, users can save the messages without having to redirect the output on the command line. The default behavior is no logfile.

6. The **-v level** option indicates the desired level of verbosity. The contents of "level" will override the default setting of 2. Setting the verbosity to 0 will make the tool run with no log messages, while increasing the verbosity above 1 will increase the amount of logging.

tc_rmw Configuration File
-------------------------

The default configuration file for the TC-RMW tool named **TCRMWConfig_default** can be found in the installed *share/met/config/* directory. It is encouraged for users to copy these default files before modifying their contents. The contents of the configuration file are described in the subsections below.

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

The configuration options listed above are common to many MET tools and are described in :numref:`config_options`. The name and level entries in the data dictionary define the data to be processed.  The regrid dictionary defines if and how regridding will be performed.

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

  delta_range_km = 10.0;

The **delta_range_km** parameter specifies the spacing of the range rings, in kilometers. The range values start with 0 km and extend out to **n_range - 1** times this delta spacing.

_______________________

.. code-block:: none

  rmw_scale = NA;

If changed from its default value of **NA**, the **rmw_scale** parameter overrides the **delta_range_km** parameter. The radial grid spacing is defined using **rmw_scale** in units of the RMW, which varies along the storm track. For example, setting **rmw_scale** to 0.2 would define the delta range spacing as 20% of the radius of maximum winds around each point. Note that RMW is defined in nautical miles but is converted to kilometers for this computation. 

_______________________

.. code-block:: none

  compute_tangential_and_radial_winds = TRUE;

The **compute_tangential_and_radial_winds** parameter is a flag controlling whether a conversion from U/V to Tangential/Radial winds is done or not. If set to TRUE, additional parameters are used, otherwise they are not. 

_______________________

.. code-block:: none

  u_wind_field_name = "UGRD";
  v_wind_field_name = "VGRD";
  
The **u_wind_field_name** and **v_wind_field_name** parameters identify which input data to use in converting to tangential/radial winds. The parameters are used only if **compute_tangential_and_radial_winds** is set to TRUE.

_______________________

.. code-block:: none

  tangential_velocity_field_name = "VT";
  tangential_velocity_long_field_name = "Tangential Velocity";

  
The **tangential_velocity_field_name** and **tangential_velocity_long_field_name** parameters define the field names to give the output tangential velocity grid in the netCDF output file. The parameters are used only if **compute_tangential_and_radial_winds** is set to TRUE.

_______________________

.. code-block:: none

  radial_velocity_field_name = "VT";
  radial_velocity_long_field_name = "Radial Velocity";

  
The **radial_velocity_field_name** and **radial_velocity_long_field_name** parameters define the field names to give the output radial velocity grid in the netCDF output file. The parameters are used only if **compute_radial_and_radial_winds** is set to TRUE.


tc_rmw Output File
------------------

The NetCDF output file contains the following dimensions:

1. *track_point* - the track points corresponding to the model output valid times

2. *pressure* - if any pressure levels are specified in the data variable list, they will be sorted and combined into a 3D NetCDF variable, which pressure as the vertical dimension and range and azimuth as the horizontal dimensions

3. *range* - the radial dimension of the range-azimuth grid

4. *azimuth* - the azimuthal dimension of the range-azimuth grid

For each data variable specified in the data variable list, a corresponding NetCDF variable will be created with the same name and units.
