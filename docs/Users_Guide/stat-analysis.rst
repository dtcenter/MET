.. _stat-analysis:

******************
Stat-Analysis Tool
******************

Introduction
============

The Stat-Analysis tool ties together results from the Point-Stat, Grid-Stat, Ensemble-Stat, Wavelet-Stat, and TC-Gen tools by providing summary statistical information and a way to filter their STAT output files. It processes the STAT output created by the other MET tools in a variety of ways which are described in this section.

MET version 9.0 adds support for the passing matched pair data (MPR) into Stat-Analysis using a Python script with the "-lookin python ..." option. An example of running Stat-Analysis with Python embedding is shown in :numref:`stat_analysis-usage`.

Scientific and statistical aspects
==================================

The Stat-Analysis tool can perform a variety of analyses, and each type of analysis is called a "job". The job types include the ability to (i) aggregate results over a user-specified time; (ii) stratify statistics based on time of day, model initialization time, lead-time, model run identifier, output filename, or wavelet decomposition scale; and (iii) compute specific verification indices such as the GO Index [1]_
and wind direction statistics. Future functionality may include information about time-trends and/or calculations based on climatology (e.g., anomaly correlation). This section summarizes the capabilities of the supported Stat-Analysis jobs.

Filter STAT lines
-----------------

The Stat-Analysis "filter" job simply filters out specific STAT lines based on user-specified search criteria. All of the STAT lines that are retained from one or many files are written to a single output file. The output file for filtered STAT lines must be specified using the **-dump_row** job command option.

Summary statistics for columns
------------------------------

The Stat-Analysis "summary" job produces summary information for columns of data. After the user specifies the column(s) of interest and any other relevant search criteria, summary information is produced from values in those column(s) of data. The summary statistics produced are: mean, standard deviation, minimum, maximum, the 10th, 25th, 50th, 75th, and 90th percentiles, the interquartile range, the range, and both weighted and unweighted means using the logic prescribed by the World Meteorological Organization (WMO).

Confidence intervals are computed for the mean and standard deviation of the column of data. For the mean, the confidence interval is computed two ways - based on an assumption of normality and also using the bootstrap method. For the standard deviation, the confidence interval is computed using the bootstrap method. In this application of the bootstrap method, the values in the column of data being summarized are resampled, and for each replicated sample, the mean and standard deviation are computed.

The columns to be summarized can be specified in one of two ways. Use the **-line_type** option exactly once to specify a single input line type and use the **-column** option one or more times to select the columns of data to be summarized. Alternatively, use the **-column** option one or more times formatting the entries as **LINE_TYPE:COLUMN**. For example, the RMSE column from the CNT line type can be selected using **-line_type CNT -column RMSE** or using **-column CNT:RMSE**. With the second option, columns from multiple input line types may be selected. For example, **-column CNT:RMSE,CNT:MAE,CTS:CSI** select two CNT columns and one CTS column.

The WMO mean values are computed in one of three ways, as determined by the configuration file settings for **wmo_sqrt_stats** and **wmo_fisher_stats**. The statistics listed in the first option are square roots. When computing WMO means, the input values are first squared, then averaged, and the square root of the average value is reported. The statistics listed in the second option are correlations to which the Fisher transformation is applied. For any statistic not listed, the WMO mean is computed as a simple arithmetic mean. The **WMO_TYPE** output column indicates the method applied (**SQRT, FISHER**, or **MEAN**). The **WMO_MEAN** and **WMO_WEIGHTED_MEAN** columns contain the unweighted and weighted means, respectively. The value listed in the **TOTAL** column of each input line is used as the weight.

The **-derive** job command option can be used to perform the derivation of statistics on the fly from input partial sums and contingency table counts. When enabled, SL1L2 and SAL1L2 input lines are converted to CNT statistics, VL1L2 input lines are converted to VCNT statistics, and CTC lines are converted to CTS statistics. Users should take care with this option. If the data passed to this job contains both partial sums and derived statistics, using the **-derive** option will effectively cause the statistics to be double counted. Use the **-line_type** job command option to filter the data passed to Stat-Analysis jobs.

.. _StA_Aggregated-values-from:

Aggregated values from multiple STAT lines
------------------------------------------

The Stat-Analysis "aggregate" job aggregates values from multiple STAT lines of the same type. The user may specify the specific line type of interest and any other relevant search criteria. The Stat-Analysis tool then creates sums of each of the values in all lines matching the search criteria. The aggregated data are output as the same line type as the user specified. The STAT line types which may be aggregated in this way are the contingency table (FHO, CTC, PCT, MCTC, NBRCTC), partial sums (SL1L2, SAL1L2, VL1L2, and VAL1L2), and other (ISC, ECNT, RPS, RHIST, PHIST, RELP, NBRCNT, SSVAR, and GRAD) line types.

Aggregate STAT lines and produce aggregated statistics
------------------------------------------------------

The Stat-Analysis "aggregate-stat" job aggregates multiple STAT lines of the same type together and produces relevant statistics from the aggregated line. This may be done in the same manner listed above in :numref:`StA_Aggregated-values-from`. However, rather than writing out the aggregated STAT line itself, the relevant statistics generated from that aggregated line are provided in the output. Specifically, if a contingency table line type (FHO, CTC, PCT, MCTC, or NBRCTC) has been aggregated, contingency table statistics (CTS, ECLV, PSTD, MCTS, or NBRCTS) line types can be computed. If a partial sums line type (SL1L2 or SAL1L2) has been aggregated, the continuous statistics (CNT) line type can be computed. If a vector partial sums line type (VL1L2) has been aggregated, the vector continuous statistics (VCNT) line type can be computed. For ensembles, the ORANK line type can be accumulated into ECNT, RPS, RHIST, PHIST, RELP, or SSVAR output. If the matched pair line type (MPR) has been aggregated, may output line types (FHO, CTC, CTS, CNT, MCTC, MCTS, SL1L2, SAL1L2, VL1L2, VCNT, WDIR, PCT, PSTD, PJC, PRC, or ECLV) can be computed. Multiple output line types may be specified for each "aggregate-stat" job, as long as each output is derivable from the input.

When aggregating the matched pair line type (MPR), additional required job command options are determined by the requested output line type(s). For example, the "-out_thresh" (or "-out_fcst_thresh" and  "-out_obs_thresh" options) are required to compute contingnecy table counts (FHO, CTC) or statistics (CTS). Those same job command options can also specify filtering thresholds when computing continuous partial sums (SL1L2, SAL1L2) or statistics (CNT). Output is written for each threshold specified.

When aggregating the matched pair line type (MPR) and computing an output contingency table statistics (CTS) or continuous statistics (CNT) line type, the bootstrapping method can be applied to compute confidence intervals. The bootstrapping method is applied here in the same way that it is applied in the statistics tools. For a set of n matched forecast-observation pairs, the matched pairs are resampled with replacement many times. For each replicated sample, the corresponding statistics are computed. The confidence intervals are derived from the statistics computed for each replicated sample.

.. _StA_Skill-Score-Index:

Skill Score Index
-----------------

The Stat-Analysis "ss_index", "go_index", and "cbs_index" jobs calculate skill score indices by weighting scores for meteorological fields at different levels and lead times. Pre-defined configuration files are provided for the GO Index and CBS Index which are special cases of the highly configurable skill score index job.

In general, a skill score index is computed over several terms and the number and definition of those terms is configurable. It is computed by aggregating the output from earlier runs of the Point-Stat and/or Grid-Stat tools over one or more cases. When configuring a skill score index job, the following requirements apply:

1. Exactly two models names must be chosen. The first is interpreted as the forecast model and the second is the reference model, against which the performance of the forecast should be measured. Specify this with the "model" configuration file entry or using the "-model" job command option.

2. The forecast variable name, level, lead time, line type, column, and weight options must be specified. If the value remains constant for all the terms, set it to an array of length one. If the value changes for at least one term, specify an array entry for each term. Specify these with the "fcst_var", "fcst_lev", "lead_time", "line_type", "column", and "weight" configuration file entries, respectively, or use the corresponding job command options.

3. While these line types are required, additional options may be provided for each term, including the observation type ("obtype"), verification region ("vx_mask"), and interpolation method ("interp_mthd"). Specify each as single value or provide a value for each term.

4. Only the SL1L2 and CTC input line types are supported, and the input Point-Stat and/or Grid-Stat output must contain these line types.

5. For the SL1L2 line type, set the "column" entry to the CNT output column that contains the statistic of interest (e.g. RMSE for root-mean-squared-error). Note, only those continuous statistics that are derivable from SL1L2 lines can be used.

6. For the CTC line type, set the "column" entry to the CTS output column that contains the statistic of intereest (e.g. PODY for probability of detecting yes). Note, consider specifying the "fcst_thresh" for the CTC line type.

For each term, all matching SL1L2 (or CTC) input lines are aggregated separately for the forecast and reference models. The requested statistic ("column") is derived from the aggregated partial sums or counts. For each term, a skill score is defined as:

.. math:: ss = 1.0 - \frac{s_{fcst}^2}{s_{ref}^2}

Where :math:`s_{fcst}` and :math:`s_{ref}` are the aggregated forecast and reference statistics, respectively. Next, a weighted average is computed from the skill scores for each term:

 .. math:: ss_{avg} = \frac{1}{n} \sum_{i=1}^{n} w_i * ss_i

Where, :math:`w_i` and :math:`ss_i` are the weight and skill score for each term and :math:`n` is the number of terms. Finally, the skill score index is computed as:

.. math:: index = \sqrt{\frac{1.0}{1.0-ss_{avg}}}

A value greater than 1.0 indicates that the forecast model outperforms the reference, while a value less than 1.0 indicates that the reference outperforms the forecast.

The default skill score index name (SS_INDEX) can be overridden using the "ss_index_name" option in the configuration file. The pre-defined configuration files for the GO Index and CBS Index use "GO_INDEX" and "CBS_INDEX", respectively.

When running a skill score index job using the "-out_stat" job command option, a .stat output file is written containing the skill score index (SSIDX) output line type. If the "-by" job command option is specified, the skill score index will be computed separately for each unique combination of values found in the column(s) specified. For example, "-by FCST_INIT_BEG,VX_MASK" runs the job separately for each combination of model initialization time and verification region found in the input. Note that increasing the Stat-Analysis verbosity level (-v 3) on the command line prints detailed information about each skill score index term.

.. _StA_Go-Index:

GO Index
--------

The "go_index" job is a special case of the "ss_index" job, described in :numref:`StA_Skill-Score-Index`. The GO Index is a weighted average of 48 skill scores of RMSE statistics for wind speed, dew point temperature, temperature, height, and pressure at several levels in the atmosphere. The variables, levels, and lead times included in the index are listed in :numref:`compute_GO_Index` and are defined by the default "STATAnalysisConfig_GO_Index" configuration file. The partial sums (SL1L2 lines in the STAT output) for each of these variables at each level and lead time must have been computed in a previous step. The Stat-Analysis tool then uses the weights in :numref:`compute_GO_Index` to compute values for the GO Index.

.. _compute_GO_Index:

.. list-table:: Variables, levels, and weights used to compute the GO Index.
  :widths: auto
  :header-rows: 2

  * - Variable
    - Level
    - Weights by Lead time
    - 
    -
    - 
  * -  
    -  
    - 12 h
    - 24 h
    - 36 h
    - 48 h
  * - Wind speed
    - 250 hPa
    - 4
    - 3
    - 2
    - 1
  * -  
    - 400 hPa
    - 4
    - 3
    - 2
    - 1
  * -  
    - 850 hPa
    - 4
    - 3
    - 2
    - 1
  * -  
    - Surface
    - 8
    - 6
    - 4
    - 2
  * - Dew point temperature
    - 400 hPa
    - 8
    - 6
    - 4
    - 2
  * -  
    - 700 hPa
    - 8
    - 6
    - 4
    - 2
  * -  
    - 850 hPa
    - 8
    - 6
    - 4
    - 2
  * -  
    - Surface
    - 8
    - 6
    - 4
    - 2
  * - Temperature
    - 400 hPa
    - 4
    - 3
    - 2
    - 1
  * -  
    - Surface
    - 8
    - 6
    - 4
    - 2
  * - Height
    - 400 hPa
    - 4
    - 3
    - 2
    - 1
  * - Pressure
    - Mean sea level
    - 8
    - 6
    - 4
    - 2

.. _StA_CBS-Index:

CBS Index
---------

The "cbs_index" job is a special case of the "ss_index" job, described in :numref:`StA_Skill-Score-Index`. The CBS Index is a weighted average of 40 skill scores of RMSE statistics for mean sea level pressure, height, and wind speed at multiple levels computed over the northern hemisphere, southern hemisphere and the tropics. The variables, levels, lead times, and regions included in the index are listed in :numref:`compute_CBS_Index` and are defined by the default "STATAnalysisConfig_CBS_Index" configuration file. The partial sums (SL1L2 lines in the STAT output) for each of these variables for each level, lead time, and masking region must have been computed in a previous step. The Stat-Analysis tool then uses the weights in :numref:`compute_CBS_Index` to compute values for the CBS Index.

.. _compute_CBS_Index:

.. list-table:: Variables, levels, and weights used to compute the CBS Index for 24, 48, 72, 96 and 120 hour lead times.
  :widths: auto
  :header-rows: 2

  * - Variable
    - Level
    - Weights by Region
    -
    -
  * -
    -
    - North Hem
    - Tropics
    - South Hem
  * - Pressure
    - Mean sea level
    - 6.4
    - x
    - 3.2
  * - Height
    - 500 hPa
    - 2.4
    - x
    - 1.2
  * - Wind speed
    - 250 hPa
    - 2.4
    - 1.2
    - 1.2
  * -
    - 850 hPa
    - x
    - 2.0
    - x

Ramp Events
-----------

The Stat-Analysis "ramp" job identifies ramp events (large increases or decreases in values over a time window) in both the forecast and observation data. It categorizes these events as hits, misses, false alarms, or correct negatives by applying a configurable matching time window and computes the corresponding categorical statistics.

Wind Direction Statistics
-------------------------

The Stat-Analysis "aggregate_stat" job can read vector partial sums and derive wind direction error statistics (WDIR). The vector partial sums (VL1L2 or VAL1L2) or matched pairs (MPR) for the UGRD and VGRD must have been computed in a previous step, i.e. by Point-Stat or Grid-Stat tools. This job computes an average forecast wind direction and an average observed wind direction along with their difference. The output is in degrees. In Point-Stat and Grid-Stat, the UGRD and VGRD can be verified using thresholds on their values or on the calculated wind speed. If thresholds have been applied, the wind direction statistics are calculated for each threshold.

The first step in verifying wind direction is running the Grid-Stat and/or Point-Stat tools to verify each forecast of interest and generate the VL1L2 or MPR line(s). When running these tools, please note:

1. To generate VL1L2 or MPR lines, the user must request the verification of both the U-component and V-component of wind at the same vertical levels.

2. To generate VL1L2 or MPR lines, the user must set the "output_flag" to indicate that the VL1L2 or MPR line should be computed and written out.

3. The user may select one or more spatial verification regions over which to accumulate the statistics.

4. The user may select one or more wind speed thresholds to be applied to the U and V wind components when computing the VL1L2 lines. It may be useful to investigate the performance of wind forecasts using multiple wind speed thresholds. For MPR line types, the wind speed threshold can be applied when computing the MPR lines, or the MPR output may be filtered afterwards by the Stat-Analysis tool.

Once the appropriate lines have been generated for each verification time of interest, the user may run the Stat-Analysis tool to analyze them. The Stat-Analysis job "aggregate_stat", along with the "-output_line_type WDIR" option, reads all of the input lines and computes statistics about the wind direction. When running this job the user is encouraged to use the many Stat-Analysis options to filter the input lines down to the set of lines of interest. The output of the wind direction analysis job consists of two lines with wind direction statistics computed in two slightly different ways. The two output lines begin with "ROW_MEAN_WDIR" and "AGGR_WDIR", and the computations are described below:

1. For the "ROW_MEAN_WDIR" line, each of the input VL1L2 lines is treated separately and given equal weight. The mean forecast wind direction, mean observation wind direction, and the associated error are computed for each of these lines. Then the means are computed across all of these forecast wind directions, observation wind directions, and their errors.

2. For the "AGGR_WDIR" line, the input VL1L2 lines are first aggregated into a single line of partial sums where the weight for each line is determined by the number of points it represents. From this aggregated line, the mean forecast wind direction, observation wind direction, and the associated error are computed and written out.

Practical information
=====================

The following sections describe the usage statement, required arguments and optional arguments for the Stat-Analysis tool.

.. _stat_analysis-usage:

stat_analysis usage
-------------------

The usage statement for the Stat-Analysis tool is shown below:

.. code-block:: none

  Usage: stat_analysis
         -lookin path
         [-out file]
         [-tmp_dir path]
         [-log file]
         [-v level]
         [-config config_file] | [JOB COMMAND LINE]

stat_analysis has two required arguments and accepts several optional ones. 

In the usage statement for the Stat-Analysis tool, some additional terminology is introduced. In the Stat-Analysis tool, the term "job" refers to a set of tasks to be performed after applying user-specified options (i.e., "filters"). The filters are used to pare down a collection of output from the MET statistics tools to only those lines that are desired for the analysis. The job and its filters together comprise the "job command line". The "job command line" may be specified either on the command line to run a single analysis job or within the configuration file to run multiple analysis jobs at the same time. If jobs are specified in both the configuration file and the command line, only the jobs indicated in the configuration file will be run. The various jobs types are described in :numref:`table_WS_format_info_ISC` and the filtering options are described in :numref:`wavelet_stat-configuration-file`.

Required arguments for stat_analysis
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. The **-lookin path** specifies the name of a directory to be searched recursively for STAT files (ending in ".stat") or any explicit file name with any suffix (such as "_ctc.txt") to be read. This option may be used multiple times to specify multiple directories and/or files to be read. If "-lookin python" is used, it must be followed by a Python embedding script and any command line arguments it takes. Python embedding can be used to pass matched pair (MPR) lines as input to Stat-Analysis.

2. Either a configuration file must be specified with the **-config** option, or a **JOB COMMAND LINE** must be denoted. The **JOB COMMAND LINE** is described in :numref:`stat_analysis-configuration-file`

Optional arguments for stat_analysis
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

3. The **-config config_file** specifies the configuration file to be used. The contents of the configuration file are discussed below.

4. The **-out file** option indicates the file to which output data should be written. If this option is not used, the output is directed to standard output.

5. The **-tmp_dir path** option selects the directory for writing out temporary files. 

6. The **-log file** option directs output and errors to the specified log file. All messages will be written to that file as well as standard out and error. Thus, users can save the messages without having to redirect the output on the command line. The default behavior is no log file. 

7. The **-v level** indicates the desired level of verbosity. The contents of "level" will override the default setting of 2. Setting the verbosity to 0 will make the tool run with no log messages, while increasing the verbosity will increase the amount of logging. 

An example of the stat_analysis calling sequence is shown below.

.. code-block:: none

  stat_analysis -lookin ../out/point_stat \
  -config STATAnalysisConfig

In this example, the Stat-Analysis tool will search for valid STAT lines located in the *../out/point_stat* directory that meet the options specified in the configuration file, *config/STATAnalysisConfig*.

.. _StA-pyembed:

Python Embedding for Matched Pairs
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The example below uses Python embedding.

.. code-block:: none

  stat_analysis \
  -lookin python MET_BASE/python/read_ascii_mpr.py point_stat_mpr.txt \
  -job aggregate_stat -line_type MPR -out_line_type CNT \
  -by FCST_VAR,FCST_LEV

In this example, rather than passing the MPR output lines from Point-Stat directly into Stat-Analysis (which is the typical approach), the read_ascii_mpr.py Python embedding script reads that file and passes the data to Stat-Analysis. The aggregate_stat job is defined on the command line and CNT statistics are derived from the MPR input data. Separate CNT statistics are computed for each unique combination of FCST_VAR and FCST_LEV present in the input. Please refer to :numref:`Appendix F, Section %s <appendixF>` for more details about Python embedding in MET.

.. _stat_analysis-configuration-file:

stat_analysis configuration file
--------------------------------

The default configuration file for the Stat-Analysis tool named **STATAnalysisConfig_default** can be found in the installed *share/met/config* directory. The version used for the example run in :numref:`installation` is also available in *scripts/config*. Like the other configuration files described in this document, it is recommended that users make a copy of these files prior to modifying their contents. 

The configuration file for the Stat-Analysis tool is optional. Users may find it more convenient initially to run Stat-Analysis jobs on the command line specifying job command options directly. Once the user has a set of or more jobs they would like to run routinely on the output of the MET statistics tools, they may find grouping those jobs together into a configuration file to be more convenient.

Most of the user-specified parameters listed in the Stat-Analysis configuration file are used to filter the ASCII statistical output from the MET statistics tools down to a desired subset of lines over which statistics are to be computed. Only output that meets all of the parameters specified in the Stat-Analysis configuration file will be retained.

The Stat-Analysis tool actually performs a two step process when reading input data. First, it stores the filtering information defined top section of the configuration file. It applies that filtering criteria when reading the input STAT data and writes the filtered data out to a temporary file. Second, each job defined in the **jobs** entry reads data from that temporary file and performs the task defined for the job. After all jobs have run, the Stat-Analysis tool deletes the temporary file.

This two step process enables the Stat-Analysis tool to run more efficiently when many jobs are defined in the configuration file. If only operating on a small subset of the input data, the common filtering criteria can be applied once rather than re-applying it for each job. In general, filtering criteria common to all tasks defined in the **jobs** entry should be moved to the top section of the configuration file.

As described above, filtering options specified in the first section of the configuration file will be applied to every task in the **jobs** entry. However, if an individual job specifies a particular option that was specified above, it will be applied for that job. For example, if the **model[]** option is set at the top to ["Run 1", "Run2"], but a job in the joblist sets the **-model** option as "Run1", that job will be performed only on "Run1" data. Also note that environment variables may be used when editing configuration files, as described in the :numref:`pb2nc configuration file` for the PB2NC tool.

________________________

.. code-block:: none

  boot           = { interval = PCTILE; rep_prop = 1.0; n_rep = 1000;
                   rng = "mt19937"; seed = ""; }
  hss_ec_value   = NA;
  rank_corr_flag = TRUE;
  tmp_dir        = "/tmp";
  version        = "VN.N";

The configuration options listed above are common to many MET tools and are described in :numref:`config_options`.

___________________

.. code-block:: none

  model = [];

The user may specify a comma-separated list of model names to be used for all analyses performed. The names must be in double quotation marks. If multiple models are listed, the analyses will be performed on their union. These selections may be further refined by using the "**-model**" option within the job command lines.

___________________

.. code-block:: none

  desc = [];

The user may specify a comma-separated list of description strings to be used for all analyses performed. The names must be in double quotation marks. If multiple description strings are listed, the analyses will be performed on their union. These selections may be further refined by using the "**-desc**" option within the job command lines.

___________________

.. code-block:: none

  fcst_lead = [];
  obs_lead  = [];

The user may specify a comma-separated list of forecast and observation lead times in HH[MMSS] format to be used for any analyses to be performed. If multiple times are listed, the analyses will be performed on their union. These selections may be further refined by using the "**-fcst_lead**" and "**-obs_lead**" options within the job command lines.

___________________

.. code-block:: none

  fcst_valid_beg  = "";
  fcst_valid_end  = "";
  fcst_valid_inc  = [];
  fcst_valid_exc  = [];
  fcst_valid_hour = [];

  obs_valid_beg   = "";
  obs_valid_end   = "";
  obs_valid_inc   = [];
  obs_valid_exc   = [];
  obs_valid_hour  = [];

The user may filter data based on its valid time. The fcst/obs_valid_beg and fcst/obs_valid_end options are strings in YYYYMMDD[_HH[MMSS]] format which define retention time windows for all analyses to be performed. The analyses are performed on all data whose valid time falls within these windows. If left as empty strings, no valid time window filtering is applied.

The fcst/obs_valid_hour options are arrays of strings in HH format which define the valid hour(s) of the data to be used. If specified, only data whose valid hour appears in the list of hours is used. The fcst/obs_valid_inc/exc options are arrays of strings in YYYYMMDD[_HH[MMSS]] format which explicitly define the valid times for data to be included or excluded from all analyses.

These selections may be further refined by using the **"-fcst_valid_beg", "-fcst_valid_end", "-fcst_valid_inc", "-fcst_valid_exc", "-fcst_valid_hour", "-obs_valid_beg", "-obs_valid_end", "-obs_valid_inc", "-obs_valid_exc",** and **"-obs_valid_hour"** options within the job command line.

___________________

.. code-block:: none

  fcst_init_beg  = "";
  fcst_init_end  = "";
  fcst_init_inc  = [];
  fcst_init_exc  = [];
  fcst_init_hour = [];

  obs_init_beg   = "";
  obs_init_end   = "";
  obs_init_inc   = [];
  obs_init_exc   = [];
  obs_init_hour  = [];

These time filtering options are the same as described above but applied to initialization times rather than valid times. These selections may be further refined by using the **"-fcst_init_beg", "-fcst_init_end", "-fcst_init_inc", "-fcst_init_exc", "-fcst_init_hour"," "-obs_init_beg", "-obs_init_end", "-obs_init_inc", "-obs_init_exc"** and **"-obs_init_hour"** options within the job command line.

___________________

.. code-block:: none

  fcst_var = [];
  obs_var  = [];

The user may specify a comma-separated list of forecast and observation variable types to be used for any analyses to be performed. If multiple variable types are listed, the analyses will be performed on their union. These selections may be further refined by using the **"-fcst_var"** and **"-obs_var"** options within the job command lines.

___________________

.. code-block:: none

  fcst_units = [];
  obs_units  = [];

The user may specify a comma-separated list of forecast and observation units to be used for any analyses to be performed. If multiple units are listed, the analyses will be performed on their union. These selections may be further refined by using the **"-fcst_units"** and **"-obs_units"** options within the job command lines.

___________________

.. code-block:: none

  fcst_lev = [];
  obs_lev  = [];

The user may specify a comma-separated list of forecast and observation level types to be used for any analyses to be performed. If multiple level types are listed, the analyses will be performed on their union. These selections may be further refined by using the **"-fcst_lev"** and **"-obs_lev"** options within the job command lines.

___________________

.. code-block:: none

  obtype = [];

The user may specify a comma-separated list of observation types to be used for all analyses. If multiple observation types are listed, the analyses will be performed on their union. These selections may be further refined by using the **"-obtype"** option within the job command line.

___________________

.. code-block:: none

  vx_mask = [];

The user may specify a comma-separated list of verification masking regions to be used for all analyses. If multiple verification masking regions are listed, the analyses will be performed on their union. These selections may be further refined by using the **"-vx_mask"** option within the job command line. 

___________________

.. code-block:: none

  interp_mthd = [];

The user may specify a comma-separated list of interpolation methods to be used for all analyses. If multiple interpolation methods are listed, the analyses will be performed on their union. These selections may be further refined by using the **"-interp_mthd"** option within the job command line.

___________________

.. code-block:: none

  interp_pnts = [];

The user may specify a comma-separated list of interpolation points to be used for all analyses. If multiple interpolation points are listed, the analyses will be performed on their union. These selections may be further refined by using the **"-interp_pnts"** option within the job command line.

___________________

.. code-block:: none

  fcst_thresh = [];
  obs_thresh  = [];
  cov_thresh  = [];

The user may specify comma-separated lists of forecast, observation, and coverage thresholds to be used for any analyses to be performed. If multiple thresholds are listed, the analyses will be performed on their union. These selections may be further refined by using the **"-fcst_thresh", "-obs_thresh"**, and **"-cov_thresh"** options within the job command lines.

___________________

.. code-block:: none

  alpha = [];

The user may specify a comma-separated list alpha confidence values to be used for all analyses. If alpha values are listed, the analyses will be performed on their union. These selections may be further refined by using the **"-alpha"** option within the job command line.

___________________

.. code-block:: none

  line_type = [];

The user may specify a comma-separated list of line types to be used for all analyses. If multiple line types are listed, the analyses will be performed on their union. These selections may be further refined by using the **"-line_type"** option within the job command line. 

___________________

.. code-block:: none

  column = [];
  weight = [];

The column and weight entries are used to define a skill score index. They can either be set to a constant value of length one or specify a separate value for each term of the index.

___________________

.. code-block:: none

  ss_index_name       = "SS_INDEX";
  ss_index_vld_thresh = 1.0;

The ss_index_name and ss_index_vld_thresh options are used to define a skill score index. The ss_index_name entry is a string which defines the output name for the current skill score index configuration. The ss_index_vld_thresh entry is a number between 0.0 and 1.0 that defines the required ratio of valid terms. If the ratio of valid skill score index terms to the total is less than than this number, no output is written for that case. The default value of 1.0 indicates that all terms are required.

___________________

.. code-block:: none

  jobs = [
   "-job filter -dump_row ./filter_job.stat"
  ];

The user may specify one or more analysis jobs to be performed on the STAT lines that remain after applying the filtering parameters listed above. Each entry in the joblist contains the task and additional filtering options for a single analysis to be performed. The format for an analysis job is as follows:

**-job_name** REQUIRED and OPTIONAL ARGUMENTS

All possible tasks for **job_name** are listed in :numref:`Des_components_STAT_analysis_tool`.

.. role:: raw-html(raw)
    :format: html

.. _Des_components_STAT_analysis_tool:
	 
.. list-table:: Description of components of the job command lines for the Stat-Analysis tool.Variables, levels, and weights used to compute the GO Index.
  :widths: 15 55 20
  :header-rows: 1

  * - Job Name
    - Job commandDescription
    - Required Arguments
  * - filter
    - Filters out the statistics lines based on applying options* (See note below table)
    - \-dump_row
  * - summary
    - Computes the mean, standard deviation, percentiles (min, 10th, 25th, 50th, 75th, 90th, and max), interquartile range, range, wmo_mean, and wmo_weighted_mean
    - \-line_type :raw-html:`<br />` \-column
  * - aggregate
    - Aggregates the statistics output, computing the statistic specified for the entire collection of valid lines
    - \-line_type
  * - aggregate_stat
    - Aggregates the statistics output, and converts the input line type to the output line type specified
    - \-line_type  :raw-html:`<br />`   \-out_line_type
  * - ss_index
    - Calculates a user-defined Skill Score index as described in section :numref:`StA_Skill-Score-Index`.
    - \-model forecast :raw-html:`<br />`  \-model reference
  * - go_index
    - Calculates the GO Index as described in section :numref:`StA_GO-Index`.
    - \-model forecast :raw-html:`<br />`   \-model reference
  * - cbs_index
    - Calculates the CBS Index as described in section :numref:`StA_CBS-Index`.
    - \-model forecast :raw-html:`<br />`   \-model reference
  * - ramp
    - Defines a ramp event on a time-series of forecast and observed values. The amount of change from one time to the next is computed for forecast and observed values. Those changes are thresholded to define events which are used to populate a 2x2 contingency table.
    - \-ramp_type :raw-html:`<br />` \-ramp_thresh :raw-html:`<br />` \-out_line_type :raw-html:`<br />` \-column :raw-html:`<br />` \-ramp_time :raw-html:`<br />` \-ramp_exact :raw-html:`<br />` \-ramp_window 

___________________

.. code-block:: none

  out_alpha = 0.05;

This entry specifies the alpha value to be used when computing confidence intervals for output statistics. It is similar to the **ci_alpha** entry described in :numref:`config_options`.

___________________

.. code-block:: none

  wmo_sqrt_stats = [ "CNT:FSTDEV",  "CNT:OSTDEV",  "CNT:ESTDEV",
                     "CNT:RMSE",    "CNT:RMSFA",   "CNT:RMSOA", 
                     "VCNT:FS_RMS", "VCNT:OS_RMS", "VCNT:RMSVE",
                     "VCNT:FSTDEV", "VCNT:OSTDEV" ];

  wmo_fisher_stats = [ "CNT:PR_CORR", "CNT:SP_CORR",
                       "CNT:KT_CORR", "CNT:ANOM_CORR", "CNT:ANOM_CORR_UNCNTR" ];


These entries specify lists of statistics in the form LINE_TYPE:COLUMN to which the various WMO mean logic types should be applied for the summary job type.

___________________

.. code-block:: none

  vif_flag = FALSE;

The variance inflation factor (VIF) flag indicates whether to apply a first order variance inflation when calculating normal confidence intervals for an aggregated time series of contingency table counts or partial sums. The VIF adjusts the variance estimate for the lower effective sample size caused by autocorrelation of the statistics through time. A value of **FALSE** will not compute confidence intervals using the VIF. A value of **TRUE** will include the VIF, resulting in a slightly wider normal confidence interval.

___________________

The Stat-Analysis tool supports several additional job command options which may be specified either on the command line when running a single job or within the **jobs** entry within the configuration file. These additional options are described below:

.. code-block:: none

  -by col_name

This job command option is extremely useful. It can be used multiple times to specify a list of STAT header column names. When reading each input line, the Stat-Analysis tool concatenates together the entries in the specified columns and keeps track of the unique cases. It applies the logic defined for that job to each unique subset of data. For example, if your output was run over many different model names and masking regions, specify **-by MODEL,VX_MASK** to get output for each unique combination rather than having to run many very similar jobs.

.. code-block:: none
		
  -column_min     col_name value
  -column_max     col_name value
  -column_eq      col_name value
  -column_thresh  col_name thresh
  -column_str     col_name string
  -column_str_exc col_name string

The column filtering options may be used when the **-line_type** has been set to a single value. These options take two arguments, the name of the data column to be used followed by a value, string, or threshold to be applied. If multiple column_min/max/eq/thresh/str options are listed, the job will be performed on their intersection. Each input line is only retained if its value meets the numeric filtering criteria defined, matches one of the strings defined by the **-column_str** option, or does not match any of the string defined by the **-column_str_exc** option. Multiple filtering strings may be listed using commas. Defining thresholds in MET is described in :numref:`config_options`.

.. code-block:: none
		
  -dump_row file

Each analysis job is performed over a subset of the input data. Filtering the input data down to a desired subset is often an iterative process. The **-dump_row** option may be used for each job to specify the name of an output file to which the exact subset of data used for that job will be written. When initially constructing Stat-Analysis jobs, users are strongly encouraged to use the option and check its contents to ensure that the analysis was actually done over the intended subset.

.. code-block:: none
		
  -out_line_type name

This option specifies the desired output line type(s) for the **aggregate_stat** job type.

.. code-block:: none
		
  -out_stat file
  -set_hdr  col_name string

The Stat-Analysis tool writes its output to either the log file or the file specified using the **-out** command line option. However the **aggregate** and **aggregate_stat** jobs create STAT output lines and the standard output written lacks the full set of STAT header columns. The **-out_stat** job command option may be used for these jobs to specify the name of an output file to which full STAT output lines should be written. When the **-out_stat** job command option is used for **aggregate** and **aggregate_stat** jobs the output is sent to the **-out_stat** file instead of the log or **-out** file.

Jobs will often combine output with multiple entries in the header columns. For example, a job may aggregate output with three different values in the **VX_MASK** column, such as "mask1", "mask2", and "mask3". The output **VX_MASK** column will contain the unique values encountered concatenated together with commas: "mask1,mask2,mask3". Alternatively, the **-set_hdr** option may be used to specify what should be written to the output header columns, such as "-set_hdr VX_MASK all_three_masks".

When using the "-out_stat" option to create a .stat output file and stratifying results using one or more "-by" job command options, those columns may be referenced in the "-set_hdr" option. When using multiple "-by" options, use "CASE" to reference the full case information string:

.. code-block:: none
		
  -job aggregate_stat -line_type MPR -out_line_type CNT -by FCST_VAR,OBS_SID \
  -set_hdr VX_MASK OBS_SID -set_hdr DESC CASE

The example above reads MPR lines, stratifies the data by forecast variable name and station ID, and writes the output for each case to a .stat output file. When creating the .stat file, write the full case information to the DESC output column and the station ID to the VX_MASK column.

.. code-block:: none

  -mask_grid name
  -mask_poly file
  -mask_sid  file|list

When processing input MPR lines, these options may be used to define a masking grid, polyline, or list of station ID's to filter the matched pair data geographically prior to computing statistics. The **-mask_sid** option is a station ID masking file or a comma-separated list of station ID's for filtering the matched pairs spatially. See the description of the "sid" entry in :numref:`config_options`.

.. code-block:: none

  -out_fcst_thresh thresh
  -out_obs_thresh  thresh
  -out_thresh      thresh
  -out_cnt_logic   string

When processing input MPR lines, these options are used to define the forecast, observation, or both thresholds to be applied when computing statistics. For categorical output line types (FHO, CTC, CTS, MCTC, MCTS) these define the categorical thresholds. For continuous output line types (SL1L2, SAL1L2, CNT), these define the continuous filtering thresholds and **-out_cnt_logic** defines how the forecast and observed logic should be combined.

.. code-block:: none
		
  -out_fcst_wind_thresh thresh
  -out_obs_wind_thresh  thresh
  -out_wind_thresh      thresh
  -out_wind_logic       string

These job command options are analogous to the options listed above but apply when processing input MPR lines and deriving wind direction statistics.

.. code-block:: none

  -out_bin_size value

When processing input ORANK lines and writing output RHIST or PHIST lines, this option defines the output histogram bin width to be used.

stat-analysis tool output
-------------------------

The output generated by the Stat-Analysis tool contains statistics produced by the analysis. It also records information about the analysis job that produced the output for each line. Generally, the output is printed to the screen. However, it can be redirected to an output file using the "**-out**" option. The format of output from each STAT job command is described below.

The "**-by column**" job command option may be used to run the same job multiple times on unique subsets of data. Specify the "**-by column**" option one or more times to define a search key, and that job will be run once for each unique search key found. For example, use "-by VX_MASK" to run the same job for multiple masking regions, and output will be generated for each unique masking region found. Use "-by VX_MASK -by FCST_LEAD" to generate output for each unique combination of masking region and lead time.

Job: filter
^^^^^^^^^^^

This job command finds and filters STAT lines down to those meeting criteria specified by the filter's options. The filtered STAT lines are written to a file specified by the "**-dump_row**" option. 

The output of this job is the same STAT format described in sections :numref:`point_stat-output`, :numref:`grid_stat-output`, and :numref:`wavelet_stat-output`.

Job: summary
^^^^^^^^^^^^

This job produces summary statistics for the column name and line type specified by the "**-column**" and "**-line_type**" options. The output of this job type consists of three lines. The first line contains "**JOB_LIST**", followed by a colon, then the filtering and job definition parameters used for this job. The second line contains "**COL_NAME**", followed by a colon, then the column names for the data in the next line. The third line contains the word "**SUMMARY**", followed by a colon, then the total, mean with confidence intervals, standard deviation with confidence intervals, minimum value, percentiles (10th, 25th, 50th, 75th, and 90th), the maximum value, the interquartile range, the range, and WMO mean information. The output columns are shown in :numref:`Columnar_output` below.

.. _Columnar_output:

.. list-table:: Columnar output of "summary" job output from the Stat-Analysis tool.
  :widths: auto
  :header-rows: 1

  * - Column Number
    - Description 
  * - 1
    - SUMMARY: (job type)
  * - 2
    - Total
  * - 3-7
    - Mean including normal and bootstrap upper and lower confidence limits
  * - 8-10
    - Standard deviation including bootstrap upper and lower confidence limits
  * - 11
    - Minimum value
  * - 12
    - 10th percentile
  * - 13
    - 25th percentile
  * - 14
    - Median (50th percentile)
  * - 15
    - 75th percentile
  * - 16
    - 90th percentile
  * - 17
    - Maximum value
  * - 18
    - Interquartile range (75th - 25th percentile)
  * - 19
    - Range (Maximum - Minimum)
  * - 20
    - WMO Mean type
  * - 21
    - WMO Unweighted Mean value
  * - 22
    - WMO Weighted Mean value

Job: aggregate
^^^^^^^^^^^^^^

This job aggregates output from the STAT line type specified using the "**-line_type**" argument. The output of this job type is in the same format as the line type specified (see :numref:`point_stat-output`, :numref:`grid_stat-output`, and :numref:`wavelet_stat-output`). Again the output consists of three lines. The first line contains "**JOB_LIST**", as described above. The second line contains "**COL_NAME**", followed by a colon, then the column names for the line type selected. The third line contains the name of the line type selected followed by the statistics for that line type.

The STAT line types which may be aggregated in this way are the contingency table (FHO, CTC, PCT, MCTC, NBRCTC), partial sums (SL1L2, SAL1L2, VL1L2, and VAL1L2), and other (ISC, ECNT, RPS, RHIST, PHIST, RELP, NBRCNT, SSVAR, and GRAD) line types.

Job: aggregate_stat
^^^^^^^^^^^^^^^^^^^

This job is similar to the "**aggregate**" job listed above, however the format of its output is determined by the "**-out_line_type**" argument. Again the output consists of three lines for "**JOB_LIST**", "**COL_NAME**", and the name of the output STAT line, as described above. Valid combinations of the "**-line_type**" and "**-out_line_type**" arguments are listed in :numref:`arg_agg_stat_job` below.

.. _arg_agg_stat_job:

.. list-table:: Valid combinations of "-line_type" and "-out_line_type" arguments for the "aggregate_stat" job.
  :widths: auto
  :header-rows: 1

  * - Input Line Type
    - Output Line Type
  * - FHO or CTC
    - CTS
  * - MCTC
    - MCTS
  * - SL1L2 or SAL1L2
    - CNT
  * - VL1L2 or VAL1L2
    - WDIR (wind direction)
  * - PCT
    - PSTD, PJC, PRC
  * - NBRCTC
    - NBRCTS
  * - ORANK
    - RHIST, PHIST, RELP, SSVAR
  * - MPR
    - CNT, SL1L2, SAL1L2, WDIR
  * - MPR
    - FHO, CTC, CTS, MCTC, MCTS, PCT, PSTD, PJC, or PRC  (must specify "**-out_fcst_thresh**" and "**-out_obs_thresh**" arguments)
    
Job: ss_index, go_index, cbs_index
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

While the inputs for the "ss_index", "go_index", and "cbs_index" jobs may vary, the output is the same. By default, the job output is written to the screen or to a "-out" file, if specified. If the "-out_stat" job command option is specified, a STAT output file is written containing the skill score index (SSIDX) output line type.

The SSIDX line type consists of the common STAT header columns described in :numref:`table_PS_header_info_point-stat_out` followed by the columns described below. In general, when multiple input header strings are encountered, the output is reported as a comma-separated list of the unique values. The "-set_hdr" job command option can be used to override any of the output header strings (e.g. "-set_hdr VX_MASK MANY" sets the output VX_MASK column to "MANY"). Special logic applied to some of the STAT header columns are also described below.

..  _table_SA_format_info_SSIDX:

.. list-table:: Format information for the SSIDX (Skill Score Index) output line type.
  :widths: auto
  :header-rows: 2

  * - SSIDX OUTPUT FORMAT
    -
    -
  * - Column Number
    - SSIDX Column Name
    - Description
  * - 4
    - FCST_LEAD
    - Maximum input forecast lead time
  * - 5
    - FCST_VALID_BEG
    - Minimum input forecast valid start time
  * - 6
    - FCST_VALID_END
    - Maximum input forecast valid end time
  * - 7
    - OBS_LEAD
    - Maximum input observation lead time
  * - 8
    - OBS_VALID_BEG
    - Minimum input observation valid start time
  * - 9
    - OBS_VALID_END
    - Maximum input observation valid end time
  * - 10
    - FCST_VAR
    - Skill score index name from the "ss_index_name" option
  * - 11
    - OBS_VAR
    - Skill score index name from the "ss_index_name" option
  * - 24
    - SSIDX
    - Skill score index line type
  * - 25
    - FCST_MODEL
    - Forecast model name
  * - 26
    - REF_MODEL
    - Reference model name
  * - 27
    - N_INIT
    - Number of unique input model initialization times
  * - 28
    - N_TERM
    - Number of skill score index terms
  * - 29
    - N_VLD
    - Number of terms for which a valid skill score was computed
  * - 30
    - SS_INDEX
    - Skill score index value

Job: ramp
^^^^^^^^^

The ramp job operates on a time-series of forecast and observed values and is analogous to the RIRW (Rapid Intensification and Weakening) job described in :numref:`tc_stat-output`. The amount of change from one time to the next is computed for forecast and observed values. Those changes are thresholded to define events which are used to populate a 2x2 contingency table.

See :numref:`config_options` for a detailed description of the job command options available for ramp job type.

The default output for this job is contingency table counts and statistics (**-out_line_type CTC,CTS**). Matched pair information may also be output by requesting MPR output (**-out_line_type CTC,CTS,MPR**).

.. [1] The GO Index is a summary measure for NWP models that is used by the US Air Force. It combines verification statistics for several forecast variables and lead times.
