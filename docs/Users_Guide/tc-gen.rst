.. _tc-gen:

***********
TC-Gen Tool
***********

Introduction
============

The TC-Gen tool provides verification of deterministic and probabilistic tropical cyclone genesis forecasts in the ATCF file and shapefile formats. Producing reliable tropical cyclone genesis forecasts is an important metric for global numerical weather prediction models. This tool ingests deterministic model output post-processed by genesis tracking software (e.g. GFDL vortex tracker), ATCF edeck files containing probability of genesis forecasts, operational shapefile warning areas, and ATCF reference track dataset(s) (e.g. Best Track analysis and CARQ operational tracks). It writes categorical counts and statistics. The capability to modify the spatial and temporal tolerances when matching forecasts to reference genesis events, as well as scoring those matched pairs, gives users the ability to condition the criteria based on model performance and/or conduct sensitivity analyses. Statistical aspects are outlined in :numref:`tc-gen_stat_aspects` and practical aspects of the TC-Gen tool are described in :numref:`tc-gen_practical_info`.

.. _tc-gen_stat_aspects:

Statistical Aspects
===================

The TC-Gen tool processes both deterministic and probabilistic forecasts.

For deterministic forecasts specified using the **-track** command line option, it identifies genesis events in both the forecasts and reference datasets, typically Best tracks. It applies user-specified configuration options to pair up the forecast and reference genesis events and categorize each pair as a hit, miss, or false alarm.

As with other extreme events (where the event occurs much less frequently than the non-event), the correct negative category is not computed since the non-events would dominate the contingency table. Therefore, only statistics that do not include correct negatives should be considered for this tool. The following CTS statistics are relevant: Base rate (BASER), Mean forecast (FMEAN), Frequency Bias (FBIAS), Probability of Detection (PODY), False Alarm Ratio (FAR), Critical Success Index (CSI), Gilbert Skill Score (GSS), Extreme Dependency Score (EDS), Symmetric Extreme Dependency Score (SEDS), Bias-Adjusted Gilbert Skill Score (BAGSS).

For probabilistic forecasts specified using the **-edeck** command line option, it identifies genesis events in the reference dataset. It applies user-specified configuration options to pair the forecast probabilities to the reference genesis events. These pairs are added to an Nx2 probabilistic contingency table. If the reference genesis event occurs within in the predicted time window, the pair is counted in the observation-yes column. Otherwise, it is added to the observation-no column.

For warning area shapefiles specified using the **-shape** command line option, it processes metadata from the corresponding database files. The database file is assumed to exist at exactly the same path as the shapefile, but with a ".dbf" suffix instead of ".shp". Note that only shapefiles exactly following the NOAA National Hurricane Center's (NHC) "gtwo_areas_YYYYMMDDHHMM.shp" file naming and corresonding metadata conventions are supported. For each shapefile record, the database file defines corresponding probability values for one or more time periods. Percentages may be provided for the probability of genesis inside the shape within 2, 5, or 7 days from issuance time that is parsed from the file name. Note that 5 day probabilities were discontinued in 2023. The 2 and 7 day probabilities are provided in database file fields named "PROB2DAY" and "PROB7DAY", respectively. Care is taken to identify and either ignore or update duplicate shapes found in the input.

The shapes are then subset based on the filtering criteria in the configuration file. For each probability and shape, the reference genesis events are searched for a match within the defined time window. These pairs are added to an Nx2 probabilistic contingency table. The probabilistic contingeny tables and statistics are computed and reported separately for filter defined and lead hour encountered in the input.

Other considerations for interpreting the output of the TC-Gen tool involve the size of the contingency table output. The size of the contingency table will change depending on the number of matches. Additionally, the number of misses is based on the forecast duration and interval (specified in the configuration file). This change is due to the number of model opportunities to forecast the event, which is determined by the specified duration/interval.

Care should be taken when interpreting the statistics for filtered data. In some cases, variables (e.g. storm name) are only available in either the forecast or reference datasets, rather than both. When filtering on a field that is only present in one dataset, the contingency table counts will be impacted. Similarly, the initialization field only impacts the model forecast data. If the valid time (which will impact the reference dataset) isn't also specified, the forecasts will be filtered and matched such that the number of misses will erroneously increase. See :numref:`tc-gen_practical_info` for more detail.

.. _tc-gen_practical_info:

Practical Information
=====================

This section describes how to configure and run the TC-Gen tool. The following sections describe the usage statement, required arguments, and optional arguments for tc_gen.

tc_gen Usage
------------

The usage statement for tc_gen is shown below:

.. code-block:: none

  Usage: tc_gen
         -genesis source
         -edeck source
         -shape source
         -track source 
         -config file
         [-out base]
         [-log file]
         [-v level]

TC-Gen has three required arguments and accepts optional ones.

Required Arguments for tc_gen
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. The **-genesis source** argument is the path to one or more ATCF or fort.66 (see documentation listed below) files generated by the Geophysical Fluid Dynamics Laboratory (GFDL) Vortex Tracker when run in tcgen mode or an ASCII file list or a top-level directory containing them. The required file format is described in the "Output formats" section of the `GFDL Vortex Tracker users guide. <https://dtcenter.org/sites/default/files/community-code/gfdl/standalone_tracker_UG_v3.9a.pdf>`_

2. The **-edeck source** argument is the path to one or more ATCF edeck files, an ASCII file list containing them, or a top-level directory with files matching the regular expression ".dat". The probability of genesis are read from each edeck input file and verified against at the **-track** data.

3. The **-shape source** argument is the path to one or more NHC genesis warning area shapefiles, an ASCII file list containing them, or a top-level directory with files matching the regular expression "gtwo_areas.*.shp". The genesis warning areas and corresponding forecast probability values area verified against the **-track** data.

Note: At least one of the **-genesis**, **-edeck**, or **-shape** command line options are required.

4. The **-track source** argument is one or more ATCF reference track files or an ASCII file list or top-level directory containing them, with files ending in ".dat". This tool processes either Best track data from bdeck files, or operational track data (e.g. CARQ) from adeck files, or both. Providing both bdeck and adeck files will result in a richer dataset to match with the **-genesis** files.  Both adeck and bdeck data should be provided using the **-track** option. The **-track** option must be used at least once.

5. The **-config** file argument indicates the name of the configuration file to be used. The contents of the configuration file are discussed below.

Optional Arguments for tc_gen
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

6. The **-out base** argument indicates the path of the output file base. This argument overrides the default output file base (./tc_gen)

7. The **-log file** option directs output and errors to the specified log file. All messages will be written to that file as well as standard out and error. Thus, users can save the messages without having to redirect the output on the command line. The default behavior is no log file.

8. The **-v level** option indicates the desired level of verbosity. The contents of "level" will override the default setting of 2. Setting the verbosity to 0 will make the tool run with no log messages, while increasing the verbosity above 1 will increase the amount of logging.

Scoring Logic
^^^^^^^^^^^^^

The TC-Gen tool implements the following logic:

* Parse the Best and operational track data, and identify Best track genesis events. Note that Best tracks with a cyclone number greater than 50 are automatically discarded from the analysis. Large cyclone numbers are used for pre-season testing or to track invests prior to a storm actually forming. Running this tool at verbosity level 6 (-v 6) prints details about which tracks are discarded.

* For **-track** inputs:

 * Parse the forecast genesis data and identify forecast genesis events separately for each model present.

 * Loop over the filters defined in the configuration file and apply the following logic for each.

  * For each Best track genesis event meeting the filter critera, determine the initialization and lead times for which the model had an opportunity to forecast that genesis event. Store an unmatched genesis pair for each case.
 
  * For each forecast genesis event, search for a matching Best track. A configurable boolean option controls whether all Best track points are considered for a match or only the single Best track genesis point. A match occurs if the Best track point valid time is within a configurable window around the forecast genesis time and the Best track point location is within a configurable radius of the forecast genesis location. If a Best track match is found, store the storm ID.
 
  * If no Best track match is found, apply the same logic to search the operational track points with lead time of 0 hours. If an operational match is found, store the storm ID.
 
  * If a matching storm ID is found, match the forecast genesis event to the Best track genesis event for that storm ID.
 
  * If no matching storm ID is found, store an unmatched pair for the genesis forecast.

  * Loop through the genesis pairs and populate contingency tables using two methods, the development (dev) and operational (ops) methods. For each pair, if the forecast genesis event is unmatched, score it as a dev and ops FALSE ALARM. If the Best track genesis event is unmatched, score it as a dev and ops MISS. Score each matched genesis pair as follows:

   * If the forecast initialization time is at or after the Best track genesis event, DISCARD this case and exclude it from the statistics.
  
   * Compute the difference between the forecast and Best track genesis events in time and space. If they are both within the configurable tolerance, score it as a dev HIT. If not, score it as a dev FALSE ALARM.
  
   * Compute the difference between the Best track genesis time and model initialization time. If it is within the configurable tolerance, score it as an ops HIT. If not, score it as an ops FALSE ALARM.

  * Do not count any CORRECT NEGATIVES.

 * Report the contingency table hits, misses, and false alarms separately for each forecast model and configuration file filter. The development (dev) scoring method is indicated in the output as *GENESIS_DEV* while the operational (ops) scoring method is indicated as *GENESIS_OPS*.

* For **-edeck** inputs:

 * Parse the ATCF edeck files. Ignore any lines not containing "GN" and "genFcst", which indicate a genesis probability forecast. Also, ignore any lines which do not contain a predicted genesis location (latitude and longitude) or genesis time.

 * Loop over the filters defined in the configuration file and apply the following logic for each.

  * Subset the genesis probability forecasts based on the current filter criteria. Typically, genesis probability forecast are provided for multiple lead times. Create separate Nx2 probabilistic contingency tables for each unique combination of predicted lead time and model name.

  * For each genesis probability forecast, search for a matching Best track. A configurable boolean option controls whether all Best track points are considered for a match or only the single Best track genesis point. A match occurs if the Best track point valid time is within a configurable window around the forecast genesis time and the Best track point location is within a configurable radius of the forecast genesis location. If a Best track match is found, store the storm ID.

  * If no Best track match is found, apply the same logic to search the operational track points with lead time of 0 hours. If an operational match is found, store the storm ID.

  * If no matching storm ID is found, add the unmatched forecast to the observation-no column of the Nx2 probabilistic contingency table.

  * If a matching storm ID is found, check whether that storm's genesis occurred within the predicted time window: between the forecast initialization time and the predicted lead time. If so, add the matched forecast to the observation-yes column. If not, add it to observation-no column.

 * Report the Nx2 probabilistic contingency table counts and statistics for each forecast model, lead time, and configuration file filter. These counts and statistics are identified in the output files as *PROB_GENESIS*.

* For **-shape** inputs:

 * For each input shapefile, parse the timestamp from the "gtwo_areas_YYYYMMDDHHMM.shp" naming convention, and error out otherwise. Round the timestamp to the nearest synoptic time (e.g. 00, 06, 12, 18) and store that as the issuance time.

 * Open the shapefile and corresponding database file. Process each record.

  * For each record, extract the shape and metadata which defines the basin and 2, 5, and 7 day probabilities.

  * Check if this shape is a duplicate that has already been processed. If it is an exact duplicate, with the same basin, file timestamp, issue time, and min/max lat/lon values, ignore it. If the file timestamp is older than the existing shape, also ignore it. If the file timestamp is newer than the existing shape, replace the existing shape with the new one.

 * Loop over the filters defined in the configuration file and apply the following logic for each.

  * Subset the list of genesis shapes based on the current filter criteria.

  * Search the Best track genesis events to see if any occurred inside the shape within 7 days of the issuance time. If multiple genesis events occurred, choose the one closest to the issuance time.

  * If not found, score each probability as a miss.

  * If found, further check the 2 and 5 day time windows to classify each probability as a hit or miss.

  * Add each probability pair to an Nx2 probabilistic contingency table, tracking results separately for each lead time.

  * Report the Nx2 probabilistic contingency table counts and statistics for each lead time. These counts and statistics are identified in the output files as *GENESIS_SHAPE*.

tc_gen Configuration File
-------------------------

The default configuration file for the **TC-Gen** tool named **TCGenConfig_default** can be found in the installed *share/met/config* directory. Like the other configuration files described in this document, it is recommended that users make a copy of these files prior to modifying their contents.

The tc_gen configuration file is divided into three main sections: criteria to define genesis events, options to subset and filter those events, and options to control the output. The contents of this configuration file are described below.

______________________

.. code-block:: none

  init_freq = 6;

The **init_freq** variable is an integer specifying the model initialization frequency in hours, starting at 00Z. The default value of 6 indicates that the model is initialized every day at 00Z, 06Z, 12Z, and 18Z. The same frequency is applied to all models processed. Models initialized at different frequencies should be processed with separate calls to tc_gen. The initialization frequency is used when defining the model opportunities to forecast the Best track genesis events.

______________________

.. code-block:: none

  valid_freq = 6;

The **valid_freq** variable is an integer specifying the valid time of the track points to be analyzed in hours, starting at 00Z. The default value of 6 indicates that only track points with valid times of 00Z, 06Z, 12Z, and 18Z will be checked for genesis events. Since Best and operational tracks are typically only available at those times, a match to a forecast genesis event is only possible for those hours.

______________________

.. code-block:: none

  fcst_hr_window = {
     beg = 24;
     end = 120;
  }

The **fcst_hr_window** option is a dictionary defining the beginning (**beg**) and ending (**end**) model forecast hours to be searched for genesis events. Model genesis events occurring outside of this window are ignored. This forecast hour window is also used when defining the model opportunities to forecast the Best track genesis events.

______________________

.. code-block:: none

  min_duration = 12;

The **min_duration** variable is an integer specifying the minimum number of hours a track must persist for its initial point to be counted as a genesis event. Some models spin up many short-lived storms, and this setting enables them to be excluded from the analysis.

______________________

.. code-block:: none

  fcst_genesis = {
     vmax_thresh = NA;
     mslp_thresh = NA;
  }

The **fcst_genesis** dictionary defines the conditions required for a model track's genesis point to be included in the analysis. Thresholds for the maximum wind speed (**vmax_thresh**) and minimum sea level pressure (**mslp_thresh**) may be defined. These conditions must be satisfied for at least one track point for the genesis event to be included in the analysis. The default thresholds (**NA**) always evaluate to true.

______________________

.. code-block:: none

  best_genesis = {
     technique   = "BEST";
     category    = [ "TD", "TS" ];
     vmax_thresh = NA;
     mslp_thresh = NA;
  }

The **best_genesis** dictionary defines genesis criteria for the Best tracks. Like the **fcst_genesis** dictionary, the **vmax_thresh** and **mslp_thresh** thresholds define required genesis criteria. In addition, the **category** array defines the ATCF storm categories that should qualify as genesis events. The **technique** string defines the ATCF ID for the Best track.

______________________

.. code-block:: none

  oper_technique = "CARQ";

The **oper_technique** entry is a string which defines the ATCF ID for the operational track data that should be used. For each forecast genesis event, the Best tracks are searched for a track point valid at the time of forecast genesis and within the search radius. If no match is found, the 0-hour operational track points are searched for a match.

______________________

.. code-block:: none

  filter = [];

The **filter** entry is an array of dictionaries defining genesis filtering criteria to be applied. Each of the entries listed below (from **desc** to **best_unique_flag**) may be specified separately within each filter dictionary. If left empty, the default setting, a single filter is applied using the top-level filtering criteria. If multiple filtering dictionaries are defined, the **desc** entry must be specified for each to differentiate the output data. Output is written for each combination of filter dictionary and model ATCF ID encountered in the data.

______________________

.. code-block:: none

  desc = "ALL";

The **desc** configuration option is common to many MET tools and is described in :numref:`config_options`.

______________________

.. code-block:: none

  model = [];

The **model** entry is an array defining the model ATCF ID's for which output should be computed. If left empty, the default setting, output will be computed for each model encountered in the data. Otherwise, output will be computed only for the ATCF ID's listed. Note that when reading ATCF track data, all instances of the string AVN are automatically replaced with GFS.

______________________

.. code-block:: none

  storm_id   = [];
  storm_name = [];

The **storm_id** and **storm_name** entries are arrays indicating the ATCF storm ID's and storm names to be processed. If left empty, all tracks will be processed. Otherwise, only those tracks which meet these criteria will be included. Note that these strings only appear in the Best and operational tracks, not the forecast genesis data. Therefore, these filters only apply to the Best and operational tracks. Care should be given when interpreting the contingency table results for filtered data.

______________________

.. code-block:: none

  init_beg = "";
  init_end = "";
  init_inc = [];
  init_exc = [];

The **init_beg**, **init_end**, **init_inc**, and **init_exc** entries define strings in YYYYMMDD[_HH[MMSS]] format which defines which forecast and operational tracks initializations to be processed. If left empty, all tracks will be used. Otherwise, only those tracks whose initialization time meets all the criteria will be processed. The initialization time must fall between **init_beg**, and **init_end**, must appear in **init_inc** inclusion list, and must not appear in the **init_exc** exclusion list. Note that these settings only apply to the forecast and operational tracks, not the Best tracks, for which the initialization time is undefined. Care should be given when interpreting the contingency table results for filtered data.

For genesis shapes, these options are used to filter the warning issuance time.

______________________

.. code-block:: none

  valid_beg = "";
  valid_end = "";

The **valid_beg** and **valid_end** entries are similar to **init_beg** and **init_end**, described above. However, they are applied to all genesis data sources. Only those tracks falling completely inside this window are included in the analysis.

______________________

.. code-block:: none

  init_hour = [];
  lead      = [];

The **init_hour** and **lead** entries are arrays of strings in HH[MMSS] format defining which forecast tracks should be included. If left empty, all tracks will be used. Otherwise, only those forecast tracks whose initialization hour and lead times appear in the list will be used. Note that these settings only apply to the forecast tracks, not the Best tracks, for which the initialization time is undefined. Care should be given when interpreting the contingency table results for filtered data.

For genesis shapes, the **init_hour** option is used to filter the warning issuance hour.

______________________

.. code-block:: none

  vx_mask = "";

The **vx_mask** entry is a string defining the path to a Lat/Lon polyline file or a gridded data file that MET can read to subset the results spatially. If specified, only those genesis events whose Lat/Lon location falls within the specified area will be included.

If specified for genesis shapes, the lat/lon of the central location of the shape will be checked. The central location is computed as the average of the min/max lat/lon values of the shape points.

______________________

.. code-block:: none

  basin_mask = [];

The **basin_mask** entry is an array of strings listing tropical cycline basin abbreviations (e.g. AL, EP, CP, WP, NI, SI, AU, and SP). The configuration entry **basin_file** defines the path to a NetCDF file which defines these regions. The default file (**basin_global_tenth_degree.nc**) is bundled with MET. If **basin_mask** is left empty, genesis events for all basins will be included. If non-empty, the union of specified basins will be used. If **vx_mask** is also specified, the analysis is done on the intersection of those masking areas.

The **vx_mask** and **basin_mask** names are concatenated and written to the **VX_MASK** output column.

If **vx_mask** is not specified for genesis shapes and **basin_mask** is, the basin name is extracted from the shapefile metadata and compared to the **basin_mask** list.

______________________

.. code-block:: none

  dland_thresh = NA;

The **dland_thresh** entry is a threshold defining whether the genesis event should be included based on its distance to land. The default threshold (**NA**) always evaluates to true.

______________________

.. code-block:: none

  genesis_match_point_to_track = TRUE;

The **genesis_match_point_to_track** entry is a boolean which controls the matching logic. When set to its default value of TRUE, for each forecast genesis event, all Best track points are searched for a match. This logic implements the method used by the NOAA National Hurricane Center. When set to FALSE, only the single Best track genesis point is considered for a match. When selecting FALSE, users are encouraged to adjust the **genesis_match_radius** and/or **gensesis_match_window** options, described below, to enable matches to be found.

______________________

.. code-block:: none

  genesis_match_radius = 500;

The **genesis_match_radius** entry defines a search radius, in km, relative to the forecast genesis location. When searching for a match, only Best or operational tracks with a track point within this radius will be considered. Increasing this search radius should lead to an increase in the number of matched genesis pairs.

______________________

.. code-block:: none

  genesis_match_window = {
     beg = 0;
     end = 0;
  }

The **genesis_match_window** entry defines a time window, in hours, relative to the forecast genesis time. When searching for a match, only Best or operational tracks with a track point falling within this time window will be considered. The default time window of 0 requires a Best or operational track to exist at the forecast genesis time for a match to be found. Increasing this time window should lead to an increase in the number matched genesis pairs. For example, setting *end = 12;* would allow forecast genesis events to match Best tracks up to 12 hours prior to their existence.

______________________

.. code-block:: none

  dev_hit_radius = 500;

The **dev_hit_radius** entry defines the maximum distance, in km, that the forecast and Best track genesis events may be separated in order for them to be counted as a contingency table HIT for the development scoring method. Users should set this hit radius less than or equal to the genesis match radius. Reducing this radius may cause development method HITS to become FALSE ALARMS.

______________________

.. code-block:: none

  dev_hit_window = {
     beg = -24;
     end =  24;
  }

The **dev_hit_window** entry defines a time window, in hours, relative to the forecast genesis time. The Best track genesis event must occur within this time window for the pair to be counted as a contingency table HIT for the development scoring method. Tightening this window may cause development method HITS to become FALSE ALARMS.

______________________

.. code-block:: none

  ops_hit_window = {
     beg =  0;
     end = 48;
  }

The **ops_hit_window** entry defines a time window, in hours, relative to the Best track genesis time. The model initialization time for the forecast genesis event must occur within this time window for the pairs to be counted as a contingency table HIT for the operationl scoring method. Otherwise, the pair is counted as a FALSE ALARM.

______________________

.. code-block:: none

  discard_init_post_genesis_flag = TRUE;

The **discard_init_post_genesis_flag** entry is a boolean which indicates whether or not forecast genesis events from model intializations occurring at or after the matching Best track genesis time should be discarded. If true, those cases are not scored in the contingency table. If false, they are included in the counts.

______________________

.. code-block:: none

  dev_method_flag = TRUE;
  ops_method_flag = TRUE;

The **dev_method_flag** and **ops_method_flag** entries are booleans which indicate whether the development and operational scoring methods should be applied and written to the output. At least one of these flags must be set to true.

______________________

.. code-block:: none

  nc_pairs_flag = {
     latlon       = TRUE;
     fcst_genesis = TRUE;
     fcst_tracks  = TRUE;
     fcst_fy_oy   = TRUE;
     fcst_fy_on   = TRUE;
     best_genesis = TRUE;
     best_tracks  = TRUE;
     best_fy_oy   = TRUE;
     best_fn_oy   = TRUE;
  }

The **nc_pairs_flag** entry is a dictionary of booleans indicating which fields should be written to the NetCDF genesis pairs output file. Each type of output is enabled by setting it to TRUE and disabled by setting it to FALSE. The **latlon** option writes the latitude and longitude values of the output grid. The remaining options write a count of the number of points occuring within each grid cell. The **fcst_genesis** and **best_genesis** options write counts of the forecast and Best track genesis locations. The **fcst_track** and **best_track** options write counts of the full set of track point locations, which can be refined by the **valid_minus_genesis_diff_thresh** option, described below. The **fcst_fy_oy** and **fcst_fy_on** options write counts for the locations of forecast genesis event HITS and FALSE ALARMS. The **best_fy_oy** and **best_fn_oy** options write counts for the locations of Best track genesis event HITS and MISSES. Note that since matching forecast and Best track genesis events may occur in different grid cells, their counts are reported separately.

______________________


.. code-block:: none

  valid_minus_genesis_diff_thresh = NA;

The **valid_minus_genesis_diff_thresh** is a threshold which affects the counts in the NetCDF pairs output file. The fcst_tracks and best_tracks options, described above, turn on counts for the forecast and Best track points. This option defines which of those track points should be counted by thresholding the track point valid time minus genesis time difference. If set to NA, the default threshold which always evaluates to true, all track points will be counted. Setting <=0 would count the genesis point and all track points prior. Setting >0 would count all points after genesis. And setting >=-12||<=12 would could all points within 12 hours of the genesis time.

______________________


.. code-block:: none

  best_unique_flag = TRUE;

The **best_unique_flag** entry is a boolean which affects the counts in the NetCDF pairs output file. If true, the Best track HIT and MISS locations are counted for each genesis pair. If false, each Best track genesis event is counted only once. If it is a HIT in at least one genesis pair, it is counted as a HIT in the output. Otherwise, it is counted as a MISS.

______________________

.. code-block:: none

  basin_file = "MET_BASE/tc_data/basin_global_tenth_degree.nc";

The **basin_file** entry defines the path to the NetCDF basin data file that is included with MET. When a Best track storm moves from one basin to another, the Best track dataset can include two tracks for the same storm, one for each basin. However, both tracks have the same genesis point. When this occurs, this basin data file is read and used to determine the basin in which genesis actually occurred. The corresponding Best track is retained and the other is discarded.

______________________

.. code-block:: none

  nc_pairs_grid = "G001";

The **nc_pairs_grid** entry is a string which defines the grid to be used for the NetCDF genesis pairs output file. It can be specified as a named grid, the path to a gridded data file, or a grid specification string.

______________________

.. code-block:: none

  prob_genesis_thresh = ==0.25;

The **prob_genesis_thresh** entry defines the probability thresholds used to create the output Nx2 contingency table when verifying edeck probability of genesis forecasts and probabilistic shapefile warning areas. The default is probability bins of width 0.25. These probabilities may be specified as a list (>0.00,>0.25,>0.50,>0.75,>1.00) or using shorthand notation (==0.25) for bins of equal width.

______________________

.. code-block:: none

  ci_alpha = 0.05;
  output_flag = {
     fho    = BOTH;
     ctc    = BOTH;
     cts    = BOTH;
     pct    = NONE;
     pstd   = NONE;
     pjc    = NONE;
     prc    = NONE;
     genmpr = NONE;
  }
  dland_file = "MET_BASE/tc_data/dland_global_tenth_degree.nc";
  version    = "VN.N";

The configuration options listed above are common to many MET tools and are described in :numref:`config_options`. TC-Gen writes output for 2x2 contingency tables to the **FHO**, **CTC**, and **CTS** line types when verifying deterministic genesis forecasts specified using the **-track** command line option. TC-Gen writes output for Nx2 probabilistic contingency tables to the **PCT**, **PSTD**, **PJC**, and **PRC** line types when verifying the probability of genesis forecasts specified using the **-edeck** command line option and probabilistic shapefiles using the **-shape** command line option. Note that the **genmpr** line type is specific to TC-Gen and describes individual genesis matched pairs.

tc_gen Output
-------------

TC-Gen produces output in STAT and, optionally, ASCII and NetCDF formats. The ASCII output duplicates the STAT output but has the data organized by line type. The output files are created based on the **-out** command line argument. The default output base name, **./tc_gen** writes output files in the current working directory named **tc_gen.stat** and, optionally, **tc_gen_pairs.nc** and **tc_gen_{TYPE}.txt** for each of the supported output line types. These output files can easily be redirected to another location using the **-out** command line option. The format of the STAT and ASCII output of the TC-Gen tool matches the output of other MET tools with the exception of the genesis matched pair line type. Please refer to the tables in :numref:`point_stat-output` for a description of the common output line types. The genesis matched pair line type and NetCDF output file are described below.

.. _table_TG_header_info_tg_outputs:

.. list-table:: Header information for each file tc-gen outputs
  :widths: auto
  :header-rows: 2

  * - HEADER
    -
    -
  * - Column Number
    - Header Column Name
    - Description
  * - 1
    - VERSION
    - Version number
  * - 2
    - MODEL
    - Current ATCF Technique name
  * - 3
    - DESC
    - User provided text string describing the "filter" options
  * - 4
    - FCST_LEAD
    - Forecast lead time in HHMMSS format
  * - 5
    - FCST_VALID_BEG
    - Minimum forecast valid time in YYYYMMDD_HHMMSS format
  * - 6
    - FCST_VALID_END
    - Maximum forecast valid time in YYYYMMDD_HHMMSS format
  * - 7
    - OBS_LEAD
    - Does not apply and is set to NA
  * - 8
    - OBS_VALID_BEG
    - Minimum Best track valid time in YYYYMMDD_HHMMSS format
  * - 9
    - OBS_VALID_END
    - Maximum Best track valid time in YYYYMMDD_HHMMSS format
  * - 10
    - FCST_VAR
    - Genesis methodology (GENESIS_DEV, GENESIS_OPS, PROB_GENESIS, or GENESIS_SHAPE)
  * - 11
    - FCST_UNITS
    - Does not apply and is set to NA
  * - 12
    - FCST_LEV
    - Does not apply and is set to NA
  * - 13
    - OBS_VAR
    - Genesis methodology (GENESIS_DEV, GENESIS_OPS, PROB_GENESIS, or GENESIS_SHAPE)
  * - 14
    - OBS_UNITS
    - Does not apply and is set to NA
  * - 15
    - OBS_LEV
    - Does not apply and is set to NA
  * - 16
    - OBTYPE
    - Verifying Best track technique name
  * - 17
    - VX_MASK
    - Verifying masking region
  * - 18
    - INTERP_MTHD
    - Does not apply and is set to NA
  * - 19
    - INTERP_PNTS
    - Does not apply and is set to NA
  * - 20
    - FCST_THRESH
    - Does not apply and is set to NA
  * - 21
    - OBS_THRESH
    - Does not apply and is set to NA
  * - 22
    - COV_THRESH
    - Does not apply and is set to NA
  * - 23
    - ALPHA
    - Error percent value used in confidence intervals
  * - 24
    - LINE_TYPE
    - Various line type options, refer to :numref:`point_stat-output` and the tables below.

.. _table_TG_format_info_GENMPR:

.. list-table:: Format information for GENMPR (Genesis Matched Pairs) output line type
  :widths: auto
  :header-rows: 2

  * - GENMPR OUTPUT FORMAT
    -
    -
  * - Column Number
    - GENMPR Column Name
    - Description
  * - 5, 6
    - FCST_VALID_BEG, FCST_VALID_END
    - Forecast genesis time in YYYYMMDD_HHMMSS format
  * - 8, 9
    - OBS_VALID_BEG, OBS_VALID_END
    - Best track genesis time in YYYYMMDD_HHMMSS format
  * - 24
    - GENMPR
    - Genesis Matched Pairs line type
  * - 25
    - TOTAL
    - Total number of genesis pairs
  * - 26
    - INDEX
    - Index for the current matched pair
  * - 27
    - STORM_ID
    - BBCCYYYY designation of storm (basin, cyclone number, and year)
  * - 28
    - PROB_LEAD
    - Lead time in HHH format for the predicted probability of genesis (only for **-edeck** inputs)
  * - 29
    - PROB_VAL
    - Predicted probability of genesis (only for **-edeck** inputs)
  * - 30
    - AGEN_INIT
    - Forecast initialization time
  * - 31
    - AGEN_FHR
    - Forecast hour of genesis event
  * - 32
    - AGEN_LAT
    - Latitude position of the forecast genesis event
  * - 33
    - AGEN_LON
    - Longitude position of the forecast genesis event
  * - 34
    - AGEN_DLAND
    - Forecast genesis event distance to land (nm)
  * - 35
    - BGEN_LAT
    - Latitude position of the verifying Best track genesis event
  * - 36
    - BGEN_LON
    - Longitude position of the verifying Best track genesis event
  * - 37
    - BGEN_DLAND
    - Best track genesis event distance to land (nm)
  * - 38
    - GEN_DIST
    - Distance between the forecast and Best track genesis events (km) (only for **-track** inputs)
  * - 39
    - GEN_TDIFF
    - Forecast minus Best track genesis time in HHMMSS format (only for **-track** inputs)
  * - 40
    - INIT_TDIFF
    - Best track genesis minus forecast initialization time in HHMMSS format (only for **-track** inputs)
  * - 41
    - DEV_CAT
    - Category for the development methodology (FYOY, FYON, FNOY, or DISCARD) (only for **-track** inputs)
  * - 42
    - OPS_CAT
    - Category for the operational methodology (FYOY, FYON, FNOY, or DISCARD for **-track** inputs and FYOY or FYON for **-edeck** inputs)

.. _table_TG_var_NetCDF_matched_pair_out:

.. list-table:: A selection of variables that can appear in the NetCDF matched pair output which can be controlled by the nc_pairs_flag configuration option.
  :widths: auto
  :header-rows: 2

  * - tc_gen NETCDF VARIABLES
    -
    -
  * - NetCDF Variable
    - Dimension
    - Description
  * - DESC_MODEL_GENESIS
    - lat, lon
    - For each filter entry (DESC) and forecast ATCF ID (MODEL), count the number of forecast genesis events within each grid box.
  * - DESC_MODEL_TRACKS
    - lat, lon
    - For each filter entry (DESC) and forecast ATCF ID (MODEL), count the number of track points within each grid box.
  * - DESC_BEST_GENESIS
    - lat, lon
    - For each filter entry (DESC), count the number of Best track genesis events within each grid box.
  * - DESC_BEST_GENESIS
    - lat, lon
    - For each filter entry (DESC), count the number of Best track points within each grid box.
  * - DESC_MODEL_[DEV|OPS]_FY_OY
    - lat, lon
    - For each filter entry (DESC) and forecast ATCF ID (MODEL), count the number of forecast genesis events classified as hits by the development (DEV) or operational (OPS) methodology.
  * - DESC_MODEL_[DEV|OPS]_FY_ON
    - lat, lon
    - For each filter entry (DESC) and forecast ATCF ID (MODEL), count the number of forecast genesis events classified as false alarms by the development (DEV) or operational (OPS) methodology.
  * - DESC_MODEL_BEST_[DEV|OPS]_FY_OY
    - lat, lon
    - For each filter entry (DESC) and forecast ATCF ID (MODEL), count the number of Best track genesis events classified as hits by the development (DEV) or operational (OPS) methodology.
  * - DESC_MODEL_BEST_[DEV|OPS]_FN_OY
    - lat, lon
    - For each filter entry (DESC) and forecast ATCF ID (MODEL), count the number of Best track genesis events classified as misses by the development (DEV) or operational (OPS) methodology.

Like all STAT output, the output of TC-Gen may be further processed using the Stat-Analysis tool, described in :numref:`stat-analysis`.
