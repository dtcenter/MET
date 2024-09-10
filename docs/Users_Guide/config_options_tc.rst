.. _config_options_tc:

**************************************
Tropical Cyclone Configuration Options
**************************************

See :numref:`config_options` for a description of the configuration file syntax.

Configuration Settings Common to Multiple Tools
===============================================

storm_id
--------

Specify a comma-separated list of storm id's to be used:

| 2-letter basin, 2-digit cyclone number, 4-digit year
|

An empty list indicates that all should be used.

For example:

| storm_id = [ "AL092011" ];
| 

This may also be set using basin, cyclone, and timing information below.

.. code-block:: none

   storm_id = [];

basin
-----

Specify a comma-separated list of basins to be used. Expected format is
a 2-letter basin identifier. An empty list indicates that all should be used.

|  Valid basins: WP, IO, SH, CP, EP, AL, SL
|

For example:

| basin = [ "AL", "EP" ];
|

.. code-block:: none
		
   basin = [];


cyclone
-------

Specify a comma-separated list of cyclone numbers (01-99) to be used.
An empty list indicates that all should be used.

For example:

| cyclone = [ "01", "02", "03" ];
| 

.. code-block:: none
		
  cyclone = [];

storm_name
----------

Specify a comma-separated list of storm names to be used. An empty list
indicates that all should be used.

For example:

| storm_name = [ "KATRINA" ];
| 

.. code-block:: none
		
  storm_name = [];

init_beg end inc exc
--------------------

Specify a model initialization time window in YYYYMMDD[_HH[MMSS]] format
or provide a list of specific initialization times to include (inc)
or exclude (exc). Tracks whose initial time meets the specified
criteria will be used. An empty string indicates that all times
should be used.

In TC-Stat, the **-init_beg**, **-init_end**, **init_inc** and **-int_exc** job command options can be used to further refine these selections.

For example:

| init_beg = "20100101";
| init_end = "20101231";
| init_inc = [ "20101231_06" ];
| init_exc = [ "20101231_00" ];
| 

.. code-block:: none

  init_beg = "";
  init_end = "";
  init_inc = [];
  init_exc = [];

valid_beg end inc exc
---------------------
  
Specify a model valid time window in YYYYMMDD[_HH[MMSS]] format or provide a
list of specific valid times to include (inc) or exclude (exc). If a time
window is specified, only tracks for which all points are contained within
the window will be used. If valid times to include or exclude are specified,
tracks will be subset down to the points which meet that criteria. Empty
begin/end time strings and empty include/exclude lists indicate that all
valid times should be used.

In TC-Stat, the **-valid_beg**, **-valid_end**, **valid_inc** and **-valid_exc** job command options can be used to further refine these selections.

For example:

| valid_beg = "20100101";
| valid_end = "20101231_12";
| valid_inc = [ "20101231_06" ];
| valid_exc = [ "20101231_00" ];
|

.. code-block:: none

  valid_beg = "";
  valid_end = "";
  valid_inc = [];
  valid_exc = [];

init_hour
---------

Specify a comma-separated list of model initialization hours to be used
in HH[MMSS] format. An empty list indicates that all hours should be used.

For example:

| init_hour = [ "00", "06", "12", "18" ];
| 

.. code-block:: none
		
  init_hour = [];

lead_req
--------

Specify the required lead time in HH[MMSS] format.
Tracks that contain all of these required times will be
used. If a track has additional lead times, it will be
retained.  An empty list indicates that no lead times
are required to determine which tracks are to be used;
all lead times will be used.

.. code-block:: none
		
  lead_req  = [];

version
-------

Indicate the version number for the contents of this configuration file.
The value should generally not be modified.

.. code-block:: none
		
  version = "VN.N";


Settings Specific to Individual Tools
=====================================


TCPairsConfig_default
---------------------

model
^^^^^

The "model" entry specifies an array of model names to be verified. If
verifying multiple models, choose descriptive model names (no whitespace)
to distinguish between their output.

For example:
		
| model = [ "AHW4", "AHWI" ];
| 

.. code-block:: none
		  
  model  = [];

init_mask, valid_mask
^^^^^^^^^^^^^^^^^^^^^

Specify lat/lon polylines defining masking regions to be applied.
Tracks whose initial location falls within init_mask will be used.
Tracks for which all locations fall within valid_mask will be used.

For example:

| init_mask  = "MET_BASE/poly/EAST.poly";
|

.. code-block:: none

  init_mask  = "";
  valid_mask = "";

check_dup
^^^^^^^^^

Specify whether the code should check for duplicate ATCF lines when
building tracks.  Setting this to FALSE makes the parsing of tracks quicker.

For example:

| check_dup = FALSE;
| 

.. code-block:: none
		
  check_dup = FALSE;

interp12
^^^^^^^^

Specify whether special processing should be performed for interpolated model
names ending in 'I' (e.g. AHWI).  Search for corresponding tracks whose model
name ends in '2' (e.g. AHW2) and apply the following logic:

* NONE to do nothing.

* FILL to create a copy of '2' track and rename it as 'I' only when the
  'I' track does not already exist.
     
* REPLACE to create a copy of the '2' track and rename it as 'I' in all
  cases, replacing any 'I' tracks that may already exist.

.. code-block:: none
		
  interp12 = REPLACE;

consensus
^^^^^^^^^

Specify how consensus forecasts should be defined:

| name = consensus model name
| members = array of consensus member model names
| required = array of TRUE/FALSE for each member if empty, default is FALSE
| min_req = minimum number of members required for the consensus
| 

For example:

| consensus = [
|    {
|       name     = "CON1";
|       members  = [ "MOD1", "MOD2", "MOD3" ];
|       required = [ TRUE, FALSE, FALSE ];
|       min_req  = 2;
|    }
| ];
| 

.. code-block:: none

  consensus = [];


lag_time
^^^^^^^^

Specify a comma-separated list of forecast lag times to be used in HH[MMSS]
format.  For each ADECK track identified, a lagged track will be derived
for each entry listed.

For example:

| lag_time = [ "06", "12" ];
| 

.. code-block:: none
		
  lag_time = [];


best
^^^^

Specify comma-separated lists of CLIPER/SHIFOR baseline forecasts to be
derived from the BEST and operational tracks, as defined by the
best_technique and oper_technique settings.

| Derived from BEST tracks:
|   BCLP, BCS5, BCD5, BCLA
| Derived from OPER tracks:
|   OCLP, OCS5, OCD5, OCDT
|

For example:
		
| best_technique = [ "BEST" ];
| 

.. code-block:: none
		
  best_technique = [ "BEST" ];
  best_baseline  = [];
  oper_technique = [ "CARQ" ];
  oper_baseline  = [];


anly_track
^^^^^^^^^^

Analysis tracks consist of multiple track points with a lead time of zero
for the same storm. An analysis track may be generated by running model
analysis fields through a tracking algorithm. Specify which datasets should
be searched for analysis track data by setting this to NONE, ADECK, BDECK,
or BOTH. Use BOTH to create pairs using two different analysis tracks.

For example:

| anly_track = BDECK;
| 

.. code-block:: none
		
  anly_track = BDECK;


match_points
^^^^^^^^^^^^

Specify whether only those track points common to both the ADECK and BDECK
tracks should be written out.


For example:

| match_points = FALSE;
| 

.. code-block:: none
		
  match_points = FALSE;


dland_file
^^^^^^^^^^

Specify the NetCDF output of the gen_dland tool containing a gridded
representation of the minimum distance to land.


.. code-block:: none

  dland_file = "MET_BASE/tc_data/dland_nw_hem_tenth_degree.nc";


watch_warn
^^^^^^^^^^

Specify watch/warning information.  Specify an ASCII file containing
watch/warning information to be used.  At each track point, the most severe
watch/warning status in effect, if any, will be written to the output.
Also specify a time offset in seconds to be added to each watch/warning
time processed.  NHC applies watch/warning information to all track points
occurring 4 hours (-14400 second) prior to the watch/warning time.


.. code-block:: none

  watch_warn = {
     file_name   = "MET_BASE/tc_data/wwpts_us.txt";
     time_offset = -14400;
  }


basin_map
^^^^^^^^^

The basin_map entry defines a mapping of input names to output values.
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

.. code-block:: none

  basin_map = [
     { key = "SI"; val = "SH"; },
     { key = "SP"; val = "SH"; },
     { key = "AU"; val = "SH"; },
     { key = "AB"; val = "IO"; },
     { key = "BB"; val = "IO"; }
  ];

TCStatConfig_default
--------------------

amodel, bmodel
^^^^^^^^^^^^^^

Stratify by the AMODEL or BMODEL columns.
Specify comma-separated lists of model names to be used for all analyses
performed.  May add to this list using the "-amodel" and "-bmodel"
job command options.

For example:

| amodel = [ "AHW4" ];
| bmodel = [ "BEST" ];
| 

.. code-block:: none
		
  amodel = [];
  bmodel = [];

init valid_hour lead req
^^^^^^^^^^^^^^^^^^^^^^^^

Stratify by the initialization and valid hours and lead time.
Specify a comma-separated list of initialization hours,
valid hours, and lead times in HH[MMSS] format.
May add using the "-init_hour", "-valid_hour", "-lead",
and "-lead_req" job command options.

For example:

| init_hour  = [ "00" ];
| valid_hour = [ "12" ];
| lead       = [ "24", "36" ];
| lead_req   = [ "72", "84", "96", "108" ];
| 

.. code-block:: none
		
  init_hour  = [];
  valid_hour = [];
  lead       = [];
  lead_req   = [];

init_mask, valid_mask
^^^^^^^^^^^^^^^^^^^^^

Stratify by the contents of the INIT_MASK and VALID_MASK columns.
Specify a comma-separated list of strings for these options.
May add using the "-init_mask" and "-valid_mask" job command
options.

For example:

| init_mask  = [ "AL_BASIN", "EP_BASIN" ];
|

.. code-block:: none

  init_mask  = [];
  valid_mask = [];

line_type
^^^^^^^^^

Stratify by the LINE_TYPE column.  May add using the "-line_type"
job command option.

For example:

| line_type = [ "TCMPR" ];
| 

.. code-block:: none
		
  line_type = [];

track_watch_warn
^^^^^^^^^^^^^^^^

Stratify by checking the watch/warning status for each track point
common to both the ADECK and BDECK tracks. If the watch/warning status
of any of the track points appears in the list, retain the entire track.
Individual watch/warning status by point may be specified using the
-column_str options below, but this option filters by the track maximum.
May add using the "-track_watch_warn" job command option.
The value "ALL" matches HUWARN, TSWARN, HUWATCH, and TSWATCH.

For example:

|  track_watch_warn = [ "HUWATCH", "HUWARN" ];
| 

.. code-block:: none
		
  track_watch_warn = [];


column_thresh_name_and_val
^^^^^^^^^^^^^^^^^^^^^^^^^^

Stratify by applying thresholds to numeric data columns.
Specify a comma-separated list of columns names and thresholds
to be applied.  May add using the "-column_thresh name thresh" job command
options.

For example:

| column_thresh_name = [ "ADLAND", "BDLAND" ];
| column_thresh_val  = [ >200,     >200     ];
| 

.. code-block:: none
		
  column_thresh_name = [];
  column_thresh_val  = [];

column_str_name, column_str_val
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Stratify by performing string matching on non-numeric data columns.
Specify a comma-separated list of columns names and values
to be included in the analysis.
May add using the "-column_str name string" job command options.

For example:

| column_str_name = [ "LEVEL", "LEVEL" ];
| column_str_val  = [ "HU",    "TS"    ];
|

.. code-block:: none
		
  column_str_name = [];
  column_str_val  = [];

column_str_name val
^^^^^^^^^^^^^^^^^^^
  
Stratify by performing string matching on non-numeric data columns.
Specify a comma-separated list of columns names and values
to be excluded from the analysis.
May add using the "-column_str_exc name string" job command options.

For example:

| column_str_exc_name = [ "LEVEL" ];
| column_str_exc_val  = [ "TD"    ];
|

.. code-block:: none
		
  column_str_exc_name = [];
  column_str_exc_val  = [];

init_thresh_name, init_thresh_val
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Just like the column_thresh options above, but apply the threshold only
when lead = 0.  If lead = 0 value does not meet the threshold, discard
the entire track.  May add using the "-init_thresh name thresh" job command
options.

For example:

| init_thresh_name = [ "ADLAND" ];
| init_thresh_val  = [ >200     ];
| 

.. code-block:: none
		
  init_thresh_name = [];
  init_thresh_val  = [];
  
init_str_name, init_str_val
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Just like the column_str options above, but apply the string matching only
when lead = 0.  If lead = 0 string does not match, discard the entire track.
May add using the "-init_str name thresh" job command options.

For example:

| init_str_name = [ "LEVEL" ];
| init_str_val  = [ "HU"    ];
| 

.. code-block:: none

  init_str_name = [];
  init_str_val  = [];

init_str_exc_name and _exc_val
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Just like the column_str_exc options above, but apply the string matching only
when lead = 0.  If lead = 0 string does match, discard the entire track.
May add using the "-init_str_exc name thresh" job command options.

For example:

| init_str_exc_name = [ "LEVEL" ];
| init_str_exc_val  = [ "HU"    ];
|

.. code-block:: none

  init_str_exc_name = [];
  init_str_exc_val  = [];

water_only
^^^^^^^^^^

Stratify by the ADECK and BDECK distances to land.  Once either the ADECK or
BDECK track encounters land, discard the remainder of the track.

For example:

| water_only = FALSE;
| 

.. code-block:: none
		
  water_only = FALSE;

rirw
^^^^

Specify whether only those track points for which rapid intensification
or weakening of the maximum wind speed occurred in the previous time
step should be retained.

The NHC considers a 24-hour change >=30 kts to constitute rapid
intensification or weakening.

May modify using the following job command options:

| "-rirw_track"
| "-rirw_time" for both or "-rirw_time_adeck" and "-rirw_time_bdeck"
| "-rirw_exact" for both or "-rirw_exact_adeck" and "-rirw_exact_bdeck"
| "-rirw_thresh" for both or "-rirw_thresh_adeck" and "-rirw_thresh_bdeck"
| 

.. code-block:: none

  rirw = {
     track  = NONE;       Specify which track types to search (NONE, ADECK,
                          BDECK, or BOTH)
     adeck = {
        time   = "24";    Rapid intensification/weakening time period in HHMMSS
                          format.
        exact  = TRUE;    Use the exact or maximum intensity difference over the
                          time period.
        thresh = >=30.0;  Threshold for the intensity change.
     }
     bdeck = adeck;       Copy settings to the BDECK or specify different logic.
  }

landfall beg end
^^^^^^^^^^^^^^^^

Specify whether only those track points occurring near landfall should be
retained, and define the landfall retention window as a time string in HH[MMSS]
format (or as an integer number of seconds) offset from the landfall time.
Landfall is defined as the last BDECK track point before the distance to land
switches from positive to 0 or negative.

May modify using the "-landfall_window" job command option, which
automatically sets -landfall to TRUE.

The "-landfall_window" job command option takes 1 or 2 arguments in  HH[MMSS]
format.  Use 1 argument to define a symmetric time window.  For example,
"-landfall_window 06" defines the time window +/- 6 hours around the landfall
time.  Use 2 arguments to define an asymmetric time window.  For example,
"-landfall_window 00 12" defines the time window from the landfall event to 12
hours after.

For example:

| landfall     = FALSE;
| landfall_beg = "-24"; (24 hours prior to landfall)
| landfall_end = "00";
| 

.. code-block:: none

  landfall     = FALSE;
  landfall_beg = "-24";
  landfall_end = "00";

event_equal
^^^^^^^^^^^

Specify whether only those cases common to all models in the dataset should
be retained.  May modify using the "-event_equal" job command option.

For example:

| event_equal = FALSE;
| 

.. code-block:: none
		
  event_equal = FALSE;


event_equal_lead
^^^^^^^^^^^^^^^^

Specify lead times that must be present for a track to be included in the
event equalization logic.

.. code-block:: none

  event_equal_lead = [ "12", "24", "36" ];


out_int_mask
^^^^^^^^^^^^

Apply polyline masking logic to the location of the ADECK track at the
initialization time.  If it falls outside the mask, discard the entire track.
May modify using the "-out_init_mask" job command option.

For example:

| out_init_mask = "";
| 

.. code-block:: none

  out_init_mask = "";


out_valid_mask
^^^^^^^^^^^^^^

Apply polyline masking logic to the location of the ADECK track at the
valid time.  If it falls outside the mask, discard only the current track
point.  May modify using the "-out_valid_mask" job command option.

For example:

| out_valid_mask = "";
| 

.. code-block:: none

  out_valid_mask = "";

job
^^^

The "jobs" entry is an array of TCStat jobs to be performed.
Each element in the array contains the specifications for a single analysis
job to be performed.  The format for an analysis job is as follows:

| -job job_name   
| OPTIONAL ARGS
| 

Where "job_name" is set to one of the following:

* "filter"
  
  To filter out the TCST lines matching the job filtering criteria
  specified above and using the optional arguments below.  The
  output TCST lines are written to the file specified using the
  "-dump_row" argument.
  
  Required Args: -dump_row

  To further refine the TCST data: Each optional argument may be used
  in the job specification multiple times unless otherwise indicated.
  When multiple optional arguments of the same type are indicated, the
  analysis will be performed over their union.

  .. code-block:: none
		  
    "-amodel            name"
    "-bmodel            name"
    "-lead        HHMMSS"
    "-valid_beg   YYYYMMDD[_HH[MMSS]]" (use once)
    "-valid_end   YYYYMMDD[_HH[MMSS]]" (use once)
    "-valid_inc   YYYYMMDD[_HH[MMSS]]" (use once)
    "-valid_exc   YYYYMMDD[_HH[MMSS]]" (use once)
    "-init_beg    YYYYMMDD[_HH[MMSS]]" (use once)
    "-init_end    YYYYMMDD[_HH[MMSS]]" (use once)
    "-init_inc    YYYYMMDD[_HH[MMSS]]" (use once)
    "-init_exc    YYYYMMDD[_HH[MMSS]]" (use once)
    "-init_hour   HH[MMSS]"
    "-valid_hour  HH[MMSS]
    "-init_mask          name"
    "-valid_mask         name"
    "-line_type          name"
    "-track_watch_warn   name"
    "-column_thresh      name thresh"
    "-column_str         name string"
    "-column_str_exc     name string"
    "-init_thresh        name thresh"
    "-init_str           name string"
    "-init_str_exc       name string"

  Additional filtering options that may be used only when -line_type
  has been listed only once. These options take two arguments: the name
  of the data column to be used and the min, max, or exact value for
  that column. If multiple column eq/min/max/str options are listed,
  the job will be performed on their intersection:

  .. code-block:: none
		  
    "-column_min     col_name value" For example: -column_min TK_ERR 100.00
    "-column_max     col_name value"
    "-column_eq      col_name value"
    "-column_str     col_name string" separate multiple filtering strings
                                      with commas
    "-column_str_exc col_name string" separate multiple filtering strings
                                      with commas

  Required Args: -dump_row
  
| 

* "summary"
  
  To compute the mean, standard deviation, and percentiles
  (0th, 10th, 25th, 50th, 75th, 90th, and 100th) for the statistic
  specified using the "-line_type" and "-column" arguments.
  For TCStat, the "-column" argument may be set to:

  * "TRACK" for track, along-track, and cross-track errors.
  * "WIND" for all wind radius errors.
  * "TI" for track and maximum wind intensity errors.
  * "AC" for along-track and cross-track errors.
  * "XY" for x-track and y-track errors.
  * "col" for a specific column name.
  * "col1-col2" for a difference of two columns.
  * "ABS(col or col1-col2)" for the absolute value.

  Use the -column_union TRUE/FALSE job command option to compute
  summary statistics across the union of input columns rather than
  processing them separately.

  Required Args: -line_type, -column

  Optional Args:

  .. code-block:: none

    -by column_name to specify case information
    -out_alpha to override default alpha value
    -column_union to summarize multiple columns

* "rirw"
  
  To define rapid intensification/weakening contingency table using
  the ADECK and BDECK RI/RW settings and the matching time window
  and output contingency table counts and statistics.

  Optional Args:

  .. code-block:: none
		  
    -rirw_window width in HH[MMSS] format to define a symmetric time window
    -rirw_window beg end in HH[MMSS] format to define an asymmetric time window
     Default search time window is 0 0, requiring exact match
    -rirw_time or -rirw_time_adeck and -rirw_time_bdeck to override defaults
    -rirw_exact or -rirw_exact_adeck and -rirw_exact_bdeck to override defaults
    -rirw_thresh or -rirw_thresh_adeck and -rirw_thresh_bdeck to override
    defaults
    -by column_name to specify case information
    -out_alpha to override default alpha value
    -out_line_type to specify output line types (CTC, CTS, and MPR)


  Note that the "-dump_row path" option results in 4 files being
  created:

| path_FY_OY.tcst, path_FY_ON.tcst, path_FN_OY.tcst, and
| path_FN_ON.tcst, containing the TCST lines that were hits, false
| alarms, misses, and correct negatives,  respectively.  These files
| may be used as input for additional TC-Stat analysis.
| 

* "probrirw"
       
  To define an Nx2 probabilistic contingency table by reading the
  PROBRIRW line type, binning the forecast probabilities, and writing
  output probabilistic counts and statistics.

  Required Args:

  .. code-block:: none
		  
    -probrirw_thresh to define the forecast probabilities to be
       evaluated (For example: -probrirw_thresh 30)

  Optional Args:

  .. code-block:: none
		  
    -probrirw_exact TRUE/FALSE to verify against the exact (for example:
       BDELTA column) or maximum (for example: BDELTA_MAX column) intensity
       change in the BEST track
    -probrirw_bdelta_thresh to define BEST track change event
       threshold (For example: -probrirw_bdelta_thresh >=30)
    -probrirw_prob_thresh to define output probability thresholds
       (for example: -probrirw_prob_thresh ==0.1)
    -by column_name to specify case information
    -out_alpha to override default alpha value
    -out_line_type to specify output line types (PCT, PSTD, PRC, and PJC)


  For the PROBRIRW line type, PROBRIRW_PROB is a derived column name.
  For example, the following options select 30 kt probabilities and match
  probability values greater than 0:
  
|   -probrirw_thresh 30 -column_thresh PROBRIRW_PROB >0
|

  For example:

| jobs = [
|   "-job filter -amodel AHW4 -dumprow ./tc_filter_job.tcst",
|   "-job filter -column_min TK_ERR 100.000 \
|   -dumprow ./tc_filter_job.tcst",
|   "-job summary -line_type TCMPR -column AC \
|   -dumprow  ./tc_summary_job.tcst",
|   "-job rirw -amodel AHW4 -dump_row ./tc_rirw_job" ]
| 

.. code-block:: none

  jobs = [];

TCGenConfig_default
-------------------

init_freq
^^^^^^^^^

Model initialization frequency in hours, starting at 0.

.. code-block:: none

  init_freq = 6;

lead_window
^^^^^^^^^^^

Lead times in hours to be searched for genesis events.


.. code-block:: none

  lead_window = {
     beg = 24;
     end = 120;
  }

min_duration
^^^^^^^^^^^^

Minimum track duration for genesis event in hours.

.. code-block:: none

  min_duration = 12;

fcst_genesis
^^^^^^^^^^^^

Forecast genesis event criteria.  Defined as tracks reaching the specified
intensity category, maximum wind speed threshold, and minimum sea-level
pressure threshold.  The forecast genesis time is the valid time of the first
track point where all of these criteria are met.


.. code-block:: none

  fcst_genesis = {
     vmax_thresh = NA;
     mslp_thresh = NA;
  }

best_genesis
^^^^^^^^^^^^

BEST track genesis event criteria.  Defined as tracks reaching the specified
intensity category, maximum wind speed threshold, and minimum sea-level
pressure threshold.  The BEST track genesis time is the valid time of the
first track point where all of these criteria are met.

.. code-block:: none

  best_genesis = {
     technique   = "BEST";
     category    = [ "TD", "TS" ];
     vmax_thresh = NA;
     mslp_thresh = NA;
  }

oper_genesis
^^^^^^^^^^^^

Operational track genesis event criteria.  Defined as tracks reaching the
specified intensity category, maximum wind speed threshold, and minimum
sea-level pressure threshold.  The operational track genesis time is valid
time of the first track point where all of these criteria are met.

.. code-block:: none

  oper_genesis = {
     technique   = "CARQ";
     category    = [ "DB", "LO", "WV" ];
     vmax_thresh = NA;
     mslp_thresh = NA;
  }

filter
^^^^^^

Filter is an array of dictionaries containing the track filtering options
listed below.  If empty, a single filter is defined using the top-level
settings.

.. code-block:: none

  filter = [];

desc
^^^^

Description written to output DESC column

.. code-block:: none
		
  desc = "NA";

model
^^^^^

Forecast ATCF ID's
If empty, all ATCF ID's found will be processed.
Statistics will be generated separately for each ATCF ID.

.. code-block:: none
		
  model = [];

init_beg, init_end
^^^^^^^^^^^^^^^^^^

Forecast and operational initialization time window, as strings in YYYYMMDD[_HH[MMSS]] format

.. code-block:: none

  init_beg = "";
  init_end = "";

valid_beg, valid_end
^^^^^^^^^^^^^^^^^^^^

Forecast, BEST, and operational valid time window, as strings in YYYYMMDD[_HH[MMSS]] format

.. code-block:: none
		
  valid_beg = "";
  valid_end = "";

lead
^^^^

Forecast and operational lead times, as strings in HH[MMSS] format

.. code-block:: none

  lead = [];

vx_mask
^^^^^^^

Spatial masking region (path to gridded data file or polyline file)

.. code-block:: none

  vx_mask = "";

dland_thresh
^^^^^^^^^^^^

Distance to land threshold

.. code-block:: none

  dland_thresh = NA;

genesis_window
^^^^^^^^^^^^^^

Genesis matching time window, in hours relative to the forecast genesis time

.. code-block:: none
		
  genesis_window = {
     beg = -24;
     end =  24;
  }

genesis_radius
^^^^^^^^^^^^^^

Genesis matching search radius in km.

.. code-block:: none
		
  genesis_radius = 300;

ci_alpha
^^^^^^^^

Confidence interval alpha value

.. code-block:: none
		
  ci_alpha = 0.05;

output_flag
^^^^^^^^^^^

Statistical output types

.. code-block:: none

  output_flag = {
     fho    = NONE;
     ctc    = BOTH;
     cts    = BOTH;
     pct    = NONE;
     pstd   = NONE;
     pjc    = NONE;
     prc    = NONE;
     genmpr = NONE;
  }
