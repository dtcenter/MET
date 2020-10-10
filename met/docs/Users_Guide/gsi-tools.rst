.. _gsi_tools:

GSI Tools
=========

Gridpoint Statistical Interpolation (GSI) diagnostic files are binary files written out from the data assimilation code before the first and after each outer loop. The files contain useful information about how a single observation was used in the analysis by providing details such as the innovation (O-B), observation values, observation error, adjusted observation error, and quality control information.

For more detail on generating GSI diagnostic files and their contents, see the `GSI User's Guide. <http://www.dtcenter.org/com-GSI/users/docs/index.php>`_

When MET reads GSI diagnostic files, the innovation (O-B; generated prior to the first outer loop) or analysis increment (O-A; generated after the final outer loop) is split into separate values for the observation (OBS) and the forecast (FCST), where the forecast value corresponds to the background (O-B) or analysis (O-A).

MET includes two tools for processing GSI diagnostic files. The gsid2mpr tool reformats individual GSI diagnostic files into the MET matched pair (MPR) format, similar to the output of the Point-Stat tool. The gsidens2orank tool processes an ensemble of GSI diagnostic files and reformats them into the MET observation rank (ORANK) line type, similar to the output of the Ensemble-Stat tool. The output of both tools may be passed to the Stat-Analysis tool to compute a wide variety of continuous, categorical, and ensemble statistics.

GSID2MPR tool
_____________

This section describes how to run the tool gsid2mpr tool. The gsid2mpr tool reformats one or more GSI diagnostic files into an ASCII matched pair (MPR) format, similar to the MPR output of the Point-Stat tool. The output MPR data may be passed to the Stat-Analysis tool to compute a wide variety of continuous or categorical statistics.

gsid2mpr usage
~~~~~~~~~~~~~~

The usage statement for the gsid2mpr tool is shown below:

.. code-block:: none
		
  Usage: gsid2mpr
         gsi_file_1 [gsi_file_2 ... gsi_file_n]
         [-swap]
         [-no_check_dup]
         [-channel n]
         [-set_hdr col_name value]
         [-suffix string]
         [-outdir path]
         [-log file]
         [-v level]

gsid2mpr has one required argument and and accepts several optional ones.

Required arguments for gsid2mpr
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. The **gsi_file_1 [gsi_file2 ... gsi_file_n]** argument indicates the GSI diagnostic files (conventional or radiance) to be reformatted.
   
Optional arguments for gsid2mpr
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

2. The **-swap** option switches the endianness when reading the input binary files.

3. The **-no_check_dup** option disables the checking for duplicate matched pairs which slows down the tool considerably for large files.

4. The **-channel n** option overrides the default processing of all radiance channels with the values of a comma-separated list.

5. The **-set_hdr col_name value** option specifies what should be written to the output header columns.

6. The **-suffix string** option overrides the default output filename suffix (.stat).

7. The **-outdir path** option overrides the default output directory (./).

8. The **-log file** option outputs log messages to the specified file.

9. The **-v level** option overrides the default level of logging (2).

An example of the gsid2mpr calling sequence is shown below:

.. code-block:: none
		
  gsid2mpr diag_conv_ges.mem001 \
  -set_hdr MODEL GSI_MEM001 \
  -outdir out

In this example, the gsid2mpr tool will process a single input file named **diag_conv_ges.mem001** file, set the output **MODEL** header column to **GSI_MEM001**, and write output to the **out** directory. The output file is named the same as the input file but a **.stat** suffix is added to indicate its format.

gsid2mpr output
~~~~~~~~~~~~~~~

The gsid2mpr tool performs a simple reformatting step and thus requires no configuration file. It can read both conventional and radiance binary GSI diagnostic files. Support for additional GSI diagnostic file type may be added in future releases. Conventional files are determined by the presence of the string **conv** in the filename. Files that are not conventional are assumed to contain radiance data. Multiple files of either type may be passed in a single call to the gsid2mpr tool. For each input file, an output file will be generated containing the corresponding matched pair data.

The gsid2mpr tool writes the same set of MPR output columns for the conventional and radiance data types. However, it also writes additional columns at the end of the MPR line which depend on the input file type. Those additional columns are described in the following tables.


.. list-table:: Format information for GSI Diagnostic Conventional MPR (Matched Pair) output line type.
  :widths: auto
  :header-rows: 2

  * -  
    - 
    - GSI DIAGNOSTIC CONVENTIONAL MPR OUTPUT FILE
  * - Column Number
    - Column Name
    - Description
  * - 1-37
    - 
    - Standard MPR columns described in :numref:`table_PS_format_info_MPR`.
  * - 38
    - OBS_PRS
    - Model pressure value at the observation height (hPa)
  * - 39
    - OBS_ERR_IN
    - PrepBUFR inverse observation error
  * - 40
    - OBS_ERR_ADJ
    - read_PrepBUFR inverse observation error
  * - 41
    - OBS_ERR_FIN
    - Final inverse observation error
  * - 42
    - PREP_USE
    - read_PrepBUFR usage
  * - 43
    - ANLY_USE
    - Analysis usage (1 for yes, -1 for no)
  * - 44
    - SETUP_QC
    - Setup quality control
  * - 45
    - QC_WGHT
    - Non-linear quality control relative weight


.. role:: raw-html(raw)
    :format: html

.. list-table:: Format information for GSI Diagnostic Radiance MPR (Matched Pair) output line type.
  :widths: auto
  :header-rows: 2

  * - 
    - 
    - GSI DIAGNOSTIC RADIANCE MPR OUTPUT FILE
  * - Column Number
    - Column Name
    - Description
  * - 1-37
    -  
    - Standard MPR columns described in :numref:`table_PS_format_info_MPR`.
  * - 38
    - CHAN_USE
    - Channel used (1 for yes, -1 for no)
  * - 39
    - SCAN_POS
    - Sensor scan position
  * - 40
    - SAT_ZNTH
    - Satellite zenith angle (degrees)
  * - 41
    - SAT_AZMTH
    - Satellite azimuth angle (degrees)
  * - 42
    - SUN_ZNTH
    - Solar zenith angle (degrees)
  * - 43
    - SUN_AZMTH
    - Solar azimuth angle (degrees)
  * - 44
    - SUN_GLNT
    - Sun glint angle (degrees)
  * - 45
    - FRAC_WTR
    - Fractional coverage by water
  * - 46
    - FRAC_LND
    - Fractional coverage by land
  * - 47
    - FRAC_ICE
    - Fractional coverage by ice
  * - 48
    - FRAC_SNW
    - Fractional coverage by snow
  * - 49
    - SFC_TWTR
    - Surface temperature over water (K)
  * - 50
    - SFC_TLND
    - Surface temperature over land (K)
  * - 51
    - SFC_TICE
    - Surface temperature over ice (K)
  * - 52
    - SFC_TSNW
    - Surface temperature over snow (K)
  * - 53
    - TSOIL
    - Soil temperature (K)
  * - 54
    - SOILM
    - Soil moisture
  * - 55
    - LAND_TYPE
    - Surface land type
  * - 56
    - FRAC_VEG
    - Vegetation fraction
  * - 57
    - SNW_DPTH
    - Snow depth
  * - 58
    - SFC_WIND
    - Surface wind speed (m/s)
  * - 59
    - FRAC_CLD  CLD_LWC
    - Cloud fraction (%) :raw-html:`<br />`  Cloud liquid water (kg/m**2) (microwave only)
  * - 60
    - CTOP_PRS   TC_PWAT
    - Cloud top pressure (hPa) :raw-html:`<br />`  Total column precip. water (km/m**2) (microwave only)
  * - 61
    - TFND
    - Foundation temperature: Tr
  * - 62
    - TWARM
    - Diurnal warming: d(Tw) at depth zob
  * - 63
    - TCOOL
    - Sub-layer cooling: d(Tc) at depth zob
  * - 64
    - TZFND
    - d(Tz)/d(Tr)
  * - 65
    - OBS_ERR
    - Inverse observation error
  * - 66
    - FCST_NOBC
    - Brightness temperature with no bias correction (K)
  * - 67
    - SFC_EMIS
    - Surface emissivity
  * - 68
    - STABILITY
    - Stability index
  * - 69
    - PRS_MAX_WGT
    - Pressure of the maximum weighing function

The gsid2mpr output may be passed to the Stat-Analysis tool to derive additional statistics. In particular, users should consider running the **aggregate_stat** job type to read MPR lines and compute partial sums (SL1L2), continuous statistics (CNT), contingency table counts (CTC), or contingency table statistics (CTS). Stat-Analysis has been enhanced to parse any extra columns found at the end of the input lines. Users can filter the values in those extra columns using the **-column_thresh** and **-column_str** job command options.

An example of the Stat-Analysis calling sequence is shown below:

.. code-block:: none

  stat_analysis -lookin diag_conv_ges.mem001.stat \
  -job aggregate_stat -line_type MPR -out_line_type CNT \
  -fcst_var t -column_thresh ANLY_USE eq1

In this example, the Stat-Analysis tool will read MPR lines from the input file named **diag_conv_ges.mem001.stat**, retain only those lines where the **FCST_VAR** column indicates temperature (**t**) and where the **ANLY_USE** column has a value of 1.0, and derive continuous statistics.

GSIDENS2ORANK tool
__________________

This section describes how to run the gsidens2orank tool. The gsidens2orank tool processes an ensemble of GSI diagnostic files and reformats them into the MET observation rank (ORANK) line type, similar to the output of the Ensemble-Stat tool. The ORANK line type contains ensemble matched pair information and is analogous to the MPR line type for a deterministic model. The output ORANK data may be passed to the Stat-Analysis tool to compute ensemble statistics.

gsidens2orank usage
~~~~~~~~~~~~~~~~~~~

The usage statement for the gsidens2orank tool is shown below:

.. code-block:: none
		
  Usage: gsidens2orank
         ens_file_1 ... ens_file_n | ens_file_list
         -out path
         [-ens_mean path]
         [-swap]
         [-rng_name str]
         [-rng_seed str]
         [-set_hdr col_name value]
         [-log file]
         [-v level]

gsidens2orank has three required arguments and accepts several optional ones.

Required arguments for gsidens2orank
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. The **ens_file_1 ... ens_file_n** argument is a list of ensemble binary GSI diagnostic files to be reformatted.

2. The **ens_file_list** argument is an ASCII file containing a list of ensemble GSI diagnostic files.

3. The **-out path** argument specifies the name of the output **.stat** file.

Optional arguments for gsidens2orank
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

4. The **-ens_mean path** option is the ensemble mean binary GSI diagnostic file.

5. The **-swap** option switches the endianness when reading the input binary files.

6. The **-channel n** option overrides the default processing of all radiance channels with a comma-separated list.

7. The **-rng_name str** option overrides the default random number generator name (mt19937).

8. The **-rng_seed str** option overrides the default random number generator seed.

9. The **-set_hdr col_name value** option specifies what should be written to the output header columns.

10. The **-log file** option outputs log messages to the specified file.

11. The **-v level** option overrides the default level of logging (2).

An example of the gsidens2orank calling sequence is shown below:

.. code-block:: none

  gsidens2orank diag_conv_ges.mem* \
  -ens_mean diag_conv_ges.ensmean \
  -out diag_conv_ges_ens_mean_orank.txt

In this example, the gsidens2orank tool will process all of the ensemble members whose file name **matches diag_conv_ges.mem\*,** write output to the file named **diag_conv_ges_ens_mean_orank.txt**, and populate the output **ENS_MEAN** column with the values found in the **diag_conv_ges.ensmean** file rather than computing the ensemble mean values from the ensemble members on the fly.

gsidens2orank output
~~~~~~~~~~~~~~~~~~~~

The gsidens2orank tool performs a simple reformatting step and thus requires no configuration file. The multiple files passed to it are interpreted as members of the same ensemble. Therefore, each call to the tool processes exactly one ensemble. All input ensemble GSI diagnostic files must be of the same type. Mixing conventional and radiance files together will result in a runtime error. The gsidens2orank tool processes each ensemble member and keeps track of the observations it encounters. It constructs a list of the ensemble values corresponding to each observation and writes an output ORANK line listing the observation value, its rank, and all the ensemble values. The random number generator is used by the gsidens2orank tool to randomly assign a rank value in the case of ties.

The gsid2mpr tool writes the same set of ORANK output columns for the conventional and radiance data types. However, it also writes additional columns at the end of the ORANK line which depend on the input file type. The extra columns are limited to quantities which remain constant over all the ensemble members and are therefore largely a subset of the extra columns written by the gsid2mpr tool. Those additional columns are described in the following tables.

.. list-table:: Format information for GSI Diagnostic Conventional ORANK (Observation Rank) output line type.
  :widths: auto
  :header-rows: 2

  * - 
    - 
    - GSI DIAGNOSTIC CONVENTIONAL ORANK OUTPUT FILE
  * - Column Number
    - Column Name
    - Description
  * - 1-?
    -  
    - Standard ORANK columns described in :numref:`table_ES_header_info_es_out_ORANK`.
  * - Last-2
    - N_USE
    - Number of members with ANLY_USE = 1
  * - Last-1
    - PREP_USE
    - read_PrepBUFR usage
  * - Last
    - SETUP_QC
    - Setup quality control


.. list-table:: Format information for GSI Diagnostic Radiance ORANK (Observation Rank) output line type.
  :widths: auto
  :header-rows: 2

  * - 
    - 
    - GSI DIAGNOSTIC RADIANCE ORANK OUTPUT FILE
  * - Column Number
    - Column Name
    - Description
  * - 1-?
    -  
    - Standard ORANK columns described in :numref:`table_ES_header_info_es_out_ORANK`.
  * - Last-24
    - N_USE
    - Number of members with OBS_QC = 0
  * - Last-23
    - CHAN_USE
    - Channel used (1 for yes, -1 for no)
  * - Last-22
    - SCAN_POS
    - Sensor scan position
  * - Last-21
    - SAT_ZNTH
    - Satellite zenith angle (degrees)
  * - Last-20
    - SAT_AZMTH
    - Satellite azimuth angle (degrees)
  * - Last-19
    - SUN_ZNTH
    - Solar zenith angle (degrees)
  * - Last-18
    - SUN_AZMTH
    - Solar azimuth angle (degrees)
  * - Last-17
    - SUN_GLNT
    - Sun glint angle (degrees)
  * - Last-16
    - FRAC_WTR
    - Fractional coverage by water
  * - Last-15
    - FRAC_LND
    - Fractional coverage by land
  * - Last-14
    - FRAC_ICE
    - Fractional coverage by ice
  * - Last-13
    - FRAC_SNW
    - Fractional coverage by snow
  * - Last-12
    - SFC_TWTR
    - Surface temperature over water (K)
  * - Last-11
    - SFC_TLND
    - Surface temperature over land (K)
  * - Last-10
    - SFC_TICE
    - Surface temperature over ice (K)
  * - Last-9
    - SFC_TSNW
    - Surface temperature over snow (K)
  * - Last-8
    - TSOIL
    - Soil temperature (K)
  * - Last-7
    - SOILM
    - Soil moisture
  * - Last-6
    - LAND_TYPE
    - Surface land type
  * - Last-5
    - FRAC_VEG
    - Vegetation fraction
  * - Last-4
    - SNW_DPTH
    - Snow depth
  * - Last-3
    - TFND
    - Foundation temperature: Tr
  * - Last-2
    - TWARM
    - Diurnal warming: d(Tw) at depth zob
  * - Last-1
    - TCOOL
    - Sub-layer cooling: d(Tc) at depth zob
  * - Last
    - TZFND
    - d(Tz)/d(Tr)

The gsidens2orank output may be passed to the Stat-Analysis tool to derive additional statistics. In particular, users should consider running the **aggregate_stat** job type to read ORANK lines and ranked histograms (RHIST), probability integral transform histograms (PHIST), and spread-skill variance output (SSVAR). Stat-Analysis has been enhanced to parse any extra columns found at the end of the input lines. Users can filter the values in those extra columns using the **-column_thresh** and **-column_str** job command options.

An example of the Stat-Analysis calling sequence is shown below:

.. code-block:: none

  stat_analysis -lookin diag_conv_ges_ens_mean_orank.txt \
  -job aggregate_stat -line_type ORANK -out_line_type RHIST \
  -by fcst_var -column_thresh N_USE eq20

In this example, the Stat-Analysis tool will read ORANK lines from **diag_conv_ges_ens_mean_orank.txt**, retain only those lines where the **N_USE** column indicates that all 20 ensemble members were used, and write ranked histogram (RHIST) output lines for each unique value of encountered in the **FCST_VAR** column.
