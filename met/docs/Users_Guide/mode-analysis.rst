.. _mode-analysis:

MODE-Analysis Tool
==================

Introduction
____________

Users may wish to summarize multiple ASCII files produced by MODE across many cases. The MODE output files contain many output columns making it very difficult to interpret the results by simply browsing the files. Furthermore, for particular applications some data fields in the MODE output files may not be of interest. The MODE-Analysis tool provide a simple way to compute basic summary statistics and filtering capabilities for these files. Users who are not proficient at writing scripts can use the tool directly, and even those using their own scripts can use this tool as a filter, to extract only the MODE output lines that are relevant for their application.

.. _MODE_A-Scientific-and-statistical:

Scientific and statistical aspects
__________________________________

The MODE-Analysis tool operates in two modes, called “summary” and “bycase”. In summary mode, the user specifies on the command line the MODE output columns of interest as well as filtering criteria that determine which input lines should be used. For example, a user may be interested in forecast object areas, but only if the object was matched, and only if the object centroid is inside a particular region. The summary statistics generated for each specified column of data are the minimum, maximum, mean, standard deviation, and the 10th, 25th, 50th, 75th and 90th percentiles. In addition, the user may specify a “dump'” file: the individual MODE lines used to produce the statistics will be written to this file. This option provides the user with a filtering capability. The dump file will consist only of lines that match the specified criteria.

The other option for operating the analysis tool is “bycase”. Given initial and final values for forecast lead time, the tool will output, for each valid time in the interval, the matched area, unmatched area, and the number of forecast and observed objects that were matched or unmatched. For the areas, the user can specify forecast or observed objects, and also simple or cluster objects. A dump file may also be specified in this mode.

Practical information
_____________________

The MODE-Analysis tool reads lines from MODE ASCII output files and applies filtering and computes basic statistics on the object attribute values. For each job type, filter parameters can be set to determine which MODE output lines are used. The following sections describe the **mode_analysis** usage statement, required arguments, and optional arguments.

.. _mode_analysis-usage:

mode_analysis usage
~~~~~~~~~~~~~~~~~~~

The usage statement for the MODE-Analysis tool is shown below:

.. code-block:: none

  Usage: mode_analysis
         -lookin path
         -summary | -bycase
         [-column name]
         [-dump_row filename]
         [-out filename]
         [-log file]
         [-v level]
         [-help]
         [MODE FILE LIST]
         [-config config_file] | [MODE LINE OPTIONS]

The MODE-Analysis tool has two required arguments and can accept several optional arguments.

Required arguments for mode_analysis:
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. The **-lookin path** specifies the name of a specific STAT file (any file ending in .stat) or the name of a directory where the Stat-Analysis tool will search for STAT files. This option may be used multiple times to specify multiple locations.

2. The MODE-Analysis tool can perform two basic types of jobs -summary or -bycase. Exactly one of these job types must be specified. 

Specifying **-summary** will produce summary statistics for the MODE output column specified. For this job type, a column name (or column number) must be specified using the **-column** option. Column names are not case sensitive. The column names are the same as described in :numref:`MODE-output`. More information about this option is provided in subsequent sections.

Specifying **-bycase** will produce a table of metrics for each case undergoing analysis. Any columns specified are ignored for this option.

Optional arguments for mode_analysis
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

3. The **mode_analysis** options are described in the following section. These are divided into sub-sections describing the analysis options and mode line options.

Analysis options
^^^^^^^^^^^^^^^^

The general analysis options described below provide a way for the user to indicate configuration files to be used, where to write lines used to perform the analysis, and over which fields to generate statistics.

____________________

.. code-block:: none

  -config filename

This option gives the name of a configuration file to be read. The contents of the configuration file are described in :numref:`mode_analysis-configuration-file`.

____________________

.. code-block:: none

  -dump_row filename

Any MODE lines kept from the input files are written to *filename*.

____________________

.. code-block:: none

  -column column

Specifies which columns in the MODE output files to generate statistics for. Fields may be indicated by name (case insensitive) or column number (beginning at one). This option can be repeated to specify multiple columns.



MODE Command Line Options
^^^^^^^^^^^^^^^^^^^^^^^^^

MODE command line options are used to create filters that determine which of the MODE output lines that are read in, are kept. The MODE line options are numerous. They fall into seven categories: toggles, multiple set string options, multiple set integer options, integer max/min options, date/time max/min options, floating-point max/min options, and miscellaneous options. These options are described here.

Toggles
^^^^^^^

The MODE line options described in this section are shown in pairs. These toggles represent parameters that can have only one (or none) of two values. Any of these toggles may be left unspecified. However, if neither option for each toggle is indicated, the analysis will produce results that combine data from both toggles. This may produce unintended results.

____________________

.. code-block:: none

  -fcst | -obs

This toggle indicates whether forecast or observed lines should be used for analysis.

____________________

.. code-block:: none

  -single | -pair

This toggle indicates whether single object or object pair lines should be used.

____________________

.. code-block:: none

  -simple | -cluster

This toggle indicates whether simple object or cluster object lines should be used.

____________________

.. code-block:: none

  -matched | -unmatched

This toggle indicates whether matched or unmatched object lines should be used.



Multiple-set string options
^^^^^^^^^^^^^^^^^^^^^^^^^^^

The following options set various string attributes. They can be set multiple times on the command line but must be separated by spaces. Each of these options must be indicated as a string. String values that include spaces may be used by enclosing the string in quotation marks.

____________________

.. code-block:: none

  -model value

This option specifies which model to use; value must be a string.

____________________

.. code-block:: none

  -fcst_thr value
  -obs_thr  value

These two options specify thresholds for forecast and observation objects to be used in the analysis, respectively. 

____________________

.. code-block:: none

  -fcst_var value
  -obs_var  value

These options indicate the names of variables to be used in the analysis for forecast and observed fields.

____________________

.. code-block:: none

  -fcst_units value
  -obs_units  value

These options indicate the units to be used in the analysis for forecast and observed fields.


____________________

.. code-block:: none

  -fcst_lev value
  -obs_lev  value

These options indicate vertical levels for forecast and observed fields to be used in the analysis.

____________________

Multiple-set integer options
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The following options set various integer attributes. They can be set multiple times on the command line but must be separated by spaces. Each of the following options may only be indicated as an integer.

____________________

.. code-block:: none

  -fcst_lead value
  -obs_lead  value

These options are integers of the form HH[MMSS] specifying an (hour-minute-second) lead time.


____________________

.. code-block:: none

  -fcst_accum value
  -obs_accum  value

These options are integers of the form HHMMSS specifying an (hour-minute-second) accumulation time.


____________________

.. code-block:: none

  -fcst_rad value
  -obs_rad  value

These options indicate the convolution radius used for forecast or observed objects, respectively.

_____________________

Integer max/min options
^^^^^^^^^^^^^^^^^^^^^^^

These options set limits on various integer attributes. Leaving a maximum value unset means no upper limit is imposed on the value of the attribute. The option works similarly for minimum values. 

____________________

.. code-block:: none

  -area_min value
  -area_max value

These options are used to indicate minimum/maximum values for the area attribute to be used in the analysis.

____________________

.. code-block:: none

  -area_filter_min value
  -area_filter_max value

These options are used to indicate minimum/maximum values accepted for the area filter. The area filter refers to the number of non-zero values of the raw data found within the object.


____________________

.. code-block:: none

  -area_thresh_min value
  -area_thresh_max value

These options are used to indicate minimum/maximum values accepted for the area thresh. The area thresh refers to the number of values of the raw data found within the object that meet the object definition threshold criteria used.


____________________

.. code-block:: none

  -intersection_area_min value
  -intersection_area_max value

These options refer to the minimum/maximum values accepted for the intersection area attribute.


____________________

.. code-block:: none

  -union_area_min value
  -union_area_max value

These options refer to the minimum/maximum union area values accepted for analysis.

____________________

.. code-block:: none

  -symmetric_diff_min value
  -symmetric_diff_max value

These options refer to the minimum/maximum values for symmetric difference for objects to be used in the analysis.


Date/time max/min options
^^^^^^^^^^^^^^^^^^^^^^^^^

These options set limits on various date/time attributes. The values can be specified in one of three ways: 

First, the options may be indicated by a string of the form YYYYMMDD_HHMMSS. This specifies a complete calendar date and time. 

Second, they may be indicated by a string of the form YYYYMMDD_HH. Here, the minutes and seconds are assumed to be zero.

The third way of indicating date/time attributes is by a string of the form YYYYMMDD. Here, hours, minutes and seconds are assumed to be zero.


____________________

.. code-block:: none

  -fcst_valid_min YYYYMMDD[_HH[MMSS]]
  -fcst_valid_max YYYYMMDD[_HH[MMSS]]
  -obs_valid_min  YYYYMMDD[_HH[MMSS]]
  -obs_valid_max  YYYYMMDD[_HH[MMSS]]

These options indicate minimum/maximum values for the forecast and observation valid times.

____________________

.. code-block:: none

  -fcst_init_min YYYYMMDD[_HH[MMSS]]
  -fcst_init_max YYYYMMDD[_HH[MMSS]]
  -obs_init_min  YYYYMMDD[_HH[MMSS]]
  -obs_init_max  YYYYMMDD[_HH[MMSS]]

These two options indicate minimum/maximum values for forecast and observation initialization times.

_____________________

Floating-point max/min options
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Setting limits on various floating-point attributes. One may specify these as integers (i.e., without a decimal point), if desired. The following pairs of options indicate minimum and maximum values for each MODE attribute that can be described as a floating-point number. Please refer to :numref:`MODE-output` for a description of these attributes as needed.


____________________

.. code-block:: none

  -centroid_x_min value
  -centroid_x_max value


____________________

.. code-block:: none

  -centroid_y_min value
  -centroid_y_max value


____________________

.. code-block:: none

  -centroid_lat_min value
  -centroid_lat_max value


____________________

.. code-block:: none

  -centroid_lon_min value 
  -centroid_lon_max value


____________________

.. code-block:: none

  -axis_ang_min value
  -axis_ang_max value


____________________

.. code-block:: none

  -length_min value
  -length_max value


____________________

.. code-block:: none

  -width_min value
  -width_max value


____________________

.. code-block:: none

  -curvature_min value
  -curvature_max value


____________________

.. code-block:: none

  -curvature_x_min value
  -curvature_x_max value


____________________

.. code-block:: none

  -curvature_y_min value
  -curvature_y_max value


____________________

.. code-block:: none

  -complexity_min value
  -complexity_max value


____________________

.. code-block:: none

  -intensity_10_min value
  -intensity_10_max value


____________________

.. code-block:: none

  -intensity_25_min value
  -intensity_25_max value


____________________

.. code-block:: none

  -intensity_50_min value
  -intensity_50_max value


____________________

.. code-block:: none

  -intensity_75_min value
  -intensity_75_max value


____________________

.. code-block:: none

  -intensity_90_min value
  -intensity_90_max value


____________________

.. code-block:: none

  -intensity_user_min value
  -intensity_user_max value


____________________

.. code-block:: none

  -intensity_sum_min value
  -intensity_sum_max value


____________________

.. code-block:: none

  -centroid_dist_min value
  -centroid_dist_max value


____________________

.. code-block:: none

  -boundary_dist_min value
  -boundary_dist_max value


____________________

.. code-block:: none

  -convex_hull_dist_min value
  -convex_hull_dist_max value


____________________

.. code-block:: none

  -angle_diff_min value
  -angle_diff_max value


____________________

.. code-block:: none

  -aspect_diff_min value
  -aspect_diff_max value


____________________

.. code-block:: none

  -area_ratio_min value
  -area_ratio_max value


____________________

.. code-block:: none

  -intersection_over_area_min value
  -intersection_over_area_max value


____________________

.. code-block:: none

  -curvature_ratio_min value
  -curvature_ratio_max value


____________________

.. code-block:: none

  -complexity_ratio_min value
  -complexity_ratio_max value


____________________

.. code-block:: none

  -percentile_intensity_ratio_min value
  -percentile_intensity_ratio_max value


____________________

.. code-block:: none

  -interest_min value
  -interest_max value


Miscellaneous options
^^^^^^^^^^^^^^^^^^^^^

These options are used to indicate parameters that did not fall into any of the previous categories.


____________________

.. code-block:: none

  -mask_poly filename

This option indicates the name of a polygon mask file to be used for filtering. The format for these files is the same as that of the polyline files for the other MET tools.


____________________

.. code-block:: none

  -help

This option prints the usage message.

.. _mode_analysis-configuration-file:

mode_analysis configuration file
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To use the MODE-Analysis tool, the user must un-comment the options in the configuration file to apply them and comment out unwanted options. The options in the configuration file for the MODE-Analysis tools are the same as the MODE command line options described in :numref:`mode_analysis-usage`.

The parameters that are set in the configuration file either add to or override parameters that are set on the command line. For the “set string” and “set integer type” options enclosed in brackets, the values specified in the configuration file are added to any values set on the command line. For the “toggle” and “min/max type” options, the values specified in the configuration file override those set on the command line.

mode_analysis output
~~~~~~~~~~~~~~~~~~~~

The output of the MODE-Analysis tool is a self-describing tabular format written to standard output. The length and contents of the table vary depending on whether **-summary** or **-bycase** is selected. The contents also change for **-summary** depending on the number of columns specified by the user.
