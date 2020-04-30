.. _point-stat:

Point-Stat Tool
===============

7.1 Introduction
________________

The Point-Stat tool provides verification statistics for forecasts at observation points (as opposed to over gridded analyses). The Point-Stat tool matches gridded forecasts to point observation locations and supports several different interpolation options. The tool then computes continuous, categorical, spatial, and probabilistic verification statistics. The categorical and probabilistic statistics generally are derived by applying a threshold to the forecast and observation values. Confidence intervals - representing the uncertainty in the verification measures - are computed for the verification statistics.

Scientific and statistical aspects of the Point-Stat tool are discussed in the following section. Practical aspects of the Point-Stat tool are described in Section[sec:Practical-information].

Scientific and statistical aspects
__________________________________

The statistical methods and measures computed by the Point-Stat tool are described briefly in this section. In addition, Section [subsec:PS_Interpolation/matching-methods] discusses the various interpolation options available for matching the forecast grid point values to the observation points. The statistical measures computed by the Point-Stat tool are described briefly in Section [subsec:PS_Statistical-measures] and in more detail in Appendix [chap:App_C-Verification-Measures]. Section [subsec:PS_Statistical-confidence-intervals] describes the methods for computing confidence intervals that are applied to some of the measures computed by the Point-Stat tool; more detail on confidence intervals is provided in Appendix [chap:App_D-Confidence-Intervals].

7.2.1 Interpolation/matching methods
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This section provides information about the various methods available in MET to match gridded model output to point observations. Matching in the vertical and horizontal are completed separately using different methods.

In the vertical, if forecasts and observations are at the same vertical level, then they are paired as-is. If any discrepancy exists between the vertical levels, then the forecasts are interpolated to the level of the observation. The vertical interpolation is done in natural log of pressure coordinates, except for specific humidity, which is interpolated using the natural log of specific humidity in natural log of pressure coordinates. Vertical interpolation for heights above ground are done linear in height coordinates. When forecasts are for the surface, no interpolation is done. They are matched to observations with message types that are mapped to SURFACE in the message_type_group_map configuration option. By default, the surface message types include ADPSFC, SFCSHP, and MSONET. 

To match forecasts and observations in the horizontal plane, the user can select from a number of methods described below. Many of these methods require the user to define the width of the forecast grid W, around each observation point P, that should be considered. In addition, the user can select the interpolation shape, either a SQUARE or a CIRCLE. For example, a square of width 2 defines the 2 x 2 set of grid points enclosing P, or simply the 4 grid points closest to P. A square of width of 3 defines a 3 x 3 square consisting of 9 grid points centered on the grid point closest to P. Figure [MET_interpolation_methods] provides illustration. The point P denotes the observation location where the interpolated value is calculated. The interpolation width W, shown is five. 

This section describes the options for interpolation in the horizontal.

Diagram illustrating matching and interpolation methods used in MET. See text for explanation.

Illustration of some matching and interpolation methods used in MET. See text for explanation.



Nearest Neighbor

The forecast value at P is assigned the value at the nearest grid point. No interpolation is performed. Here, "nearest" means spatially closest in horizontal grid coordinates. This method is used by default when the interpolation width, W, is set to 1.



Geography Match

The forecast value at P is assigned the value at the nearest grid point in the interpolation area where the land/sea mask and topography criteria are satisfied.



Gaussian

The forecast value at P is a weighted sum of the values in the interpolation area. The weight given to each forecast point follows the Gaussian distribution with nearby points contributing more the far away points. The shape of the distribution is configured using sigma.

When used for regridding, with the regrid configuration option, or smoothing, with the interp configuration option in grid-to-grid comparisons, the Gaussian method is named MAXGAUSS and is implemented as a 2-step process. First, the data is regridded or smoothed using the maximum value interpolation method described below, where the width and shape define the interpolation area. Second, the Gaussian smoother, defined by the gaussian_dx and gaussian_radius configuration options, is applied.



Minimum value

The forecast value at P is the minimum of the values in the interpolation area.



Maximum value

The forecast value at P is the maximum of the values in the interpolation area.



Distance-weighted mean

The forecast value at P is a weighted sum of the values in the interpolation area. The weight given to each forecast point is the reciprocal of the square of the distance (in grid coordinates) from P. The weighted sum of forecast values is normalized by dividing by the sum of the weights. 



Unweighted mean

This method is similar to the distance-weighted mean, except all the weights are equal to 1. The distance of any point from P is not considered.



Median

The forecast value at P is the median of the forecast values in the interpolation area.



Least-Squares Fit

To perform least squares interpolation of a gridded field at a location P, MET uses an WxW subgrid centered (as closely as possible) at P. Figure [MET_interpolation_methods] shows the case where N = 5.

If we denote the horizontal coordinate in this subgrid by x, and vertical coordinate by y, then we can assign coordinates to the point P relative to this subgrid. These coordinates are chosen so that the center of the grid is. For example, in Figure [Fig_Interpolation_Methods], P has coordinates (-0.4, 0.2). Since the grid is centered near P, the coordinates of P should always be at most 0.5 in absolute value. At each of the vertices of the grid (indicated by black dots in the figure), we have data values. We would like to use these values to interpolate a value at P. We do this using least squares. If we denote the interpolated value by z, then we fit an expression of the form $z=\alpha (x) + \beta (y) + \gamma$ over the subgrid. The values of $\alpha, \beta, \gamma$ are calculated from the data values at the vertices. Finally, the coordinates (x,y) of P are substituted into this expression to give z, our least squares interpolated data value at P.



Bilinear Interpolation

This method is performed using the four closest grid squares. The forecast values are interpolated linearly first in one dimension and then the other to the location of the observation.



Upper Left, Upper Right, Lower Left, Lower Right Interpolation

This method is performed using the four closest grid squares. The forecast values are interpolated to the specified grid point.



Best Interpolation

The forecast value at P is the chosen as the grid point inside the interpolation area whose value most closely matches the observation value.

7.2.2 HiRA framework
~~~~~~~~~~~~~~~~~~~~

The Point-Stat tool has been enhanced to include the High Resolution Assessment (HiRA) verification logic (Mittermaier, 2014). HiRA is analogous to neighborhood verification but for point observations. The HiRA logic interprets the forecast values surrounding each point observation as an ensemble forecast. These ensemble values are processed in two ways. First, the ensemble continuous statistics (ECNT) and the ranked probability score (RPS) line types are computed directly from the ensemble values. Second, for each categorical threshold specified, a fractional coverage value is computed as the ratio of the nearby forecast values that meet the threshold criteria. Point-Stat evaluates those fractional coverage values as if they were a probability forecast. When applying HiRA, users should enable the matched pair (MPR), probabilistic (PCT, PSTD, PJC, or PRC), continuous ensemble statistics (ECNT), or ranked probability score (RPS) line types in the output_flag dictionary. The number of probabilistic HiRA output lines is determined by the number of categorical forecast thresholds and HiRA neighborhood widths chosen.

The HiRA framework provides a unique method for evaluating models in the neighborhood of point observations, allowing for some spatial and temporal uncertainty in the forecast and/or the observations. Additionally, the HiRA framework can be used to compare deterministic forecasts to ensemble forecasts. In MET, the neighborhood is a circle or square centered on the grid point closest to the observation location. An event is defined, then the proportion of points with events in the neighborhood is calculated. This proportion is treated as an ensemble probability, though it is likely to be uncalibrated. 

Figure1.3 shows a couple of examples of how the HiRA proportion is derived at a single model level using square neighborhoods. Events (in our case, model accretion values > 0) are separated from non-events (model accretion value = 0). Then, in each neighborhood, the total proportion of events is calculated. In the leftmost panel, four events exist in the 25 point neighborhood, making the HiRA proportion is 4/25 = 0.16. For the neighborhood of size 9 centered in that same panel, the HiRA proportion is 1/9. In the right panel, the size 25 neighborhood has HiRA proportion of 6/25, with the centered 9-point neighborhood having a HiRA value of 2/9. To extend this method into 3-dimensions, all layers within the user-defined layer are also included in the calculation of the proportion in the same manner. 



Often, the neighborhood size is chosen so that multiple models to be compared have approximately the same horizontal resolution. Then, standard metrics for probabilistic forecasts, such as Brier Score, can be used to compare those forecasts. HiRA was developed using surface observation stations so the neighborhood lies completely within the horizontal plane. With any type of upper air observation, the vertical neighborhood must also be defined. 

7.2.3 Statistical measures
~~~~~~~~~~~~~~~~~~~~~~~~~~

The Point-Stat tool computes a wide variety of verification statistics. Broadly speaking, these statistics can be subdivided into statistics for categorical variables and statistics for continuous variables. The categories of measures are briefly described here; specific descriptions of the measures are provided in Appendix [chap:App_C-Verification-Measures]. Additional information can be found in Wilks (2011) and Jolliffe and Stephenson (2003), and on the world-wide web at

http://www.bom.gov.au/bmrc/wefor/staff/eee/verif/verif_web_page.html.

In addition to these verification measures, the Point-Stat tool also computes partial sums and other FHO statistics that are produced by the NCEP verification system. These statistics are also described in Appendix [chap:App_C-Verification-Measures].

Measures for categorical variables

Categorical verification statistics are used to evaluate forecasts that are in the form of a discrete set of categories rather than on a continuous scale. If the original forecast is continuous, the user may specify one or more threhsolds in the configuration file to divide the continuous measure into categories. Currently, Point-Stat computes categorical statistics for variables in two or more categories. The special case of dichotomous (i.e., 2-category) variables has several types of statistics calculated from the resulting contingency table and are available in the CTS output line type. For multi-category variables, fewer statistics can be calculated so these are available separately, in line type MCTS. Categorical variables can be intrinsic (e.g., rain/no-rain) or they may be formed by applying one or more thresholds to a continuous variable (e.g., temperature < 273.15 K or cloud coverage percentages in 10% bins). See Appendix [chap:App_C-Verification-Measures] for more information.

Measures for continuous variables

For continuous variables, many verification measures are based on the forecast error (i.e., f - o). However, it also is of interest to investigate characteristics of the forecasts, and the observations, as well as their relationship. These concepts are consistent with the general framework for verification outlined by Murphy and Winkler (1987). The statistics produced by MET for continuous forecasts represent this philosophy of verification, which focuses on a variety of aspects of performance rather than a single measure. See Appendix [chap:App_C-Verification-Measures] for specific information.

A user may wish to eliminate certain values of the forecasts from the calculation of statistics, a process referred to here as``'conditional verification''. For example, a user may eliminate all temperatures above freezing and then calculate the error statistics only for those forecasts of below freezing temperatures. Another common example involves verification of wind forecasts. Since wind direction is indeterminate at very low wind speeds, the user may wish to set a minimum wind speed threshold prior to calculating error statistics for wind direction. The user may specify these threhsolds in the configuration file to specify the conditional verification. Thresholds can be specified using the usual Fortran conventions (<, <=, ==, !-, >=, or >) followed by a numeric value. The threshold type may also be specified using two letter abbreviations (lt, le, eq, ne, ge, gt). Further, more complex thresholds can be achieved by defining multiple thresholds and using && or || to string together event definition logic. The forecast and observation threshold can be used together according to user preference by specifying one of: UNION, INTERSECTION, or SYMDIFF (symmetric difference). 

Measures for probabilistic forecasts and dichotomous outcomes

For probabilistic forecasts, many verification measures are based on reliability, accuracy and bias. However, it also is of interest to investigate joint and conditional distributions of the forecasts and the observations, as in Wilks (2011). See Appendix [chap:App_C-Verification-Measures] for specific information.

Probabilistic forecast values are assumed to have a range of either 0 to 1 or 0 to 100. If the max data value is > 1, we assume the data range is 0 to 100, and divide all the values by 100. If the max data value is <= 1, then we use the values as is. Further, thresholds are applied to the probabilities with equality on the lower end. For example, with a forecast probability p, and thresholds t1 and t2, the range is defined as: t1 <= p < t2. The exception is for the highest set of thresholds, when the range includes 1: t1 <= p <= 1. To make configuration easier, in METv6.0, these probabilities may be specified in the configuration file as a list (>=0.00,>=0.25,>=0.50,>=0.75,>=1.00) or using shorthand notation (==0.25) for bins of equal width.

When the "prob" entry is set as a dictionary to define the field of interest, setting "prob_as_scalar = TRUE" indicates that this data should be processed as regular scalars rather than probabilities. For example, this option can be used to compute traditional 2x2 contingency tables and neighborhood verification statistics for probability data. It can also be used to compare two probability fields directly.

Measures for comparison against climatology

For each of the types of statistics mentioned above (categorical, continuous, and probabilistic), it is possible to calculate measures of skill relative to climatology. MET will accept a climatology file provided by the user, and will evaluate it as a reference forecast. Further, anomalies, i.e. departures from average conditions, can be calculated. As with all other statistics, the available measures will depend on the nature of the forecast. Common statistics that use a climatological reference include: the mean squared error skill score (MSESS), the Anomaly Correlation (ANOM_CORR), scalar and vector anomalies (SAL1L2 and VAL1L2), continuous ranked probability skill score (CRPSS), Brier Skill Score (BSS) (Wilks, 2011; Mason, 2004).

Often, the sample climatology is used as a reference by a skill score. The sample climatology is the average over all included observations and may be transparent to the user. This is the case in most categorical skill scores. The sample climatology will probably prove more difficult to improve upon than a long term climatology, since it will be from the same locations and time periods as the forecasts. This may mask legitimate forecast skill. However, a more general climatology, perhaps covering many years, is often easier to improve upon and is less likely to mask real forecast skill.

7.2.4 Statistical confidence intervals
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A single summary score gives an indication of the forecast performance, but it is a single realization from a random process that neglects uncertainty in the score's estimate. That is, it is possible to obtain a good score, but it may be that the "good" score was achieved by chance and does not reflect the "true" score. Therefore, when interpreting results from a verification analysis, it is imperative to analyze the uncertainty in the realized scores. One good way to do this is to utilize confidence intervals. A confidence interval indicates that if the process were repeated many times, say 100, then the true score would fall within the interval $100(1-\alpha)\%$ of the time. Typical values of $\alpha$ are 0.01, 0.05, and 0.10. The Point-Stat tool allows the user to select one or more specific $\alpha$-values to use.

For continuous fields (e.g., temperature), it is possible to estimate confidence intervals for some measures of forecast performance based on the assumption that the data, or their errors, are normally distributed. The Point-Stat tool computes confidence intervals for the following summary measures: forecast mean and standard deviation, observation mean and standard deviation, correlation, mean error, and the standard deviation of the error. In the case of the respective means, the central limit theorem suggests that the means are normally distributed, and this assumption leads to the usual $100(1-\alpha)\%$ confidence intervals for the mean. For the standard deviations of each field, one must be careful to check that the field of interest is normally distributed, as this assumption is necessary for the interpretation of the resulting confidence intervals.

For the measures relating the two fields (i.e., mean error, correlation and standard deviation of the errors), confidence intervals are based on either the joint distributions of the two fields (e.g., with correlation) or on a function of the two fields. For the correlation, the underlying assumption is that the two fields follow a bivariate normal distribution. In the case of the mean error and the standard deviation of the mean error, the assumption is that the errors are normally distributed, which for continuous variables, is usually a reasonable assumption, even for the standard deviation of the errors.

Bootstrap confidence intervals for any verification statistic are available in MET. Bootstrapping is a nonparametric statistical method for estimating parameters and uncertainty information. The idea is to obtain a sample of the verification statistic(s) of interest (e.g., bias, ETS, etc.) so that inferences can be made from this sample. The assumption is that the original sample of matched forecast-observation pairs is representative of the population. Several replicated samples are taken with replacement from this set of forecast-observation pairs of variables (e.g., precipitation, temperature, etc.), and the statistic(s) are calculated for each replicate. That is, given a set of n forecast-observation pairs, we draw values at random from these pairs, allowing the same pair to be drawn more than once, and the statistic(s) is (are) calculated for each replicated sample. This yields a sample of the statistic(s) based solely on the data without making any assumptions about the underlying distribution of the sample. It should be noted, however, that if the observed sample of matched pairs is dependent, then this dependence should be taken into account somehow. Currently, in the confidence interval methods in MET do not take into account dependence, but future releases will support a robust method allowing for dependence in the original sample. More detailed information about the bootstrap algorithm is found in the appendix.

Confidence intervals can be calculated from the sample of verification statistics obtained through the bootstrap algorithm. The most intuitive method is to simply take the appropriate quantiles of the sample of statistic(s). For example, if one wants a 95% CI, then one would take the 2.5 and 97.5 percentiles of the resulting sample. This method is called the percentile method, and has some nice properties. However, if the original sample is biased and/or has non-constant variance, then it is well known that this interval is too optimistic. The most robust, accurate, and well-behaved way to obtain accurate CIs from bootstrapping is to use the bias corrected and adjusted percentile method (or BCa). If there is no bias, and the variance is constant, then this method will yield the usual percentile interval. The only drawback to the approach is that it is computationally intensive. Therefore, both the percentile and BCa methods are available in MET, with the considerably more efficient percentile method being the default.

The only other option associated with bootstrapping currently available in MET is to obtain replicated samples smaller than the original sample (i.e., to sample $m<n$ points at each replicate). Ordinarily, one should use $m=n$, and this is the default. However, there are cases where it is more appropriate to use a smaller value of m (e.g., when making inference about high percentiles of the original sample). See Gilleland (2008) for more information and references about this topic.

MET provides parametric confidence intervals based on assumptions of normality for the following categorical statistics:

• Base Rate

• Forecast Mean

• Accuracy

• Probability of Detection

• Probability of Detection of the non-event

• Probability of False Detection

• False Alarm Ratio

• Critical Success Index

• Hanssen-Kuipers Discriminant

• Odds Ratio

• Log Odds Ratio

• Odds Ratio Skill Score

• Extreme Dependency Score

• Symmetric Extreme Dependency Score

• Extreme Dependency Index

• Symmetric Extremal Dependency Index

MET provides parametric confidence intervals based on assumptions of normality for the following continuous statistics:

• Forecast and Observation Means

• Forecast, Observation, and Error Standard Deviations

• Pearson Correlation Coefficient

• Mean Error

MET provides parametric confidence intervals based on assumptions of normality for the following probabilistic statistics:

• Brier Score

• Base Rate

MET provides non-parametric bootstrap confidence intervals for many categorical and continuous statistics. Kendall's Tau and Spearman's Rank correlation coefficients are the only exceptions. Computing bootstrap confidence intervals for these statistics would be computationally unrealistic.

For more information on confidence intervals pertaining to verification measures, see Wilks (2011), Jolliffe and Stephenson (2003), and Bradley (2008).

7.3 Practical information
_________________________

The Point-Stat tool is used to perform verification of a gridded model field using point observations. The gridded model field to be verified must be in one of the supported file formats. The point observations must be formatted as the NetCDF output of the point reformatting tools described in Chapter [chap:Re-Formatting-of-Point]. The Point-Stat tool provides the capability of interpolating the gridded forecast data to the observation points using a variety of methods as described in Section [subsec:PS_Interpolation/matching-methods]. The Point-Stat tool computes a number of continuous statistics on the matched pair data as well as discrete statistics once the matched pair data have been thresholded.

7.3.1 point_stat usage
~~~~~~~~~~~~~~~~~~~~~~

The usage statement for the Point-Stat tool is shown below:

Usage: point_stat

{\hskip 0.5in}fcst_file

{\hskip 0.5in}obs_file

{\hskip 0.5in}config_file

{\hskip 0.5in}[-point_obs file]

{\hskip 0.5in}[-obs_valid_beg time]

{\hskip 0.5in}[-obs_valid_end time]

{\hskip 0.5in}[-outdir path]

{\hskip 0.5in}[-log file]

{\hskip 0.5in}[-v level]

point_stat has three required arguments and can take many optional ones.

Required arguments for point_stat

1. The fcst_file argument names the gridded file in either GRIB or NetCDF containing the model data to be verified.

2. The obs_file argument indicates the NetCDF file (output of PB2NC or ASCII2NC) containing the point observations to be used for verifying the model.

3. The config_file argument indicates the name of the configuration file to be used. The contents of the configuration file are discussed below.

Optional arguments for point_stat

4. The -point_obs file may be used to pass additional NetCDF point observation files to be used in the verification. 

5. The -obs_valid_beg time option in YYYYMMDD[_HH[MMSS]] format sets the beginning of the observation matching time window, overriding the configuration file setting.

6. The -obs_valid_end time option in YYYYMMDD[_HH[MMSS]] format sets the end of the observation matching time window, overriding the configuration file setting.

7. The -outdir path indicates the directory where output files should be written. 

8. The -log file option directs output and errors to the specified log file. All messages will be written to that file as well as standard out and error. Thus, users can save the messages without having to redirect the output on the command line. The default behavior is no log file. 

9. The -v level option indicates the desired level of verbosity. The value of "level" will override the default setting of 2. Setting the verbosity to 0 will make the tool run with no log messages, while increasing the verbosity will increase the amount of logging.

An example of the point_stat calling sequence is shown below:

point_stat sample_fcst.grb \

sample_pb.nc \

PointStatConfig

In this example, the Point-Stat tool evaluates the model data in the sample_fcst.grb GRIB file using the observations in the NetCDF output of PB2NC, sample_pb.nc, applying the configuration options specified in the PointStatConfig file.

7.3.2 point_stat configuration file
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The default configuration file for the Point-Stat tool named PointStatConfig_default can be found in the installed share/met/config directory. Another version is located in scripts/config. We encourage users to make a copy of these files prior to modifying their contents. The contents of the configuration file are described in the subsections below.

Note that environment variables may be used when editing configuration files, as described in Section [subsec:pb2nc-configuration-file] for the PB2NC tool.



model          = "WRF";

desc           = "NA";

regrid         = { ... }

climo_mean     = { ... }

climo_stdev    = { ... }

climo_cdf      = { ... }

obs_window     = { beg = -5400; end =  5400; }

mask           = { grid = [ "FULL" ]; poly = []; sid = []; }

ci_alpha       = [ 0.05 ];

boot           = { interval = PCTILE; rep_prop = 1.0; n_rep = 1000;

                   rng = "mt19937"; seed = ""; }

interp         = { vld_thresh = 1.0; shape = SQUARE;

                   type = [ { method = NEAREST; width = 1; } ]; }



censor_thresh  = [];

censor_val     = [];

eclv_points    = 0.05;

rank_corr_flag = TRUE;



sid_inc        = [];

sid_exc        = [];

duplicate_flag = NONE;

obs_quality    = [];

obs_summary    = NONE;

obs_perc_value = 50;



message_type_group_map = [...];



tmp_dir        = "/tmp";

output_prefix  = "";

version        = "VN.N";

The configuration options listed above are common to many MET tools and are described in Section [subsec:IO_General-MET-Config-Options].



Setting up the fcst and obs dictionaries of the configuration file is described in Section [subsec:IO_General-MET-Config-Options]. The following are some special consideration for the Point-Stat tool.

The obs dictionary looks very similar to the fcst dictionary. When the forecast and observation variables follow the same naming convention, one can easily copy over the forecast settings to the observation dictionary using obs = fcst;. However when verifying forecast data in NetCDF format or verifying against not-standard observation variables, users will need to specify the fcst and obs dictionaries separately. The number of fields specified in the fcst and obs dictionaries must match.

The message_type entry, defined in the obs dictionary, contains a comma-separated list of the message types to use for verification. At least one entry must be provided. The Point-Stat tool performs verification using observations for one message type at a time. See http://www.emc.ncep.noaa.gov/mmb/data_processing/PrepBUFR.doc/table_1.htm for a list of the possible types. If using obs = fcst;, it can be defined in the forecast dictionary and the copied into the observation dictionary.



land_mask = {

   flag      = FALSE;

   file_name = [];

   field     = { name = "LAND"; level = "L0"; }

   regrid    = { method = NEAREST; width = 1; }

   thresh = eq1;

}

The land_mask dictionary defines the land/sea mask field which is used when verifying at the surface. For point observations whose message type appears in the LANDSF entry of the message_type_group_map setting, only use forecast grid points where land = TRUE. For point observations whose message type appears in the WATERSF entry of the message_type_group_map setting, only use forecast grid points where land = FALSE. The flag entry enables/disables this logic. If the file_name is left empty, then the land/sea is assumed to exist in the input forecast file. Otherwise, the specified file(s) are searched for the data specified in the field entry. The regrid settings specify how this field should be regridded to the verification domain. Lastly, the thresh entry is the threshold which defines land (threshold is true) and water (threshold is false).



topo_mask = {

   flag               = FALSE;

   file_name          = [];

   field              = { name = "TOPO"; level = "L0"; }

   regrid             = { method = BILIN; width = 2; }

   use_obs_thresh     = ge-100&&le100;

   interp_fcst_thresh = ge-50&&le50;

}

The topo_mask dictionary defines the model topography field which is used when verifying at the surface. This logic is applied to point observations whose message type appears in the SURFACE entry of the message_type_group_map setting. Only use point observations where the topo - station elevation difference meets the use_obs_thresh threshold entry. For the observations kept, when interpolating forecast data to the observation location, only use forecast grid points where the topo - station difference meets the interp_fcst_thresh threshold entry. The flag entry enables/disables this logic. If the file_name is left empty, then the topography data is assumed to exist in the input forecast file. Otherwise, the specified file(s) are searched for the data specified in the field entry. The regrid settings specify how this field should be regridded to the verification domain.



hira = {

   flag            = FALSE;

   width           = [ 2, 3, 4, 5 ]

   vld_thresh      = 1.0;

   cov_thresh      = [ ==0.25 ];

   shape           = SQUARE;

   prob_cat_thresh = [];

}

The hira dictionary that is very similar to the interp and nbrhd entries. It specifies information for applying the High Resolution Assessment (HiRA) verification logic described in section [subsec:PS_HiRA_framework]. The flag entry is a boolean which toggles HiRA on (TRUE) and off (FALSE). The width and shape entries define the neighborhood size and shape, respectively. Since HiRA applies to point observations, the width may be even or odd. The vld_thresh entry is the required ratio of valid data within the neighborhood to compute an output value. The cov_thresh entry is an array of probabilistic thresholds used to populate the Nx2 probabilistic contingency table written to the PCT output line and used for computing probabilistic statistics. The prob_cat_thresh entry defines the thresholds to be used in computing the ranked probability score in the RPS output line type. If left empty but climatology data is provided, the climo_cdf thresholds will be used instead of prob_cat_thresh.



output_flag = {

   fho    = BOTH;

   ctc    = BOTH;

   cts    = BOTH;

   mctc   = BOTH;

   mcts   = BOTH;

   cnt    = BOTH;

   sl1l2  = BOTH;

   sal1l2 = BOTH;

   vl1l2  = BOTH;

   vcnt   = BOTH;

   val1l2 = BOTH;

   pct    = BOTH;

   pstd   = BOTH;

   pjc    = BOTH;

   prc    = BOTH;

   ecnt   = BOTH;  // Only for HiRA

   rps    = BOTH;  // Only for HiRA

   eclv   = BOTH;

   mpr    = BOTH;

}

The output_flag array controls the type of output that the Point-Stat tool generates. Each flag corresponds to an output line type in the STAT file. Setting the flag to NONE indicates that the line type should not be generated. Setting the flag to STAT indicates that the line type should be written to the STAT file only. Setting the flag to BOTH indicates that the line type should be written to the STAT file as well as a separate ASCII file where the data is grouped by line type. The output flags correspond to the following output line types:

1. FHO for Forecast, Hit, Observation Rates

2. CTC for Contingency Table Counts

3. CTS for Contingency Table Statistics

4. MCTC for Multi-category Contingency Table Counts

5. MCTS for Multi-category Contingency Table Statistics

6. CNT for Continuous Statistics

7. SL1L2 for Scalar L1L2 Partial Sums

8. SAL1L2 for Scalar Anomaly L1L2 Partial Sums when climatological data is supplied

9. VL1L2 for Vector L1L2 Partial Sums

10. VCNT for Vector Continuous Statistics (Note that bootstrap confidence intervals are not currently calculated for this line type.)

11. VAL1L2 for Vector Anomaly L1L2 Partial Sums when climatological data is supplied

12. PCT for Contingency Table counts for Probabilistic forecasts

13. PSTD for contingency table Statistics for Probabilistic forecasts with Dichotomous outcomes

14. PJC for Joint and Conditional factorization for Probabilistic forecasts

15. PRC for Receiver Operating Characteristic for Probabilistic forecasts

16. ECNT for Ensemble Continuous Statistics is only computed for the HiRA methodology

17. RPS for Ranked Probability Score is only computed for the HiRA methodology

18. ECLV for Economic Cost/Loss Relative Value

19. MPR for Matched Pair data

Note that the first two line types are easily derived from each other. Users are free to choose which measures are most desired. The output line types are described in more detail in Section [subsec:point_stat-output].

Note that writing out matched pair data (MPR lines) for a large number of cases is generally not recommended. The MPR lines create very large output files and are only intended for use on a small set of cases.

If all line types corresponding to a particular verification method are set to NONE, the computation of those statistics will be skipped in the code and thus make the Point-Stat tool run more efficiently. For example, if FHO, CTC, and CTS are all set to NONE, the Point-Stat tool will skip the categorical verification step.

7.3.3 point_stat output
~~~~~~~~~~~~~~~~~~~~~~~

point_stat produces output in STAT and, optionally, ASCII format. The ASCII output duplicates the STAT output but has the data organized by line type. The output files will be written to the default output directory or the directory specified using the "-outdir" command line option.

The output STAT file will be named using the following naming convention:

point_stat_PREFIX_HHMMSSL_YYYYMMDD_HHMMSSV.stat where PREFIX indicates the user-defined output prefix, HHMMSSL indicates the forecast lead time and YYYYMMDD_HHMMSS indicates the forecast valid time. 

The output ASCII files are named similarly: 

point_stat_PREFIX_HHMMSSL_YYYYMMDD_HHMMSSV_TYPE.txt where TYPE is one of mpr, fho, ctc, cts, cnt, mctc, mcts, pct, pstd, pjc, prc, ecnt, rps, eclv, sl1l2, sal1l2, vl1l2, vcnt or val1l2 to indicate the line type it contains.

The first set of header columns are common to all of the output files generated by the Point-Stat tool. Tables describing the contents of the header columns and the contents of the additional columns for each line type are listed in the following tables. The ECNT line type is described in Section [table_ES_header_info_es_out_ECNT]. The RPS line type is described in Section [table_ES_header_info_es_out_RPS].

Header information for each file point-stat outputs.







Format information for CNT(Continuous Statistics) output line type.

Format information for CNT(Continuous Statistics) output line type continued from above tableFormat information for MCTC (Multi-category Contingency Table Count) output line type.

Format information for MCTS (Multi- category Contingency Table Statistics) output line type.

Format information for PCT (Contingency Table Counts for Probabilistic forecasts) output line type.

Format information for PSTD (Contingency Table Statistics for Probabilistic forecasts) output line type.

Format information for PJC (Joint and Conditional factorization for Probabilistic forecasts) output line type.

Format information for PRC (PRC for Receiver Operating Characteristic for Probabilistic forecasts) output line type.

Format information for ECLV (ECLV for Economic Cost/Loss Relative Value) output line type.

Format information for SL1L2 (Scalar Partial Sums) output line type.

Format information for SAL1L2 (Scalar Anomaly Partial Sums) output line type.

Format information for VL1L2 (Vector Partial Sums) output line type.

Format information for VAL1L2 (Vector Anomaly Partial Sums) output line type.

Format information for VAL1L2 (Vector Anomaly Partial Sums) output line type. Note that each statistic (except TOTAL) is followed by two columns giving bootstrap confidence intervals. These confidence intervals are not currently calculated for this release of MET, but will be in future releases.

Format information for MPR (Matched Pair) output line type.

The STAT output files described for point_stat may be used as inputs to the Stat-Analysis tool. For more information on using the Stat-Analysis tool to create stratifications and aggregations of the STAT files produced by point_stat, please see Chapter [chap:The-Stat-Analysis-Tool]. 
