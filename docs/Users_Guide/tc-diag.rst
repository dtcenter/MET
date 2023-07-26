.. _tc-diag:

************
TC-Diag Tool
************

Introduction
============

.. note:: As of MET version 11.1.0, the TC-Diag tool is a beta release that lacks full functionality. The current version of the tool generates intermediate NetCDF output files of the input model’s data transformed onto an azimuth-range grid. When the full functionality of the tc_diag tool is released in MET v12.0.0, the tool will also output environmental diagnostics computed from callable Python scripts. For now, each time it is run, the warning message listed below is printed.

.. code-block:: none

  WARNING:
  WARNING: The TC-Diag tool is provided in BETA status for MET V11.1.0.
  WARNING: Please see the release notes of future MET versions for updates.
  WARNING:

A diagnosis of the large-scale environment of tropical cyclones (TCs) is foundational for many prediction techniques, including statistical-dynamical forecast aids and techniques based on artificial intelligence. Such diagnostics can also be used by forecasters seeking to understand how a given model's forecast will pan out. Finally, TC diagnostics can be useful in verification to stratify the performance of models in different environmental regimes over a longer period of time, thereby providing useful insights on model biases or deficiencies for model developers and forecasters.

Originally developed for the Statistical Hurricane Intensity Prediction Scheme (SHIPS), and later as a stand-alone package called 'Model Diagnostics', by the Cooperative Institute for Research in the Atmosphere (CIRA), MET now integrates these capabilities into the an extensible framework called the TC-Diag tool. This tool allows users to compute diagnostics for the large-scale environment of TCs using ATCF track and gridded model data inputs. The current version of the TC-Diag tool requires that the tracks and fields be self-consistent [i.e., the track should be the model's (or ensemble's) own predicted track(s)]. The reason is that the diagnostics are computed in a coordinate system centered on the model's moving model storm and the current version of the tool does not yet include vortex removal. If the track is not consistent with the underlying fields, the diagnostics output are unlikely to be useful because the model's simulated storm would contaminate the diagnostics calculations.

.. note:: A future version of the tool will include the capability to remove the model's own vortex, which will allow the user to specify any arbitrary track (such as the operational center's official forecast). Until then, users are advised that the track selected must be consistent with the model's predicted track.

TC-Diag is run once for each initialization time to produce diagnostics for each user-specified combination of TC tracks and model fields. The user provides track data (such as one or more ATCF a-deck track files), along with track filtering criteria as needed, to select one or more tracks to be processed. The user also provides gridded model data from which diagnostics should be computed. Gridded data can be provided for multiple concurrent storms, multiple models, and/or multiple domains (i.e. parent and nest) in a single run.

TC-Diag first determines the list of valid times that appear in any one of the tracks. For each valid time, it processes all track points for that time. For each track point, it reads the gridded model fields requested in the configuration file and transforms the gridded data to a range-azimuth cylindrical coordinates grid. For each domain, it writes the range-azimuth data to a temporary NetCDF file.

.. note:: The current version of the tool does not yet include the capabilities described in the next three paragraphs. These additional capabilities are planned to be added in the MET v12.0.0 release later in 2023.

Once the input data have been processed into the temporary NetCDF files, TC-Diag then calls one or more Python diagnostics scripts, as specified in the configuration file, to compute tropical cyclone diagnostic values. The computed diagnostics values are retrieved from the Python script and stored in memory.

After processing all valid times and all corresponding track points, the computed diagnostics are written to ASCII and/or NetCDF output files. If requested in the configuration file, the temporary range-azimuth cylindrical coordinates files are combined into a single NetCDF file and written to the output for each combination of model track and domain.

The default Python diagnostics scripts included with the MET release provide the standard set of CIRA diagnostics. However, users can copy/modify the logic in those scripts as they see fit to refine and/or add to the diagnostics computed.

.. _tc-diag_practical_info:

Practical information
=====================

tc_diag usage
-------------

The following sections describe the usage statement, required arguments, and optional arguments for tc_diag.

.. code-block:: none

  Usage: tc_diag
         -data domain tech_id_list [ file_1 ... file_n | data_file_list ]
         -deck file
         -config file
         [-outdir path]
         [-log file]
         [-v level]

tc_diag has required arguments and can accept several optional arguments.

Required arguments for tc_diag
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. The **-data domain tech_id_list [ file_1 ... file_n | data_file_list ]** option specifies a domain name, a comma-separated list of ATCF tech ID's, and a list of gridded data files or an ASCII file containing a list of files to be used. Specify **-data** one for each gridded data source.

2. The **-deck source** option is the ATCF format track data source.

3. The **-config file** option is the TCDiagConfig file to be used. The contents of the configuration file are discussed below.

Optional arguments for tc_diag
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

4. The **-outdir path** option overrides the default output directory (current working directory) with the output directory path provided.

5. The **-log file** option directs output and errors to the specified log file. All messages will be written to that file as well as standard out and error. Thus, users can save the messages without having to redirect the output on the command line. The default behavior is no logfile.

6. The **-v level** option indicates the desired level of verbosity. The contents of "level" will override the default setting of 2. Setting the verbosity to 0 will make the tool run with no log messages, while increasing the verbosity above 1 will increase the amount of logging.

tc_diag configuration file
--------------------------

The default configuration file for the TC-Diag tool named **TCDiagConfig_default** can be found in the installed *share/met/config/* directory. Users are encouraged to copy these default files before modifying their contents. The contents of the configuration file are described in the subsections below.

Configuring input tracks and time
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: none

  model = [ "GFSO", "OFCL" ];
  storm_id = "";
  basin = "";
  cyclone = "";
  init_inc = "";
  valid_beg = "";
  valid_end = "";
  valid_inc = [];
  valid_exc = [];
  valid_hour = [];

The TC-Diag tool should be configured to filter the input track data (**-deck**) down to the subset of tracks that correspond to the gridded data files provided (**-data**). The filtered tracks should contain data for only *one initialization time* but may contain tracks for multiple models.

The configuration options listed above are used to filter the input track data down to those that should be processed in the current run. These options are common to multiple MET tools and are described in :numref:`config_options_tc`.

.. code-block:: none

lead = [   "0",    "6",  "12",  "18",  "24",
          "30",   "36",  "42",  "48",  "54",
          "60",   "66",  "72",  "78",  "84",
          "90",   "96", "102", "108", "114",
          "120", "126" ];

The **lead** entry is an array of strings specifying lead times in HH[MMSS] format. By default, diagnostics are computed every 6 hours out to 126 hours. Lead times for which no track point or gridded model data exist produce a warning message and diagnostics set to a missing data value.

Configuring domain information
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: none

  diag_script = [ "MET_BASE/python/tc_diag/compute_tc_diagnostics.py" ];

  domain_info = [
     {
        domain         = "parent";
        n_range        = 150;
        n_azimuth      = 8;
        delta_range_km = 10.0;
     },
     {
        domain         = "nest";
        n_range        = 150;
        n_azimuth      = 8;
        delta_range_km = 2.0;
     }
  ];

The **domain_info** entry is an array of dictionaries. Each dictionary consists of five entries. The **domain** entry is a user-specified string that provides a name for the domain. Each **domain** name must also appear in a **-deck** command line option, and the reverse is also true.

The **n_range** entry is an integer specifying the number of equally spaced range intervals in the range-azimuth grid to be used for this data source.

The **n_azimuth** entry is an integer specifying the number of equally spaced azimuth intervals in the range-azimuth grid to be used for this data source. The azimuthal grid spacing is 360 / **n_azimuth** degrees.

The **delta_range_km** entry is a floating point value specifying the spacing of the range rings in kilometers.

The **diag_script** entry is an array of strings. Each string specifies the path to a Python script to be executed to compute diagnostics from the transformed cylindrical coordinates data for this domain. While the **diag_script** entry can be specified separately for each **domain_info** array entry, specifying it once at a higher level of context, as seen above, allows the same setting to be applied to all array entries. When multiple Python diagnostics scripts are run, the union of the diagnostics computed are written to the output.

.. note:: As of MET version 11.1.0, no tropical cyclone diagnostics are actually computed or written to the output.

Configuring data censoring and conversion options
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: none

  censor_thresh = [];
  censor_val    = [];
  convert(x)    = x;

These data censoring and conversion options are common to multiple MET tools and are described in :numref:`config_options`. They can be specified separately in each **data.field** array entry, described below. If provided, those operations are performed after reading the gridded data but prior to converting to the cylindrical coordinate range-azimuth grid.

Configuring fields, levels, and domains
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: none

  data = {

     // If empty, the field is processed for all domains
     domain = [];

     // Pressure levels to be used, unless overridden below
     level = [ "P1000", "P925", "P850", "P700", "P500",
               "P400",  "P300", "P250", "P200", "P150",
               "P100" ];

     field = [
        { name = "TMP";                  },
        { name = "UGRD";                 },
        { name = "VGRD";                 },
        { name = "RH";                   },
        { name = "HGT";                  },
        { name = "PRMSL"; level = "Z0";  },
        { name = "PWAT";  level = "L0";  },
        { name = "TMP";   level = "Z0";  },
        { name = "TMP";   level = "Z2";  },
        { name = "RH";    level = "Z2";  },
        { name = "UGRD";  level = "Z10"; },
        { name = "VGRD";  level = "Z10"; }
     ];
  }

The **data** entry is a dictionary that contains the **field** entry to define what gridded data should be processed. The **field** entry is an array of dictionaries. Each **field** dictionary consists of at least three entries.

The **name** and **level** entries are common to multiple MET tools and are described in :numref:`config_options`.

The **domain** entry is an array of strings. Each string specifies a domain name. If the **domain_info** domain name appears in this **domain** list, then this field will be read from that **domain_info** data source. If **domain** is set to an empty list, then this field will be read from all domain data sources.

Configuring regridding options
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: none

  regrid = { ... }

These **regrid** dictionary is common to multiple MET tools and is described in :numref:`config_options`. These regridding options control the transformation to cylindrical coordinates.

.. note:: As of MET version 11.1.0, the nearest neighbor regridding method is used rather than this configuration file option.

Configuring vortex removal option
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: none

  vortex_removel = FALSE;

The **vortex_removal** flag entry is a boolean specifying whether or not vortex removal logic should be applied.

.. note:: As of MET version 11.1.0, vortex removal logic is not yet supported.

Configuring data input and output options
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: none

  one_time_per_file_flag = TRUE;

The **one_time_per_file_flag** entry controls the logic for reading data from input files. Users should set this to true or false to describe how data is stored in the gridded input files specified with the **-data** command line option. Set this to true if each input file contains all of the data for a single initialization time and for a single valid time. If the input files contain data for multiple initialization or valid times, or if data for one valid time is spread across multiple files, set this to false.

If true, all input fields are read efficiently from each file in a single call. If false, each field is processed separately in a less efficient manner.

.. code-block:: none

  nc_rng_azi_flag = TRUE;
  nc_diag_flag    = FALSE;
  cira_diag_flag  = FALSE;

These three flag entries are booleans specifying what output data types should be written. The **nc_rng_azi_flag** entry controls the writing of a NetCDF file containing the cylindrical coordinate range-azimuth data used to compute the diagnostics. The **nc_diag_file** entry controls the writing of the computed diagnostics to a NetCDF file. The **cira_diag_flage** entry controls the writing of the computed diagnostics to a formatted ASCII output file. At least one of these flags must be set to true.

.. note:: As of MET version 11.1.0, **nc_rng_azi_flag** is the only supported output type. These configuration options will automatically be reset at runtime to the settings listed above.

Configuring MET version, output prefix, and temp directory
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: none

  tmp_dir       = "/tmp";
  output_prefix = "";
  version       = "V11.1.0";

These options are common to multiple MET tools and are described in :numref:`config_options`.

tc_diag output
--------------

The TC-Diag tool writes up to three output data types, as specified by flags in the configuration file. Each time TC-Diag is run it processes track data for a single initialization time. The actual number of output files varies depending on the number of model tracks provided.

.. note:: As of MET version 11.1.0, **nc_rng_azi_flag** is the only supported output type.

**CIRA Diagnostics Output**

When the **cira_diag_flag** configuration entry is set to true, an ASCII CIRA diagnostics output file is written for each model track provided.

Details will be provided when support for this output type is added.

**NetCDF Diagnostics Output**

When the **nc_diag_flag** configuration entry is set to true, a NetCDF output file containing the computed diagnostics is written for each model track provided.

Details will be provided when support for this output type is added.

**NetCDF Range-Azimuth Output**

When the **nc_rng_azi_flag** configuration entry is set to true, a NetCDF output file containing the cylindrical coordinate range-azimuth data is written for each combination of model track provided and domain specified. For example, if three model tracks are provided and data for both *parent* and *nest* domains are provided, six of these NetCDF output files will be written.

The NetCDF range-azimuth output is named using the following naming convention:

**tc_diag_STORMID_TECH_YYYYMMDDHH_cyl_grid_DOMAIN.nc** where STORMID is the 2-letter basin name, 2-digit storm number, and 4-digit year, TECH is the acronym for the objective technique, YYYYMMDDHH is the track initialization time, and DOMAIN is the domain name.

The NetCDF range-azimuth file contains the dimensions and variables shown in :numref:`table_TC-Diag_Dimensions_NetCDF_range_azimuth` and :numref:`table_TC-Diag_Variables_NetCDF_range_azimuth`.

.. _table_TC-Diag_Dimensions_NetCDF_range_azimuth:

.. list-table:: Dimensions defined in NetCDF Range-Azimuth output
  :widths: auto
  :header-rows: 2

  * - tc_diag NETCDF DIMENSIONS
    -
  * - NetCDF Dimension
    - Description
  * - track_line
    - Dimension for the raw ATCF track lines written to the **TrackLines** variable
  * - time
    - Time dimension for the number of track point valid times
  * - range
    - Dimension for the number of range rings in the range-azimuth grid
  * - azimuth
    - Dimension for the number of azimuths in the range-azimuth grid
  * - pressure
    - Vertical dimension for the number of pressure levels

.. role:: raw-html(raw)
    :format: html

.. _table_TC-Diag_Variables_NetCDF_range_azimuth:

.. list-table:: Variables defined in NetCDF Range-Azimuth output
  :widths: auto
  :header-rows: 2

  * - tc_diag NETCDF VARIABLES
    -
    -
  * - NetCDF Variable
    - Dimension
    - Description
  * - storm_id
    - NA
    - Tropical Cyclone Storm ID (BBNNYYYY) consisting of 2-letter basin name, 2-digit storm number, and 4-digit year
  * - model
    - NA
    - Track ATCF ID model name
  * - TrackLines
    - track_lines
    - Raw input ATCF track lines
  * - TrackLat
    - time
    - Track point location latitude
  * - TrackLon
    - time
    - Track point location longitude
  * - TrackMSLP
    - time
    - Track point minimum sea level pressure
  * - TrackVMax
    - time
    - Track point maximum wind speed
  * - init_time
    - NA
    - Track initialization time string in YYYYMMDD_HHMMSS format
  * - init_time_ut
    - NA
    - Track initialization time string in unixtime (seconds since January 1, 1970) format
  * - valid_time
    - time
    - Track point valid time string in YYYYMMDD_HHMMSS format
  * - valid_time_ut
    - time
    - Track point valid time string in unixtime (seconds since January 1, 1970) format
  * - lead_time
    - time
    - Track point forecast lead time string in HHMMSS format
  * - lead_time_sec
    - time
    - Track point forecast lead time integer number of seconds
  * - range
    - range
    - Range ring coordinate variable in kilometers
  * - azimuth
    - azimuth
    - Azimuth coordinate variable in degrees clockwise from north
  * - pressure
    - pressure
    - Vertical level pressure coordinate variable in millibars
  * - lat
    - time, range, azimuth
    - Latitude in degrees north for each range-azimuth grid point
  * - lon
    - time, range, azimuth
    - Longitude in degrees east for each range-azimuth grid point
  * - single level data
      (e.g. TMP_Z2, PRMSL_L0)
    - time, range, azimuth
    - Gridded range-azimuth data on a single level
  * - pressure level data
      (e.g. TMP, HGT)
    - time, pressure, range, azimuth
    - Gridded range-azimuth data on pressure levels
