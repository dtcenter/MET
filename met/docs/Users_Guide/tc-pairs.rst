.. _tc-pairs:

TC-Pairs Tool
=============

Introduction
____________

The TC-Pairs tool provides verification for tropical cyclone forecasts in ATCF file format. It matches an ATCF format tropical cyclone (TC) forecast with a second ATCF format reference TC dataset (most commonly the Best Track analysis). The TC-Pairs tool processes both track and intensity adeck data and probabilistic edeck data. The adeck matched pairs contain position errors, as well as wind, sea level pressure, and distance to land values for each TC dataset. The edeck matched pairs contain probabilistic forecast values and the verifying observation values. The pair generation can be subset based on user-defined filtering criteria. Practical aspects of the TC-Pairs tool are described in :numref:`TC-Pairs_Practical-information`. 

.. _TC-Pairs_Practical-information:

Practical information
_____________________

This section describes how to configure and run the TC-Pairs tool. The TC-Pairs tool is used to match a tropical cyclone model forecast to a corresponding reference dataset. Both tropical cyclone forecast/reference data must be in ATCF format. Output from the TC-dland tool (NetCDF gridded distance file) is also a required input for the TC-Pairs tool. It is recommended to run **tc_pairs** on a storm-by-storm basis, rather than over multiple storms or seasons to avoid memory issues.

tc_pairs usage
~~~~~~~~~~~~~~

The usage statement for tc_pairs is shown below:

.. code-block:: none

  Usage: tc_pairs
         -adeck source and/or -edeck source
         -bdeck source
         -config file
         [-out base]
         [-log file]
         [-v level]

tc_pairs has required arguments and can accept several optional arguments.

Required arguments for tc_pairs
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. The **-adeck source** argument indicates the adeck ATCF format data source containing tropical cyclone model forecast (output from tracker) data to be verified. It specifies the name of an ATCF format file or top-level directory containing ATCF format files ending in “.dat” to be processed. The **-adeck** or **-edeck** option must be used at least once.

2. The **-edeck source** argument indicates the edeck ATCF format data source containing probabilistic track data to be verified. It specifies the name of an ATCF format file or top-level directory containing ATCF format files ending in “.dat” to be processed. The **-adeck** or **-edeck** option must be used at least once.

3. The **-bdeck source** argument indicates the ATCF format data source containing the tropical cyclone reference dataset to be used for verifying the adeck source. It specifies the name of an ATCF format file or top-level directory containing ATCF format files ending in “.dat” to be processed. This source is expected to be the NHC Best Track Analysis, but could also be any ATCF-formatted reference.

4. The **-config file** argument indicates the name of the configuration file to be used. The contents of the configuration file are discussed below.

Optional arguments for tc_pairs
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

5. The -**out base** argument indicates the path of the output file base. This argument overrides the default output file base (**./out_tcmpr**).

6. The **-log file** option directs output and errors to the specified log file. All messages will be written to that file as well as standard out and error. Thus, users can save the messages without having to redirect the output on the command line. The default behavior is no log file. 

7. The **-v level** option indicates the desired level of verbosity. The contents of “level” will override the default setting of 2. Setting the verbosity to 0 will make the tool run with no log messages, while increasing the verbosity above 1 will increase the amount of logging.

This tool currently only supports the rapid intensification (**RI**) edeck probability type but support for additional edeck probability types will be added in future releases. At least one **-adeck** or **-edeck** option must be specified. The **-adeck, -edeck**, and **-bdeck** options may optionally be followed with **suffix=string** to append that string to all model names found within that data source. This option may be useful when processing track data from two different sources which reuse the same model names.

An example of the tc_pairs calling sequence is shown below:

.. code-block:: none

  tc_pairs -adeck aal092010.dat -bdeck bal092010.dat -config TCPairsConfig

In this example, the TC-Pairs tool matches the model track (aal092010.dat) and the best track analysis (bal092010.dat) for the 9th Atlantic Basin storm in 2010. The track matching and subsequent error information is generated with configuration options specified in the **TCPairsConfig** file.

The TC-Pairs tool implements the following logic:

• Parse the adeck, edeck, and bdeck data files and store them as track objects.

• Apply configuration file settings to filter the adeck, edeck, and bdeck track data down to a subset of interest.

• Apply configuration file settings to derive additional adeck track data, such as interpolated tracks, consensus tracks, time-lagged tracks, and statistical track and intensity models.

• For each adeck track that was parsed or derived, search for a matching bdeck track with the same basin and cyclone number and overlapping valid times. If not matching against the BEST track, also ensure that the model initialization times match.

• For each adeck/bdeck track pair, match up their track points in time, lookup distances to land, compute track location errors, and write an output TCMPR line for each track point.

• For each set of edeck probabilities that were parsed, search for a matching bdeck track.

• For each edeck/bdeck pair, write paired edeck probabilities and matching bdeck values to output PROBRIRW lines.

tc_pairs configuration file
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The default configuration file for the TC-Pairs tool named **'TCPairsConfig_default'** can be found in the installed **share/met/config/** directory. Users are encouraged to copy these default files before modifying their contents. The contents of the configuration file are described in the subsections below.

The contents of the tc_pairs configuration file are described below.

____________________

.. code-block:: none

  storm_id     = [];
  basin        = [];
  cyclone      = [];
  storm_name   = [];
  init_beg     = "";
  init_end     = "";
  init_inc     = [];
  init_exc     = [];
  valid_beg    = "";
  valid_end    = "";
  init_hour    = [];
  init_mask    = [];
  lead_req     = [];
  valid_mask   = [];
  match_points = TRUE;
  version      = "VN.N";

The configuration options listed above are common to multiple MET tools and are described in :numref:`Data IO MET-TC Configuration File Options`.

____________________

.. code-block:: none

  model = [ "DSHP", "LGEM", "HWRF" ];

The **model** variable contains a list of comma-separated models to be used. Each model is identified with an ATCF TECH ID (normally four unique characters). This model identifier should match the model column in the ATCF format input file. An empty list indicates that all models in the input file(s) will be processed. Note that when reading ATCF track data, all instances of the string AVN are automatically replaced with GFS.

____________________

.. code-block:: none

  check_dup = FALSE;

The **check_dup** flag expects either TRUE and FALSE, indicating whether the code should check for duplicate ATCF lines when building tracks. Setting **check_dup** to TRUE will check for duplicated lines, and produce output information regarding the duplicate. Any duplicated ATCF line will not be processed in the tc_pairs output. Setting **check_dup** to FALSE, will still exclude tracks that decrease with time, and will overwrite repeated lines, but specific duplicate log information will not be output. Setting **check_dup** to FALSE will make parsing the track quicker.

____________________

.. code-block:: none

  interp12 = NONE;

The **interp12** flag expects the entry NONE, FILL, or REPLACE, indicating whether special processing should be performed for interpolated forecasts. The NONE option indicates no changes are made to the interpolated forecasts. The FILL and REPLACE (default) options determine when the 12-hour interpolated forecast (normally indicated with a "2" or "3" at the end of the ATCF ID) will be renamed with the 6-hour interpolated ATCF ID (normally indicated with the letter "I" at the end of the ATCF ID). The FILL option renames the 12-hour interpolated forecasts with the 6-hour interpolated forecast ATCF ID only when the 6-hour interpolated forecasts is missing (in the case of a 6-hour interpolated forecast which only occurs every 12-hours (e.g. EMXI, EGRI), the 6-hour interpolated forecasts will be "filled in" with the 12-hour interpolated forecasts in order to provide a record every 6-hours). The REPLACE option renames all 12-hour interpolated forecasts with the 6-hour interpolated forecasts ATCF ID regardless of whether the 6-hour interpolated forecast exists. The original 12-hour ATCF ID will also be retained in the output file (all modified ATCF entries will appear at the end of the TC-Pairs output file). This functionality expects both the 12-hour and 6-hour early (interpolated) ATCF IDs to be listed in the model field.

____________________

.. code-block:: none

  consensus = [
     {
        name     = "CON1";
        members  = [ "MOD1", "MOD2", "MOD3" ];
        required = [   true,  false, false  ];
        min_req  = 2;
     }
  ];

The **consensus** field allows the user to generate a user-defined consensus forecasts from any number of models. All models used in the consensus forecast need to be included in the **model** field (first entry in **TCPairsConfig_default**). The name field is the desired consensus model name. The **members** field is a comma-separated list of model IDs that make up the members of the consensus. The **required** field is a comma-separated list of true/false values associated with each consensus member. If a member is designated as true, the member is required to be present in order for the consensus to be generated. If a member is false, the consensus will be generated regardless of whether the member is present. The length of the required array must be the same length as the members array. The **min_req** field is the number of members required in order for the consensus to be computed. The required and min_req field options are applied at each forecast lead time. If any member of the consensus has a non-valid position or intensity value, the consensus for that valid time will not be generated.

____________________

.. code-block:: none

  lag_time = [ “06”, “12” ];

The **lag_time** field is a comma-separated list of forecast lag times to be used in HH[MMSS] format. For each adeck track identified, a lagged track will be derived for each entry. In the tc_pairs output, the original adeck record will be retained, with the lagged entry listed as the adeck name with "_LAG_HH" appended.

____________________

.. code-block:: none

  best_technique = [ "BEST" ];
  best_baseline  = [ "BCLP", "BCD5", "BCLA" ];

The **best_technique** field specifies a comma-separated list of technique name(s) to be interpreted as BEST track data. The default value (BEST) should suffice for most users. The **best_baseline** field specifies a comma-separated list of CLIPER/SHIFOR baseline forecasts to be derived from the best tracks. Specifying multiple **best_technique** values and at least one **best_baseline** value results in a warning since the derived baseline forecast technique names may be used multiple times.

The following are valid baselines for the **best_baseline** field:

**BTCLIP**: Neumann original 3-day CLIPER in best track mode. Used for the Atlantic basin only. Specify model as BCLP.

**BTCLIP5**: 5-day CLIPER (:ref:`Aberson, 1998 <Aberson-1998>`)/SHIFOR (:ref:`DeMaria and Knaff, 2003 <Knaff-2003>` in best track mode for either Atlantic or eastern North Pacific basins. Specify model as BCS5.

**BTCLIPA**: Sim Aberson's recreation of Neumann original 3-day CLIPER in best-track mode. Used for Atlantic basin only. Specify model as BCLA.

____________________

.. code-block:: none

  oper_technique = [ "CARQ" ];
  oper_baseline  = [ "OCLP", "OCS5", "OCD5" ];

The **oper_technique** field specifies a comma-separated list of technique name(s) to be interpreted as operational track data. The default value (CARQ) should suffice for most users. The **oper_baseline** field specifies a comma-separated list of CLIPER/SHIFOR baseline forecasts to be derived from the operational tracks. Specifying multiple **oper_technique** values and at least one **oper_baseline** value results in a warning since the derived baseline forecast technique names may be used multiple times.

The following are valid baselines for the **oper_baseline** field:

**OCLIP**: Merrill modified (operational) 3-day CLIPER run in operational mode. Used for Atlantic basin only. Specify model as OCLP.

**OCLIP5**: 5-day CLIPER (:ref:`Aberson, 1998 <Aberson-1998>`)/ SHIFOR (:ref:`DeMaria and Knaff, 2003 <Knaff-2003>`) in operational mode, rerun using CARQ data. Specify model as OCS5.

**OCLIPD5**: 5-day CLIPER (:ref:`Aberson, 1998 <Aberson-1998>`)/ DECAY-SHIFOR (:ref:`DeMaria and Knaff, 2003 <Knaff-2003>`). Specify model as OCD5.

____________________

.. code-block:: none

  anly_track = BDECK;

Analysis tracks consist of multiple track points with a lead time of zero for the same storm. An analysis track may be generated by running model analysis fields through a tracking algorithm. The **anly_track** field specifies which datasets should be searched for analysis track data and may be set to **NONE, ADECK, BDECK**, or **BOTH**. Use **BOTH** to create pairs using two different analysis tracks.

____________________

.. code-block:: none

  match_points = TRUE;

The **match_points** field specifies whether only those track points common to both the adeck and bdeck tracks should be written out. If **match_points** is selected as FALSE, the union of the adeck and bdeck tracks will be written out, with "NA" listed for unmatched data.

____________________

.. code-block:: none

  dland_file = "MET_BASE/tc_data/dland_global_tenth_degree.nc";

The **dland_file** string specifies the path of the NetCDF format file (default file: dland_global_tenth_degree.nc) to be used for the distance to land check in the **tc_pairs code**. This file is generated using tc_dland (default file provided in installed **share/met/tc_data** directory).

____________________

.. code-block:: none

 watch_warn = {
     file_name   = "MET_BASE/tc_data/wwpts_us.txt";
     time_offset = -14400;
  }

The **watch_warn** field specifies the file name and time applied offset to the **watch_warn** flag. The **file_name** string specifies the path of the watch/warning file to be used to determine when a watch or warning is in affect during the forecast initialization and verification times. The default file is named **wwpts_us.txt**, which is found in the installed **share/met/tc_data/** directory within the MET build. The **time_offset** string is the time window (in seconds) assigned to the watch/warning. Due to the non-uniform time watches and warnings are issued, a time window is assigned for which watch/warnings are included in the verification for each valid time. The default watch/warn file is static, and therefore may not include warned storms beyond the current MET code release date; therefore users may wish to contact met_help@ucar.edu to obtain the most recent watch/warning file if the static file does not contain storms of interest.

.. code-block:: none

  basin_map = [
     { key = "SI"; val = "SH"; },
     { key = "SP"; val = "SH"; },
     { key = "AU"; val = "SH"; },
     { key = "AB"; val = "IO"; },
     { key = "BB"; val = "IO"; }
  ];

The **basin_map** entry defines a mapping of input names to output values.
Whenever the basin string matches "key" in the input ATCF files, it is
replaced with "val". This map can be used to modify basin names to make them
consistent across the ATCF input files.

Many global modeling centers use ATCF basin identifiers based on region
(e.g., 'SP' for South Pacific Ocean, etc.), however the best track data
provided by the Joint Typhoon Warning Center (JTWC) use just one basin
identifier 'SH' for all of the Southern Hemisphere basins. Additionally,
some modeling centers may report basin identifiers separately for the Bay
of Bengal (BB) and Arabian Sea (AB) whereas JTWC uses 'IO'.

The basin mapping allows MET to map the basin identifiers to the expected
values without having to modify your data. For example, the first entry
in the list below indicates that any data entries for 'SI' will be matched
as if they were 'SH'. In this manner, all verification results for the
Southern Hemisphere basins will be reported together as one basin.

An empty list indicates that no basin mapping should be used. Use this if
you are not using JTWC best tracks and you would like to match explicitly
by basin or sub-basin. Note that if your model data and best track do not
use the same basin identifier conventions, using an empty list for this
parameter will result in missed matches.

.. _tc_pairs-output:

tc_pairs output
~~~~~~~~~~~~~~~

TC-Pairs produces output in TCST format. The default output file name can be overwritten using the -out file argument in the usage statement. The TCST file output from TC-Pairs may be used as input into the TC-Stat tool. The header column in the TC-Pairs output is described in :numref:`TCST Header`.

.. _TCST Header:

.. list-table:: Header information for TC-Pairs TCST output.
  :widths: auto
  :header-rows: 2

  * - 
    - 
    - HEADER
  * - Column Number
    - Header Column Name
    - Description
  * - 1
    - VERSION
    - Version number
  * - 2
    - AMODEL
    - User provided text string designating model name
  * - 3
    - BMODEL
    - User provided text string designating model name
  * - 4
    - STORM_ID
    - BBCCYYY designation of storm
  * - 5
    - BASIN
    - Basin (BB in STORM_ID)
  * - 6
    - CYCLONE
    - Cyclone number (CC in STORM_ID)
  * - 7
    - STORM_NAME
    - Name of Storm
  * - 8
    - INIT
    - Initialization time of forecast in YYYYMMDD_HHMMSS format.
  * - 9
    - LEAD
    - Forecast lead time in HHMMSS format.
  * - 10
    - VALID
    - Forecast valid time in YYYYMMDD_HHMMSS format.
  * - 11
    - INIT_MASK
    - Initialization time masking grid applied
  * - 12
    - VALID_MASK
    - Valid time masking grid applied
  * - 13
    - LINE_TYPE
    - Output line type (TCMPR or PROBRI)

.. _TCMPR Line Type:

.. list-table:: Format information for TCMPR (Tropical Cyclone Matched Pairs) output line type.
  :widths: auto
  :header-rows: 2

  * - 
    - 
    - TCMPR OUTPUT FORMAT
  * - Column Number
    - Header Column Name
    - Description
  * - 13
    - TCMPR
    - Tropical Cyclone Matched Pair line type
  * - 14
    - TOTAL
    - Total number of pairs in track
  * - 15
    - INDEX
    - Index of the current track pair
  * - 16
    - LEVEL
    - Level of storm classification
  * - 17
    - WATCH_WARN
    - HU or TS watch or warning in effect
  * - 18
    - INITIALS
    - Forecaster initials
  * - 19
    - ALAT
    - Latitude position of adeck model
  * - 20
    - ALON
    - Longitude position of adeck model
  * - 21
    - BLAT
    - Latitude position of bdeck model
  * - 22
    - BLON
    - Longitude position of bdeck model
  * - 23
    - TK_ERR
    - Track error of adeck relative to bdeck (nm)
  * - 24
    - X_ERR
    - X component position error (nm)
  * - 25
    - Y_ERR
    - Y component position error (nm)
  * - 26
    - ALTK_ERR
    - Along track error (nm)
  * - 27
    - CRTK_ERR
    - Cross track error (nm)
  * - 28
    - ADLAND
    - adeck distance to land (nm)
  * - 29
    - BDLAND
    - bdeck distance to land (nm)
  * - 30
    - AMSLP
    - adeck mean sea level pressure
  * - 31
    - BMSLP
    - bdeck mean sea level pressure
  * - 32
    - AMAX_WIND
    - adeck maximum wind speed
  * - 33
    - BMAX_WIND
    - bdeck maximum wind speed
  * - 34, 35
    - A/BAL_WIND_34
    - a/bdeck 34-knot radius winds in full circle
  * - 36, 37
    - A/BNE_WIND_34
    - a/bdeck 34-knot radius winds in NE quadrant
  * - 38, 39
    - A/BSE_WIND_34
    - a/bdeck 34-knot radius winds in SE quadrant
  * - 40, 41
    - A/BSW_WIND_34
    - a/bdeck 34-knot radius winds in SW quadrant
  * - 42, 43
    - A/BNW_WIND_34
    - a/bdeck 34-knot radius winds in NW quadrant
  * - 44, 45
    - A/BAL_WIND_50
    - a/bdeck 50-knot radius winds in full circle
  * - 46, 47
    - A/BNE_WIND_50
    - a/bdeck 50-knot radius winds in NE quadrant
  * - 48, 49
    - A/BSE_WIND_50
    - a/bdeck 50-knot radius winds in SE quadrant
  * - 50, 51
    - A/BSW_WIND_50
    - a/bdeck 50-knot radius winds in SW quadrant
  * - 52, 53
    - A/BNW_WIND_50
    - a/bdeck 50-knot radius winds in NW quadrant
  * - 54, 55
    - A/BAL_WIND_64
    - a/bdeck 64-knot radius winds in full circle
  * - 56, 57
    - A/BNE_WIND_64
    - a/bdeck 64-knot radius winds in NE quadrant
  * - 58, 59
    - A/BSE_WIND_64
    - a/bdeck 64-knot radius winds in SE quadrant
  * - 60, 61
    - A/BSW_WIND_64
    - a/bdeck 64-knot radius winds in SW quadrant
  * - 62, 63
    - A/BNW_WIND_64
    - a/bdeck 64-knot radius winds in NW quadrant
  * - 64, 65
    - A/BRADP
    - pressure in millibars of the last closed isobar, 900 - 1050 mb
  * - 66, 67
    - A/BRRP
    - radius of the last closed isobar in nm, 0 - 9999 nm
  * - 68, 69
    - A/BMRD
    - radius of max winds, 0 - 999 nm
  * - 70, 71
    - A/BGUSTS
    - gusts, 0 through 995 kts
  * - 72, 73
    - A/BEYE
    - eye diameter, 0 through 999 nm
  * - 74, 75
    - A/BDIR
    - storm direction in compass coordinates, 0 - 359 degrees
  * - 76, 77
    - A/BSPEED
    - storm speed, 0 - 999 kts
  * - 78, 79
    - A/BDEPTH
    - system depth, D-deep, M-medium, S-shallow, X-unknown

.. _PROBRI Line Type:

.. list-table:: Format information for PROBRIRW (Probability of Rapid Intensification) output line type.
  :widths: auto
  :header-rows: 2

  * - 
    - 
    - PROBRIRW OUTPUT FORMAT
  * - Column Number
    - Header Column Name
    - Description
  * - 13
    - PROBRI
    - Probability of Rapid Intensification line type
  * - 14
    - ALAT
    - Latitude position of edeck model
  * - 15
    - ALON
    - Longitude position of edeck model
  * - 16
    - BLAT
    - Latitude position of bdeck model
  * - 17
    - BLON
    - Longitude position of bdeck model
  * - 18
    - INITIALS
    - Forecaster initials
  * - 19
    - TK_ERR
    - Track error of adeck relative to bdeck (nm)
  * - 20
    - X_ERR
    - X component position error (nm)
  * - 21
    - Y_ERR
    - Y component position error (nm)
  * - 22
    - ADLAND
    - adeck distance to land (nm)
  * - 23
    - BDLAND
    - bdeck distance to land (nm)
  * - 24
    - RI_BEG
    - Start of RI time window in HH format
  * - 25
    - RI_END
    - End of RI time window in HH format
  * - 26
    - RI_WINDOW
    - Width of RI time window in HH format
  * - 27
    - AWIND_END
    - Forecast maximum wind speed at RI end
  * - 28
    - BWIND_BEG
    - Best track maximum wind speed at RI begin
  * - 29
    - BWIND_END
    - Best track maximum wind speed at RI end
  * - 30
    - BDELTA
    - Exact Best track wind speed change in RI window
  * - 31
    - BDELTA_MAX
    - Maximum Best track wind speed change in RI window
  * - 32
    - BLEVEL_BEG
    - Best track storm classification at RI begin
  * - 33
    - BLEVEL_END
    - Best track storm classification at RI end
  * - 34
    - N_THRESH
    - Number of probability thresholds
  * - 35
    - THRESH_i
    - The ith probability threshold value (repeated)
  * - 36
    - PROB_i
    - The ith probability value (repeated)
