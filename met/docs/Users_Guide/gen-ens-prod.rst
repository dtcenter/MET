.. _gen-ens-prod:

*****************
Gen-Ens-Prod Tool
*****************

Introduction
============

The Gen-Ens-Prod tool generates simple ensemble products (mean, spread, probability, etc) from gridded ensemble member input files. While it processes model inputs, but it does not compare them to observations or compute statistics. However, the output products can be passed as input to the MET statistics tools for comparison against observations. Climatological mean and standard deviation data may also be provided to define thresholds based on the climatological distribution at each grid point.

Note that this ensemble product generation step was provided by the Ensemble-Stat tool in earlier versions of MET. The Gen-Ens-Prod tool replaces and extends that functionality. Users are strongly encouraged to migrate ensemble product generation from Ensemble-Stat to Gen-Ens-Prod, as new features will only be added to Gen-Ens-Prod and the existing Ensemble-Stat functionality will be deprecated in a future version.

Scientific and statistical aspects
==================================

Ensemble forecasts derived from a set of deterministic ensemble members
-----------------------------------------------------------------------

Ensemble forecasts are often created as a set of deterministic forecasts. The ensemble members are rarely used separately. Instead, they can be combined in various ways to produce a forecast. MET can combine the ensemble members into some type of summary forecast according to user specifications. Ensemble means are the most common, and can be paired with the ensemble variance or spread. Maximum, minimum and other summary values are also available, with details in the practical information section.

The -ctrl command line option specifies an input file for the ensemble control member. The fields specified in the configuration file are read from the control member file. Those fields are included in the computation of the ensemble mean and probabilities but excluded from the ensemble spread.

The ensemble relative frequency is the simplest method for turning a set of deterministic forecasts into something resembling a probability forecast. MET will create the ensemble relative frequency as the proportion of ensemble members forecasting some event. For example, if 5 out of 10 ensemble members predict measurable precipitation at a grid location, then the ensemble relative frequency of precipitation will be :math:`5/10=0.5`. If the ensemble relative frequency is calibrated (unlikely) then this could be thought of as a probability of precipitation.

The neighborhood ensemble probability (NEP) and neighborhood maximum ensemble probability (NMEP) methods are described in :ref:`Schwartz and Sobash (2017) <Schwartz-2017>`. They are an extension of the ensemble relative frequencies described above. The NEP value is computed by averaging the relative frequency of the event within the neighborhood over all ensemble members. The NMEP value is computed as the fraction of ensemble members for which the event is occurring somewhere within the surrounding neighborhood. The NMEP output is typically smoothed using a Gaussian kernel filter. The neighborhood sizes and smoothing options can be customized in the configuration file.

The Gen-Ens-Prod tool writes the gridded relative frequencies, NEP, and NMEP fields to a NetCDF output file. Probabilistic verification methods can then be applied to those fields by evaluating them with the Grid-Stat and/or Point-Stat tools.

Climatology data
----------------

The ensemble relative frequencies derived by Gen-Ens-Prod are computed by applying threshold(s) to the input ensemble member data. Those thresholds can be simple and remain constant over the entire domain (e.g. >0) or can be defined relative to the climatological distribution at each grid point (e.g. >CDP90, for exceeding the 90-th percentile of climatology). When using climatological distribution percentile (CDP) thresholds, the climatological mean and standard deviation must be provided in the configuration file.

Practical Information
=====================

This section contains information about configuring and running the Gen-Ens-Prod tool. The Gen-Ens-Prod tool writes a NetCDF output file containing the requested ensemble product fields for each input field specified. If provided, the climatology data files must be gridded. All input gridded model and climatology datasets must be on the same grid. However, users may leverage the automated regridding feature in MET if the desired output grid is specified in the configuration file.

gen_ens_prod usage
------------------

The usage statement for the Ensemble Stat tool is shown below:

.. code-block:: none

  Usage: gen_ens_prod
         -ens file_1 ... file_n | ens_file_list
         -out file
         -config file
         [-ctrl file]
         [-log file]
         [-v level]

gen_ens_prod has three required arguments and accepts several optional ones.

Required arguments gen_ens_prod
-------------------------------

1. The **-ens file_1 ... file_n** option specifies the ensemble member file names. This argument is not required when ensemble files are specified in the **ens_file_list**, detailed below.

2. The **ens_file_list** option is an ASCII file containing a list of ensemble member file names. This is not required when a file list is included on the command line, as described above.

3. The **-out file** option specifies the NetCDF output file name to be written.

4. The **-config file** option is a **GenEnsProdConfig** file containing the desired configuration settings.

Optional arguments for gen_ens_prod
-----------------------------------

4. The **-ctrl file** option specifies the input file for the ensemble control member. Data for this member is included in the computation of the ensemble mean, but excluded from the spread. The control file should not appear in the **-ens** list of ensemble member files (unless processing a single file that contains all ensemble members).

5. The **-log** file outputs log messages to the specified file.

6. The **-v level** option indicates the desired level of verbosity. The value of "level" will override the default setting of 2. Setting the verbosity to 0 will make the tool run with no log messages, while increasing the verbosity will increase the amount of logging.

An example of the gen_ens_prod calling sequence is shown below:

.. code-block:: none

     gen_ens_prod \
     -ens sample_fcst/2009123112/*gep*/d01_2009123112_02400.grib \
     -out out/gen_ens_prod/gen_ens_prod_20100101_120000V_ens.nc \
     -config config/GenEnsProdConfig -v 2

In this example, the Gen-Ens-Prod tool derives products from the input ensemble members listed on the command line.

gen_ens_prod configuration file
-------------------------------

The default configuration file for the Gen-Ens-Prod tool named **GenEnsProdConfig_default** can be found in the installed *share/met/config* directory. Another version is located in *scripts/config*. We encourage users to make a copy of these files prior to modifying their contents. The contents of the configuration file are described in the subsections below.

Note that environment variables may be used when editing configuration files, as described in :numref:`pb2nc configuration file` for the PB2NC tool.

____________________

.. code-block:: none

  model          = "WRF";
  desc           = "NA";
  regrid         = { ... }
  censor_thresh  = [];
  censor_val     = [];
  nc_var_str     = "";
  climo_mean     = { ... } // Corresponding to ens.field entries
  climo_stdev    = { ... } // Corresponding to ens.field entries
  rng            = { ... }
  version        = "VN.N";

The configuration options listed above are common to many MET tools and are described in :numref:`config_options`.

_____________________

.. code-block:: none

  ens = {
    ens_thresh = 1.0;
    vld_thresh = 1.0;
    field = [
      {
        name       = "APCP";
        level      = "A03";
        cat_thresh = [ >0.0, >=5.0 ];
      }
    ];
  }

The **ens** dictionary defines which ensemble fields should be processed.

When summarizing the ensemble, compute a ratio of the number of valid ensemble fields to the total number of ensemble members. If this ratio is less than the **ens_thresh**, then quit with an error. This threshold must be between 0 and 1. Setting this threshold to 1 requires that all ensemble members input files exist and all requested data be present.

When summarizing the ensemble, for each grid point compute a ratio of the number of valid data values to the number of ensemble members. If that ratio is less than **vld_thresh**, write out bad data for that grid point. This threshold must be between 0 and 1. Setting this threshold to 1 requires  that each grid point contain valid data for all ensemble members in order to compute ensemble product values for that grid point.

For each dictionary entry in the **field** array, give the name and vertical or accumulation level, plus one or more categorical thresholds in the **cat_thresh** entry. The formatting for threshold are described in :numref:`config_options`. It is the user's responsibility to know the units for each model variable and choose appropriate threshold values. The thresholds are used to define ensemble relative frequencies. For example, a threshold of >=5 is used to define the proportion of ensemble members predicting precipitation of at least 5mm at each grid point.

_______________________

.. code-block:: none

  ens_member_ids = [];
  control_id = "";

The **ens_member_ids** array is only used if reading a single file that contains all ensemble members.
It should contain a list of string identifiers that are substituted into the **ens** dictionary fields
to determine which data to read from the file.
The length of the array determines how many ensemble members will be processed for a given field.
Each value in the array will replace the text **MET_ENS_MEMBER_ID**.

**NetCDF Example:**

.. code-block:: none

  ens = {
    field = [
      {
        name  = "fcst";
        level = "(MET_ENS_MEMBER_ID,0,*,*)";
      }
    ];
  }

**GRIB Example:**

.. code-block:: none

  ens = {
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

  normalize = NONE;

The **normalize** option defines if and how the input ensemble member data should be normalized. Options are provided to normalize relative to an external climatology, specified using the **climo_mean** and **climo_stdev** dictionaries, or relative to current ensemble forecast being processed. The anomaly is computed by subtracting the (climatological or ensemble) mean from each ensemble memeber. The standard anomaly is computed by dividing the anomaly by the (climatological or ensemble) standard deviation. Values for the **normalize** option are described below:

• **NONE** (default) to skip the normalization step and process the raw ensemble member data.

• **CLIMO_ANOM** to subtract the climatological mean field.

• **CLIMO_STD_ANOM** to subtract the climatological mean field and divide by the climatological standard deviation.

• **FCST_ANOM** to subtract the current ensemble mean field.

• **FCST_STD_ANOM** to subtract the current ensemble mean field and divide by the current ensemble standard deviation.

Note that the **normalize** option may be specified separately for each entry in the **ens.field** array.

_______________________

.. code-block:: none

  nbrhd_prob = {
     width      = [ 5 ];
     shape      = CIRCLE;
     vld_thresh = 0.0;
  }

The **nbrhd_prob** dictionary defines the neighborhoods used to compute NEP and NMEP output.

The neighborhood **shape** is a **SQUARE** or **CIRCLE** centered on the current point, and the **width** array specifies the width of the square or diameter of the circle as an odd integer. The **vld_thresh** entry is a number between 0 and 1 specifying the required ratio of valid data in the neighborhood for an output value to be computed.

If **ensemble_flag.nep** is set to TRUE, NEP output is created for each combination of the categorical threshold (**cat_thresh**) and neighborhood width specified.

_____________________

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

Similar to the **interp** dictionary, the **nmep_smooth** dictionary includes a **type** array of dictionaries to define one or more methods for smoothing the NMEP data. Setting the interpolation method to nearest neighbor (**NEAREST**) effectively disables this smoothing step.

If **ensemble_flag.nmep** is set to TRUE, NMEP output is created for each combination of the categorical threshold (**cat_thresh**), neighborhood width (**nbrhd_prob.width**), and smoothing method(**nmep_smooth.type**) specified.

_____________________

.. code-block:: none
		
  ensemble_flag = {
    latlon    = TRUE;
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
	 climo     = FALSE;
	 climo_cdp = FALSE;
  }

The **ensemble_flag** specifies which derived ensemble fields should be calculated and output. Setting the flag to TRUE produces output of the specified field, while FALSE produces no output for that field type. The flags correspond to the following output line types:

1. Grid Latitude and Longitude Fields

2. Ensemble Mean Field

3. Ensemble Standard Deviation Field

4. Ensemble Mean - One Standard Deviation Field

5. Ensemble Mean + One Standard Deviation Field

6. Ensemble Minimum Field

7. Ensemble Maximum Field

8. Ensemble Range Field

9. Ensemble Valid Data Count

10. Ensemble Relative Frequency (i.e. uncalibrate probability forecast) for each categorical threshold (**cat_thresh**) specified

11. Neighborhood Ensemble Probability for each categorical threshold (**cat_thresh**) and neighborhood width (**nbrhd_prob.width**) specified

12. Neighborhood Maximum Ensemble Probability for each categorical threshold (**cat_thresh**), neighborhood width (**nbrhd_prob.width**), and smoothing method (**nmep_smooth.type**) specified

13. Climatology mean (**climo_mean**) and standard deviation (**climo_stdev**) data regridded to the model domain

14. Climatological Distribution Percentile field for each CDP threshold specified

gen_ens_prod output
-------------------

The Gen-Ens-Prod tools writes a gridded NetCDF output file whose file name is specified using the -out command line option. The contents of that file depend on the contents of the **ens.field** array, the **ensemble_flag** options selected, and the presence of climatology data. The NetCDF variable names are self-describing and include the name/level of the field being processed, the type of ensemble product, and any relevant threshold information. If **nc_var_str** is defined for an **ens.field** array entry, that string is included in the corresponding NetCDF output variable names.

The Gen-Ens-Prod NetCDF output can be passed as input to the MET statistics tools, like Point-Stat and Grid-Stat, for futher processing and comparison against observations.
