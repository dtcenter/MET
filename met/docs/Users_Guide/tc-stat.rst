.. _tc-stat:

TC-Stat Tool
============

Introduction
____________

The TC-Stat tool ties together results from the TC-Pairs tool by providing summary statistics and filtering jobs on TCST output files. The TC-Stat tool requires TCST output from the TC-Pairs tool. See :numref:`tc_stat-output` of this user's guide for information on the TCST output format of the TC-Pairs tool. The TC-Stat tool supports several analysis job types. The **filter** job stratifies the TCST data using various conditions and thresholds described in :numref:`tc_stat-configuration-file`. The **summary** job produces summary statistics including frequency of superior performance, time-series independence calculations, and confidence intervals on the mean. The **rirw** job processes TCMPR lines, identifies adeck and bdeck rapid intensification or weakening events, populates a 2x2 contingency table, and derives contingency table statistics. The **probrirw job** processes PROBRIRW lines, populates an Nx2 probabilistic contingency table, and derives probabilistic statistics. The statistical aspects are described in :numref:`Statistical-aspects`, and practical use information for the TC-Stat tool is described in :numref:`Practical-information-1`.

.. _Statistical-aspects:

Statistical aspects
___________________

Filter TCST lines
~~~~~~~~~~~~~~~~~

The TC-Stat tool can be used to simply filter specific lines of the TCST file based on user-defined filtering criteria. All of the TCST lines that are retained from one or more files are written out to a single output file. The output file is also in TCST format.

Filtering options are outlined below in :numref:`tc_stat-configuration-file` (configuration file). If multiple filtering options are listed, the job will be performed on their intersection.

Summary statistics for columns
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The TC-Stat tool can be used to produce summary information for a single column of data. After the user specifies the specific column of interest, and any other relevant search criteria, summary information is produced from values in that column of data. The summary statistics produced are listed in :numref:`table_columnar_output_summary_tc_stat`.

Confidence intervals are computed for the mean of the column of data. Confidence intervals are computed using the assumption of normality for the mean. For further information on computing confidence intervals, refer to :numref:`App_D-Confidence-Intervals` of the MET user's guide.

When operating on columns, a specific column name can be listed (e.g. TK_ERR), as well as the differences of two columns (e.g. AMAX_WIND-BMAX_WIND), and the absolute difference of the column(s) (e.g. abs(AMAX_WIND-BMAX_WIND)). Additionally, several shortcuts can be applied to choose multiple columns with a single entry. Shortcut options for the -column entry are as follows:

**TRACK:** track error (TK_ERR), along-track error (ALTK_ERR), and cross-track error (CRTK_ERR)

**WIND:** all wind radii errors (34-, 50-, and 64-kt) for each quadrant

**TI:** track error (TK_ERR) and absolute intensity error (abs(AMAX_WIND-BMAX_WIND))

**AC:** along- and cross-track errors (ALTK_ERR, CRTK_ERR)

**XY:** X- and Y-component track errors (X_ERR, Y_ERR)

The TC-Stat tool can also be used to generate frequency of superior performance and the time to independence calculations when using the TC-Stat summary job.

Frequency of Superior Performance
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The frequency of superior performance (FSP) looks at multiple model forecasts (adecks), and ranks each model relative to other model performance for the column specified in the summary job. The summary job output lists the total number of cases included in the FSP, the number of cases where the model of interest is the best (e.g.: lowest track error), the number of ties between the models, and the FSP (percent). Ties are not included in the FSP percentage; therefore the percentage may not equal 100%.

Time-Series Independence
^^^^^^^^^^^^^^^^^^^^^^^^

The time-series independence evaluates effective forecast separation time using the Siegel method, by comparing the number of runs above and below the mean error to an expected value. This calculation expects the columns in the summary job to be a time series. The output includes the forecast hour interval and the number of hours to independence.

Rapid Intensification/Weakening
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The TC-Stat tool can be used to read TCMPR lines and compare the occurrence of rapid intensification (i.e. increase in intensity) or weakening (i.e. decrease in intensity) between the adeck and bdeck. The rapid intensification or weakening is defined by the change of maximum wind speed (i.e. **AMAX_WIND** and **BMAX_WIND** columns) over a specified amount of time. Accurately forecasting large changes in intensity is a challenging problem and this job helps quantify a model's ability to do so.

Users may specify several job command options to configure the behavior of this job. Using these configurable options, the TC-Stat tool analyzes paired tracks and for each track point (i.e. each TCMPR line) determines whether rapid intensification or weakening occurred. For each point in time, it uses the forecast and BEST track event occurrence to populate a 2x2 contingency table. The job may be configured to require that forecast and BEST track events occur at exactly the same time to be considered a hit. Alternatively, the job may be configured to define a hit as long as the forecast and BEST track events occurred within a configurable time window. Using this relaxed matching criteria false alarms may be considered hits and misses may be considered correct negatives as long as the adeck and bdeck events were close enough in time. Each rirw job applies a single intensity change threshold. Therefore, assessing a model's performance with rapid intensification and weakening requires that two separate jobs be run.

Probability of Rapid Intensification
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The TC-Stat tool can be used to accumulate multiple PROBRIRW lines and derive probabilistic statistics summarizing performance. The PROBRIRW line contains a probabilistic forecast for a specified intensity change along with the actual intensity change that occurred in the BEST track. Accurately forecasting the likelihood of large changes in intensity is a challenging problem and this job helps quantify a model's ability to do so.

Users may specify several job command options to configure the behavior of this job. The TC-Stat tools reads the input PROBI lines, applies the configurable options to extract a forecast probability value and BEST track event, and bins those probabilistic pairs into an Nx2 contingency table. This job writes up to four probabilistic output line types summarizing the performance.

.. _Practical-information-1:

Practical information
_____________________

The following sections describe the usage statement, required arguments, and optional arguments for **tc_stat**.

tc_stat usage
~~~~~~~~~~~~~

The usage statement for **tc_stat** is shown below:

.. code-block:: none

  Usage: tc_stat
         -lookin source
         [-out file]
         [-log file]
         [-v level]
         [-config file] | [JOB COMMAND LINE]

TC-Stat has one required argument and accepts optional ones. 

The usage statement for the TC-Stat tool includes the "job" term, which refers to the set of tasks to be performed after applying user-specified filtering options. The filtering options are used to pare down the TC-Pairs output to only those lines that are desired for the analysis. The job and its filters together comprise a "job command line". The "job command line" may be specified either on the command line to run a single analysis job or within the configuration file to run multiple analysis jobs at the same time. If jobs are specified in both the configuration file and the command line, only the jobs indicated in the configuration file will be run. The various jobs are described in :numref:`table_columnar_output_summary_tc_stat` and the filtering options are described in :numref:`tc_stat-configuration-file`.

Required arguments for tc_stat
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. The **-lookin source** argument indicates the location of the input TCST files generated from **tc_pairs**. This argument can be used one or more times to specify the name of a TCST file or top-level directory containing TCST files to be processed. Multiple tcst files may be specified by using a wild card (\*).

2. Either a configuration file must be specified with the **-config** option, or a **JOB COMMAND LINE** must be denoted. The **JOB COMMAND LINE** options are described in :numref:`tc_stat-configuration-file`.

Optional arguments for tc_stat
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

3. The **-out file** argument indicates the desired name of the TCST format output file.

4. The **-log file** option directs output and errors to the specified log file. All messages will be written to that file as well as standard out and error. Thus, users can save the messages without having to redirect the output on the command line. The default behavior is no log file. 

5. The **-v level** option indicates the desired level of verbosity. The contents of “level” will override the default setting of 2. Setting the verbosity to 0 will make the tool run with no log messages, while increasing the verbosity above 1 will increase the amount of logging.

6. The **-config file** argument indicates the name of the configuration file to be used. The contents of the configuration file are discussed below.

An example of the **tc_stat** calling sequence is shown below:

.. code-block:: none

  tc_stat -lookin /home/tc_pairs/*al092010.tcst -config TCStatConfig

In this example, the TC-Stat tool uses any TCST file (output from **tc_pairs**) in the listed directory for the 9th Atlantic Basin storm in 2010. Filtering options and aggregated statistics are generated following configuration options specified in the **TCStatConfig** file. Further, using flags (e.g. **-basin, -column, -storm_name,** etc...) option within the job command lines may further refine these selections. See :numref:`tc_stat-configuration-file` for options available for the job command line and :numref:`Data IO MET-TC Configuration File Options` for how to use them.

.. _tc_stat-configuration-file:

tc_stat configuration file
^^^^^^^^^^^^^^^^^^^^^^^^^^

The default configuration file for the **TC-Stat** tool named **TCStatConfig_default** can be found in the installed **share/met/config** directory. Like the other configuration files described in this document, it is recommended that users make a copy of these files prior to modifying their contents.

The contents of the tc_stat configuration file are described below.

_________________________

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
  lead_req     = [];
  init_mask    = [];
  valid_mask   = [];
  match_points = TRUE;
  version      = "VN.N";

The configuration options listed above are common to many MET tools and are described in :numref:`Data IO MET-TC Configuration File Options`.

Note that the options specified in the first section of the configuration file, prior to the job list, will be applied to every job specified in the joblist. However, if an individual job specifies an option listed above, it will be applied to that job. For example, if model = [ **"GFSI", "LGEM", "DSHP"** ]; is set at the top, but the job in the joblist sets the -model option to "**LGEM**", that job will only run using the LGEM model data.

_________________________

.. code-block:: none

  amodel = [];
  bmodel = [];

The **amodel** and **bmodel** fields stratify by the amodel and bmodel columns based on a comma-separated list of model names used for all analysis performed. The names must be in double quotation marks (e.g.: "HWFI"). The **amodel** list specifies the model to be verified against the listed bmodel. The **bmodel** specifies the reference dataset, generally the BEST track analysis. Using the **-amodel** and **-bmodel** options within the job command lines may further refine these selections.

_________________________

.. code-block:: none

  valid_inc = [];
  valid_exc = [];

The **valid_inc** and **valid_exc** fields stratify by valid times, based on a comma-separated list of specific valid times to include (inc) or exclude (exc). Time strings are defined by YYYYMMDD[_HH[MMSS]]. Using the **-valid_inc** and **-valid_exc** options within the job command lines may further refine these selections.

_________________________

.. code-block:: none

  valid_hour = [];
  lead       = [];

The **valid_hour**, and **lead** fields stratify by the initialization time, valid time, and lead time, respectively. This field specifies a comma-separated list of initialization times, valid times, and lead times in **HH[MMSS]** format. Using the **-valid_hour** and **-lead** options within the job command lines may further refine these selections.

_________________________

.. code-block:: none

  line_type = [];

The **line_type** field stratifies by the line_type column.

_________________________

.. code-block:: none

  track_watch_warn = [];

The **track_watch_warn** flag stratifies over the watch_warn column in the TCST files. If any of the watch/warning statuses are present in a forecast track, the entire track is verified. The value "ALL" matches HUWARN, HUWATCH, TSWARN, TSWATCH. Using the **-track_watch_warn** option within the job command lines may further refine these selections.

Other uses of the WATCH_WARN column include filtering when:

1. A forecast is issued when a watch/warning is in effect

2. A forecast is verifying when a watch/warning is in effect

3. A forecast is issued when a watch/warning is NOT in effect

4. A forecast is verified when a watch/warning is NOT in effect

The following filtering options can be achieved by the following:

Option 1. A forecast is issued when a watch/warning is in effect

.. code-block:: none
		
  init_str_name = ["WATCH_WARN"];
  init_str_val = ["ALL"];
 
Option 2. A forecast is verifying when a watch/warning is in effect

.. code-block:: none

  column_str_name = ["WATCH_WARN"];  
  column_str_val = ["ALL"];
   
Option 3. A forecast is issued when a watch/warning is NOT in effect

.. code-block:: none

  init_str_name = ["WATCH_WARN"];  
  init_str_val = ["NA"];
    
Option 4. A forecast is verified when a watch/warning is NOT in effect

.. code-block:: none

  column_str_name = ["WATCH_WARN"];
  column_str_val = ["NA"];
    
Further information on the **column_str** and **init_str** fields are described below. Listing a comma-separated list of watch/warning types in the **column_str_val** field will be stratified by a single or multiple types of warnings.

_________________________

.. code-block:: none

  column_thresh_name = [];
  column_thresh_val  = [];

The **column_thresh_name** and **column_thresh_val** fields stratify by applying thresholds to numeric data columns. Specify a comma-separated list of column names and thresholds to be applied. The length of **column_thresh_val** should match that of **column_thresh_name**. Using the **-column_thresh name thresh** option within the job command lines may further refine these selections.

_________________________

.. code-block:: none

  column_str_name = [];
  column_str_val  = [];

The **column_str_name** and **column_str_val** fields stratify by performing string matching on non-numeric data columns. Specify a comma-separated list of columns names and values to be checked. The length of the **column_str_val** should match that of the **column_str_name**. Using the **-column_str name val** option within the job command lines may further refine these selections.

_________________________

.. code-block:: none

  init_thresh_name = [];
  init_thresh_val  = [];

The **init_thresh_name** and **init_thresh_val** fields stratify by applying thresholds to numeric data columns only when lead = 0. If lead =0, but the value does not meet the threshold, discard the entire track. The length of the **init_thresh_val** should match that of the **init_thresh_name**. Using the **-init_thresh name val** option within the job command lines may further refine these selections.

_________________________

.. code-block:: none

  init_str_name = [];
  init_str_val  = [];

The **init_str_name** and **init_str_val** fields stratify by performing string matching on non-numeric data columns only when lead = 0. If lead =0, but the string does not match, discard the entire track. The length of the **init_str_val** should match that of the **init_str_name**. Using the **-init_str name val** option within the job command lines may further refine these selections.

_________________________

.. code-block:: none

  water_only = FALSE;

The **water_only** flag stratifies by only using points where both the amodel and bmodel tracks are over water. When **water_only = TRUE;** once land is encountered the remainder of the forecast track is not used for the verification, even if the track moves back over water.

_________________________

.. code-block:: none

  rirw = {
     track  = NONE;
     time   = "24";
     exact  = TRUE;
     thresh = >=30.0;
  }

The **rirw** field specifies those track points for which rapid intensification (RI) or rapid weakening (RW) occurred, based on user defined RI/RW thresholds. The **track** entry specifies that RI/RW is not turned on **(NONE)**, is computed based on the bmodel only **(BDECK)**, is computed based on the amodel only **(ADECK)**, or computed when both the amodel and bmodel (the union of the two) indicate RI/RW (BOTH). If **track** is set to **ADECK, BDECK**, or **BOTH**, only tracks exhibiting rapid intensification will be retained. Rapid intensification is officially defined as when the change in the maximum wind speed over a 24-hour period is greater than or equal to 30 kts. This is the default setting, however flexibility in this definition is provided through the use of the **time, exact** and **thresh** options. The **time** field specifies the time window (HH[MMSS] format) for which the RI/RW occurred. The **exact** field specifies whether to only count RI/RW when the intensity change is over the exact time window (TRUE), which follows the official RI definition, or if the intensity threshold is met anytime during the time window (FALSE). Finally, the **thresh** field specifies the user defined intensity threshold (where ">=" indicates RI, and "<=" indicates RW). 

Using the **-rirw_track, -rirw_time_adeck, -rirw_time_bdeck, -rirw_exact_adeck, -rirw_exact_bdeck, -rirw_thresh_adeck, -rirw_thresh_bdeck** options within the job command lines may further refine these selections. See README_TC in data/config for how to use these options.

_________________________

.. code-block:: none

  landfall     = FALSE;
  landfall_beg = "-24";
  landfall_end = "00";

The **landfall, landfall_beg**, and **landfall_end** fields specify whether only those track points occurring near landfall should be retained. The landfall retention window is defined as the hours offset from the time of landfall. Landfall is defined as the last bmodel track point before the distance to land switches from water to land. When **landfall_end** is set to zero, the track is retained from the **landfall_beg** to the time of landfall. Using the **-landfall_window** option with the job command lines may further refine these selections. The **-landfall_window** job command option takes one or two arguments in HH[MMSS] format. Use one argument to define a symmetric time window. For example, **-landfall_window 06** defines the time window +/- six hours around the landfall time. Use two arguments to define an asymmetric time window. For example, **-landfall_window 00 12** defines the time window from the landfall event to twelve hours after. 

_________________________

.. code-block:: none

  event_equal = FALSE;

The **event_equal** flag specifies whether only those track points common to all models in the dataset should be retained. The event equalization is performed only using cases common to all listed amodel entries. A case is defined by comparing the following columns in the TCST files: BMODEL, BASIN, CYCLONE, INIT, LEAD, VALID. This option may be modified using the **-event_equal** option within the job command lines.

_________________________

.. code-block:: none

  event_equal_lead = [];

The **event_equal_lead** flag specifies lead times that must be present for a track to be included in the event equalization logic. The event equalization is performed only using cases common to all lead times listed, enabling the verification at each lead time to be performed on a consistent dataset. This option may be modified using the **-event_equal_lead** option within the job command lines.

_________________________

.. code-block:: none

  out_init_mask = "";

The **out_init_mask** field applies polyline masking logic to the location of the amodel track at the initialization time. If the track point falls outside the mask, discard the entire track. This option may be modified using the **-out_init_mask** option within the job command lines.

_________________________

.. code-block:: none

  out_valid_mask = "";

The **out_valid_mask** field applies polyline masking logic to the location of the amodel track at the valid time. If the track point falls outside the mask, discard the entire track. This option may be modified using the **-out_valid_mask** option within the job command lines.

_________________________

.. code-block:: none

  jobs = [];

The user may specify one or more analysis jobs to be performed on the TCST lines that remain after applying the filtering parameters listed above. Each entry in the joblist contains the task and additional filtering options for a single analysis to be performed. There are three types of jobs available including *filter, summary, and rirw.* Please refer to the README_TC in data/config for details on how to call each job. The format for an analysis job is as follows:

_________________________

.. code-block:: none

  -job job_name REQUIRED and OPTIONAL ARGUMENTS
  
  e.g.: -job filter  -line_type TCMPR  -amodel HWFI   -dump_row ./tc_filter_job.tcst
        -job summary -line_type TCMPR  -column TK_ERR -dump_row ./tc_summary_job.tcst
        -job rirw    -line_type TCMPR  -rirw_time 24 -rirw_exact false -rirw_thresh ge20
        -job probrirw -line_type PROBRIRW -column_thresh RI_WINDOW ==24 \
                      -probri_thresh 30 -probri_prob_thresh ==0.25

.. _tc_stat-output:

tc_stat output
~~~~~~~~~~~~~~

The output generated from the TC-Stat tool contains statistics produced by the analysis. Additionally, it includes information about the analysis job that produced the output for each line. The output can be redirected to an output file using the **-out** option. The format of output from each **tc_stat** job command is listed below.

**Job: Filter**

This job command finds and filters TCST lines down to those meeting the criteria selected by the filter's options. The filtered TCST lines are written to a file specified by the **-dump_row** option. The TCST output from this job follows the TCST output description in :numref:`tc-dland` and :numref:`tc-pairs`.

**Job: Summary**

This job produces summary statistics for the column name specified by the **-column** option. The output of the summary job consists of three rows:

1.
"JOB_LIST", which shows the job definition parameters used for this job;

2.
"COL_NAME", followed by the summary statistics that are applied;

3.
“SUMMARY”, which is followed by the total, mean (with confidence intervals), standard deviation, minimum value, percentiles (10th, 25th, 50th, 75th, 90th), maximum value, interquartile range, range, sum, time to independence, and frequency of superior performance.

The output columns are shown below in :numref:`table_columnar_output_summary_tc_stat` The **-by** option can also be used one or more times to make this job more powerful. Rather than running the specified job once, it will be run once for each unique combination of the entries found in the column(s) specified with the **-by** option. 

.. _table_columnar_output_summary_tc_stat:

.. list-table:: Columnar output of “summary” job output from the TC-Stat tool.
  :widths: auto
  :header-rows: 2


  * - 
    - tc_stat Summary Job Output Options
  * - Column number
    - Description
  * - 1
    - SUMMARY: (job type)
  * - 2
    - Column (dependent parameter)
  * - 3
    - Case (storm + valid time)
  * - 4
    - Total
  * - 5
    - Valid
  * - 6-8
    - Mean, including normal upper and lower confidence limits
  * - 9
    - Standard deviation
  * - 10
    - Minimum value
  * - 11-15
    - Percentiles (10th, 25th, 50th, 75th, 90th)
  * - 16
    - Maximum Value
  * - 17
    - Interquartile range (75th - 25th percentile)
  * - 18
    - Range (Maximum - Minimum)
  * - 19
    - Sum
  * - 20-21
    - Independence time
  * - 22-25
    - Frequency of superior performance

**Job: RIRW**

The RIRW job produces contingency table counts and statistics defined by identifying rapid intensification or weakening events in the adeck and bdeck track. Users may specify several job command options to configure the behavior of this job:

• The **-rirw_time HH[MMSS]** option (or **-rirw_time_adeck** and **-rirw_time_bdeck** to specify different settings) defines the time window of interest. The default is 24 hours.

• The **-rirw_exact bool** option (or **-rirw_exact_adeck** and **-rirw_exact_bdeck** to specify different settings) is a boolean defining whether the exact intensity change or maximum intensity change over that time window should be used. For rapid intensification, the maximum increase is computed. For rapid weakening, the maximum decrease is used. The default is true.

• The **-rirw_thresh threshold** option (or **-rirw_thresh_adeck** and **-rirw_thresh_bdeck** to specify different settings) defines the intensity change event threshold. The default is greater than or equal to 30 kts.

• The **-rirw_window** option may be passed one or two arguments in HH[MMSS] format to define how close adeck and bdeck events must be to be considered hits or correct negatives. One time string defines a symmetric time window while two time strings define an asymmetric time window. The default is 0, requiring an exact match in time.

• The **-out_line_type** option defines the output data that should be written. This job can write contingency table counts (CTC), contingency table statistics (CTS), and RIRW matched pairs (MPR). The default is CTC and CTS, but the MPR output provides a great amount of detail.

Users may also specify the **-out_alpha** option to define the alpha value for the confidence intervals in the CTS output line type. In addition, the **-by column_name** option is a convenient way of running the same job across multiple stratifications of data. For example, **-by AMODEL** runs the same job for each unique AMODEL name in the data.

**Job: PROBRIRW**

The PROBRIRW job produces probabilistic contingency table counts and statistics defined by placing forecast probabilities and BEST track rapid intensification events into an Nx2 contingency table. Users may specify several job command options to configure the behavior of this job:

• The **-prob_thresh n** option is required and defines which probability threshold should be evaluated. It determines which **PROB_i** column from the PROBRIRW line type is selected for the job. For example, use **-prob_thresh 30** to evaluate forecast probabilities of a 30 kt increase or use **-prob_thresh -30** to evaluate forecast probabilities of a 30 kt decrease in intensity. The default is a 30 kt increase.

• The **-prob_exact bool** option is a boolean defining whether the exact or maximum BEST track intensity change over the time window should be used. If true, the values in the **BDELTA** column are used. If false, the values in the **BDELTA_MAX** column are used. The default is true.

• The **-probri_bdelta_thresh** threshold option defines the BEST track intensity change event threshold. This should typically be set consistent with the probability threshold (**-prob_thresh**) chosen above. The default is greater than or equal to 30 kts.

• The **-probri_prob_thresh threshold_list** option defines the probability thresholds used to create the output Nx2 contingency table. The default is probability bins of width 0.1. These probabilities may be specified as a list (>0.00,>0.25,>0.50,>0.75,>1.00) or using shorthand notation (==0.25) for bins of equal width.

• The **-out_line_type** option defines the output data that should be written. This job can write PCT, PSTD, PJC, and PRC output line types. The default is PCT and PSTD.  Please see :numref:`table_PS_format_info_PCT` through :numref:`table_PS_format_info_PRC` for more details.

Users may also specify the **-out_alpha** option to define the alpha value for the confidence intervals in the PSTD output line type. Multiple values in the **RI_WINDOW** column cannot be combined in a single PROBRIRW job since the BEST track intensity threshold should change for each. Using the **-by RI_WINDOW** option or **-column_thresh RI_WINDOW ==24** option provide convenient ways avoiding this problem.

Users should note that for the PROBRIRW line type, **PROBRI_PROB** is a derived column name. The -probri_thresh option defines the probabilities of interest (e.g. **-probri_thresh 30**) and the **PROBRI_PROB** column name refers to those probability values, regardless of their column number. For example, the job command options **-probri_thresh 30 -column_thresh PROBRI_PROB >0** select 30 kt probabilities and match probability values greater than 0.
