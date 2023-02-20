.. _config_options:

***************************
Configuration File Overview
***************************

The configuration files that control many of the MET tools contain formatted
ASCII text. This format has been updated for MET version |version| and
continues to be used in subsequent releases.

Settings common to multiple tools are described in the top part of this
file and settings specific to individual tools are described beneath the common
settings. Please refer to the MET User's Guide for more details about the
settings if necessary.

A configuration file entry is an entry name, followed by an equal sign (=),
followed by an entry value, and is terminated by a semicolon (;). The
configuration file itself is one large dictionary consisting of entries, some of
which are dictionaries themselves.

The configuration file language supports the following data types:

* Dictionary:
  
  * Grouping of one or more entries enclosed by curly braces {}.

* Array:
  
  * List of one or more entries enclosed by square braces [].
    
  * Array elements are separated by commas.

* String:
  
  * A character string enclosed by double quotation marks "".
    
* Integer:
  
  * A numeric integer value.
    
* Float:
  
  * A numeric float value.
    
* Boolean:
  
  * A boolean value (TRUE or FALSE).
    
* Threshold:
  
  * A threshold type (<, <=, ==, !-, >=, or >) followed by a numeric value.
    
  * The threshold type may also be specified using two letter abbreviations
    (lt, le, eq, ne, ge, gt).
      
  * Multiple thresholds may be combined by specifying the logic type of AND
    (&&) or OR (||). For example, ">=5&&<=10" defines the numbers between 5
    and 10 and "==1||==2" defines numbers exactly equal to 1 or 2.
     
* Percentile Thresholds:

  * A threshold type (<, <=, ==, !=, >=, or >), followed by a percentile
    type description (SFP, SOP, SCP, USP, CDP, or FBIAS), followed by a
    numeric value, typically between 0 and 100.

  * Note that the two letter threshold type abbreviations (lt, le, eq, ne,
    ge, gt) are not supported for percentile thresholds.
  
  * Thresholds may be defined as percentiles of the data being processed in
    several places:
    
    * In Point-Stat and Grid-Stat when setting "cat_thresh", "wind_thresh"
      and "cnt_thresh".
      
    * In Wavelet-Stat when setting "cat_thresh".
      
    * In MODE when setting "conv_thresh" and "merge_thresh".
      
    * In Ensemble-Stat when setting "obs_thresh".
	
    * When using the "censor_thresh" config option.
	
    * In the Stat-Analysis "-out_fcst_thresh" and "-out_obs_thresh" job
      command options.
	
    * In the Gen-Vx-Mask "-thresh" command line option.
	
  * The following percentile threshold types are supported:
    
    * "SFP" for a percentile of the sample forecast values.
      e.g. ">SFP33.3" means greater than the 33.3-rd forecast percentile.
      
    * "SOP" for a percentile of the sample observation values.
      e.g. ">SOP75" means greater than the 75-th observation percentile.
      
    * "SCP" for a percentile of the sample climatology values.
      e.g. ">SCP90" means greater than the 90-th climatology percentile.
      
    * "USP" for a user-specified percentile threshold.
      e.g. "<USP90(2.5)" means less than the 90-th percentile values which
      the user has already determined to be 2.5 outside of MET.
      
    * "==FBIAS" for a user-specified frequency bias value.
      e.g. "==FBIAS1" to automatically de-bias the data, "==FBIAS0.9" to select a low-bias threshold, or "==FBIAS1.1" to select a high-bias threshold.
      This option must be used in
      conjunction with a simple threshold in the other field. For example,
      when "obs.cat_thresh = >5.0" and "fcst.cat_thresh = ==FBIAS1;",
      MET applies the >5.0 threshold to the observations and then chooses a
      forecast threshold which results in a frequency bias of 1.
      The frequency bias can be any float value > 0.0.
      
    * "CDP" for climatological distribution percentile thresholds.
      These thresholds require that the climatological mean and standard
      deviation be defined using the climo_mean and climo_stdev config file
      options, respectively. The categorical (cat_thresh), conditional
      (cnt_thresh), or wind speed (wind_thresh) thresholds are defined
      relative to the climatological distribution at each point. Therefore,
      the actual numeric threshold applied can change for each point.
      e.g. ">CDP50" means greater than the 50-th percentile of the
      climatological distribution for each point.
      
  * When percentile thresholds of type SFP, SOP, SCP, or CDP are requested
    for continuous filtering thresholds (cnt_thresh), wind speed thresholds
    (wind_thresh), or observation filtering thresholds (obs_thresh in
    ensemble_stat), the following special logic is applied. Percentile
    thresholds of type equality are automatically converted to percentile
    bins which span the values from 0 to 100.
    For example, "==CDP25" is automatically expanded to 4 percentile bins:
    >=CDP0&&<CDP25,>=CDP25&&<CDP50,>=CDP50&&<CDP75,>=CDP75&&<=CDP100
     
  * When sample percentile thresholds of type SFP, SOP, SCP, or FBIAS are
    requested, MET recomputes the actual percentile that the threshold
    represents. If the requested percentile and actual percentile differ by
    more than 5%, a warning message is printed. This may occur when the
    sample size is small or the data values are not truly continuous.
     
  * When percentile thresholds of type SFP, SOP, SCP, or USP are used, the
    actual threshold value is appended to the FCST_THRESH and OBS_THRESH
    output columns. For example, if the 90-th percentile of the current set
    of forecast values is 3.5, then the requested threshold "<=SFP90" is
    written to the output as "<=SFP90(3.5)".
     
  * When parsing FCST_THRESH and OBS_THRESH columns, the Stat-Analysis tool
    ignores the actual percentile values listed in parentheses.
     
* Piecewise-Linear Function (currently used only by MODE):
  
  * A list of (x, y) points enclosed in parenthesis ().
    
  * The (x, y) points are *NOT* separated by commas.
    
* User-defined function of a single variable:
  
  * Left side is a function name followed by variable name in parenthesis.
    
  * Right side is an equation which includes basic math functions (+,-,*,/),
    built-in functions (listed below), or other user-defined functions.
    
  * Built-in functions include:
    sin, cos, tan, sind, cosd, tand, asin, acos, atan, asind, acosd, atand,
    atan2, atan2d, arg, argd, log, exp, log10, exp10, sqrt, abs, min, max,
    mod, floor, ceil, step, nint, sign

The context of a configuration entry matters. If an entry cannot be found in
the expected dictionary, the MET tools recursively search for that entry in the
parent dictionaries, all the way up to the top-level configuration file
dictionary. If you'd like to apply the same setting across all cases, you can
simply specify it once at the top-level. Alternatively, you can specify a
setting at the appropriate dictionary level to have finer control over the
behavior.

In order to make the configuration files more readable, several descriptive
integer types have been defined in the ConfigConstants file. These integer
names may be used on the right-hand side for many configuration file entries.

Each of the configurable MET tools expects a certain set of configuration
entries. Examples of the MET configuration files can be found in *data/config*
and *scripts/config*.

When you pass a configuration file to a MET tool, the tool actually parses up
to four different configuration files in the following order:

   1. Reads *share/met/config/ConfigConstants* to define constants.

   2. If the tool produces PostScript output, it reads *share/met/config/ConfigMapData* to define the map data to be plotted.

   3. Reads the default configuration file for the tool from *share/met/config*.

   4. Reads the user-specified configuration file from the command line.

Many of the entries from step (3) are overwritten by the user-specified entries
from step (4). Therefore, the configuration file you pass in on the command
line really only needs to contain entries that differ from the defaults.

Any of the configuration entries may be overwritten by the user-specified
configuration file. For example, the map data to be plotted may be included in
the user-specified configuration file and override the default settings defined
in the *share/met/config/ConfigMapData* file.

The configuration file language supports the use of environment variables. They
are specified as ${ENV_VAR}, where ENV_VAR is the name of the environment
variable. When scripting up many calls to the MET tools, you may find it
convenient to use them. For example, when applying the same configuration to
the output from multiple models, consider defining the model name as an
environment variable which the controlling script sets prior to verifying the
output of each model. Setting MODEL to that environment variable enables you
to use one configuration file rather than maintaining many very similar ones.

An error in the syntax of a configuration file will result in an error from the
MET tool stating the location of the parsing error.

Runtime Environment Variables
-----------------------------

.. _config_env_vars:

User-Specified Environment Variables
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

When editing configuration files, environment variables may be used for setting
the configurable parameters if convenient. The configuration file parser expands
environment variables to their full value before proceeding. Within the configuration
file, environment variables must be specified in the form **${VAR_NAME}**.

For example, using an environment variable to set the message_type (see below)
parameter to use ADPUPA and ADPSFC message types might consist of the following.

Setting the environment variable in a Bash Shell:

.. code :: bash

  export MSG_TYP='"ADPUPA", "ADPSFC"'

Referencing that environment variable inside a MET configuration file:

.. code ::

  message_type = [ ${MSG_TYP} ];

In addition to supporting user-specified environment variables within configuration
files, the environment variables listed below have special meaning if set at runtime.

.. _met_airnow_stations:

MET_AIRNOW_STATIONS
^^^^^^^^^^^^^^^^^^^

The MET_AIRNOW_STATIONS environment variable can be used to specify a file that
will override the default file. If set, it should be the full path to the file.
The default table can be found in the installed
*share/met/table_files/airnow_monitoring_site_locations_v2.dat*. This file contains
ascii column data that allows lookups of latitude, longitude, and elevation for all
AirNow stations based on stationId and/or AqSid.

Additional information and updated site locations can be found at the
`EPA AirNow website <https://www.airnow.gov>`_. While some monitoring stations are
permanent, others are temporary, and theirs locations can change. When running the
ascii2nc tool with the `-format airnowhourly` option, users should
`download <https://test.airnowtech.org/>`_ the `Monitoring_Site_Locations_V2.dat` data file
data file corresponding to the date being processed and set the MET_AIRNOW_STATIONS
envrionment variable to define its location.

.. _met_ndbc_stations:

MET_NDBC_STATIONS
^^^^^^^^^^^^^^^^^

The MET_NDBC_STATIONS environment variable can be used to specify a file that
will override the default file. If set it should be a full path to the file.
The default table can be found in the installed
*share/met/table_files/ndbc_stations.xml*. This file contains
XML content for all stations that allows lookups of latitude, longitude,
and, in some cases, elevation for all stations based on stationId.

MET_BASE
^^^^^^^^

The MET_BASE variable is defined in the code at compilation time as the path
to the MET shared data. These are things like the default configuration files,
common polygons and color scales. MET_BASE may be used in the MET configuration
files when specifying paths and the appropriate path will be substituted in.
If MET_BASE is defined as an environment variable, its value will be used
instead of the one defined at compilation time.

MET_OBS_ERROR_TABLE
^^^^^^^^^^^^^^^^^^^

The MET_OBS_ERROR_TABLE environment variable can be set to specify the location
of an ASCII file defining observation error information. The default table can
be found in the installed *share/met/table_files/obs_error_table.txt*. This
observation error logic is applied in Ensemble-Stat to perturb ensemble member
values and/or define observation bias corrections.

When processing point and gridded observations, Ensemble-Stat searches the table
to find the entry defining the observation error information. The table
consists of 15 columns and includes a header row defining each column. The
special string "ALL" is interpreted as a wildcard in these files. The first 6
columns (OBS_VAR, MESSAGE_TYPE, PB_REPORT_TYPE, IN_REPORT_TYPE, INSTRUMENT_TYPE,
and STATION_ID) may be set to a comma-separated list of strings to be matched.
In addition, the strings in the OBS_VAR column are interpreted as regular
expressions when searching for a match. For example, setting the OBS_VAR column
to 'APCP_[0-9]+' would match observations for both APCP_03 and APCP_24. The
HGT_RANGE, VAL_RANGE, and PRS_RANGE columns should either be set to "ALL" or
"BEG,END" where BEG and END specify the range of values to be used. The
INST_BIAS_SCALE and INST_BIAS_OFFSET columns define instrument bias adjustments
which are applied to the observation values. The DIST_TYPE and DIST_PARM
columns define the distribution from which random perturbations should be drawn
and applied to the ensemble member values. See the obs_error description below
for details on the supported error distributions. The last two columns, MIN and
MAX, define the bounds for the valid range of the bias-corrected observation
values and randomly perturbed ensemble member values. Values less than MIN are
reset to the mimimum value and values greater than MAX are reset to the maximum
value. A value of NA indicates that the variable is unbounded.

MET_GRIB_TABLES
^^^^^^^^^^^^^^^

The MET_GRIB_TABLES environment variable can be set to specify the location of
custom GRIB tables. It can either be set to a specific file name or to a
directory containing custom GRIB tables files. These file names must begin with
a "grib1" or "grib2" prefix and end with a ".txt" suffix. Their format must
match the format used by the default MET GRIB table files, described below.
The custom GRIB tables are read prior to the default tables and their settings
take precedence.

At runtime, the MET tools read default GRIB tables from the installed
*share/met/table_files* directory, and their file formats are described below:

GRIB1 table files begin with "grib1" prefix and end with a ".txt" suffix.
The first line of the file must contain "GRIB1".
The following lines consist of 4 integers followed by 3 strings:

| Column 1: GRIB code (e.g. 11 for temperature)
| Column 2: parameter table version number
| Column 3: center id (e.g. 07 for US Weather Service- National Met. Center)
| Column 4: subcenter id
| Column 5: variable name
| Column 6: variable description
| Column 7: units
|


References:

| `Office Note 388 GRIB1 <http://www.nco.ncep.noaa.gov/pmb/docs/on388>`_
| `A Guide to the Code Form FM 92-IX Ext. GRIB Edition 1 <http://www.wmo.int/pages/prog/www/WMOCodes/Guides/GRIB/GRIB1-Contents.html>`_
| 

GRIB2 table files begin with "grib2" prefix and end with a ".txt" suffix.
The first line of the file must contain "GRIB2".
The following lines consist of 8 integers followed by 3 strings.

| Column 1:  Section 0 Discipline
| Column 2:  Section 1 Master Tables Version Number
| Column 3:  Section 1 Master Tables Version Number, low range of tables
| Column 4:  Section 1 Master Table Version Number, high range of tables
| Column 5:  Section 1 originating center
| Column 6:  Local Tables Version Number
| Column 7:  Section 4 Template 4.0 Parameter category
| Column 8:  Section 4 Template 4.0 Parameter number
| Column 9:  variable name
| Column 10: variable description
| Column 11: units
| 

References:

| `NCEP WMO GRIB2 Documentation <http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_doc>`_
|

OMP_NUM_THREADS
^^^^^^^^^^^^^^^

**Introduction**

There are a number of different ways of parallelizing code. OpenMP offers
parallelism within a single shared-memory workstation or supercomputer node.
The programmer writes OpenMP directives into the code to parallelize
particular code regions.

When a parallelized code region is reached, which we shall hereafter call a
parallel region, a number of threads are spawned and work is shared among them.
Running on different cores, this reduces the execution time. At the end of the
parallel region, the code returns to single-thread execution.

A limited number of code regions are parallelized in MET. As a consequence,
there are limits to the overall speed gains acheivable. Only the parallel
regions of code will get faster with more threads, leaving the remaining
serial portions to dominate the runtime.

Not all top-level executables use parallelized code. If OpenMP is available,
a log message will appear inviting the user to increase the number of threads
for faster runtimes.

**Setting the number of threads**

The number of threads is controlled by the environment variable
*OMP_NUM_THREADS* . For example, on a quad core machine, the user might choose
to run on 4 threads:

.. code :: bash

  export OMP_NUM_THREADS=4

Alternatively, the variable may be specified as a prefix to the executable
itself. For example:

.. code :: bash

  OMP_NUM_THREADS=4 <exec>

The case where this variable remains unset is handled inside the code, which
defaults to a single thread.

There are choices when deciding how many threads to use. To perform a single run
as fast as possible, it would likely be appropriate to use as many threads as
there are (physical) cores available on the specific system. However, it is not
a cast-iron guarantee that more threads will always translate into more speed.
In theory, there is a chance that running across multiple non-uniform memory
access (NUMA) regions may carry negative performance impacts. This has not been
observed in practice, however.

A lower thread count is appropriate when time-to-solution is not so critical,
because cores remain idle when the code is not inside a parallel region. Fewer
threads typically means better resource utilization.

**Which code is parallelized?**

Regions of parallelized code are:

  * :code:`fractional_coverage (data_plane_util.cc)`

Only the following top-level executables can presently benefit from OpenMP
parallelization:

  * :code:`grid_stat`
  * :code:`ensemble_stat`
  * :code:`grid_ens_prod`

**Thread Binding**    

It is normally beneficial to bind threads to particular cores, sometimes called
*affinitization*. There are a few reasons for this, but at the very least it
guarantees that threads remain evenly distributed across the available cores.
Otherwise, the operating system may migrate threads between cores during a run.

OpenMP provides some environment variables to handle this: :code:`OMP_PLACES`
and  :code:`OMP_PROC_BIND`.  We anticipate that the effect of setting only
:code:`OMP_PROC_BIND=true` would be neutral-to-positive.

However, there are sometimes compiler-specific environment variables. Instead,
thread affinitization is sometimes handled by MPI launchers, since OpenMP is
often used in MPI codes to reduce intra-node communications.

Where code is running in a production context, it is worth being familiar with
the binding / affinitization method on the particular system and building it
into any relevant scripting.

Settings common to multiple tools
---------------------------------

exit_on_warning
^^^^^^^^^^^^^^^

The "exit_on_warning" entry in ConfigConstants may be set to true or false.
If set to true and a MET tool encounters a warning, it will immediately exit
with bad status after writing the warning message.

.. code-block:: none
		
  exit_on_warning = FALSE;

nc_compression
^^^^^^^^^^^^^^

The "nc_compression" entry in ConfigConstants defines the compression level
for the NetCDF variables. Setting this option in the config file of one of
the tools overrides the default value set in ConfigConstants. The
environment variable MET_NC_COMPRESS overrides the compression level
from configuration file. The command line argument "-compress n" for some
tools overrides it.
The range is 0 to 9.

* 0 is to disable the compression.

* 1 to 9: Lower number is faster, higher number for smaller files.

WARNING: Selecting a high compression level may slow down the reading and
writing of NetCDF files within MET significantly.

.. code-block:: none
		
  nc_compression = 0;

output_precision
^^^^^^^^^^^^^^^^
  
The "output_precision" entry in ConfigConstants defines the precision
(number of significant decimal places) to be written to the ASCII output
files. Setting this option in the config file of one of the tools will
override the default value set in ConfigConstants.

.. code-block:: none
		
  output_precision = 5;

tmp_dir
^^^^^^^
      
The "tmp_dir" entry in ConfigConstants defines the directory for the
temporary files. The directory must exist and be writable. The environment
variable MET_TMP_DIR overrides the default value at the configuration file.
Some tools override the temporary directory by the command line argument
"-tmp_dir <diretory_name>".

.. code-block:: none
		
  tmp_dir = "/tmp";

message_type_group_map
^^^^^^^^^^^^^^^^^^^^^^
      
The "message_type_group_map" entry is an array of dictionaries, each
containing a "key" string and "val" string. This defines a mapping of
message type group names to a comma-separated list of values. This map is
defined in the config files for PB2NC, Point-Stat, or Ensemble-Stat. Modify
this map to define sets of message types that should be processed together as
a group. The "SURFACE" entry defines message types for which surface verification
logic should be applied. If not defined, the default values listed below are
used.

.. code-block:: none
		
  mesage_type_group_map = [
     { key = "SURFACE"; val = "ADPSFC,SFCSHP,MSONET";               },
     { key = "ANYAIR";  val = "AIRCAR,AIRCFT";                      },
     { key = "ANYSFC";  val = "ADPSFC,SFCSHP,ADPUPA,PROFLR,MSONET"; },
     { key = "ONLYSF";  val = "ADPSFC,SFCSHP";                      }
  ];

message_type_map
^^^^^^^^^^^^^^^^
  
The "message_type_map" entry is an array of dictionaries, each containing
a "key" string and "val" string. This defines a mapping of input strings
to output message types. This mapping is applied in ASCII2NC when
converting input little_r report types to output message types. This mapping
is also supported in PBN2NC as a way of renaming input PREPBUFR message
types.

.. code-block:: none
		
  message_type_map = [
     { key = "FM-12 SYNOP";  val = "ADPSFC"; },
     { key = "FM-13 SHIP";   val = "SFCSHP"; },
     { key = "FM-15 METAR";  val = "ADPSFC"; },
     { key = "FM-18 BUOY";   val = "SFCSHP"; },
     { key = "FM-281 QSCAT"; val = "ASCATW"; },
     { key = "FM-32 PILOT";  val = "ADPUPA"; },
     { key = "FM-35 TEMP";   val = "ADPUPA"; },
     { key = "FM-88 SATOB";  val = "SATWND"; },
     { key = "FM-97 ACARS";  val = "AIRCFT"; }
  ];

model
^^^^^
      
The "model" entry specifies a name for the model being verified. This name
is written to the MODEL column of the ASCII output generated. If you're
verifying multiple models, you should choose descriptive model names (no
whitespace) to distinguish between their output.
e.g. model = "GFS";

.. code-block:: none
		
  model = "WRF";

desc
^^^^
      
The "desc" entry specifies a user-specified description for each verification
task. This string is written to the DESC column of the ASCII output
generated. It may be set separately in each "obs.field" verification task
entry or simply once at the top level of the configuration file. If you're
verifying the same field multiple times with different quality control
flags, you should choose description strings (no whitespace) to distinguish
between their output.
e.g. desc = "QC_9";

.. code-block:: none
		
  desc = "NA";

obtype
^^^^^^
      
The "obtype" entry specifies a name to describe the type of verifying gridded
observation used. This name is written to the OBTYPE column in the ASCII
output generated. If you're using multiple types of verifying observations,
you should choose a descriptive name (no whitespace) to distinguish between
their output. When verifying against point observations the point
observation message type value is written to the OBTYPE column. Otherwise,
the configuration file obtype value is written.

.. code-block:: none
		
  obtype = "ANALYS";

.. _regrid:
  
regrid
^^^^^^
      
The "regrid" entry is a dictionary containing information about how to handle
input gridded data files. The "regrid" entry specifies regridding logic
using the following entries:

* The "to_grid" entry may be set to NONE, FCST, OBS, a named grid, the path
  to a gridded data file defining the grid, or an explicit grid specification
  string.
  
  * to_grid = NONE;   To disable regridding.
    
  * to_grid = FCST;   To regrid observations to the forecast grid.
    
  * to_grid = OBS;    To regrid forecasts to the observation grid.
    
  * to_grid = "G218"; To regrid both to a named grid.
    
  * to_grid = "path"; To regrid both to a grid defined by a file.
    
  * to_grid = "spec"; To define a grid specification string, as
    described in :ref:`appendixB`.

* The "vld_thresh" entry specifies a proportion between 0 and 1 to define
  the required ratio of valid data points. When regridding, compute
  a ratio of the number of valid data points to the total number of
  points in the neighborhood. If that ratio is less than this threshold,
  write bad data for the current point.

* The "method" entry defines the regridding method to be used.
  
  * Valid regridding methods:
    
    * MIN         for the minimum value
      
    * MAX         for the maximum value
      
    * MEDIAN      for the median value
      
    * UW_MEAN     for the unweighted average value
      
    * DW_MEAN     for the distance-weighted average value (weight =
      distance^-2)
      
    * AW_MEAN     for an area-weighted mean when regridding from
      high to low resolution grids (width = 1)
      
    * LS_FIT      for a least-squares fit
      
    * BILIN       for bilinear interpolation (width = 2)
      
    * NEAREST     for the nearest grid point (width = 1)
      
    * BUDGET      for the mass-conserving budget interpolation
      
    * FORCE       to compare gridded data directly with no interpolation
      as long as the grid x and y dimensions match.
      
    * UPPER_LEFT  for the upper left grid point (width = 1)
      
    * UPPER_RIGHT for the upper right grid point (width = 1)
      
    * LOWER_RIGHT for the lower right grid point (width = 1)
      
    * LOWER_LEFT  for the lower left grid point (width = 1)
      
    * MAXGAUSS    to compute the maximum value in the neighborhood
      and apply a Gaussian smoother to the result

    The BEST, GEOG_MATCH, and HIRA options are not valid for regridding.

* The "width" entry specifies a regridding width, when applicable.
  - width = 4;    To regrid using a 4x4 box or circle with diameter 4.

* The "shape" entry defines the shape of the neighborhood.
  Valid values are "SQUARE" or "CIRCLE"

* The "gaussian_dx" entry specifies a delta distance for Gaussian
  smoothing. The default is 81.271. Ignored if not Gaussian method.

* The "gaussian_radius" entry defines the radius of influence for Gaussian
  smoothing. The default is 120. Ignored if not Gaussian method.

* The "gaussian_dx" and "gaussian_radius" settings must be in the same
  units, such as kilometers or degress. Their ratio
  (sigma = gaussian_radius / gaussian_dx) determines the Guassian weighting
  function.

* The "convert", "censor_thresh", and "censor_val" entries are described
  below. When specified, these operations are applied to the output of the
  regridding step. The conversion operation is applied first, followed by
  the censoring operation. Note that these operations are limited in scope.
  They are only applied if defined within the regrid dictionary itself.
  Settings defined at higher levels of config file context are not applied. 

.. code-block:: none
		
  regrid = {
     to_grid         = NONE;
     method          = NEAREST;
     width           = 1;
     vld_thresh      = 0.5;
     shape           = SQUARE;
     gaussian_dx     = 81.271;
     gaussian_radius = 120;
     convert(x)      = x;
     censor_thresh   = [];
     censor_val      = [];
  }

fcst
^^^^
  
The "fcst" entry is a dictionary containing information about the field(s)
to be verified. This dictionary may include the following entries:

* The "field" entry is an array of dictionaries, each specifying a
  verification task. Each of these dictionaries may include:

  * The "name" entry specifies a name for the field.

  * The "level" entry specifies level information for the field.

  * Setting "name" and "level" is file-format specific. See below.

  * The "prob" entry in the forecast dictionary defines probability
    information. It may either be set as a boolean (i.e. TRUE or FALSE)
    or as a dictionary defining probabilistic field information.

    When set as a boolean to TRUE, it indicates that the "fcst.field" data
    should be treated as probabilities. For example, when verifying the
    probabilistic NetCDF output of Ensemble-Stat, one could configure the
    Grid-Stat or Point-Stat tools as follows:

    .. code-block:: none
    
      fcst = {
         field = [ { name  = "APCP_24_A24_ENS_FREQ_gt0.0";
                     level = "(*,*)";
                     prob  = TRUE; } ];
         }

    Setting "prob = TRUE" indicates that the "APCP_24_A24_ENS_FREQ_gt0.0"
    data should be processed as probabilities.

    When set as a dictionary, it defines the probabilistic field to be
    used. For example, when verifying GRIB files containing probabilistic
    data,  one could configure the Grid-Stat or Point-Stat tools as
    follows:

    .. code-block:: none

      fcst = {
         field = [ { name = "PROB"; level = "A24";
                     prob = { name = "APCP"; thresh_lo = 2.54; } },
                   { name = "PROB"; level = "P850";
                     prob = { name = "TMP"; thresh_hi = 273; } } ];
      }

    The example above selects two probabilistic fields. In both, "name"
    is set to "PROB", the GRIB abbreviation for probabilities. The "level"
    entry defines the level information (i.e. "A24" for a 24-hour
    accumulation and "P850" for 850mb). The "prob" dictionary defines the
    event for which the probability is defined. The "thresh_lo"
    (i.e. APCP > 2.54) and/or "thresh_hi" (i.e. TMP < 273) entries are
    used to define the event threshold(s).

    Probability fields should contain values in the range
    [0, 1] or [0, 100]. However, when MET encounters a probability field
    with a range [0, 100], it will automatically rescale it to be [0, 1]
    before applying the probabilistic verification methods.

  * Set "prob_as_scalar = TRUE" to override the processing of probability
    data. When the "prob" entry is set as a dictionary to define the
    field of interest, setting "prob_as_scalar = TRUE" indicates that this
    data should be processed as regular scalars rather than probabilities.
    For example, this option can be used to compute traditional 2x2
    contingency tables and neighborhood verification statistics for
    probability data. It can also be used to compare two probability
    fields directly. When this flag is set, probability values are
    automatically rescaled from the range [0, 100] to [0, 1].

  * The "convert" entry is a user-defined function of a single variable
    for processing input data values. Any input values that are not bad
    data are replaced by the value of this function. The convert function
    is applied prior to regridding or thresholding. This function may
    include any of the built-in math functions (e.g. sqrt, log10)
    described above.
    Several standard unit conversion functions are already defined in
    *data/config/ConfigConstants*.
    Examples of user-defined conversion functions include:

    .. code-block:: none

      convert(x) = 2*x;
      convert(x) = x^2;
      convert(a) = log10(a);
      convert(a) = a^10;
      convert(t) = max(1, sqrt(abs(t)));
      convert(x) = K_to_C(x); where K_to_C(x) is defined in
                              ConfigConstants

  * The "censor_thresh" entry is an array of thresholds to be applied
    to the input data. The "censor_val" entry is an array of numbers
    and must be the same length as "censor_thresh". These arguments must
    appear together in the correct format (threshold and number). For each
    censor threshold, any input values meeting the threshold criteria will
    be reset to the corresponding censor value. An empty list indicates
    that no censoring should be performed. The censoring logic is applied
    prior to any regridding but after the convert function. All statistics
    are computed on the censored data. These entries may be used to apply
    quality control logic by resetting data outside of an expected range
    to the bad data value of -9999. These entries are not indicated in the
    metadata of any output files, but the user can set the "desc" entry
    accordingly.

    Examples of user-defined data censoring operations include:

    .. code-block:: none
		    
      censor_thresh = [ >12000 ];
      censor_val    = [  12000 ];

  * Several configuration options are provided to override and correct the
    metadata read from the input file. The supported options are listed
    below:

    .. code-block:: none

      // Data attributes
      set_attr_name      = "string";
      set_attr_level     = "string";
      set_attr_units     = "string";
      set_attr_long_name = "string";

      // Time attributes
      set_attr_init  = "YYYYMMDD[_HH[MMSS]]";
      set_attr_valid = "YYYYMMDD[_HH[MMSS]]";
      set_attr_lead  = "HH[MMSS]";
      set_attr_accum = "HH[MMSS]";

      // Grid definition (must match the actual data dimensions)
      set_attr_grid  = "named grid or grid specification string";

      // Flags
      is_precipitation     = boolean;
      is_specific_humidity = boolean;
      is_u_wind            = boolean;
      is_v_wind            = boolean;
      is_grid_relative     = boolean;
      is_wind_speed        = boolean;
      is_wind_direction    = boolean;
      is_prob              = boolean;

  * The "mpr_column" and "mpr_thresh" entries are arrays of strings and
    thresholds to specify which matched pairs should be included in the
    statistics. These options apply to the Point-Stat and Grid-Stat tools.
    They are parsed seperately for each "obs.field" array entry.
    The "mpr_column" strings specify MPR column names ("FCST", "OBS",
    "CLIMO_MEAN", "CLIMO_STDEV", or "CLIMO_CDF"), differences of columns
    ("FCST-OBS"), or the absolute value of those differences ("ABS(FCST-OBS)").
    The number of "mpr_thresh" thresholds must match the number of "mpr_column"
    entries, and the n-th threshold is applied to the n-th column. Any matched
    pairs which do not meet any of the specified thresholds are excluded from
    the analysis. For example, the following settings exclude matched pairs
    where the observation value differs from the forecast or climatological
    mean values by more than 10:
    
    .. code-block:: none

      mpr_column = [ "ABS(OBS-FCST)", "ABS(OBS-CLIMO_MEAN)" ];
      mpr_thresh = [ <=10, <=10 ];

  * The "cat_thresh" entry is an array of thresholds to be used when
    computing categorical statistics.

  * The "cnt_thresh" entry is an array of thresholds for filtering
    data prior to computing continuous statistics and partial sums.

  * The "cnt_logic" entry may be set to UNION, INTERSECTION, or SYMDIFF
    and controls the logic for how the forecast and observed cnt_thresh
    settings are combined when filtering matched pairs of forecast and
    observed values.

* The "file_type" entry specifies the input gridded data file type rather
  than letting the code determine it. MET determines the file type by
  checking for known suffixes and examining the file contents. Use this
  option to override the code's choice. The valid file_type values are
  listed the "data/config/ConfigConstants" file and are described below.
  This entry should be defined within the "fcst" and/or "obs" dictionaries.
  For example:

  .. code-block:: none
		    
    fcst = {
       file_type = GRIB1;         GRIB version 1
       file_type = GRIB2;         GRIB version 2
       file_type = NETCDF_MET;    NetCDF created by another MET tool
       file_type = NETCDF_PINT;   NetCDF created by running the p_interp
                                  or wrf_interp utility on WRF output.
                                  May be used to read unstaggered raw WRF
                                  NetCDF output at the surface or a
                                  single model level.
       file_type = NETCDF_NCCF;   NetCDF following the Climate Forecast
                                  (CF) convention.
       file_type = PYTHON_NUMPY;  Run a Python script to load data into
                                  a NumPy array.
       file_type = PYTHON_XARRAY; Run a Python script to load data into
                                  an xarray object.
    }

* The "wind_thresh" entry is an array of thresholds used to filter wind
  speed values when computing VL1L2 vector partial sums. Only those U/V
  pairs that meet this wind speed criteria will be included in the sums.
  Setting this threshold to NA will result in all U/V pairs being used.

* The "wind_logic" entry may be set to UNION, INTERSECTION, or SYMDIFF
  and controls the logic for how the forecast and observed wind_thresh
  settings are combined when filtering matched pairs of forecast and
  observed wind speeds.

* The "eclv_points" entry specifies the economic cost/loss ratio points
  to be evaluated. For each cost/loss ratio specified, the relative value
  will be computed and written to the ECLV output line. This entry may
  either be specified as an array of numbers between 0 and 1 or as a single
  number. For an array, each array entry will be evaluated. For a single
  number, all evenly spaced points between 0 and 1 will be evaluated, where
  eclv_points defines the spacing. Cost/loss values are omitted for
  ratios of 0.0 and 1.0 since they are undefined.

* The "init_time" entry specifies the initialization time in
  YYYYMMDD[_HH[MMSS]]
  format. This entry can be included in the "fcst" entry as shown below or
  included in the "field" entry if the user would like to use different
  initialization times for different fields.

* The "valid_time" entry specifies the valid time in YYYYMMDD[_HH[MMSS]]
  format. This entry can be included in the "fcst" entry as shown below or
  included in the "field" entry if the user would like to use different
  valid times for different fields.

* The "lead_time" entry specifies the lead time in HH[MMSS]
  format. This entry can be included in the "fcst" entry as shown below or
  included in the "field" entry if the user would like to use different
  lead times for different fields.

It is only necessary to use the "init_time", "valid_time", and/or "lead_time"
settings when verifying a file containing data for multiple output times.
For example, to verify a GRIB file containing data for many lead times, you
could use "lead_time" to specify the record to be verified.

File-format specific settings for the "field" entry:

  * GRIB1 and GRIB2:

    * For custom GRIB tables, see note about MET_GRIB_TABLES.

    * The "name" entry specifies a GRIB code number or abbreviation.

      * `GRIB1 Product Definition Section <http://www.nco.ncep.noaa.gov/pmb/docs/on388/table2.html>`_

      * `GRIB2 Product Definition Section <http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_doc>`_
	 
    * The "level" entry specifies a level type and value:
       
      * ANNN for accumulation interval NNN
	 
      * ZNNN for vertical level NNN
	 
      * ZNNN-NNN for a range of vertical levels
	 
      * PNNN for pressure level NNN in hPa
	 
      * PNNN-NNN for a range of pressure levels in hPa
	 
      * LNNN for a generic level type
	 
      * RNNN for a specific GRIB record number
	 
    * The "GRIB_lvl_typ" entry is an integer specifying the level type.
       
    * The "GRIB_lvl_val1" and "GRIB_lvl_val2" entries are floats specifying
      the first and second level values.
       
    * The "GRIB_ens" entry is a string specifying NCEP's usage of the
      extended PDS for ensembles. Set to "hi_res_ctl", "low_res_ctl",
      "+n", or "-n", for the n-th ensemble member.
       
    * The "GRIB1_ptv" entry is an integer specifying the GRIB1 parameter
      table version number.
       
    * The "GRIB1_code" entry is an integer specifying the GRIB1 code (wgrib
      kpds5 value).
       
    * The "GRIB1_center" is an integer specifying the originating center.
       
    * The "GRIB1_subcenter" is an integer specifying the originating
      subcenter.
       
    * The "GRIB1_tri" is an integer specifying the time range indicator.
       
    * The "GRIB2_mtab" is an integer specifying the master table number.
       
    * The "GRIB2_ltab" is an integer specifying the local table number.
       
    * The "GRIB2_disc" is an integer specifying the GRIB2 discipline code.
       
    * The "GRIB2_parm_cat" is an integer specifying the parameter category
      code.
       
    * The "GRIB2_parm" is an integer specifying the parameter code.
       
    * The "GRIB2_pdt" is an integer specifying the product definition
      template (Table 4.0).
       
    * The "GRIB2_process" is an integer specifying the generating process
      (Table 4.3).
       
    * The "GRIB2_cntr" is an integer specifying the originating center.
       
    * The "GRIB2_ens_type" is an integer specifying the ensemble type
      (Table 4.6).
       
    * The "GRIB2_der_type" is an integer specifying the derived product
      type (Table 4.7).
       
    * The "GRIB2_stat_type" is an integer specifying the statistical
      processing type (Table 4.10).

    * The "GRIB2_perc_val" is an integer specifying the requested percentile
      value (0 to 100) to be used. This applies only to GRIB2 product
      definition templates 4.6 and 4.10.
       
    * The "GRIB2_ipdtmpl_index" and "GRIB2_ipdtmpl_val" entries are arrays
      of integers which specify the product description template values to
      be used. The indices are 0-based. For example, use the following to
      request a GRIB2 record whose 9-th and 27-th product description
      template values are 1 and 2, respectively:

      GRIB2_ipdtmpl_index=[8, 26]; GRIB2_ipdtmpl_val=[1, 2];
	  
  * NetCDF (from MET tools, CF-compliant, p_interp, and wrf_interp):
     
    * The "name" entry specifies the NetCDF variable name.
       
    * The "level" entry specifies the dimensions to be used:
       
      * (i,...,j,*,*) for a single field, where i,...,j specifies fixed
        dimension values and *,* specifies the two dimensions for the
        gridded field. @ specifies the vertical level value or time value
        instead of offset, (i,...,@NNN,*,*). For example:

      .. code-block:: none

        field = [
             {
               name       = "QVAPOR";
               level      = "(0,5,*,*)";
             },
             {
               name       = "TMP_P850_ENS_MEAN";
               level      = [ "(*,*)" ];
             }
           ];

        field = [
             {
               name       = "QVAPOR";
               level      = "(@20220601_1200,@850,*,*)";
             },
             {
               name       = "TMP_P850_ENS_MEAN";
               level      = [ "(*,*)" ];
             }
           ];

  * Python (using PYTHON_NUMPY or PYTHON_XARRAY):
     
    * The Python interface for MET is described in Appendix F of the MET
      User's Guide.
       
    * Two methods for specifying the Python command and input file name
      are supported. For tools which read a single gridded forecast and/or
      observation file, both options work. However, only the second option
      is supported for tools which read multiple gridded data files, such
      as Ensemble-Stat, Series-Analysis, and MTD.

    Option 1:
     
      * On the command line, replace the path to the input gridded data
        file with the constant string PYTHON_NUMPY or PYTHON_XARRAY.
	 
      * Specify the configuration "name" entry as the Python command to be
        executed to read the data.
	 
      * The "level" entry is not required for Python.

        For example:

         .. code-block:: none
        
           field = [
             { name = "read_ascii_numpy.py data/python/fcst.txt FCST"; }
           ];

    Option 2:

      * On the command line, leave the path to the input gridded data
        as is.
	 
      * Set the configuration "file_type" entry to the constant
        PYTHON_NUMPY or PYTHON_XARRAY.
	 
      * Specify the configuration "name" entry as the Python command to be
        executed to read the data, but replace the input gridded data file
        with the constant MET_PYTHON_INPUT_ARG.
	 
      * The "level" entry is not required for Python.

        For example:

        .. code-block:: none
			 
	  file_type = PYTHON_NUMPY;
          field     = [
            { name = "read_ascii_numpy.py MET_PYTHON_INPUT_ARG FCST"; }
          ];

	  
	  
.. code-block:: none
		
  fcst = {
     censor_thresh = [];
     censor_val    = [];
     cnt_thresh    = [ NA ];
     cnt_logic     = UNION;
     wind_thresh   = [ NA ];
     wind_logic    = UNION;
     eclv_points   = 0.05;
     message_type  = [ "ADPSFC" ];
     init_time     = "20120619_12";
     valid_time    = "20120620_00";
     lead_time     = "12";
  
     field = [
        {
          name       = "APCP";
          level      = [ "A03" ];
          cat_thresh = [ >0.0, >=5.0 ];
        }
     ];
  }

obs
^^^

The "obs" entry specifies the same type of information as "fcst", but for
the observation data. It will often be set to the same things as "fcst",
as shown in the example below. However, when comparing forecast and
observation files of different format types, this entry will need to be set
in a non-trivial way. The length of the "obs.field" array must match the
length of the "fcst.field" array.  For example:

.. code-block:: none
		
        obs = fcst;

or

.. code-block:: none
		
   fcst = {
     censor_thresh = [];
     censor_val    = [];
     cnt_thresh    = [ NA ];
     cnt_logic     = UNION;
     wind_thresh   = [ NA ];
     wind_logic    = UNION;

     field = [
        {
           name       = "PWAT";
           level      = [ "L0" ];
           cat_thresh = [ >2.5 ];
        }
      ];
   }


   obs = {
     censor_thresh = [];
     censor_val    = [];
     mpr_column    = [];
     mpr_thresh    = [];
     cnt_thresh    = [ NA ];
     cnt_logic     = UNION;
     wind_thresh   = [ NA ];
     wind_logic    = UNION;

     field = [
        {
           name       = "IWV";
           level      = [ "L0" ];
           cat_thresh = [ >25.0 ];
        }
      ];
   }

* The "message_type" entry is an array of point observation message types
  to be used. This only applies to the tools that verify against point
  observations. This may be specified once at the top-level "obs"
  dictionary or separately for each "field" array element. In the example
  shown above, this is specified in the "fcst" dictionary and copied to
  "obs".

* Simplified vertical level matching logic is applied for surface message
  types. Observations for the following message types are assumed to be at
  the surface, as defined by the default message_type_group_map:
  ADPSFC, SFCSHP, MSONET

* The "message_type" would be placed in the "field" array element if more
  than one "message_type" entry is desired within the config file. For example:

  .. code-block:: none
     
     fcst = {
         censor_thresh = [];
         censor_val    = [];
         cnt_thresh    = [ NA ];
         cnt_logic     = UNION;
         wind_thresh   = [ NA ];
         wind_logic    = UNION;

         field = [
            {
              message_type = [ "ADPUPA" ];
              sid_inc      = [];
              sid_exc      = [];
              name         = "TMP";
              level        = [ "P250", "P500", "P700", "P850", "P1000" ];
              cat_thresh   = [ <=273.0 ];
            },
            {
              message_type = [ "ADPSFC" ];
              sid_inc      = [];
              sid_exc      = [ "KDEN", "KDET" ];
              name         = "TMP";
              level        = [ "Z2" ];
              cat_thresh   = [ <=273.0 ];
            }
         ];
       }

 * The "sid_inc" entry is an array of station ID groups indicating which
   station ID's should be included in the verification task. If specified,
   only those station ID's appearing in the list will be included.  Note
   that filtering by station ID may also be accomplished using the "mask.sid"
   option. However, when using the "sid_inc" option, statistics are reported
   separately for each masking region.
   
 * The "sid_exc" entry is an array of station ID groups indicating which
   station ID's should be excluded from the verification task.
   
 * Each element in the "sid_inc" and "sid_exc" arrays is either the name of
   a single station ID or the full path to a station ID group file name.
   A station ID group file consists of a name for the group followed by a
   list of station ID's. All of the station ID's indicated will be concatenated
   into one long list of station ID's to be included or excluded.
   
 * As with "message_type" above, the "sid_inc" and "sid_exc" settings can be
   placed in the in the "field" array element to control which station ID's
   are included or excluded for each verification task.

.. code-block:: none
		
  obs = fcst;

climo_mean
^^^^^^^^^^
      
The "climo_mean" dictionary specifies climatology mean data to be read by the
Grid-Stat, Point-Stat, Ensemble-Stat, and Series-Analysis tools. It consists
of several entires defining the climatology file names and fields to be used.

* The "file_names" entry specifies one or more file names containing
  the gridded climatology data to be used.

* The "field" entry is an array of dictionaries, specified the same
  way as those in the "fcst" and "obs" dictionaries. If the array has
  length zero, not climatology data will be read and all climatology
  statistics will be written as missing data. Otherwise, the array
  length must match the length of "field" in the "fcst" and "obs"
  dictionaries.

* The "regrid" dictionary defines how the climatology data should be
  regridded to the verification domain.

* The "time_interp_method" entry specifies how the climatology data should
  be interpolated in time to the forecast valid time:
  
 * NEAREST for data closest in time
 * UW_MEAN for average of data before and after
 * DW_MEAN for linear interpolation in time of data before and after

* The "day_interval" entry is an integer specifying the spacing in days of
  the climatology data. Use 31 for monthly data or 1 for daily data.
  Use "NA" if the timing of the climatology data should not be checked.

* The "hour_interval" entry is an integer specifying the spacing in hours of
  the climatology data for each day. This should be set between 0 and 24,
  with 6 and 12 being common choices. Use "NA" if the timing of the
  climatology data should not be checked.

* The "day_interval" and "hour_interval" entries replace the deprecated
  entries "match_month", "match_day", and "time_step".

.. code-block:: none
		
  climo_mean = {
  
     file_name = [ "/path/to/climatological/mean/files" ];
     field     = [];
  
     regrid = {
        method     = NEAREST;
        width      = 1;
        vld_thresh = 0.5;
     }
  
     time_interp_method = DW_MEAN;
     day_interval       = 31;
     hour_interval      = 6;
  }

climo_stdev
^^^^^^^^^^^
      
The "climo_stdev" dictionary specifies climatology standard deviation data to
be read by the Grid-Stat, Point-Stat, Ensemble-Stat, and Series-Analysis
tools. The "climo_mean" and "climo_stdev" data define the climatological
distribution for each grid point, assuming normality. These climatological
distributions are used in two ways:

(1)
    To define climatological distribution percentile (CDP) thresholds which
    can be used as categorical (cat_thresh), continuous (cnt_thresh), or wind
    speed (wind_thresh) thresholds.

(2)
    To subset matched pairs into climatological bins based on where the
    observation value falls within the climatological distribution. See the
    "climo_cdf" dictionary.

This dictionary is identical to the "climo_mean" dictionary described above
but points to files containing climatological standard deviation values
rather than means. In the example below, this dictionary is set by copying
over the "climo_mean" setting and then updating the "file_name" entry.

.. code-block:: none
		
  climo_stdev = climo_mean;
  climo_stdev = {
     file_name = [ "/path/to/climatological/standard/deviation/files" ];
  }

climo_cdf
^^^^^^^^^
      
The "climo_cdf" dictionary specifies how the the climatological mean
("climo_mean") and standard deviation ("climo_stdev") data are used to
evaluate model performance relative to where the observation value falls
within the climatological distribution. This dictionary consists of the
following entries:

(1)
    The "cdf_bins" entry defines the climatological bins either as an integer
    or an array of floats between 0 and 1.

(2)
    The "center_bins" entry may be set to TRUE or FALSE.

(3)
    The "write_bins" entry may be set to TRUE or FALSE.

(4) The "direct_prob" entry may be set to TRUE or FALSE.

MET uses the climatological mean and standard deviation to construct a normal
PDF at each observation location. The total area under the PDF is 1, and the
climatological CDF value is computed as the area of the PDF to the left of
the observation value. Since the CDF is a value between 0 and 1, the CDF
bins must span that same range.

When "cdf_bins" is set to an array of floats, they explicitly define the
climatological bins. The array must begin with 0.0 and end with 1.0.
For example:

.. code-block:: none
		
  cdf_bins = [ 0.0, 0.10, 0.25, 0.75, 0.90, 1.0 ];

When "cdf_bins" is set to an integer, it defines the number of bins to be
used. The "center_bins" flag indicates whether or not the bins should be
centered on 0.5. An odd number of bins can be centered or uncentered while
an even number of bins can only be  uncentered. For example:

.. code-block:: none
		
  4 uncentered bins (cdf_bins = 4; center_bins = FALSE;) yields:
    0.0, 0.25, 0.50, 0.75, 1.0
  5 uncentered bins (cdf_bins = 5; center_bins = FALSE;) yields:
    0.0, 0.2, 0.4, 0.6, 0.8, 0.9, 1.0
  5   centered bins (cdf_bins = 5; center_bins = TRUE;) yields:
    0.0, 0.125, 0.375, 0.625, 0.875, 1.0

When multiple climatological bins are used for Point-Stat and Grid-Stat,
statistics are computed separately for each bin, and the average of the
statistics across those bins is written to the output. When "write_bins"
is true, the statistics for each bin are also written to the output.
The bin number is appended to the contents of the VX_MASK output column.

Setting the number of bins to 1 effectively disables this logic by grouping
all pairs into a single climatological bin.

.. code-block:: none
		
  climo_cdf = {
     cdf_bins    = 11;    or an array of floats
     center_bins = TRUE;  or FALSE
     write_bins  = FALSE; or TRUE
     direct_prob = FALSE; or TRUE
  }

climato_data
^^^^^^^^^^^^
      
When specifying climatology data for probability forecasts, either supply a
probabilistic "climo_mean" field or non-probabilistic "climo_mean" and
"climo_stdev" fields from which a normal approximation of the climatological
probabilities should be derived.

When "climo_mean" is set to a probability field with a range of [0, 1] and
"climo_stdev" is unset, the MET tools use the "climo_mean" probability values
directly to compute Brier Skill Score (BSS).

When "climo_mean" and "climo_stdev" are both set to non-probability fields,
the MET tools use the mean, standard deviation, and observation event
threshold to derive a normal approximation of the climatological
probabilities.

The "direct_prob" option controls the derivation logic. When "direct_prob" is
true, the climatological probability is computed directly from the
climatological distribution at each point as the area to the left of
the event threshold value. For greater-than or greater-than-or-equal-to
thresholds, 1.0 minus the area is used. When "direct_prob" is false, the
"cdf_bins" values are sampled from climatological distribution. The probability
is computed as the proportion of those samples which meet the threshold criteria.
In this way, the number of bins impacts the resolution of the climatological
probabilities. These derived probability values are used to compute the
climatological Brier Score and Brier Skill Score.


The "seeps_p1_thresh" option controls the threshold of p1 (probability of being dry) values.

.. code-block:: none
		
  seeps_p1_thresh = >=0.1&&<=0.85;


mask_missing_flag
^^^^^^^^^^^^^^^^^

The "mask_missing_flag" entry specifies how missing data should be handled
in the Wavelet-Stat and MODE tools:

 * "NONE" to perform no masking of missing data
   
 * "FCST" to mask the forecast field with missing observation data
   
 * "OBS" to mask the observation field with missing forecast data
   
 * "BOTH" to mask both fields with missing data from the other

.. code-block:: none
		
  mask_missing_flag = BOTH;


obs_window
^^^^^^^^^^

The "obs_window" entry is a dictionary specifying a beginning ("beg"
entry) and ending ("end" entry) time offset values in seconds. It defines
the time window over which observations are retained for scoring. These time
offsets are defined relative to a reference time t, as [t+beg, t+end].
In PB2NC, the reference time is the PREPBUFR files center time. In
Point-Stat and Ensemble-Stat, the reference time is the forecast valid time.

.. code-block:: none
		
  obs_window = {
     beg = -5400;
     end =  5400;
  }

mask
^^^^
     
The "mask" entry is a dictionary that specifies the verification masking
regions to be used when computing statistics. Each mask defines a
geographic extent, and any matched pairs falling inside that area will be
used in the computation of statistics. Masking regions may be specified
in the following ways:

* The "grid" entry is an array of named grids. It contains a
  comma-separated list of pre-defined NCEP grids over which to perform
  verification. An empty list indicates that no masking grids should be
  used. The standard NCEP grids are named "GNNN" where NNN indicates the
  three digit grid number. Supplying a value of "FULL" indicates that the
  verification should be performed over the entire grid on which the data
  resides.
  See: `ON388 - TABLE B, GRID IDENTIFICATION (PDS Octet 7), MASTER LIST OF NCEP STORAGE GRIDS, GRIB Edition 1 (FM92) <http://www.nco.ncep.noaa.gov/pmb/docs/on388/tableb.html>`_.  
  The "grid" entry can be the gridded data file defining grid.

* The "poly" entry contains a comma-separated list of files that define
  verification masking regions. These masking regions may be specified in
  two ways: as a lat/lon polygon or using a gridded data file such as the
  NetCDF output of the Gen-Vx-Mask tool.

  * An ASCII file containing a lat/lon polygon.
    Latitude in degrees north and longitude in degrees east.
    The first and last polygon points are connected.
    For example, "MET_BASE/poly/EAST.poly" which consists of n points:
    "poly_name lat1 lon1 lat2 lon2... latn lonn"

    Several masking polygons used by NCEP are predefined in the
    installed *share/met/poly* directory. Creating a new polygon is as
    simple as creating a text file with a name for the polygon followed
    by the lat/lon points which define its boundary. Adding a new masking
    polygon requires no code changes and no recompiling. Internally, the
    lat/lon polygon points are converted into x/y values in the grid. The
    lat/lon values for the observation points are also converted into x/y
    grid coordinates. The computations performed to check whether the
    observation point falls within the polygon defined is done in x/y
    grid space.

  * The NetCDF output of the gen_vx_mask tool.

  * Any gridded data file that MET can read may be used to define a
    verification masking region. Users must specify a description of the
    field to be used from the input file and, optionally, may specify a
    threshold to be applied to that field. Once this threshold is
    applied, any grid point where the resulting field is 0, the mask is
    turned off. Any grid point where it is non-zero, the mask is turned
    on.
    For example,  "sample.grib {name = \"TMP\"; level = \"Z2\";} >273"

* The "sid" entry is an array of strings which define groups of
  observation station ID's over which to compute statistics. Each entry
  in the array is either a filename of a comma-separated list.
  
  * For a filename, the strings are whitespace-separated. The first
    string is the mask "name" and the remaining strings are the station
    ID's to be used.
  * For a comma-separated list, optionally use a colon to specify a name.
    For "MY_LIST:SID1,SID2", name = MY_LIST and values = SID1 and SID2.
  * For a comma-separated list of length one with no name specified, the
    mask "name" and value are both set to the single station ID string.
    For "SID1", name = SID1 and value = SID1.
  * For a comma-separated list of length greater than one with no name
    specified, the name is set to MASK_SID and the values are the station
    ID's to be used.
    For "SID1,SID2", name = MASK_SID and values = SID1 and SID2.
  * The "name" of the station ID mask is written to the VX_MASK column
    of the MET output files.
* The "llpnt" entry is either a single dictionary or an array of
  dictionaries. Each dictionary contains three entries, the "name" for
  the masking region, "lat_thresh", and "lon_thresh". The latitude and
  longitude thresholds are applied directly to the point observation
  latitude and longitude values. Only observations whose latitude and
  longitude values meet this threshold criteria are used. A threshold set
  to "NA" always evaluates to true.

The masking logic for processing point observations in Point-Stat and
Ensemble-Stat fall into two cateogries. The "sid" and "llpnt" options apply
directly to the point observations. Only those observations for the specified
station id's are included in the "sid" masks. Only those observations meeting
the latitude and longitude threshold criteria are included in the "llpnt"
masks.

The "grid" and "poly" mask options are applied to the grid points of the
verification domain. Each grid point is determined to be inside or outside
the masking region. When processing point observations, their latitude and
longitude values are rounded to the nearest grid point of the verification
domain. If the nearest grid point is inside the mask, that point observation
is included in the mask.

.. code-block:: none
		
  mask = {
     grid    = [ "FULL" ];
     poly    = [ "MET_BASE/poly/LMV.poly",
                 "MET_BASE/out/gen_vx_mask/CONUS_poly.nc",
                 "MET_BASE/sample_fcst/2005080700/wrfprs_ruc13_12.tm00_G212 \
                 {name = \"TMP\"; level = \"Z2\";} >273"
               ];
     sid     = [ "CONUS.stations" ];
     llpnt   = [ { name       = "LAT30TO40";
                   lat_thresh = >=30&&<=40;
                   lon_thresh = NA; },
                 { name       = "BOX";
                   lat_thresh = >=20&&<=40;
                   lon_thresh = >=-110&&<=-90; } ];
  }


ci_alpha
^^^^^^^^

The "ci_alpha" entry is an array of floats specifying the values for alpha
to be used when computing confidence intervals. Values of alpha must be
between 0 and 1. The confidence interval computed is 1 minus the alpha
value. Therefore, an alpha value of 0.05 corresponds to a 95% confidence
interval.

.. code-block:: none
		
  ci_alpha = [ 0.05, 0.10 ];

boot
^^^^

The "boot" entry defines the parameters to be used in calculation of
bootstrap confidence intervals. The interval variable indicates what method
should be used for computing bootstrap confidence intervals:

* The "interval" entry specifies the confidence interval method:
  
  * "BCA" for the BCa (bias-corrected percentile) interval method is
    highly accurate but computationally intensive.
    
  * "PCTILE" uses the percentile method which is somewhat less accurate
    but more efficient.

* The "rep_prop" entry specifies a proportion between 0 and 1 to define
  the replicate sample size to be used when computing percentile
  intervals. The replicate sample size is set to boot_rep_prop * n,
  where n is the number of raw data points.

  When computing bootstrap confidence intervals over n sets of matched
  pairs, the size of the subsample, m, may be chosen less than or equal to
  the size of the sample, n. This variable defines the size of m as a
  proportion relative to the size of n. A value of 1 indicates that the
  size of the subsample, m, should be equal to the size of the sample, n.

* The "n_rep" entry defines the number of subsamples that should be taken
  when computing bootstrap confidence intervals. This variable should be
  set large enough so that when confidence intervals are computed multiple
  times for the same set of data, the intervals do not change much.
  Setting this variable to zero disables the computation of bootstrap
  confidence intervals, which may be necessary to run MET in realtime or
  near-realtime over large domains since bootstrapping is computationally
  expensive. Setting this variable to 1000 indicates that bootstrap
  confidence interval should be computed over 1000 subsamples of the
  matched pairs.

* The "rng" entry defines the random number generator to be used in the
  computation of bootstrap confidence intervals. Subsamples are chosen at
  random from the full set of matched pairs. The randomness is determined
  by the random number generator specified. Users should refer to detailed
  documentation of the
  `GNU Scientific Library <https://www.gnu.org/software/gsl/doc/html/rng.html>`_
  for a listing of the random number generators available for use.
  
* The "seed" entry may be set to a specific value to make the computation
  of bootstrap confidence intervals fully repeatable. When left empty
  the random number generator seed is chosen automatically which will lead
  to slightly different bootstrap confidence intervals being computed each
  time the data is run. Specifying a value here ensures that the bootstrap
  confidence intervals will be reproducable over multiple runs on the same
  computing platform.

.. code-block:: none
		
  boot = {
     interval = PCTILE;
     rep_prop = 1.0;
     n_rep    = 0;
     rng      = "mt19937";
     seed     = "";
  }

interp
^^^^^^

The "interp" entry is a dictionary that specifies what interpolation or
smoothing (for the Grid-Stat tool) methods should be applied.
This dictionary may include the following entries:

* The "field" entry specifies to which field(s) the interpolation method
  should be applied. This does not apply when doing point verification
  with the Point-Stat or Ensemble-Stat tools:
  
  * "FCST" to interpolate/smooth the forecast field.
    
  * "OBS" to interpolate/smooth the observation field.
    
  * "BOTH" to interpolate/smooth both the forecast and the observation.

* The "vld_thresh" entry specifies a number between 0 and 1. When
  performing interpolation over some neighborhood of points the ratio of
  the number of valid data points to the total number of points in the
  neighborhood is computed. If that ratio is less than this threshold,
  the matched pair is discarded. Setting this threshold to 1, which is the
  default, requires that the entire neighborhood must contain valid data.
  This variable will typically come into play only along the boundaries of
  the verification region chosen.

* The "shape" entry may be set to SQUARE or CIRCLE to specify the shape
  of the smoothing area.

* The "type" entry is an array of dictionaries, each specifying one or more
  interpolation methods and widths. Interpolation is performed over an N by N
  box centered on each point, where N is the width specified. Each of these
  dictionaries must include:

  * The "width" entry is an array of integers to specify the size of the
    interpolation area. The area is either a square or circle containing
    the observation point. The width value specifies the width of the
    square or diameter of the circle. A width value of 1 is interpreted
    as the nearest neighbor model grid point to the observation point.
    For squares, a width of 2 defines a 2 x 2 box of grid points around
    the observation point (the 4 closest model grid points), while a width
    of 3 defines a 3 x 3 box of grid points around the observation point,
    and so on. For odd widths in grid-to-point comparisons
    (i.e. Point-Stat), the interpolation area is centered on the model
    grid point closest to the observation point. For grid-to-grid
    comparisons (i.e. Grid-Stat), the width must be odd.

  * The "method" entry is an array of interpolation procedures to be
    applied to the points in the box:
    
    * MIN         for the minimum value
    
    * MAX         for the maximum value
    
    * MEDIAN      for the median value
    
    * UW_MEAN     for the unweighted average value
    
    * DW_MEAN     for the distance-weighted average value
      where weight = distance^-2
		
    * LS_FIT      for a least-squares fit
    
    * BILIN       for bilinear interpolation (width = 2)
    
    * NEAREST     for the nearest grid point (width = 1)
    
    * BEST        for the value closest to the observation
    
    * UPPER_LEFT  for the upper left grid point (width = 1)

    * UPPER_RIGHT for the upper right grid point (width = 1)
    
    * LOWER_RIGHT for the lower right grid point (width = 1)
    
    * LOWER_LEFT  for the lower left grid point (width = 1)

    * GAUSSIAN    for the Gaussian kernel

    * MAXGAUSS    for the maximum value followed by a Gaussian smoother
    
    * GEOG_MATCH  for the nearest grid point where the land/sea mask
      and geography criteria are satisfied

    * HIRA        for all neighborhood points to define a spatial
      ensemble (only in Ensemble-Stat)

    The BUDGET, FORCE, GAUSSIAN, and MAXGAUSS methods are not valid for
    interpolating to point locations. For grid-to-grid comparisons, the
    only valid smoothing methods are MIN, MAX, MEDIAN, UW_MEAN, and
    GAUSSIAN, and MAXGAUSS.

  * If multiple "method" and "width" options are specified, all possible
    permutations of their values are applied.

.. code-block:: none
		
  interp = {
     field      = BOTH;
     vld_thresh = 1.0;
     shape      = SQUARE;
  
     type = [
        {
           method = [ NEAREST ];
           width  = [ 1 ];
        }
     ];
  }

land_mask
^^^^^^^^^
     
The "land_mask" dictionary defines the land/sea mask field used when
verifying at the surface. The "flag" entry enables/disables this logic.
When enabled, the "message_type_group_map" dictionary must contain entries
for "LANDSF" and "WATERSF". For point observations whose message type
appears in the "LANDSF" entry, only use forecast grid points where land =
TRUE. For point observations whose message type appears in the "WATERSF"
entry, only use forecast grid points where land = FALSE. If the "file_name"
entry is left empty, the land/sea is assumed to exist in the input forecast
file. Otherwise, the specified file(s) are searched for the data specified
in the "field" entry. The "regrid" settings specify how this field should be
regridded to the verification domain. Lastly, the "thresh" entry is the
threshold which defines land (threshold is true) and water (threshold is false).

The "land_mask.flag" entry may be set separately in each "obs.field" entry.

.. code-block:: none
		
  land_mask = {
     flag      = FALSE;
     file_name = [];
     field     = { name = "LAND"; level = "L0"; }
     regrid    = { method = NEAREST; width = 1; }
     thresh    = eq1;
  }

topo_mask
^^^^^^^^^
     
The "topo_mask" dictionary defines the model topography field used when
verifying at the surface. The flag entry enables/disables this logic.
When enabled, the "message_type_group_map" dictionary must contain an entry
for "SURFACE". This logic is applied to point observations whose message type
appears in the "SURFACE" entry. Only use point observations where the
topo minus station elevation difference meets the "use_obs_thresh" threshold
entry. For the observations kept, when interpolating forecast data to the
observation location, only use forecast grid points where the topo minus station
difference meets the "interp_fcst_thresh" threshold entry.  If the "file_name"
is left empty, the topography data is assumed to exist in the input forecast file.
Otherwise, the specified file(s) are searched for the data specified in the "field"
entry. The "regrid" settings specify how this field should be regridded to
the verification domain.

The "topo_mask.flag" entry may be set separately in each "obs.field" entry.

.. code-block:: none
		
  topo_mask = {
     flag               = FALSE;
     file_name          = [];
     field              = { name = "TOPO"; level = "L0"; }
     regrid             = { method = BILIN; width = 2; }
     use_obs_thresh     = ge-100&&le100;
     interp_fcst_thresh = ge-50&&le50;
  }

hira
^^^^
     
The "hira" entry is a dictionary that is very similar to the "interp" and
"nbrhd" entries. It specifies information for applying the High Resolution
Assessment (HiRA) verification logic in Point-Stat. HiRA is analogous to
neighborhood verification but for point observations. The HiRA logic
interprets the forecast values surrounding each point observation as an
ensemble forecast. These ensemble values are processed in two ways. First,
the ensemble continuous statistics (ECNT) and ranked probability score (RPS)
line types are computed directly from the ensemble values. Second, for each
categorical threshold specified, a fractional coverage value is computed as
the ratio of the nearby forecast values that meet the threshold criteria.
Point-Stat evaluates those fractional coverage values as if they were a
probability forecast. When applying HiRA, users should enable the matched
pair (MPR), probabilistic (PCT, PSTD, PJC, or PRC), or ensemble statistics
(ECNT or PRS) line types in the output_flag dictionary. The number of
probabilistic HiRA output lines is determined by the number of categorical
forecast thresholds and HiRA neighborhood widths chosen.
This dictionary may include the following entries:

* The "flag" entry is a boolean which toggles "hira"
  on (TRUE) and off (FALSE).

* The "width" entry specifies the neighborhood size. Since HiRA applies
  to point observations, the width may be even or odd.

* The "vld_thresh" entry is as described above.

* The "cov_thresh" entry is an array of probabilistic thresholds used to
  populate the Nx2 probabilistic contingency table written to the PCT
  output line and used for computing probabilistic statistics.

* The "shape" entry defines the shape of the neighborhood.
  Valid values are "SQUARE" or "CIRCLE"

* The "prob_cat_thresh" entry defines the thresholds which define ensemble
  probabilities from which to compute the ranked probability score output.
  If left empty but climatology data is provided, the climo_cdf thresholds
  will be used instead. If left empty but no climatology data is provided,
  the obs.cat_thresh thresholds will be used instead.

.. code-block:: none
		
  hira = {
      flag            = FALSE;
     width           = [ 2, 3, 4, 5 ];
     vld_thresh      = 1.0;
     cov_thresh      = [ ==0.25 ];
     shape           = SQUARE;
     prob_cat_thresh = [];
  }

output_flag
^^^^^^^^^^^
     
The "output_flag" entry is a dictionary that specifies what verification
methods should be applied to the input data. Options exist for each
output line type from the MET tools. Each line type may be set to one of:

* "NONE" to skip the corresponding verification method
  
* "STAT" to write the verification output only to the ".stat" output file
  
* "BOTH" to write to the ".stat" output file as well the optional
  "_type.txt" file, a more readable ASCII file sorted by line type.

.. code-block:: none
		
  output_flag = {
     fho    = NONE;  Forecast, Hit, Observation Rates
     ctc    = NONE;  Contingency Table Counts
     cts    = NONE;  Contingency Table Statistics
     mctc   = NONE;  Multi-category Contingency Table Counts
     mcts   = NONE;  Multi-category Contingency Table Statistics
     cnt    = NONE;  Continuous Statistics
     sl1l2  = NONE;  Scalar L1L2 Partial Sums
     sal1l2 = NONE;  Scalar Anomaly L1L2 Partial Sums when climatological data
                     is supplied
     vl1l2  = NONE;  Vector L1L2 Partial Sums
     val1l2 = NONE;  Vector Anomaly L1L2 Partial Sums when climatological data
                     is supplied
     pct    = NONE;  Contingency Table Counts for Probabilistic Forecasts
     pstd   = NONE;  Contingency Table Statistics for Probabilistic Forecasts
                     with Dichotomous outcomes
     pjc    = NONE;  Joint and Conditional Factorization for Probabilistic
                     Forecasts
     prc    = NONE;  Receiver Operating Characteristic for Probabilistic
                     Forecasts
     eclv   = NONE;  Economic Cost/Loss Value derived from CTC and PCT lines
     mpr    = NONE;  Matched Pair Data
     nbrctc = NONE;  Neighborhood Contingency Table Counts
     nbrcts = NONE;  Neighborhood Contingency Table Statistics
     nbrcnt = NONE;  Neighborhood Continuous Statistics
     isc    = NONE;  Intensity-Scale
     ecnt   = NONE;  Ensemble Continuous Statistics
     rps    = NONE;  Ranked Probability Score Statistics
     rhist  = NONE;  Rank Histogram
     phist  = NONE;  Probability Integral Transform Histogram
     orank  = NONE;  Observation Rank
     ssvar  = NONE;  Spread Skill Variance
     grad   = NONE;  Gradient statistics (S1 score)
  }

nc_pairs_flag
^^^^^^^^^^^^^

The "nc_pairs_flag" can be set either to a boolean value or a dictionary
in either Grid-Stat, Wavelet-Stat or MODE. The dictionary (with slightly
different entries for the various tools ... see the default config files)
has individual boolean settings turning on or off the writing out of the
various fields in the netcdf output file for the tool. Setting all
dictionary entries to false means the netcdf file will not be generated.

"nc_pairs_flag" can also be set to a boolean value. In this case, a value
of true means to just accept the default settings (which will turn on
the output of all the different fields). A value of false means no
netcdf output will be generated.

.. code-block:: none
		
  nc_pairs_flag = {
     latlon       = TRUE;
     raw          = TRUE;
     diff         = TRUE;
     climo        = TRUE;
     climo_cdp    = FALSE;
     weight       = FALSE;
     nbrhd        = FALSE;
     fourier      = FALSE;
     gradient     = FALSE;
     distance_map = FLASE;
     apply_mask   = TRUE;
  }

nc_pairs_var_name
^^^^^^^^^^^^^^^^^
  
The "nc_pairs_var_name" entry specifies a string for each verification task
in Grid-Stat. This string is parsed from each "obs.field" dictionary entry
and is used to construct variable names for the NetCDF matched pairs output
file. The default value of an empty string indicates that the "name" and
"level" strings of the input data should be used.  If the input data "level"
string changes for each run of Grid-Stat, using this option to define a
constant string may make downstream processing more convenient.

For example:

| nc_pairs_var_name = "TMP";
|
 
.. code-block:: none
		
  nc_pairs_var_name = "";

nc_pairs_var_suffix
^^^^^^^^^^^^^^^^^^^
     
The "nc_pairs_var_suffix" entry is similar to the "nc_pairs_var_name" entry
described above.  It is also parsed from each "obs.field" dictionary entry.
However, it defines a suffix to be appended to the output variable name.
This enables the output variable names to be made unique. For example, when
verifying height for multiple level types but all with the same level value,
use this option to customize the output variable names.

For example:

| nc_pairs_var_suffix = "TROPO"; (for the tropopause height)
| nc_pairs_var_suffix = "FREEZING"; (for the freezing level height)
|

NOTE: This option was previously named "nc_pairs_var_str", which is
now deprecated.

.. code-block:: none
		
  nc_pairs_var_suffix = "";

ps_plot_flag
^^^^^^^^^^^^
     
The "ps_plot_flag" entry is a boolean value for Wavelet-Stat and MODE
indicating whether a PostScript plot should be generated summarizing
the verification.

.. code-block:: none
		
  ps_plot_flag = TRUE;

grid_weight_flag
^^^^^^^^^^^^^^^^
     
The "grid_weight_flag" specifies how grid weighting should be applied
during the computation of continuous statistics and partial sums. It is
meant to account for grid box area distortion and is often applied to global
Lat/Lon grids. It is only applied for grid-to-grid verification in Grid-Stat
and Ensemble-Stat and is not applied for grid-to-point verification.
Three grid weighting options are currently supported:

* "NONE" to disable grid weighting using a constant weight (default).
  
* "COS_LAT" to define the weight as the cosine of the grid point latitude.
  This an approximation for grid box area used by NCEP and WMO.
  
* "AREA" to define the weight as the true area of the grid box (km^2).

The weights are ultimately computed as the weight at each grid point divided
by the sum of the weights for the current masking region.

.. code-block:: none
		
  grid_weight_flag = NONE;

hss_ec_value
^^^^^^^^^^^^

The "hss_ec_value" entry is a floating point number used in the computation
of the HSS_EC statistic in the CTS and MCTS line types. It specifies the expected
correct (EC) rate by chance for multi-category contingency tables. If set
to its default value of NA, it will automatically be replaced with 1.0
divided by the CTC or MCTC table dimension. For example, for a 2x2 CTC table,
the default hss_ec_value is 1.0 / 2 = 0.5. For a 4x4 MCTC table, the
default hss_ec_value is 1.0 / 4 = 0.25.

If set, it must greater than or equal to 0.0 and less than 1.0. A value of
0.0 produces an HSS_EC statistic equal to the Accuracy statistic.

.. code-block:: none
		
  hss_ec_value = NA;

rank_corr_flag
^^^^^^^^^^^^^^

The "rank_corr_flag" entry is a boolean to indicate whether Kendall's Tau
and Spearman's Rank Correlation Coefficients (in the CNT line type) should
be computed. Computing them over large datasets is computationally
intensive and slows down the runtime significantly.

.. code-block:: none
		
  rank_corr_flag = FALSE;

duplicate_flag
^^^^^^^^^^^^^^

The "duplicate_flag" entry specifies how to handle duplicate point
observations in Point-Stat and Ensemble-Stat:

* "NONE" to use all point observations (legacy behavior)
  
* "UNIQUE" only use a single observation if two or more observations
  match. Matching observations are determined if they contain identical
  latitude, longitude, level, elevation, and time information.
  They may contain different observation values or station IDs

The reporting mechanism for this feature can be activated by specifying
a verbosity level of three or higher. The report will show information
about where duplicates were detected and which observations were used
in those cases.

.. code-block:: none
		
  duplicate_flag = NONE;

obs_summary
^^^^^^^^^^^

The "obs_summary" entry specifies how to compute statistics on
observations that appear at a single location (lat,lon,level,elev)
in Point-Stat and Ensemble-Stat. Eight techniques are
currently supported:

* "NONE" to use all point observations (legacy behavior)
  
* "NEAREST" use only the observation that has the valid
  time closest to the forecast valid time
  
* "MIN" use only the observation that has the lowest value
  
* "MAX" use only the observation that has the highest value
  
* "UW_MEAN" compute an unweighted mean of the observations
  
* "DW_MEAN" compute a weighted mean of the observations based
  on the time of the observation
  
* "MEDIAN" use the median observation
  
* "PERC" use the Nth percentile observation where N = obs_perc_value

The reporting mechanism for this feature can be activated by specifying
a verbosity level of three or higher. The report will show information
about where duplicates were detected and which observations were used
in those cases.

.. code-block:: none
		
  obs_summary = NONE;


obs_perc_value
^^^^^^^^^^^^^^
     
Percentile value to use when obs_summary = PERC

.. code-block:: none
		
  obs_perc_value = 50;

  
obs_quality_inc
^^^^^^^^^^^^^^^
		
The "obs_quality_inc" entry specifies the quality flag values that are to be
retained and used for verification. An empty list signifies that all
point observations should be used, regardless of their quality flag value.
The quality flag values will vary depending on the original source of the
observations. The quality flag values to retain should be specified as
an array of strings, even if the values themselves are numeric.
Note "obs_quality_inc" replaces the older option "obs_quality".

.. code-block:: none
		
  obs_quality_inc = [ "1", "2", "3", "9" ];

  
obs_quality_exc
^^^^^^^^^^^^^^^
		
The "obs_quality_exc" entry specifies the quality flag values that are to be
ignored and not used for verification. An empty list signifies that all
point observations should be used, regardless of their quality flag value.
The quality flag values will vary depending on the original source of the
observations. The quality flag values to ignore should be specified as
an array of strings, even if the values themselves are numeric.

.. code-block:: none
		
  obs_quality_exc = [ "1", "2", "3", "9" ];

  
met_data_dir
^^^^^^^^^^^^

The "met_data_dir" entry specifies the location of the internal MET data
sub-directory which contains data files used when generating plots. It
should be set to the installed *share/met* directory so the MET tools can
locate the static data files they need at run time.

.. code-block:: none
		
  met_data_dir = "MET_BASE";

many_plots
^^^^^^^^^^

The "fcst_raw_plot" entry is a dictionary used by Wavelet-Stat and MODE
containing colortable plotting information for the plotting of the raw
forecast field:

* The "color_table" entry specifies the location and name of the
  colortable file to be used.

* The "plot_min" and "plot_max" entries specify the range of data values.
  If they are both set to 0, the MET tools will automatically rescale
  the colortable to the range of values present in the data. If they
  are not both set to 0, the MET tools will rescale the colortable using
  their values.

* When applicable, the "colorbar_flag" enables the creation of a colorbar
  for this plot.

.. code-block:: none
		
  fcst_raw_plot = {
     color_table   = "MET_BASE/colortables/met_default.ctable";
     plot_min      = 0.0;
     plot_max      = 0.0;
     colorbar_flag = TRUE;
  }


The "obs_raw_plot", "wvlt_plot", and "object_plot" entries are dictionaries
similar to the "fcst_raw_plot" described above.

output_prefix
^^^^^^^^^^^^^

The "output_prefix" entry specifies a string to be included in the output
file name. The MET statistics tools construct output file names that
include the tool name and timing information. You can use this setting
to modify the output file name and avoid naming conflicts for multiple runs
of the same tool.

.. code-block:: none
		
  output_prefix  = "";

version
^^^^^^^

The "version" entry specifies the version number of the configuration file.
The configuration file version number should match the version number of
the MET code being run. This value should generally not be modified.

.. code-block:: none
		
  version = "VN.N";

time_summary
^^^^^^^^^^^^

This feature was implemented to allow additional processing of observations
with high temporal resolution. The "flag" entry toggles the "time_summary"
on (TRUE) and off (FALSE). Obs may be summarized across the user specified
time period defined by the "beg" and "end" entries. The "step" entry defines
the time between intervals in seconds. The "width" entry specifies the
summary interval in seconds. It may either be set as an integer number of
seconds for a centered time interval or a dictionary with beginning and
ending time offsets in seconds.

For example:

.. code-block:: none
		
   beg = "00";
   end = "235959";
   step = 300;
   width = 600;
   width = { beg = -300; end = 300; }

This example does a 10-minute time summary every 5 minutes throughout the
day. The first interval will be from 23:55:00 the previous day through
00:04:59 of the current day. The second interval will be from 0:00:00
through 00:09:59. And so on.

The two "width" settings listed above are equivalent. Both define a centered
10-minute time interval. Use the "beg" and "end" entries to define
uncentered time intervals. The following example requests observations for
one hour prior:

.. code-block:: none
		
  width = { beg = -3600; end = 0; }

The summaries will only be calculated for the specified GRIB codes
or observation variable ("obs_var") names.

When determining which observations fall within a time interval, data for the
beginning timestamp is included while data for the ending timestamp is excluded.
Users may need to adjust the "beg" and "end" settings in the "width" dictionary
to include the desired observations in each time interval.

The supported time summaries are "min" (minimum), "max" (maximum), "range",
"mean", "stdev" (standard deviation), "median", "sum", and "p##" (percentile,
with the desired percentile value specified in place of ##).

The "vld_freq" and "vld_thresh" options may be used to require that a certain
ratio of observations must be present and contain valid data within the time
window in order for a summary value to be computed. The "vld_freq" entry
defines the expected observation frequency in seconds. For example, when
summarizing 1-minute data (vld_freq = 60) over a 30 minute time window,
setting "vld_thresh = 0.5" requires that at least 15 of the 30 expected
observations be present and valid for a summary value to be written. The
default "vld_thresh = 0.0" setting will skip over this logic.

When using the "sum" option, users should specify "vld_thresh = 1.0" to avoid
missing data values from affecting the resulting sum value.

The variable names are saved to NetCDF file if they are given instead of
grib_codes which are not available for non GRIB input. The "obs_var" option
was added and works like "grib_code" option (string value VS. int value).
They are inclusive (union). All variables are included if both options
are empty. Note: grib_code 11 is equivalent to obs_var "TMP".

.. code-block:: none
		
  time_summary = {
    flag = FALSE;
    beg = "000000";
    end = "235959";
    step = 300;
    width = 600;
    width = { beg = -300; end = 300; }
    grib_code = [ 11, 204, 211 ];
    obs_var   = [];
    type = [ "min", "max", "range", "mean", "stdev", "median", "p80" ];
    vld_freq = 0;
    vld_thresh = 0.0;
  }

Settings specific to individual tools
-------------------------------------

EnsembleStatConfig_default
^^^^^^^^^^^^^^^^^^^^^^^^^^

ens
"""

The "ens" entry is a dictionary that specifies the fields for which ensemble
products should be generated. This is very similar to the "fcst" and "obs"
entries. This dictionary may include the following entries:

* The "censor_thresh" and "censor_val" entries are described above.

* The "ens_thresh" entry specifies a proportion between 0 and 1 to define
  the required ratio of valid input ensemble member files. If the ratio
  of valid input ensemble files to expected ones is too low, the tool
  will error out.

* The "vld_thresh" entry specifies a proportion between 0 and 1 to
  define the required ratio of valid data points. When computing
  ensemble products, if the ratio of valid data values is too low, the
  ensemble product will be set to bad data for that point.

* The "field" entry is as described above. However, in this case, the
  cat_thresh entry is used for calculating probabilities of exceeding
  the given threshold. In the default shown below, the probability of
  accumulated precipitation > 0.0 mm and > 5.0 mm will be calculated
  from the member accumulated precipitation fields and stored as an
  ensemble field.

.. code-block:: none
		
  ens = {
     censor_thresh = [];
     censor_val    = [];
     ens_thresh    = 1.0;
     vld_thresh    = 1.0;
  
     field = [
        {
           name       = "APCP";
           level      = "A03";
           cat_thresh = [ >0.0, >=5.0 ];
        }
     ];
  }

nbrhd_prob
""""""""""

The nbrhd_prob dictionary defines the neighborhoods used to compute NEP
and NMEP output. The neighborhood shape is a SQUARE or CIRCLE centered on
the current point, and the width array specifies the width of the square or
diameter of the circle as an odd integer. The vld_thresh entry is a number
between 0 and 1 specifying the required ratio of valid data in the
neighborhood for an output value to be computed.

If ensemble_flag.nep is set to TRUE, NEP output is created for each
combination of the categorical threshold (cat_thresh) and neighborhood width
specified.

.. code-block:: none
		
  nbrhd_prob = {
     width      = [ 5 ];
     shape      = CIRCLE;
     vld_thresh = 0.0;
  }

nmep_smooth
"""""""""""

Similar to the interp dictionary, the nmep_smooth dictionary includes a type
array of dictionaries to define one or more methods for smoothing the NMEP
data. Setting the interpolation method to nearest neighbor (NEAREST)
effectively disables this smoothing step.

If ensemble_flag.nmep is set to TRUE, NMEP output is created for each
combination of the categorical threshold (cat_thresh), neighborhood width
(nbrhd_prob.width), and smoothing method (nmep_smooth.type) specified.

.. code-block:: none
		
  nmep_smooth = {
     vld_thresh      = 0.0;
     shape           = CIRCLE;
     gaussian_dx     = 81.27;
     gaussian_radius = 120;
     type = [
        {
           method = GAUSSIAN;
           width  = 1;
        }
     ];
  }

fcst, obs
"""""""""

The fcst and obs entries define the fields for which Ensemble-Stat should
compute rank histograms, probability integral transform histograms,
spread-skill variance, relative position histograms, economic value, and
other statistics.

The "ens_ssvar_bin_size" entry sets the width of the variance bins. Smaller
bin sizes provide the user with more flexibility in how data are binned
during analysis. The actual variance of the ensemble data will determine the
number of bins written to the SSVAR output lines.

The "ens_phist_bin_size" is set to a value between 0 and 1. The number of
bins for the probability integral transform histogram in the PHIST line type
is defined as the ceiling of 1.0 / ens_phist_bin_size. For example, a bin
size of 0.05 results in 20 PHIST bins.

The "prob_cat_thresh" entry is an array of thresholds to be applied in the
computation of the ranked probability score.  If left empty, but climatology
data is provided, the climo_cdf thresholds will be used instead.

.. code-block:: none
		
  fcst = {
     message_type       = [ "ADPUPA" ];
     ens_ssvar_bin_size = 1;
     ens_phist_bin_size = 0.05;
     prob_cat_thresh    = [];
  
     field = [
        {
           name  = "APCP";
           level = [ "A03" ];
        }
     ];
  }


nc_var_str
""""""""""

The "nc_var_str" entry specifies a string for each ensemble field and
verification task in Ensemble-Stat. This string is parsed from each
"ens.field" and "obs.field" dictionary entry and is used to customize
the variable names written to the NetCDF output file. The default is an
empty string, meaning that no customization is applied to the output variable
names. When the Ensemble-Stat config file contains two fields with the same
name and level value, this entry is used to make the resulting variable names
unique.
e.g. nc_var_str = "MIN";

.. code-block:: none
		
  nc_var_str = "";

obs_thresh
""""""""""

The "obs_thresh" entry is an array of thresholds for filtering observation
values prior to applying ensemble verification logic. They specify the values
to be included in the verification, not excluded. The default setting of NA,
which always evaluates to true, means that all observations should be used.
Verification output will be computed separately for each threshold specified.
This option may be set separately for each obs.field entry.

.. code-block:: none
		
  obs_thresh = [ NA ];

skip_const
""""""""""

Setting "skip_const" to true tells Ensemble-Stat to exclude pairs where all
the ensemble members and the observation have a constant value. For example,
exclude points with zero precipitation amounts from all output line types.
This option may be set separately for each obs.field entry. When set to
false, constant points are included and the observation rank is chosen at
random.

.. code-block:: none
		
  skip_const = FALSE;

obs_error
"""""""""

Observation error options

Set dist_type to NONE to use the observation error table instead.
May be set separately in each "obs.field" entry.
The obs_error dictionary controls how observation error information should be
handled. Observation error information can either be specified directly in
the configuration file or by parsing information from an external table file.
By default, the *MET_BASE/data/table_files/obs_error_table.txt* file is read
but this may be overridden by setting the $MET_OBS_ERROR_TABLE environment
variable at runtime.

The flag entry toggles the observation error logic on (TRUE) and off (FALSE).
When flag is TRUE, random observation error perturbations are applied to the
ensemble member values. No perturbation is applied to the observation values
but the bias scale and offset values, if specified, are applied.

The dist_type entry may be set to NONE, NORMAL, EXPONENTIAL, CHISQUARED,
GAMMA, UNIFORM, or BETA. The default value of NONE indicates that the
observation error table file should be used rather than the configuration
file settings.

The dist_parm entry is an array of length 1 or 2 specifying the parameters
for the distribution selected in dist_type. The NORMAL, EXPONENTIAL, and
CHISQUARED distributions are defined by a single parameter. The GAMMA,
UNIFORM, and BETA distributions are defined by two parameters. See the
`GNU Scientific Library Reference Manual <https://www.gnu.org/software/gsl/manual>`_
for more information on these distributions.
   

The inst_bias_scale and inst_bias_offset entries specify bias scale and
offset values that should be applied to observation values prior to
perturbing them. These entries enable bias-correction on the fly.

Defining the observation error information in the configuration file is
convenient but limited. If defined this way, the random perturbations for all
points in the current verification task are drawn from the same distribution.
Specifying an observation error table file instead (by setting dist_type =
NONE;) provides much finer control, enabling the user to define observation
error distribution information and bias-correction logic separately for each
observation variable name, message type, PREPBUFR report type, input report
type, instrument type, station ID, range of heights, range of pressure
levels, and range of values.

.. code-block:: none
		
  obs_error = {
     flag             = FALSE;   TRUE or FALSE
     dist_type        = NONE;    Distribution type
     dist_parm        = [];      Distribution parameters
     inst_bias_scale  = 1.0;     Instrument bias scale adjustment
     inst_bias_offset = 0.0;     Instrument bias offset adjustment
     min              = NA;      Valid range of data
     max              = NA;
  }

ensemble_flag
"""""""""""""
  
The "ensemble_flag" entry is a dictionary of boolean value indicating
which ensemble products should be generated:

* "mean" for the simple ensemble mean
  
* "stdev" for the ensemble standard deviation
  
* "minus" for the mean minus one standard deviation
  
* "plus" for the mean plus one standard deviation
  
* "min" for the ensemble minimum
  
* "max" for the ensemble maximum
  
* "range" for the range of ensemble values
  
* "vld_count" for the number of valid ensemble members
  
* "frequency" for the ensemble relative frequency meeting a threshold
  
* "nep" for the neighborhood ensemble probability
  
* "nmep" for the neighborhood maximum ensemble probability
  
* "rank" to write the rank for the gridded observation field to separate
  NetCDF output file.
  
* "weight" to write the grid weights specified in grid_weight_flag to the
  rank NetCDF output file.

.. code-block:: none
		
  ensemble_flag = {
     mean      = TRUE;
     stdev     = TRUE;
     minus     = TRUE;
     plus      = TRUE;
     min       = TRUE;
     max       = TRUE;
     range     = TRUE;
     vld_count = TRUE;
     frequency = TRUE;
     nep       = FALSE;
     nmep      = FALSE;
     rank      = TRUE;
     weight    = FALSE;
  }

rng
"""

See: `Random Number Generator Performance <https://www.gnu.org/software/gsl/doc/html/rng.html#performance>`_
used for random assignment of ranks when they are tied.

.. code-block:: none
		
  rng = {
     type = "mt19937";
     seed = "";
  }

MODEAnalysisConfig_default
^^^^^^^^^^^^^^^^^^^^^^^^^^

MODE line options are used to create filters that determine which MODE output
lines are read in and processed. The MODE line options are numerous. They
fall into seven categories: toggles, multiple set string options, multiple
set integer options, integer max/min options, date/time max/min options,
floating-point max/min options, and miscellaneous options. **In order to be
applied, the options must be uncommented (i.e. remove  the "//" marks) before
running.** These options are described in subsequent sections. Please note
that this configuration file is processed differently than the other config
files.



Toggles: The MODE line options described in this section are shown in pairs.
These toggles represent parameters that can have only one (or none) of two
values. Any of these toggles may be left unspecified. However, if neither
option for toggle is indicated, the analysis will produce results that
combine data from both toggles. This may produce unintended results.



This toggle indicates whether forecast or observed lines should be used for
analysis.

.. code-block:: none
		
  fcst      = FALSE;
  obs       = FALSE;

This toggle indicates whether single object or object pair lines should be
used.

.. code-block:: none
		
  single    = FALSE;
  pair      = FALSE;

This toggle indicates whether simple object or object cluster object lines
should be used.

.. code-block:: none
		
  simple    = FALSE;
  cluster   = FALSE;

This toggle indicates whether matched or unmatched object lines should be
used.

.. code-block:: none
		
  matched   = FALSE;
  unmatched = FALSE;

Multiple-set string options: The following options set various string
attributes. They can be set multiple times on the command line but must be
separated by spaces. Each of these options must be indicated as a string.
String values that include spaces may be used by enclosing the string in
quotation marks.



This options specifies which model to use

.. code-block:: none

  // model    = [];


These two options specify thresholds for forecast and observations objects to
be used in the analysis, respectively.

.. code-block:: none

  // fcst_thr = [];
  // obs_thr  = [];


These options indicate the names of variables to be used in the analysis for
forecast and observed fields.

.. code-block:: none

  // fcst_var = [];
  // obs_var = [];


These options indicate vertical levels for forecast and observed fields to be
used in the analysis.

.. code-block:: none

  // fcst_lev = [];
  // obs_lev = [];


Multiple-set integer options: The following options set various integer
attributes. Each of the following options may only be indicated as an
integer.



These options are integers of the form HH[MMSS] specifying the lead_time.

.. code-block:: none

  // fcst_lead       = [];
  //obs_lead       = [];


These options are integers of the form HH[MMSS] specifying the valid hour.

.. code-block:: none

  // fcst_valid_hour = [];
  // obs_valid_hour = [];


These options are integers of the form HH[MMSS] specifying the model
initialization hour.

.. code-block:: none

  // fcst_init_hour  = [];
  // obs_init_hour  = [];


These options are integers of the form HHMMSS specifying the accumulation
time.

.. code-block:: none

  // fcst_accum      = [];
  // obs_accum      = [];


These options indicate the convolution radius used for forecast of observed
objects, respectively.

.. code-block:: none

  // fcst_rad        = [];
  // obs_rad        = [];


Integer max/min options: These options set limits on various integer
attributes. Leaving a maximum value unset means no upper limit is imposed on
the value of the attribute. The option works similarly for minimum values.



These options are used to indicate minimum/maximum values for the area
attribute to be used in the analysis.

.. code-block:: none

  // area_min              = 0;
  // area_max              = 0;


These options are used to indicate minimum/maximum values accepted for the
area thresh. The area thresh is the area of the raw field inside the object
that meets the threshold criteria.

.. code-block:: none

  // area_thresh_min       = 0;
  // area_thresh_max       = 0;


These options refer to the minimum/maximum values accepted for the
intersection area attribute.

.. code-block:: none

  // intersection_area_min = 0;
  // intersection_area_max = 0;


These options refer to the minimum/maximum union area values accepted for
analysis.

.. code-block:: none

  // union_area_min        = 0;
  // union_area_max        = 0;


These options refer to the minimum/maximum values for symmetric difference
for objects to be used in the analysis.

.. code-block:: none

  // symmetric_diff_min    = 0;
  // symmetric_diff_max    = 0;


Date/time max/min options: These options set limits on various date/time
attributes. The values can be specified in one of three ways:  First, the
options may be indicated by a string of the form YYYMMDD_HHMMSS. This
specifies a complete calendar date and time. Second, they may be indicated
by a string of the form YYYYMMMDD_HH. Here, the minutes and seconds are
assumed to be zero. The third way of indicating date/time attributes is by a
string of the form YYYMMDD. Here, hours, minutes, and seconds are assumed to
be zero.



These options indicate minimum/maximum values for the forecast valid time.

.. code-block:: none

  // fcst_valid_min = "";
  // fcst_valid_max = "";


These options indicate minimum/maximum values for the observation valid time.

.. code-block:: none

  // obs_valid_min  = "";
  // obs_valid_max  = "";


These options indicate minimum/maximum values for the forecast initialization
time.

.. code-block:: none

  // fcst_init_min  = "";
  // fcst_init_max  = "";


These options indicate minimum/maximum values for the observation
initialization time.

.. code-block:: none

  // obs_init_min   = "";
  // obs_init_max   = "";


Floating-point max/min options: Setting limits on various floating-point
attributes. One may specify these as integers (i.e., without a decimal
point), if desired. The following pairs of options indicate minimum and
maximum values for each MODE attribute that can be described as a floating-
point number. Please refer to "The MODE Tool" section on attributes in the
MET User's Guide for a description of these attributes.

.. code-block:: none

  // centroid_x_min                 = 0.0;
  // centroid_x_max                 = 0.0;
 
  // centroid_y_min                 = 0.0;
  // centroid_y_max                 = 0.0;
 
  // centroid_lat_min               = 0.0;
  // centroid_lat_max               = 0.0;
 
  // centroid_lon_min               = 0.0;
  // centroid_lon_max               = 0.0;
 
  // axis_ang_min                   = 0.0;
  // axis_ang_max                   = 0.0;
 
  // length_min                     = 0.0;
  // length_max                     = 0.0;
 
  // width_min                      = 0.0;
  // width_max                      = 0.0;
 
  // aspect_ratio_min               = 0.0;
  // aspect_ratio_max               = 0.0;
 
  // curvature_min                  = 0.0;
  // curvature_max                  = 0.0;
 
  // curvature_x_min                = 0.0;
  // curvature_x_max                = 0.0;
 
  // curvature_y_min                = 0.0;
  // curvature_y_max                = 0.0;
 
  // complexity_min                 = 0.0;
  // complexity_max                 = 0.0;
 
  // intensity_10_min               = 0.0;
  // intensity_10_max               = 0.0;
 
  // intensity_25_min               = 0.0;
  // intensity_25_max               = 0.0;

  // intensity_50_min               = 0.0;
  // intensity_50_max               = 0.0;
 
  // intensity_75_min               = 0.0;
  // intensity_75_max               = 0.0;
 
  // intensity_90_min               = 0.0;
  // intensity_90_max               = 0.0;
 
  // intensity_user_min             = 0.0;
  // intensity_user_max             = 0.0;
 
  // intensity_sum_min              = 0.0;
  // intensity_sum_max              = 0.0;
 
  // centroid_dist_min              = 0.0;
  // centroid_dist_max              = 0.0;
 
  // boundary_dist_min              = 0.0;
  // boundary_dist_max              = 0.0;
 
  // convex_hull_dist_min           = 0.0;
  // convex_hull_dist_max           = 0.0;
 
  // angle_diff_min                 = 0.0;
  // angle_diff_max                 = 0.0;
 
  // area_ratio_min                 = 0.0;
  // area_ratio_max                 = 0.0;
 
  // intersection_over_area_min     = 0.0;
  // intersection_over_area_max     = 0.0;
 
  // complexity_ratio_min           = 0.0;
  // complexity_ratio_max           = 0.0;
 
  // percentile_intensity_ratio_min = 0.0;
  // percentile_intensity_ratio_max = 0.0;
 
  // interest_min                   = 0.0;
  // interest_max                   = 0.0;



MODEConfig_default
^^^^^^^^^^^^^^^^^^

quilt
"""""

The "quilt" entry is a boolean to indicate whether all permutations of
convolution radii and thresholds should be run. If set to false, the number
of forecast and observation convolution radii and thresholds must all match.
One configuration of MODE will be run for each group of settings in those
lists. If set to true, the number of forecast and observation convolution
radii must match and the number of forecast and observation convolution
thresholds must match. For N radii and M thresholds, NxM configurations of
MODE will be run.

.. code-block:: none
		
  quilt = false;

fcst, obs
"""""""""

The object definition settings for MODE are contained within the "fcst" and
"obs" entries:

* The "censor_thresh" and "censor_val" entries are described above.
  The entries replace the previously supported "raw_thresh" entry.

* The "conv_radius" entry specifies the convolution radius in grid
  squares. The larger the convolution radius, the smoother the objects.
  Multiple convolution radii may be specified as an array. For example:

  .. code-block:: none
		  
    conv_radius = [ 5, 10, 15 ];

* The "conv_thresh" entry specifies the convolution threshold used to
  define MODE objects. The lower the threshold, the larger the objects.
  Multiple convolution thresholds may be specified as an array. For example:

  .. code-block:: none
		  
    conv_thresh = [ >=5.0, >=10.0, >=15.0 ];

* The "vld_thresh" entry is described above.

* The "filter_attr_name" and "filter_attr_thresh" entries are arrays of
  the same length which specify object filtering criteria. By default, no
  object filtering criteria is defined.

  The "filter_attr_name" entry is an array of strings specifying the MODE
  output header column names for the object attributes of interest, such
  as "AREA", "LENGTH", "WIDTH", and "INTENSITY_50". In addition,
  "ASPECT_RATIO" specifies the aspect ratio (width/length),
  "INTENSITY_101" specifies the  mean intensity value, and "INTENSITY_102"
  specifies the sum of the intensity values.

  The "filter_attr_thresh" entry is an array of thresholds for the
  object attributes. Any simple objects not meeting all of these
  filtering criteria are discarded.

  Note that the "area_thresh" and "inten_perc_thresh" entries form
  earlier versions of MODE are replaced by these options and are now
  deprecated.

* The "merge_thresh" entry specifies a lower convolution threshold used
  when the double-threshold merging method is applied. The number of
  merge thresholds must match the number of convolution thresholds.
  Multiple merge thresholds may be specified as an array. For example:

  .. code-block:: none
		  
    merge_thresh = [ >=1.0, >=2.0, >=3.0 ];

* The "merge_flag" entry specifies the merging methods to be applied:
  
   * "NONE" for no merging
     
   * "THRESH" for the double-threshold merging method. Merge objects
     that would be part of the same object at the lower threshold.
     
   * "ENGINE" for the fuzzy logic approach comparing the field to itself
     
   * "BOTH" for both the double-threshold and engine merging methods

.. code-block:: none
		
  fcst = {
     field = {
        name  = "APCP";
        level = "A03";
     }
  
     censor_thresh      = [];
     censor_val         = [];
     conv_radius        = 60.0/grid_res; in grid squares
     conv_thresh        = >=5.0;
     vld_thresh         = 0.5;
     filter_attr_name   = [];
     filter_attr_thresh = [];
     merge_thresh       = >=1.25;
     merge_flag         = THRESH;
  }

grid_res
""""""""

The "grid_res" entry is the nominal spacing for each grid square in
kilometers. The variable is not used directly in the code, but subsequent
variables in the configuration files are defined in terms of it. Therefore,
setting the appropriately will help ensure that appropriate default values
are used for these variables.

.. code-block:: none
		
  grid_res = 4;

match_flag
""""""""""

The "match_flag" entry specifies the matching method to be applied:

* "NONE" for no matching between forecast and observation objects
  
* "MERGE_BOTH" for matching allowing additional merging in both fields.
  If two objects in one field match the same object in the other field,
  those two objects are merged.
  
* "MERGE_FCST" for matching allowing only additional forecast merging
  
* "NO_MERGE" for matching with no additional merging in either field

.. code-block:: none
		
  match_flag = MERGE_BOTH;

max_centroid_dist
"""""""""""""""""

The "max_centroid_dist" entry specifies the maximum allowable distance in
grid squares between the centroids of objects for them to be compared.
Setting this to a reasonable value speeds up the runtime enabling MODE to
skip unreasonable object comparisons.

.. code-block:: none
		
  max_centroid_dist = 800.0/grid_res;

weight
""""""
     
The weight variables control how much weight is assigned to each pairwise
attribute when computing a total interest value for object pairs. The weights
need not sum to any particular value but must be non-negative. When the
total interest value is computed, the weighted sum is normalized by the
sum of the weights listed.

.. code-block:: none
		
  weight = {
     centroid_dist    = 2.0;
     boundary_dist    = 4.0;
     convex_hull_dist = 0.0;
     angle_diff       = 1.0;
     area_ratio       = 1.0;
     int_area_ratio   = 2.0;
     complexity_ratio = 0.0;
     inten_perc_ratio = 0.0;
     inten_perc_value = 50;
  }

interest_function
"""""""""""""""""
		
The set of interest function variables listed define which values are of
interest for each pairwise attribute measured. The interest functions may be
defined as a piecewise linear function or as an algebraic expression. A
piecewise linear function is defined by specifying the corner points of its
graph. An algebraic function may be defined in terms of several built-in
mathematical functions.

.. code-block:: none
		
  interest_function = {
  
     centroid_dist = (
        (            0.0, 1.0 )
        (  60.0/grid_res, 1.0 )
        ( 600.0/grid_res, 0.0 )
     );
  
     boundary_dist = (
        (            0.0, 1.0 )
        ( 400.0/grid_res, 0.0 )
     );
  
     convex_hull_dist = (
        (            0.0, 1.0 )
        ( 400.0/grid_res, 0.0 )
     );
  
     angle_diff = (
        (  0.0, 1.0 )
        ( 30.0, 1.0 )
        ( 90.0, 0.0 )
     );

     corner   = 0.8;
     ratio_if = (
        (    0.0, 0.0 )
        ( corner, 1.0 )
        (    1.0, 1.0 )
     );
  
     area_ratio = ratio_if;
  
     int_area_ratio = (
        ( 0.00, 0.00 )
        ( 0.10, 0.50 )
        ( 0.25, 1.00 )
        ( 1.00, 1.00 )
     );
  
     complexity_ratio = ratio_if;
  
     inten_perc_ratio = ratio_if;
  }

total_interest_thresh
"""""""""""""""""""""
  
The total_interest_thresh variable should be set between 0 and 1. This
threshold is applied to the total interest values computed for each pair of
objects and is used in determining matches.

.. code-block:: none
		
  total_interest_thresh = 0.7;

print_interest_thresh
"""""""""""""""""""""

The print_interest_thresh variable determines which pairs of object
attributes will be written to the output object attribute ASCII file. The
user may choose to set the print_interest_thresh to the same value as the
total_interest_thresh, meaning that only object pairs that actually match are
written to the output file. When set to zero, all object pair attributes will
be written as long as the distance between the object centroids is less than
the max_centroid_dist variable.

.. code-block:: none
		
  print_interest_thresh = 0.0;

plot_valid_flag
"""""""""""""""

When applied, the plot_valid_flag variable indicates that only the region
containing valid data after masking is applied should be plotted. TRUE
indicates the entire domain should be plotted; FALSE indicates only the
region containing valid data after masking should be plotted.

.. code-block:: none
		
  plot_valid_flag = FALSE;

plot_gcarc_flag
"""""""""""""""

When applied, the plot_gcarc_flag variable indicates that the edges of
polylines should be plotted using great circle arcs as opposed to straight
lines in the grid.

.. code-block:: none
		
  plot_gcarc_flag = FALSE;

ct_stats_flag
"""""""""""""
  
The ct_stats_flag can be set to TRUE or FALSE to produce additional output,
in the form of contingency table counts and statistics.

.. code-block:: none
		
  ct_stats_flag = TRUE;

shift_right
"""""""""""

When MODE is run on global grids, this parameter specifies how many grid
squares to shift the grid to the right. MODE does not currently connect
objects from one side of a global grid to the other, potentially causing
objects straddling that longitude to be cut in half. Shifting the grid by
some amount enables the user to control where that longitude cut line occurs.
This option provides a very specialized case of automated regridding. The
much more flexible "regrid" option may be used instead.

.. code-block:: none
		
  shift_right = 0;

PB2NCConfig_default
^^^^^^^^^^^^^^^^^^^

The PB2NC tool filters out observations from PREPBUFR or BUFR files using the
following criteria:

(1) by message type: supply a list of PREPBUFR message types to retain
 
(2) by station id: supply a list of observation stations to retain
 
(3) by valid time: supply the beginning and ending time offset values
    in the obs_window entry described above.

(4) by location: use the "mask" entry described below to supply either
    an NCEP masking grid, a masking lat/lon polygon or a file to a
    mask lat/lon polygon
 
(5) by elevation: supply min/max elevation values

(6) by report type: supply a list of report types to retain using
    pb_report_type and in_report_type entries described below

(7) by instrument type: supply a list of instrument type to
    retain
 
(8) by vertical level: supply beg/end vertical levels using the
    level_range entry described below
 
(9) by variable type: supply a list of observation variable types to
    retain using the obs_bufr_var entry described below
 
(10) by quality mark: supply a quality mark threshold
 
(11) Flag to retain values for all quality marks, or just the first
     quality mark (highest): use the event_stack_flag described below

(12) by data level category: supply a list of category types to
     retain.

     0 - Surface level (mass reports only)
     
     1 - Mandatory level (upper-air profile reports)
     
     2 - Significant temperature level (upper-air profile reports)
     
     2 - Significant temperature and winds-by-pressure level
     (future combined mass and wind upper-air reports)
	 
     3 - Winds-by-pressure level (upper-air profile reports)
     
     4 - Winds-by-height level (upper-air profile reports)
     
     5 - Tropopause level (upper-air profile reports)
     
     6 - Reports on a single level
     (e.g., aircraft, satellite-wind, surface wind,
     precipitable water retrievals, etc.)
	  
     7 - Auxiliary levels generated via interpolation from spanning levels
     (upper-air profile reports)

message_type
""""""""""""

In the PB2NC tool, the "message_type" entry is an array of message types
to be retained. An empty list indicates that all should be retained.

| List of valid message types:
| ADPUPA AIRCAR AIRCFT ADPSFC ERS1DA GOESND GPSIPW
| MSONET PROFLR QKSWND RASSDA SATEMP SATWND SFCBOG
| SFCSHP SPSSMI SYNDAT VADWND

For example:

| message_type[] = [ "ADPUPA", "AIRCAR" ];
| 

`Current Table A Entries in PREPBUFR mnemonic table <http://www.emc.ncep.noaa.gov/mmb/data_processing/prepbufr.doc/table_1.htm>`_

.. code-block:: none
		
  message_type = [];

station_id
""""""""""

The "station_id" entry is an array of station ids to be retained or
the filename which contains station ids. An array of station ids
contains a comma-separated list. An empty list indicates that all
stations should be retained.

For example:  station_id = [ "KDEN" ];

.. code-block:: none
		
  station_id = [];

elevation_range
"""""""""""""""

The "elevation_range" entry is a dictionary which contains "beg" and "end"
entries specifying the range of observing locations elevations to be
retained.

.. code-block:: none
		
  elevation_range = {
     beg =  -1000;
     end = 100000;
  }

pb_report_type
""""""""""""""

The "pb_report_type" entry is an array of PREPBUFR report types to be
retained. The numeric "pb_report_type" entry allows for further
stratification within message types. An empty list indicates that all should
be retained.

See: `Code table for PREPBUFR report types used by Regional NAM GSI analyses <http://www.emc.ncep.noaa.gov/mmb/data_processing/prepbufr.doc/table_4.htm>`_

For example:

| Report Type 120 is for message type ADPUPA but is only RAWINSONDE
| Report Type 132 is for message type ADPUPA but is only FLIGHT-LEVEL RECON
| and PROFILE DROPSONDE
|

.. code-block:: none
		
  pb_report_type  = [];

in_report_type
""""""""""""""

The "in_report_type" entry is an array of input report type values to be
retained. The numeric "in_report_type" entry provides additional
stratification of observations. An empty list indicates that all should
be retained.

See: `Code table for input report types <http://www.emc.ncep.noaa.gov/mmb/data_processing/prepbufr.doc/table_6.htm>`_

For example:

| Input Report Type 11 Fixed land RAOB and PIBAL by block and station number
| Input Report Type 12 Fixed land RAOB and PIBAL by call letters
|

.. code-block:: none
		
  in_report_type = [];

instrument_type
"""""""""""""""

The "instrument_type" entry is an array of instrument types to be retained.
An empty list indicates that all should be retained.

.. code-block:: none
		
  instrument_type = [];

level_range
"""""""""""

The "level_range" entry is a dictionary which contains "beg" and "end"
entries specifying the range of vertical levels (1 to 255) to be retained.

.. code-block:: none
		
  level_range = {
     beg = 1;
     end = 255;
  }

level_category
""""""""""""""

The "level_category" entry is an array of integers specifying which level
categories should be retained:

| 0 = Surface level (mass reports only)

| 1 = Mandatory level (upper-air profile reports)

| 2 = Significant temperature level (upper-air profile reports) 

| 2 = Significant temperature and winds-by-pressure level (future combined mass
|     and wind upper-air reports) 

| 3 = Winds-by-pressure level (upper-air profile reports) 

| 4 = Winds-by-height level (upper-air profile reports)

| 5 = Tropopause level (upper-air profile reports)

| 6 = Reports on a single level  (For example: aircraft, satellite-wind,
|     surface wind, precipitable water retrievals, etc.)

| 7 = Auxiliary levels generated via interpolation from spanning levels
|     (upper-air profile reports)

|    

An empty list indicates that all should be retained.

See: `Current Table A Entries in PREPBUFR mnemonic table <http://www.emc.ncep.noaa.gov/mmb/data_processing/prepbufr.doc/table_1.htm>`_

.. code-block:: none
		
  level_category = [];

obs_bufr_var
""""""""""""

The "obs_bufr_var" entry is an array of strings containing BUFR variable
names to be retained or derived. This replaces the "obs_grib_code" setting
from earlier versions of MET. Run PB2NC on your data with the "-index"
command line option to see the list of available observation variables.

| Observation variables that can be derived begin with "D\_":
|    D_DPT   for Dew point Temperature in K
|    D_WDIR  for Wind Direction
|    D_WIND  for Wind Speed in m/s
|    D_RH    for Relative Humidity in %
|    D_MIXR  for Humidity Mixing Ratio in kg/kg
|    D_PRMSL for Pressure Reduced to Mean Sea Level in Pa
|

.. code-block:: none
		
  obs_bufr_var = [ "QOB", "TOB", "ZOB", "UOB", "VOB" ];

obs_bufr_map
""""""""""""

Mapping of input BUFR variable names to output variables names.
The default PREPBUFR map, obs_prepbufr_map, is appended to this map.
Users may choose to rename BUFR variables to match the naming convention
of the forecast the observation is used to verify.

.. code-block:: none
		
  obs_bufr_map = [];

obs_prepbufr_map
""""""""""""""""

Default mapping for PREPBUFR. Replace input BUFR variable names with GRIB
abbreviations in the output. This default map is appended to obs_bufr_map.
This should not typically be overridden. This default mapping provides
backward-compatibility for earlier versions of MET which wrote GRIB
abbreviations to the output.

.. code-block:: none
		
  obs_prepbufr_map = [
     { key = "POB";     val = "PRES";  },
     { key = "QOB";     val = "SPFH";  },
     { key = "TOB";     val = "TMP";   },
     { key = "ZOB";     val = "HGT";   },
     { key = "UOB";     val = "UGRD";  },
     { key = "VOB";     val = "VGRD";  },
     { key = "D_DPT";   val = "DPT";   },
     { key = "D_WDIR";  val = "WDIR";  },
     { key = "D_WIND";  val = "WIND";  },
     { key = "D_RH";    val = "RH";    },
     { key = "D_MIXR";  val = "MIXR";  },
     { key = "D_PRMSL"; val = "PRMSL"; }
  ];

quality_mark_thresh
"""""""""""""""""""
     
The "quality_mark_thresh" entry specifies the maximum quality mark value
to be retained. Observations with a quality mark LESS THAN OR EQUAL TO
this threshold will be retained, while observations with a quality mark
GREATER THAN this threshold will be discarded.

See `Code table for observation quality markers <http://www.emc.ncep.noaa.gov/mmb/data_processing/prepbufr.doc/table_7.htm>`_

.. code-block:: none
		
  quality_mark_thresh = 2;

event_stack_flag
""""""""""""""""

The "event_stack_flag" entry is set to "TOP" or "BOTTOM" to
specify whether observations should be drawn from the top of the event
stack (most quality controlled) or the bottom of the event stack (most raw).

.. code-block:: none
		
  event_stack_flag = TOP;

SeriesAnalysisConfig_default
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

block_size
""""""""""

Computation may be memory intensive, especially for large grids.
The "block_size" entry sets the number of grid points to be processed
concurrently (i.e. in one pass through a time series). Smaller values
require less memory but increase the number of passes through the data.
If set less than or equal to 0, it is automatically reset to the number
of grid points, and they are all processed concurrently.

.. code-block:: none
		
  block_size = 1024;

vld_thresh
""""""""""

Ratio of valid matched pairs to total length of series for a grid
point. If valid threshold is exceeded at that grid point the statistics
are computed and stored. If not, a bad data flag is stored. The default
setting requires all data in the series to be valid.


.. code-block:: none
		
  vld_thresh = 1.0;

output_stats
""""""""""""

Statistical output types need to be specified explicitly. Refer to User's
Guide for available output types. To keep output file size reasonable,
it is recommended to process a few output types at a time, especially if the
grid is large.

.. code-block:: none
		
  output_stats = {
     fho    = [];
     ctc    = [];
     cts    = [];
     mctc   = [];
     mcts   = [];
     cnt    = [ "RMSE", "FBAR", "OBAR" ];
     sl1l2  = [];
     pct    = [];
     pstd   = [];
     pjc    = [];
     prc    = [];
  }

STATAnalysisConfig_default
^^^^^^^^^^^^^^^^^^^^^^^^^^

jobs
""""

The "jobs" entry is an array of STAT-Analysis jobs to be performed.
Each element in the array contains the specifications for a single analysis
job to be performed. The format for an analysis job is as follows:

| -job job_name
| OPTIONAL ARGS
| 

Where "job_name" is set to one of the following:

* "filter"
  
  To filter out the STAT or TCMPR lines matching the job filtering
  criteria specified below and using the optional arguments below.
  The output STAT lines are written to the file specified using the
  "-dump_row" argument.
  Required Args: -dump_row

|    

* "summary"
  
  To compute summary information for a set of statistics.
  The summary output includes the mean, standard deviation,
  percentiles (0th, 10th, 25th, 50th, 75th, 90th, and 100th), range,
  and inter-quartile range. Also included are columns summarizing the
  computation of WMO mean values. Both unweighted and weighted mean
  values are reported, and they are computed using three types of
  logic:

  * simple arithmetic mean (default)
	   
  * square root of the mean of the statistic squared
    (applied to columns listed in "wmo_sqrt_stats")
	   
  *  apply fisher transform
     (applied to columns listed in "wmo_fisher_stats")

|

  The columns of data to be summarized are specified in one of two
  ways:
	 
  * Specify the -line_type option once and specify one or more column names.
	   
  * Format the -column option as LINE_TYPE:COLUMN.

|     
    
  Use the -derive job command option to automatically derive
  statistics on the fly from input contingency tables and partial
  sums.

  Use the -column_union TRUE/FALSE job command option to compute
  summary statistics across the union of input columns rather than
  processing them separately.

  For TCStat, the "-column" argument may be set to:
  
    * "TRACK" for track, along-track, and cross-track errors.
    * "WIND" for all wind radius errors.
    * "TI" for track and maximum wind intensity errors.
    * "AC" for along-track and cross-track errors.
    * "XY" for x-track and y-track errors.
    * "col" for a specific column name.
    * "col1-col2" for a difference of two columns.
    * "ABS(col or col1-col2)" for the absolute value.

  Required Args: -line_type, -column

  Optional Args:

  .. code-block:: none
		  
    -by column_name to specify case information
    -out_alpha to override default alpha value of 0.05
    -derive to derive statistics on the fly
    -column_union to summarize multiple columns

* "aggregate"
  
  To aggregate the STAT data for the STAT line type specified using
  the "-line_type" argument. The output of the job will be in the
  same format as the input line type specified. The following line
  types may be aggregated:

  .. code-block:: none
		  
    -line_type FHO, CTC, MCTC,
               SL1L2, SAL1L2, VL1L2, VAL1L2,
               PCT, NBRCNT, NBRCTC, GRAD,
               ISC, ECNT, RPS, RHIST, PHIST, RELP, SSVAR
	       
  Required Args: -line_type
  
| 

* "aggregate_stat"
  
  To aggregate the STAT data for the STAT line type specified using
  the "-line_type" argument. The output of the job will be the line
  type specified using the "-out_line_type" argument. The valid
  combinations of "-line_type" and "-out_line_type" are listed below.

  .. code-block:: none
		  
    -line_type FHO, CTC,      -out_line_type CTS, ECLV
    -line_type MCTC           -out_line_type MCTS
    -line_type SL1L2, SAL1L2, -out_line_type CNT
    -line_type VL1L2          -out_line_type VCNT
    -line_type VL1L2, VAL1L2, -out_line_type WDIR (wind direction)
    -line_type PCT,           -out_line_type PSTD, PJC, PRC, ECLV
    -line_type NBRCTC,        -out_line_type NBRCTS
    -line_type ORANK,         -out_line_type ECNT, RPS, RHIST, PHIST,
                                             RELP, SSVAR
    -line_type MPR,           -out_line_type FHO, CTC, CTS,
                                             MCTC, MCTS, CNT,
                                             SL1L2, SAL1L2,
                                             VL1L2, VCNT,
                                             PCT, PSTD, PJC, PRC, ECLV,
                                             WDIR (wind direction)

  Required Args: -line_type, -out_line_type

  Additional Required Args for -line_type MPR:

  .. code-block:: none
		  
    -out_thresh or -out_fcst_thresh and -out_obs_thresh
     When -out_line_type FHO, CTC, CTS, MCTC, MCTS,
                         PCT, PSTD, PJC, PRC

  Additional Optional Args for -line_type MPR:

  .. code-block:: none
		  
    -mask_grid, -mask_poly, -mask_sid
    -out_thresh or -out_fcst_thresh and -out_obs_thresh
    -out_cnt_logic
    -out_wind_thresh or -out_fcst_wind_thresh and
    -out_obs_wind_thresh
    -out_wind_logic
    When -out_line_type WDIR
	    
  Additional Optional Arg for:

  .. code-block:: none
		  
    -line_type ORANK -out_line_type PHIST, SSVAR ...
    -out_bin_size
	    
  Additional Optional Args for:

  .. code-block:: none
		  
    -out_line_type ECLV ...
    -out_eclv_points

* "ss_index"
  
  The skill score index job can be configured to compute a weighted
  average of skill scores derived from a configurable set of
  variables, levels, lead times, and statistics. The skill score
  index is computed using two models, a forecast model and a
  reference model. For each statistic in the index, a skill score
  is computed as:
  
  SS = 1 - (S[model]*S[model])/(S[reference]*S[reference])

  Where S is the statistic.

  Next, a weighted average is computed over all the skill scores.

  Lastly, an index value is computed as:

  Index = sqrt(1/(1-SS[avg]))

  Where SS[avg] is the weighted average of skill scores.

  Required Args:

  .. code-block:: none

    Exactly 2 entries for -model, the forecast model and reference
    For each term of the index:
    -fcst_var, -fcst_lev, -fcst_lead, -line_type, -column, -weight
    Where -line_type is CNT or CTS and -column is the statistic.
    Optionally, specify other filters for each term, -fcst_thresh.

* "go_index"
  
  The GO Index is a special case of the skill score index consisting
  of a predefined set of variables, levels, lead times, statistics,
  and weights.
  
  For lead times of 12, 24, 36, and 48 hours, it contains RMSE for:

  .. code-block:: none
		  
    - Wind Speed at the surface(b), 850(a), 400(a), 250(a) mb
    - Dew point Temperature at the surface(b), 850(b), 700(b), 400(b) mB
    - Temperature at the surface(b), 400(a) mB
    - Height at 400(a) mB
    - Sea Level Pressure(b)
    Where (a) means weights of 4, 3, 2, 1 for the lead times, and
          (b) means weights of 8, 6, 4, 2 for the lead times.

  Required Args: None

|

* "ramp"
  
  The ramp job operates on a time-series of forecast and observed
  values and is analogous to the RIRW (Rapid Intensification and
  Weakening) job supported by the tc_stat tool. The amount of change
  from one time to the next is computed for forecast and observed
  values. Those changes are thresholded to define events which are
  used to populate a 2x2 contingency table.

  Required Args:

  .. code-block:: none
		  
    -ramp_thresh (-ramp_thresh_fcst or -ramp_thresh_obs)
       For DYDT, threshold for the amount of change required to
       define an event.
       For SWING, threshold the slope.
    -swing_width val
       Required for the swinging door algorithm width.

  Optional Args:

  .. code-block:: none
		  
    -ramp_type str
       Overrides the default ramp definition algorithm to be used.
       May be set to DYDT (default) or SWING for the swinging door
       algorithm.
    -line_type str
       Overrides the default input line type, MPR.
    -out_line_type str
       Overrides the default output line types of CTC and CTS.
       Set to CTC,CTS,MPR for all possible output types.
    -column fcst_column,obs_column
       Overrides the default forecast and observation columns
       to be used, FCST and OBS.
    -ramp_time HH[MMSS] (-ramp_time_fcst or -ramp_time_obs)
       Overrides the default ramp time interval, 1 hour.
    -ramp_exact true/false (-ramp_exact_fcst or -ramp_exact_obs)
       Defines ramps using an exact change (true, default) or maximum
       change in the time window (false).
    -ramp_window width in HH[MMSS] format
    -ramp_window beg end in HH[MMSS] format
       Defines a search time window when attempting to convert misses
       to hits and false alarms to correct negatives. Use 1 argument
       to define a symmetric time window or 2 for an asymmetric
       window. Default window is 0 0, requiring an exact match.

  Job command FILTERING options to further refine the STAT data:

  Each optional argument may be used in the job specification multiple
  times unless otherwise indicated. When multiple optional arguments of
  the same type are indicated, the analysis will be performed over their
  union:

  .. code-block:: none
		  
    "-model            name"
    "-fcst_lead        HHMMSS"
    "-obs_lead         HHMMSS"
    "-fcst_valid_beg   YYYYMMDD[_HH[MMSS]]" (use once)
    "-fcst_valid_end   YYYYMMDD[_HH[MMSS]]" (use once)
    "-obs_valid_beg    YYYYMMDD[_HH[MMSS]]" (use once)
    "-obs_valid_end    YYYYMMDD[_HH[MMSS]]" (use once)
    "-fcst_init_beg    YYYYMMDD[_HH[MMSS]]" (use once)
    "-fcst_init_end    YYYYMMDD[_HH[MMSS]]" (use once)
    "-obs_init_beg     YYYYMMDD[_HH[MMSS]]" (use once)
    "-obs_init_end     YYYYMMDD[_HH[MMSS]]" (use once)
    "-fcst_init_hour   HH[MMSS]"
    "-obs_init_hour    HH[MMSS]"
    "-fcst_valid_hour" HH[MMSS]
    "-obs_valid_hour"  HH[MMSS]
    "-fcst_var         name"
    "-obs_var          name"
    "-fcst_lev         name"
    "-obs_lev          name"
    "-obtype           name"
    "-vx_mask          name"
    "-interp_mthd      name"
    "-interp_pnts      n"
    "-fcst_thresh      t"
    "-obs_thresh       t"
    "-cov_thresh       t"
    "-thresh_logic     UNION, or, ||
                       INTERSECTION, and, &&
                       SYMDIFF, symdiff, *
    "-alpha            a"
    "-line_type        type"
    "-column           name"
    "-weight           value"


  Job command FILTERING options that may be used only when -line_type
  has been listed once. These options take two arguments: the name of the
  data column to be used and the min, max, or exact value for that column.
  If multiple column eq/min/max/str/exc options are listed, the job will be
  performed on their intersection:

  .. code-block:: none
		  
    "-column_min     col_name value"     e.g. -column_min BASER 0.02
    "-column_max     col_name value"
    "-column_eq      col_name value"
    "-column_thresh  col_name threshold" e.g. -column_thresh FCST '>273'
    "-column_str     col_name string" separate multiple filtering strings
                                      with commas
    "-column_str_exc col_name string" separate multiple filtering strings
                                      with commas


  Job command options to DEFINE the analysis job. Unless otherwise noted,
  these options may only be used ONCE per analysis job:

  .. code-block:: none

    "-dump_row        path"

  .. code-block:: none

    "-mask_grid       name"
    "-mask_poly       file"
    "-mask_sid        file|list" see description of "sid" entry above

  .. code-block:: none
		  
    "-out_line_type   name"
    "-out_thresh      value" sets both -out_fcst_thresh and -out_obs_thresh
    "-out_fcst_thresh value" multiple for multi-category contingency tables
                             and probabilistic forecasts
    "-out_obs_thresh  value" multiple for multi-category contingency tables
    "-out_cnt_logic   value"

  .. code-block:: none

    "-out_wind_thresh      value"
    "-out_fcst_wind_thresh value"
    "-out_obs_wind_thresh  value"
    "-out_wind_logic       value"

  .. code-block:: none

    "-out_bin_size    value"

  .. code-block:: none

    "-out_eclv_points value" see description of "eclv_points" config file
                             entry

  .. code-block:: none
		  
    "-out_alpha       value"

  .. code-block:: none

    "-boot_interval   value"
    "-boot_rep_prop   value"
    "-n_boot_rep      value"
    "-boot_rng        value"
    "-boot_seed       value"

  .. code-block:: none

    "-hss_ec_value    value"
    "-rank_corr_flag  value"
    "-vif_flag        value"

  For aggregate and aggregate_stat job types:

  .. code-block:: none
		    
    "-out_stat        path"   to write a .stat output file for the job
                              including the .stat header columns. Multiple
                              values for each header column are written as
                              a comma-separated list.
    "-set_hdr col_name value" may be used multiple times to explicity
                              specify what should be written to the header
                              columns of the output .stat file.

  When using the "-by" job command option, you may reference those columns
  in the "-set_hdr" job command options. For example, when computing statistics
  separately for each station, write the station ID string to the VX_MASK column
  of the output .stat output file:

  .. code-block:: none
		  
    -job aggregate_stat -line_type MPR -out_line_type CNT \
    -by OBS_SID -set_hdr VX_MASK OBS_SID -stat_out out.stat
    When using mulitple "-by" options, use "CASE" to reference the full string:
    -by FCST_VAR,OBS_SID -set_hdr DESC CASE -stat_out out.stat


.. code-block:: none
		
  jobs = [
     "-job filter         -line_type SL1L2 -vx_mask DTC165 \
      -dump_row  job_filter_SL1L2.stat",
     "-job summary        -line_type CNT   -alpha 0.050 -fcst_var TMP \
      -dump_row job_summary_ME.stat -column ME",
     "-job aggregate      -line_type SL1L2 -vx_mask DTC165 -vx_mask DTC166 \
      -fcst_var TMP -dump_row job_aggregate_SL1L2_dump.stat  \
      -out_stat job_aggregate_SL1L2_out.stat \
      -set_hdr VX_MASK CONUS",
     "-job aggregate_stat -line_type SL1L2 -out_line_type CNT -vx_mask DTC165  \
      -vx_mask DTC166 -fcst_var TMP \
      -dump_row  job_aggregate_stat_SL1L2_CNT_in.stat",
     "-job aggregate_stat -line_type MPR   -out_line_type CNT -vx_mask DTC165 \
      -vx_mask DTC166 -fcat_var TMP -dump_row job_aggregate_stat_MPR_CNT_in.stat",
     "-job aggregate      -line_type CTC   -fcst_thresh <300.000 -vx_mask DTC165 \
      -vx_mask DTC166 -fcst_var TMP -dump_row job_aggregate_CTC_in.stat",
     "-job aggregate_stat -line_type CTC   -out_line_type CTS \
      -fcst_thresh <300.000 -vx_mask DTC165 -vx_mask DTC166 -fcst_var TMP \
      -dump_row job_aggregate_stat_CTC_CTS_in.stat",
     "-job aggregate      -line_type MCTC  -column_eq N_CAT 4 -vx_mask DTC165 \
      -vx_mask DTC166 -fcst_var APCP_24 -dump_row job_aggregate_MCTC_in.stat",
     "-job aggregate_stat -line_type MCTC  -out_line_type MCTS \
      -column_eq N_CAT 4 -vx_mask DTC165 -vx_mask DTC166 -fcst_var APCP_24 \
      -dump_row job_aggregate_stat_MCTC_MCTS_in.stat",
     "-job aggregate      -line_type PCT   -vx_mask DTC165 -vx_mask DTC166 \
      -dump_row job_aggregate_PCT_in.stat",
     "-job aggregate_stat -line_type PCT   -out_line_type PSTD -vx_mask DTC165 \
      -vx_mask DTC166 -dump_row job_aggregate_stat_PCT_PSTD_in.stat",
     "-job aggregate      -line_type ISC   -fcst_thresh >0.000 -vx_mask TILE_TOT \
      -fcst_var APCP_12 -dump_row job_aggregate_ISC_in.stat",
     "-job aggregate      -line_type RHIST -obtype MC_PCP -vx_mask HUC4_1605 \
      -vx_mask HUC4_1803 -dump_row job_aggregate_RHIST_in.stat",
     "-job aggregate      -line_type SSVAR -obtype MC_PCP -vx_mask HUC4_1605 \
      -vx_mask HUC4_1803 -dump_row job_aggregate_SSVAR_in.stat",
     "-job aggregate_stat -line_type ORANK -out_line_type RHIST -obtype ADPSFC \
      -vx_mask HUC4_1605 -vx_mask HUC4_1803 \
      -dump_row job_aggregate_stat_ORANK_RHIST_in.stat"
  ];

List of statistics by the logic that should be applied when computing their
WMO mean value in the summary job. Each entry is a line type followed by the
statistic name. Statistics using the default arithemtic mean method do not
need to be listed.

.. code-block:: none
		
  wmo_sqrt_stats   = [];
  wmo_fisher_stats = [];

The "vif_flag" entry is a boolean to indicate whether a variance inflation
factor should be computed when aggregating a time series of contingency
table counts or partial sums. The VIF is used to adjust the normal
confidence intervals computed for the aggregated statistics.

.. code-block:: none
		
  vif_flag = FALSE;

WaveletStatConfig_default
^^^^^^^^^^^^^^^^^^^^^^^^^

grid_decomp_flag
""""""""""""""""

The "grid_decomp_flag" entry specifies how the grid should be decomposed in
Wavelet-Stat into dyadic (2^n x 2^n) tiles:

* "AUTO" to tile the input data using tiles of dimension n by n where n
  is the largest integer power of 2 less than the smallest dimension of
  the input data. Center as many tiles as possible with no overlap.
* "TILE" to use the tile definition specified below.
* "PAD" to pad the input data out to the nearest integer power of 2.

.. code-block:: none
		
  grid_decomp_flag = AUTO;

tile
""""

The "tile" entry is a dictionary that specifies how tiles should be defined
in Wavelet-Stat when the "grid_decomp_flag" is set to "TILE":

* The "width" entry specifies the dimension for all tiles and must be
  an integer power of 2.

* The "location" entry is an array of dictionaries where each element
  consists of an "x_ll" and "y_ll" entry specifying the lower-left (x,y)
  coordinates of the tile.

.. code-block:: none
		
  tile = {
     width = 0;
     location = [
        {
           x_ll = 0;
           y_ll = 0;
        }
     ];
  }

wavelet
"""""""

The "wavelet" entry is a dictionary in Wavelet-Stat that specifies how the
wavelet decomposition should be performed:

* The "type" entry specifies which wavelet should be used.

* The "member" entry specifies the wavelet shape.
  See: `Discrete Wavelet Transforms (DWT) initialization <https://www.gnu.org/software/gsl/doc/html/dwt.html#initialization>`_

* Valid combinations of the two are listed below:
    
  * "HAAR" for Haar wavelet (member = 2)
      
  * "HAAR_CNTR" for Centered-Haar wavelet (member = 2)

  * "DAUB" for Daubechies wavelet (member = 4, 6, 8, 10, 12, 14, 16,
    18, 20)
	
  * "DAUB_CNTR" for Centered-Daubechies wavelet (member = 4, 6, 8, 10,
    12, 14, 16, 18, 20)
	 
  * "BSPLINE" for Bspline wavelet (member = 103, 105, 202, 204, 206,
    208, 301, 303, 305, 307, 309)

  * "BSPLINE_CNTR" for Centered-Bspline wavelet (member = 103, 105, 202,
    204, 206, 208, 301, 303, 305, 307, 309)

.. code-block:: none
		
  wavelet = {
     type   = HAAR;
     member = 2;
  }

obs_raw_wvlt_object_plots
"""""""""""""""""""""""""

The "obs_raw_plot", "wvlt_plot", and "object_plot" entries are dictionaries
similar to the "fcst_raw_plot" described in the "Settings common to multiple
tools" section.

WWMCARegridConfig_default
^^^^^^^^^^^^^^^^^^^^^^^^^

to_grid
"""""""

Please see the description of the "to_grid" entry in the "regrid" dictionary above.

NetCDF output information
"""""""""""""""""""""""""

Supply the NetCDF output information.  For example:

.. code-block:: none

   variable_name = "Cloud_Pct";
   units         = "percent";
   long_name     = "cloud cover percent";
   level         = "SFC";

.. code-block:: none
		
  variable_name = "";
  units         = "";
  long_name     = "";
  level         = "";

max_minutes (pixel age)
"""""""""""""""""""""""

Maximum pixel age in minutes

.. code-block:: none

  max_minutes = 120;

swap_endian
"""""""""""

The WWMCA pixel age data is stored in binary data files in 4-byte blocks.
The swap_endian option indicates whether the endian-ness of the data should
be swapped after reading.

.. code-block:: none
		
  swap_endian = TRUE;

write_pixel_age
"""""""""""""""

By default, wwmca_regrid writes the cloud percent data specified on the
command line to the output file. This option writes the pixel age data,
in minutes, to the output file instead.

.. code-block:: none
		
  write_pixel_age = FALSE;
