.. _ensemble-stat:

******************
Ensemble-Stat Tool
******************

Introduction
============

The Ensemble-Stat tool verifies deterministic ensemble members against gridded and/or point observations. It computes ensemble statistics such as rank histograms, probability integral transform histograms, spread/skill variance, relative position and continuous ranked probability score. Climatological mean and standard deviation data may also be provided, and is used as a reference forecast in several of the output statistics. Finally, observation error perturbations can be included prior to calculation of statistics. Details about and equations for the statistics produced for ensembles are given in :numref:`Appendix C, Section %s <App_C-ensemble>`.

.. note:: Earlier versions of the Ensemble-Stat tool supported both ensemble product generation and ensemble verification. However, the ensemble product generation logic has moved to the :ref:`Gen-Ens-Prod Tool<gen-ens-prod>`, which replaces and extends that functionality. Ensemble product generation was removed from Ensemble-Stat in version 11.0.0.

Scientific and Statistical Aspects
==================================

.. _ES_HiRA_framework:

HiRA Framework
--------------

The HiRA framework described in :numref:`PS_HiRA_framework` is also supported in the Ensemble-Stat tool. That support is provided as an interpolation option via the **interp** dictionary. The interpolation dictionary defines how gridded model data is matched to each observation value. Most interpolation methods, such as **UW_MEAN** for the unweighted mean or **BILIN** for bilinear, compute a single value for each ensemble member. When the High Resolution Assessment (HiRA) interpolation method is chosen, as shown below, all of the nearby neighborhood points surrounding each observation from each member are used. Therefore, processing an N-member ensemble using a HiRA neighborhood of size M produces ensemble output with size N*M. This approach fully leverages information from all nearby grid points to evaluate the ensemble quality.

.. code ::

  interp = {
    field      = BOTH;
    vld_thresh = 1.0;
    shape      = SQUARE;

    type = [
       {
          method = HIRA;
          width  = 2;
          shape  = SQUARE;
       }
    ];
  }

In this example, all four grid points of the 2x2 square surrounding each observation point are used to define the ensemble. Therefore, an N-member ensemble is evaluated as an ensemble of size Nx4.

Ensemble Statistics
-------------------

Rank histograms and probability integral transform (PIT) histograms are used to determine if the distribution of ensemble values is the same as the distribution of observed values for any forecast field (:ref:`Hamill, 2001 <Hamill-2001>`). The rank histogram is a tally of the rank of the observed value when placed in order with each of the ensemble values from the same location. If the distributions are identical, then the rank of the observation will be uniformly distributed. In other words, it will fall among the ensemble members randomly in equal likelihood. The PIT histogram applies this same concept, but transforms the actual rank into a probability to facilitate ensembles of differing sizes or with missing members.

Often, the goal of ensemble forecasting is to reproduce the distribution of observations using a set of many forecasts. In other words, the ensemble members represent the set of all possible outcomes. When this is true, the spread of the ensemble is identical to the error in the mean forecast. Though this rarely occurs in practice, the spread / skill relationship is still typically assessed for ensemble forecasts (:ref:`Barker, 1991 <Barker-1991>`; :ref:`Buizza,1997 <Buizza-1997>`). MET calculates the spread and skill in user defined categories according to :ref:`Eckel et al. (2012) <Eckel-2012>`.

The relative position (RELP) is a count of the number of times each ensemble member is closest to the observation. For stochastic or randomly derived ensembles, this statistic is meaningless. For specified ensemble members, however, it can assist users in determining if any ensemble member is performing consistently better or worse than the others.

The ranked probability score (RPS) is included in the Ranked Probability Score (RPS) line type. It is the mean of the Brier scores computed from ensemble probabilities derived for each probability category threshold (prob_cat_thresh) specified in the configuration file. The continuous ranked probability score (CRPS) is the average the distance between the forecast (ensemble) cumulative distribution function and the observation cumulative distribution function. It is an analog of the Brier score, but for continuous forecast and observation fields. The CRPS statistic is computed using two methods: assuming a normal distribution defined by the ensemble mean and spread (:ref:`Gneiting et al., 2004 <Gneiting-2004>`) and using the empirical ensemble distribution (:ref:`Hersbach, 2000 <Hersbach-2000>`). The CRPS statistic using the empirical ensemble distribution can be adjusted (bias corrected) by subtracting 1/(2*m) times the mean absolute difference of the ensemble members, where m is the ensemble size. This is reported as a separate statistic called CRPS_EMP_FAIR. The empirical CRPS and its fair version are included in the Ensemble Continuous Statistics (ECNT) line type, along with other statistics quantifying the ensemble spread and ensemble mean skill.

The Ensemble-Stat tool can derive ensemble relative frequencies and verify them as probability forecasts all in the same run. Note however that these simple ensemble relative frequencies are not actually calibrated probability forecasts. If probabilistic line types are requested (output_flag), this logic is applied to each pair of fields listed in the forecast (fcst) and observation (obs) dictionaries of the configuration file. Each probability category threshold (prob_cat_thresh) listed for the forecast field is applied to the input ensemble members to derive a relative frequency forecast. The probability category threshold (prob_cat_thresh) parsed from the corresponding observation entry is applied to the (gridded or point) observations to determine whether or not the event actually occurred. The paired ensemble relative freqencies and observation events are used to populate an Nx2 probabilistic contingency table. The dimension of that table is determined by the probability PCT threshold (prob_pct_thresh) configuration file option parsed from the forecast dictionary. All probabilistic output types requested are derived from the this Nx2 table and written to the ascii output files. Note that the FCST_VAR name header column is automatically reset as "PROB({FCST_VAR}{THRESH})" where {FCST_VAR} is the current field being evaluated and {THRESH} is the threshold that was applied.

Note that if no probability category thresholds (prob_cat_thresh) are defined, but climatological mean and standard deviation data is provided along with climatological bins, climatological distribution percentile thresholds are automatically derived and used to compute probabilistic outputs. 

Climatology Data
----------------

The Ensemble-Stat output includes at least three statistics computed relative to external climatology data. The climatology is defined by mean and standard deviation fields, and typically both are required in the computation of ensemble skill score statistics. MET assumes that the climatology follows a normal distribution, defined by the mean and standard deviation at each point.

When computing the CRPS skill score for (:ref:`Gneiting et al., 2004 <Gneiting-2004>`) the reference CRPS statistic is computed using the climatological mean and standard deviation directly. When computing the CRPS skill score for (:ref:`Hersbach, 2000 <Hersbach-2000>`) the reference CRPS statistic is computed by selecting equal-area-spaced values from the assumed normal climatological distribution. The number of points selected is determined by the *cdf_bins* setting in the *climo_cdf* dictionary. The reference CRPS is computed empirically from this ensemble of climatology values. If the number bins is set to 1, the climatological CRPS is computed using only the climatological mean value. In this way, the empirical CRPSS may be computed relative to a single model rather than a climatological distribution.

The climatological distribution is also used for the RPSS. The forecast RPS statistic is computed from a probabilistic contingency table in which the probabilities are derived from the ensemble member values. In a simliar fashion, the climatogical probability for each observed value is derived from the climatological distribution. The area of the distribution to the left of the observed value is interpreted as the climatological probability. These climatological probabilities are also evaluated using a probabilistic contingency table from which the reference RPS score is computed. The skill scores are derived by comparing the forecast statistic to the reference climatology statistic.

Ensemble Observation Error
--------------------------

In an attempt to ameliorate the effect of observation errors on the verification of forecasts, a random perturbation approach has been implemented. A great deal of user flexibility has been built in, but the methods detailed in :ref:`Candille and Talagrand (2008) <Candille-2008>` can be replicated using the appropriate options. Additional probabilistic measures that include observational uncertainty recommended by :ref:`Ferro, 2017 <Ferro-2017>` and :ref:`Dawid and Sebastiani, 1999 <Dawid-Sebastiani-1999>` are also provided.

Observation error information can be defined directly in the Ensemble-Stat configuration file or through a more flexible observation error table lookup. The user selects a distribution for the observation error, along with parameters for that distribution. Rescaling and bias correction can also be specified prior to the perturbation. Random draws from the distribution can then be added to either, or both, of the forecast and observed fields, including ensemble members. Details about the effects of the choices on verification statistics should be considered, with many details provided in the literature (*e.g.* :ref:`Candille and Talagrand, 2008 <Candille-2008>`; :ref:`Saetra et al., 2004 <Saetra-2004>`; :ref:`Santos and Ghelli, 2012 <Santos-2012>`). Generally, perturbation makes verification statistics better when applied to ensemble members, and worse when applied to the observations themselves.

Normal and uniform are common choices for the observation error distribution. The uniform distribution provides the benefit of being bounded on both sides, thus preventing the perturbation from taking on extreme values. Normal is the most common choice for observation error. However, the user should realize that with the very large samples typical in NWP, some large outliers will almost certainly be introduced with the perturbation. For variables that are bounded below by 0, and that may have inconsistent observation errors (e.g. larger errors with larger measurements), a lognormal distribution may be selected. Wind speeds and precipitation measurements are the most common of this type of NWP variable. The lognormal error perturbation prevents measurements of 0 from being perturbed, and applies larger perturbations when measurements are larger. This is often the desired behavior in these cases, but this distribution can also lead to some outliers being introduced in the perturbation step.

Observation errors differ according to instrument, temporal and spatial representation, and variable type. Unfortunately, many observation errors have not been examined or documented in the literature. Those that have usually lack information regarding their distributions and approximate parameters. Instead, a range or typical value of observation error is often reported and these are often used as an estimate of the standard deviation of some distribution. Where possible, it is recommended to use the appropriate type and size of perturbation for the observation to prevent spurious results.

Practical Information
=====================

This section contains information about configuring and running the Ensemble-Stat tool. The Ensemble-Stat tool creates or verifies gridded model data. For verification, this tool can accept either gridded or point observations. If provided, the climatology data files must be gridded. The input gridded model, observation, and climatology datasets must be on the same grid prior to calculation of any statistics, and in one of the MET supported gridded file formats. If gridded files are not on the same grid, MET will do the regridding for you if you specify the desired output grid. The point observations must be formatted as the NetCDF output of the point reformatting tools described in :numref:`reformat_point`.

ensemble_stat Usage
-------------------

The usage statement for the Ensemble Stat tool is shown below:

.. code-block:: none

  Usage: ensemble_stat
         n_ens ens_file_1 ... ens_file_n | ens_file_list
         config_file
         [-grid_obs file]
         [-point_obs file]
         [-ens_mean file]
         [-ctrl file]
         [-obs_valid_beg time]
         [-obs_valid_end time]
         [-outdir path]
         [-log file]
         [-v level]
         [-compress level]

ensemble_stat has three required arguments and accepts several optional ones.

Required Arguments ensemble_stat
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. The **n_ens ens_file_1 ... ens_file_n** is the number of ensemble members followed by a list of ensemble member file names. This argument is not required when ensemble files are specified in the **ens_file_list**, detailed below.

2. The **ens_file_list** is an ASCII file containing a list of ensemble member file names. This is not required when a file list is included on the command line, as described above.

3. The **config_file** is an **EnsembleStatConfig** file containing the desired configuration settings.

Optional Arguments for ensemble_stat
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

4. To produce ensemble statistics using gridded observations, use the **-grid_obs file** option to specify a gridded observation file. This option may be used multiple times if your observations are in several files.

5. To produce ensemble statistics using point observations, use the **-point_obs file** option to specify a NetCDF point observation file. This option may be used multiple times if your observations are in several files. Python embedding for point observations is also supported, as described in :numref:`pyembed-point-obs-data`.

6. To override the simple ensemble mean value of the input ensemble members for the ECNT, SSVAR, and ORANK line types, the **-ens_mean file** option specifies an ensemble mean model data file. This option replaces the **-ssvar_mean file** option from earlier versions of MET.

7. The **-ctrl file** option specifies an ensemble control member data file. The control member is included in the computation of the ensemble mean but excluded from the spread. The control file should not appear in the list of ensemble member files (unless processing a single file that contains all ensemble members).

8. To filter point observations by time, use **-obs_valid_beg time** in YYYYMMDD[_HH[MMSS]] format to set the beginning of the matching observation time window.

9. As above, use **-obs_valid_end time** in YYYYMMDD[_HH[MMSS]] format to set the end of the matching observation time window.

10. Specify the **-outdir path** option to override the default output directory (./).

11. The **-log** file outputs log messages to the specified file.

12. The **-v level** option indicates the desired level of verbosity. The value of "level" will override the default setting of 2. Setting the verbosity to 0 will make the tool run with no log messages, while increasing the verbosity will increase the amount of logging.

13. The **-compress level** option indicates the desired level of compression (deflate level) for NetCDF variables. The valid level is between 0 and 9. The value of "level" will override the default setting of 0 from the configuration file or the environment variable MET_NC_COMPRESS. Setting the compression level to 0 will make no compression for the NetCDF output. Lower number is for fast compression and higher number is for better compression.

An example of the ensemble_stat calling sequence is shown below:

.. code-block:: none

     ensemble_stat \
     6 sample_fcst/2009123112/*gep*/d01_2009123112_02400.grib \
     config/EnsembleStatConfig \
     -grid_obs sample_obs/ST4/ST4.2010010112.24h \
     -point_obs out/ascii2nc/precip24_2010010112.nc \
     -outdir out/ensemble_stat -v 2

In this example, the Ensemble-Stat tool will process six forecast files specified in the file list into an ensemble forecast. Observations in both point and grid format will be included, and be used to compute ensemble statistics separately. Ensemble Stat will create a NetCDF file containing requested ensemble fields and an output STAT file.

ensemble_stat Configuration File
--------------------------------

The default configuration file for the Ensemble-Stat tool named **EnsembleStatConfig_default** can be found in the installed *share/met/config* directory. Another version is located in *scripts/config*. We encourage users to make a copy of these files prior to modifying their contents. Each configuration file (both the default and sample) contains many comments describing its contents. The contents of the configuration file are also described in the subsections below.

Note that environment variables may be used when editing configuration files, as described in the :numref:`config_env_vars`.

____________________

.. code-block:: none

  model          = "FCST";
  desc           = "NA";
  obtype         = "ANALYS";
  regrid         = { ... }
  climo_mean     = { ... }
  climo_stdev    = { ... }
  climo_cdf      = { ... }
  obs_window     = { beg = -5400; end =  5400; }
  mask           = { grid = [ "FULL" ]; poly = []; sid = []; }
  ci_alpha       = [ 0.05 ];
  interp         = { field = BOTH; vld_thresh = 1.0; shape = SQUARE;
                     type = [ { method = NEAREST; width = 1; } ]; }
  eclv_points    = [];
  sid_inc        = [];
  sid_exc        = [];
  duplicate_flag = NONE;
  obs_quality_inc  = [];
  obs_quality_exc  = [];
  obs_summary    = NONE;
  obs_perc_value = 50;
  message_type_group_map = [...];
  output_prefix  = "";
  version        = "VN.N";


The configuration options listed above are common to many MET tools and are described in :numref:`config_options`.

Note that the **HIRA** interpolation method is only supported in Ensemble-Stat.

_____________________

When processing the **fcst** data, compute a ratio of the number of valid ensemble fields to the total number of ensemble members. If this ratio is less than the **ens_thresh**, then quit with an error. This threshold must be between 0 and 1. Setting this threshold to 1 will require that all ensemble members be present to be processed.

When processing the **fcst** data, for each grid point compute a ratio of the number of valid data values to the number of ensemble members. If that ratio is less than **vld_thresh**, write out bad data. This threshold must be between 0 and 1. Setting this threshold to 1 will require each grid point to contain valid data for all ensemble members.

For each **field** listed in the forecast field, give the name and vertical or accumulation level, plus one or more categorical thresholds. The thresholds are specified using symbols, as shown above. It is the user's responsibility to know the units for each model variable and to choose appropriate threshold values. The thresholds are used to define ensemble relative frequencies, e.g. a threshold of >=5 can be used to compute the proportion of ensemble members predicting precipitation of at least 5mm at each grid point.

_______________________

.. code-block:: none

  ens_member_ids = [];
  control_id = "";


The **ens_member_ids** array is only used if reading a single file that contains all ensemble members.
It should contain a list of string identifiers that are substituted into the **ens** and/or **fcst** dictionary fields
to determine which data to read from the file.
The length of the array determines how many ensemble members will be processed for a given field.
Each value in the array will replace the text **MET_ENS_MEMBER_ID**.

**NetCDF Example:**

.. code-block:: none

  fcst = {
    field = [
      {
        name  = "fcst";
        level = "(MET_ENS_MEMBER_ID,0,*,*)";
      }
    ];
  }


**GRIB Example:**

.. code-block:: none

  fcst = {
    field = [
      {
        name     = "fcst";
        level    = "L0";
        GRIB_ens = "MET_ENS_MEMBER_ID";
      }
    ];
  }


**control_id** is a string that is substituted in the same way as the **ens_member_ids** values
to read a control member. This value is only used when the **-ctrl** command line argument is
used. The value should not be found in the **ens_member_ids** array.

_____________________

.. code-block:: none

  obs_thresh = [ NA ];


The **obs_thresh** entry is an array of thresholds for filtering observation values prior to applying ensemble verification logic. The default setting of **NA** means that no observations should be filtered out. Verification output will be computed separately for each threshold specified. This option may be set separately for each **obs.field** entry.

____________________

.. code-block:: none

  skip_const = FALSE;


Setting **skip_const** to true tells Ensemble-Stat to exclude pairs where all the ensemble members and the observation have a constant value. For example, exclude points with zero precipitation amounts from all output line types. This option may be set separately for each **obs.field** entry. When set to false, constant points are and the observation rank is chosen at random.

____________________

.. code-block:: none

  ens_ssvar_bin_size = 1.0;
  ens_phist_bin_size = 0.05;


Setting up the **fcst** and **obs** dictionaries of the configuration file is described in :numref:`config_options`. The following are some special considerations for the Ensemble-Stat tool.

The **ens** and **fcst** dictionaries do not need to include the same fields. Users may specify any number of ensemble fields to be summarized, but generally there are many fewer fields with verifying observations available. The **ens** dictionary specifies the fields to be summarized while the **fcst** dictionary specifies the fields to be verified.

The **obs** dictionary looks very similar to the **fcst** dictionary. If verifying against point observations which are assigned GRIB1 codes, the observation section must be defined following GRIB1 conventions. When verifying GRIB1 forecast data, one can easily copy over the forecast settings to the observation dictionary using **obs = fcst;**. However, when verifying non-GRIB1 forecast data, users will need to specify the **fcst** and **obs** sections separately.

The **ens_ssvar_bin_size** and **ens_phist_bin_size** specify the width of the categorical bins used to accumulate frequencies for spread-skill-variance or probability integral transform statistics, respectively.

____________________

.. code-block:: none

  prob_cat_thresh = [];
  prob_pct_thresh = [];


The **prob_cat_thresh** entry is an array of thresholds. It is applied both to the computation of the RPS line type as well as the when generating probabilistic output line types. Since these thresholds can change for each variable, they can be specified separately for each **fcst.field** entry. If left empty but climatological mean and standard deviation data is provided, the **climo_cdf** thresholds will be used instead. If no climatology data is provided, and the RPS output line type is requested, then the **prob_cat_thresh** array must be defined. When probabilistic output line types are requested, for each **prob_cat_thresh** threshold listed, ensemble relative frequencies are derived and verified against the point and/or gridded observations.

The **prob_pct_thresh** entry is an array of thresholds which define the Nx2 probabilistic contingency table used to evaluate probability forecasts. It can be specified separately for each **fcst.field** entry. These thresholds must span the range [0, 1]. A shorthand notation to create equal bin widths is provided. For example, the following setting creates 4 probability bins of width 0.25 from 0 to 1.

.. code-block:: none

  prob_pct_thresh = [ ==0.25 ];


__________________

.. code-block:: none

  obs_error = {
     flag             = FALSE;
     dist_type        = NONE;
     dist_parm        = [];
     inst_bias_scale  = 1.0;
     inst_bias_offset = 0.0;
  }


The **obs_error** dictionary controls how observation error information should be handled. This dictionary may be set separately for each **obs.field** entry. Observation error information can either be specified directly in the configuration file or by parsing information from an external table file. By default, the **MET_BASE/share/met/table_files/obs_error_table.txt** file is read but this may be overridden by setting the **$MET_OBS_ERROR_TABLE** environment variable at runtime.


The **flag** entry toggles the observation error logic on (**TRUE**) and off (**FALSE**). When the **flag** is **TRUE**, random observation error perturbations are applied to the ensemble member values. No perturbation is applied to the observation values but the bias scale and offset values, if specified, are applied.


The **dist_type** entry may be set to **NONE, NORMAL, LOGNORMAL, EXPONENTIAL,CHISQUARED, GAMMA, UNIFORM**, or **BETA**. The default value of **NONE** indicates that the observation error table file should be used rather than the configuration file settings.


The **dist_parm** entry is an array of length 1 or 2 specifying the parameters for the distribution selected in **dist_type**. The **GAMMA, UNIFORM**, and **BETA** distributions are defined by two parameters, specified as a comma-separated list (a,b), whereas all other distributions are defined by a single parameter.


The **inst_bias_scale** and **inst_bias_offset** entries specify bias scale and offset values that should be applied to observation values prior to perturbing them. These entries enable bias-correction on the fly.


Defining the observation error information in the configuration file is convenient but limited. The random perturbations for all points in the current verification task are drawn from the same distribution. Specifying an observation error table file instead (by setting **dist_type = NONE;**) provides much finer control, enabling the user to define observation error distribution information and bias-correction logic separately for each observation variable name, message type, PrepBUFR report type, input report type, instrument type, station ID, range of heights, range of pressure levels, and range of values.

_________________

.. code-block:: none

  output_flag = {
     ecnt  = NONE;
     rps   = NONE;
     rhist = NONE;
     phist = NONE;
     orank = NONE;
     ssvar = NONE;
     relp  = NONE;
     pct   = NONE;
     pstd  = NONE;
     pjc   = NONE;
     prc   = NONE;
     eclv  = NONE;
  }


The **output_flag** array controls the type of output that is generated. Each flag corresponds to an output line type in the STAT file. Setting the flag to NONE indicates that the line type should not be generated. Setting the flag to STAT indicates that the line type should be written to the STAT file only. Setting the flag to BOTH indicates that the line type should be written to the STAT file as well as a separate ASCII file where the data is grouped by line type. The output flags correspond to the following output line types:

1. **ECNT** for Continuous Ensemble Statistics

2. **RPS** for Ranked Probability Score Statistics

3. **RHIST** for Ranked Histogram Counts

4. **PHIST** for Probability Integral Transform Histogram Counts

5. **ORANK** for Ensemble Matched Pair Information when point observations are supplied

6. **SSVAR** for Binned Spread/Skill Variance Information

7. **RELP** for Relative Position Counts

8. **PCT** for Contingency Table counts for derived ensemble relative frequencies

9. **PSTD** for Probabilistic statistics for dichotomous outcomes for derived ensemble relative frequencies

10. **PJC** for Joint and Conditional factorization for derived ensemble relative frequencies

11. **PRC** for Receiver Operating Characteristic for derived ensemble relative frequencies

12. **ECLV** for Economic Cost/Loss Relative Value for derived ensemble relative frequencies

_____________________

.. code-block:: none

  nc_orank_flag = {
     latlon    = TRUE;
     mean      = TRUE;
     raw       = TRUE;
     rank      = TRUE;
     pit       = TRUE;
     vld_count = TRUE;
     weight    = FALSE;
  }


The **nc_orank_flag** specifies which gridded verification output types should be written to the Observation Rank (**_orank.nc**) NetCDF file. This output file is only created when gridded observations have been provided with the -grid_obs command line option. Setting the flag to TRUE produces output of the specified field, while FALSE produces no output for that field type. The flags correspond to the following output line types:

1. Grid Latitude and Longitude Fields

2. Ensemble mean field

3. Raw observation values

4. Observation ranks

5. Observation probability-integral transform values

6. Ensemble valid data count

7. Grid area weight values

__________________

.. code-block:: none
		
    nc_var_str = "";


The **nc_var_str** entry specifies a string for each ensemble field and verification task. This string is parsed from each **ens.field** and **obs.field** dictionary entry and is used to customize the variable names written to theNetCDF output file. The default is an empty string, meaning that no customization is applied to the output variable names. When the Ensemble-Stat config file contains two fields with the same name and level value, this entry is used to make the resulting variable names unique.

________________

.. code-block:: none

  rng = {
     type = "mt19937";
     seed = "";
     }


The **rng** group defines the random number generator **type** and **seed** to be used. In the case of a tie when determining the rank of an observation, the rank is randomly chosen from all available possibilities. The randomness is determined by the random number generator specified.


The **seed** variable may be set to a specific value to make the assignment of ranks fully repeatable. When left empty, as shown above, the random number generator seed is chosen automatically which will lead to slightly different bootstrap confidence intervals being computed each time the data is run.


Refer to the description of the **boot** entry in :numref:`config_options` for more details on the random number generator.


ensemble_stat Output
--------------------

ensemble_stat can produce output in STAT, ASCII, and NetCDF formats. The ASCII output duplicates the STAT output but has the data organized by line type. The output files are written to the default output directory or the directory specified by the -outdir command line option.


The output STAT file is named using the following naming convention:


ensemble_stat_PREFIX_YYYYMMDD_HHMMSSV.stat where PREFIX indicates the user-defined output prefix and YYYYMMDD_HHMMSSV indicates the forecast valid time. Note that the forecast lead time is not included in the output file names since it would not be well-defined for time-lagged ensembles. When verifying multiple lead times for the same valid time, users should either write the output to separate directories or specify an output prefix to ensure unique file names.


The output ASCII files are named similarly:


ensemble_stat_PREFIX_YYYYMMDD_HHMMSSV_TYPE.txt where TYPE is one of elements of the **output_flag** configuration option to indicate the line type it contains.


When verification against gridded analyses is performed, Ensemble-Stat can produce output NetCDF files using the following naming convention:


ensemble_stat_PREFIX_YYYYMMDD_HHMMSSV_orank.nc contains gridded fields of observation ranks when the -grid_obs command line option is used. Its contents are specified by the **nc_orank_flag** configuration option.

The Ensemble-Stat tool can compute the following statistics for the fields specified in the fcst and obs dictionaries of the configuration file:


Continuous Ensemble Statistics


Ranked Histograms


Probability Integral Transform (PIT) Histograms


Relative Position Histograms


Spread/Skill Variance


Ensemble Matched Pair information


The format of the STAT and ASCII output of the Ensemble-Stat tool are described below.

.. _table_ES_header_info_es_out:

.. list-table:: Header information for each file ensemble-stat outputs
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
    - User provided text string designating model name
  * - 3
    - DESC
    - User provided text string describing the verification task
  * - 4
    - FCST_LEAD
    - Forecast lead time in HHMMSS format
  * - 5
    - FCST_VALID_BEG
    - Forecast valid start time in YYYYMMDD_HHMMSS format
  * - 6
    - FCST_VALID_END
    - Forecast valid end time in YYYYMMDD_HHMMSS format
  * - 7
    - OBS_LEAD
    - Observation lead time in HHMMSS format
  * - 8
    - OBS_VALID_BEG
    - Observation valid start time in YYYYMMDD_HHMMSS format
  * - 9
    - OBS_VALID_END
    - Observation valid end time in YYYYMMDD_HHMMSS format
  * - 10
    - FCST_VAR
    - Model variable
  * - 11
    - FCST_UNITS
    - Units for model variable
  * - 12
    - FCST_LEV
    - Selected Vertical level for forecast
  * - 13
    - OBS_VAR
    - Observation variable
  * - 14
    - OBS_UNITS
    - Units for observation variable
  * - 15
    - OBS_LEV
    - Selected Vertical level for observations
  * - 16
    - OBTYPE
    - Type of observation selected
  * - 17
    - VX_MASK
    - Verifying masking region indicating the masking grid or polyline region applied
  * - 18
    - INTERP_MTHD
    - Interpolation method applied to forecasts
  * - 19
    - INTERP_PNTS
    - Number of points used in interpolation method
  * - 20
    - FCST_THRESH
    - The threshold applied to the forecast
  * - 21
    - OBS_THRESH
    - The threshold applied to the observations
  * - 22
    - COV_THRESH
    - The minimum fraction of valid ensemble members required to calculate statistics.
  * - 23
    - ALPHA
    - Error percent value used in confidence intervals
  * - 24
    - LINE_TYPE
    - Output line types are listed in :numref:`table_ES_header_info_es_out_RHIST` through :numref:`table_ES_header_info_es_out_SSVAR`.

.. _table_ES_header_info_es_out_ECNT:

.. list-table:: Format information for ECNT (Ensemble Continuous Statistics) output line type.
  :widths: auto
  :header-rows: 2

  * - ECNT OUTPUT FORMAT
    - 
    - 
  * - Column Number
    - ECNT Column Name
    - Description
  * - 24
    - ECNT
    - Ensemble Continuous Statistics line type
  * - 25
    - TOTAL
    - Count of observations
  * - 26
    - N_ENS
    - Number of ensemble values
  * - 27
    - CRPS
    - The Continuous Ranked Probability Score (normal distribution)
  * - 28
    - CRPSS
    - The Continuous Ranked Probability Skill Score (normal distribution)
  * - 29
    - IGN
    - The Ignorance Score
  * - 30
    - ME
    - The Mean Error of the ensemble mean (unperturbed or supplied)
  * - 31
    - RMSE
    - The Root Mean Square Error of the ensemble mean (unperturbed or supplied)
  * - 32
    - SPREAD
    - The square root of the mean of the variance of the unperturbed ensemble member values at each observation location
  * - 33
    - ME_OERR
    - The Mean Error of the PERTURBED ensemble mean (e.g. with Observation Error)
  * - 34
    - RMSE_OERR
    - The Root Mean Square Error of the PERTURBED ensemble mean (e.g. with Observation Error)
  * - 35
    - SPREAD_OERR
    - The square root of the mean of the variance of the PERTURBED ensemble member values (e.g. with Observation Error) at each observation location
  * - 36
    - SPREAD_PLUS_OERR
    - The square root of the sum of unperturbed ensemble variance and the observation error variance
  * - 37 
    - CRPSCL
    - Climatological Continuous Ranked Probability Score (normal distribution)
  * - 38
    - CRPS_EMP 
    - The Continuous Ranked Probability Score (empirical distribution)
  * - 39
    - CRPSCL_EMP 
    - Climatological Continuous Ranked Probability Score (empirical distribution)
  * - 40 
    - CRPSS_EMP
    - The Continuous Ranked Probability Skill Score (empirical distribution)
  * - 41 
    - CRPS_EMP_FAIR
    - The Continuous Ranked Probability Score (empirical distribution) adjusted by the mean absolute difference of the ensemble members
  * - 42
    - SPREAD_MD
    - The pairwise Mean Absolute Difference of the unperturbed ensemble members
  * - 43
    - MAE
    - The Mean Absolute Error of the ensemble mean (unperturbed or supplied)
  * - 44
    - MAE_OERR
    - The Mean Absolute Error of the PERTURBED ensemble mean (e.g. with Observation Error)
  * - 45
    - BIAS_RATIO
    - The Bias Ratio
  * - 46
    - N_GE_OBS
    - The number of ensemble values greater than or equal to their observations
  * - 47
    - ME_GE_OBS
    - The Mean Error of the ensemble values greater than or equal to their observations
  * - 48
    - N_LT_OBS
    - The number of ensemble values less than their observations
  * - 49
    - ME_LT_OBS
    - The Mean Error of the ensemble values less than or equal to their observations
  * - 50
    - IGN_CONV_OERR
    - Error-convolved logarithmic scoring rule (i.e. ignornance score) from Equation 5 of :ref:`Ferro, 2017 <Ferro-2017>`
  * - 51
    - IGN_CORR_OERR
    - Error-corrected logarithmic scoring rule (i.e. ignornance score) from Equation 7 of :ref:`Ferro, 2017 <Ferro-2017>`
  * - 52
    - IDSS
    - Scoring rule from Equation 16 of :ref:`Dawid and Sebastiani, 1999 <Dawid-Sebastiani-1999>`

.. _table_ES_header_info_es_out_RPS:
      
.. list-table:: Format information for RPS (Ranked Probability Score) output line type.
  :widths: auto
  :header-rows: 2

  * - RPS OUTPUT FORMAT
    - 
    - 
  * - Column Number
    - RPS Column Name
    - Description
  * - 24
    - RPS
    - Ranked Probability Score line type
  * - 25
    - TOTAL
    - Count of observations
  * - 26
    - N_PROB
    - Number of probability thresholds (i.e. number of ensemble members in Ensemble-Stat)
  * - 27
    - RPS_REL
    - RPS Reliability, mean of the reliabilities for each RPS threshold
  * - 28
    - RPS_RES
    - RPS Resolution, mean of the resolutions for each RPS threshold
  * - 29
    - RPS_UNC
    - RPS Uncertainty, mean of the uncertainties for each RPS threshold
  * - 30
    - RPS
    - Ranked Probability Score, mean of the Brier Scores for each RPS threshold
  * - 31
    - RPSS
    - Ranked Probability Skill Score relative to external climatology
  * - 32
    - RPSS_SMPL
    - Ranked Probability Skill Score relative to sample climatology

.. _table_ES_header_info_es_out_RHIST:
      
.. list-table:: Format information for RHIST (Ranked Histogram) output line type.
  :widths: auto
  :header-rows: 2

  * - RHIST OUTPUT FORMAT
    - 
    - 
  * - Column Number
    - RHIST Column Name
    - Description
  * - 24
    - RHIST
    - Ranked Histogram line type
  * - 25
    - TOTAL
    - Count of observations
  * - 26
    - N_RANK
    - Number of possible ranks for observation
  * - 27
    - RANK_i
    - Count of observations with the i-th rank (repeated)

.. _table_ES_header_info_es_out_PHIST:
      
.. list-table:: Format information for PHIST (Probability Integral Transform Histogram) output line type.
  :widths: auto
  :header-rows: 2

  * - PHIST OUTPUT FORMAT
    - 
    - 
  * - Column Number
    - PHIST Column Name
    - Description
  * - 24
    - PHIST
    - Probability Integral Transform line type
  * - 25
    - TOTAL
    - Count of observations
  * - 26
    - BIN_SIZE
    - Probability interval width
  * - 27
    - N_BIN
    - Total number of probability intervals
  * - 28
    - BIN_i
    - Count of observations in the ith probability bin (repeated)

.. _table_ES_header_info_es_out_RELP:

.. list-table:: Format information for RELP (Relative Position) output line type.
  :widths: auto
  :header-rows: 2

  * - RELP OUTPUT FORMAT
    - 
    - 
  * - Column Number
    - RELP Column Name
    - Description
  * - 24
    - RELP
    - Relative Position line type
  * - 25
    - TOTAL
    - Count of observations
  * - 26
    - N_ENS
    - Number of ensemble members
  * - 27
    - RELP_i
    - Number of times the i-th ensemble member's value was closest to the observation (repeated). When n members tie, 1/n is assigned to each member.

.. _table_ES_header_info_es_out_ORANK:
      
.. list-table:: Format information for ORANK (Observation Rank) output line type.
  :widths: auto
  :header-rows: 2

  * - ORANK OUTPUT FORMAT
    - 
    - 
  * - Column Number
    - ORANK Column Name
    - Description
  * - 24
    - ORANK
    - Observation Rank line type
  * - 25
    - TOTAL
    - Count of observations
  * - 26
    - INDEX
    - Line number in ORANK file
  * - 27
    - OBS_SID
    - Station Identifier
  * - 28
    - OBS_LAT
    - Latitude of the observation
  * - 29
    - OBS_LON
    - Longitude of the observation
  * - 30
    - OBS_LVL
    - Level of the observation
  * - 31
    - OBS_ELV
    - Elevation of the observation
  * - 32
    - OBS
    - Value of the observation
  * - 33
    - PIT
    - Probability Integral Transform
  * - 34
    - RANK
    - Rank of the observation
  * - 35
    - N_ENS_VLD
    - Number of valid ensemble values
  * - 36
    - N_ENS
    - Number of ensemble values
  * - 37
    - ENS_i
    - Value of the ith ensemble member (repeated)
  * - Last-7
    - OBS_QC
    - Quality control string for the observation
  * - Last-6
    - ENS_MEAN
    - The unperturbed ensemble mean value
  * - Last-5
    - CLIMO_MEAN
    - Climatological mean value (named CLIMO prior to met-10.0.0)
  * - Last-4
    - SPREAD
    - The spread (standard deviation) of the unperturbed ensemble member values
  * - Last-3
    - ENS_MEAN _OERR
    - The PERTURBED ensemble mean (e.g. with Observation Error).
  * - Last-2
    - SPREAD_OERR
    - The spread (standard deviation) of the PERTURBED ensemble member values (e.g. with Observation Error).
  * - Last-1
    - SPREAD_PLUS_OERR
    - The square root of the sum of the unperturbed ensemble variance and the observation error variance.
  * - Last
    - CLIMO_STDEV
    - Climatological standard deviation value
      
.. role:: raw-html(raw)
    :format: html

.. _table_ES_header_info_es_out_SSVAR:	     

.. list-table:: Format information for SSVAR (Spread/Skill Variance) output line type.
  :widths: auto
  :header-rows: 2

  * - SSVAR OUTPUT FORMAT
    - 
    - 
  * - Column Number
    - SSVAR Column Name
    - Description
  * - 24
    - SSVAR
    - Spread/Skill Variance line type
  * - 25
    - TOTAL
    - Count of observations
  * - 26
    - N_BIN
    - Number of bins for current forecast run
  * - 27
    - BIN_i
    - Index of the current bin
  * - 28
    - BIN_N
    - Number of points in bin i
  * - 29
    - VAR_MIN
    - Minimum variance
  * - 30
    - VAR_MAX
    - Maximum variance
  * - 31
    - VAR_MEAN
    - Average variance
  * - 32
    - FBAR
    - Average forecast value
  * - 33
    - OBAR
    - Average observed value
  * - 34
    - FOBAR
    - Average product of forecast and observation
  * - 35
    - FFBAR
    - Average of forecast squared
  * - 36
    - OOBAR
    - Average of observation squared
  * - 37-38
    - FBAR_NCL, :raw-html:`<br />` FBAR_NCU
    - Mean forecast normal upper and lower confidence limits
  * - 39-41
    - FSTDEV, :raw-html:`<br />` FSTDEV_NCL, :raw-html:`<br />` FSTDEV_NCU
    - Standard deviation of the error including normal upper and lower confidence limits
  * - 42-43
    - OBAR_NCL, :raw-html:`<br />` OBAR_NCU
    - Mean observation normal upper and lower confidence limits
  * - 44-46
    - OSTDEV, :raw-html:`<br />` OSTDEV_NCL, :raw-html:`<br />` OSTDEV_NCU
    - Standard deviation of the error including normal upper and lower confidence limits
  * - 47-49
    - PR_CORR, :raw-html:`<br />` PR_CORR_NCL, :raw-html:`<br />` PR_CORR_NCU
    - Pearson correlation coefficient including normal upper and lower confidence limits
  * - 50-52
    - ME, :raw-html:`<br />` ME_NCL, :raw-html:`<br />` ME_NCU
    - Mean error including normal upper and lower confidence limits
  * - 53-55
    - ESTDEV, :raw-html:`<br />` ESTDEV_NCL, :raw-html:`<br />` ESTDEV_NCU
    - Standard deviation of the error including normal upper and lower confidence limits
  * - 56
    - MBIAS
    - Magnitude bias
  * - 57
    - MSE
    - Mean squared error
  * - 58
    - BCMSE
    - Bias corrected root mean squared error
  * - 59
    - RMSE
    - Root mean squared error

