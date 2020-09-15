.. _mode:

MODE Tool
=========

.. _MODE_Introduction:

Introduction
____________

This section provides a description of the Method for Object-Based Diagnostic Evaluation (MODE) tool, which was developed at the Research Applications Laboratory, NCAR/Boulder, USA. More information about MODE can be found in :ref:`Davis et al. (2006a,b) <Davis-2006>`, :ref:`Brown et al. (2007) <Brown-2007>` and :ref:`Bullock et al. (2016) <Bullock-2016>`.

MODE was developed in response to a need for verification methods that can provide diagnostic information that is more directly useful and meaningful than the information that can be obtained from traditional verification approaches, especially in application to high-resolution NWP output. The MODE approach was originally developed for application to spatial precipitation forecasts, but it can also be applied to other fields with coherent spatial structures (*e.g.*, clouds, convection). MODE is only one of a number of different approaches that have been developed in recent years to meet these needs. In the future, we expect that the MET package will include additional methods. References for many of these methods are provided at the `MesoVict website <http://www.rap.ucar.edu/projects/icp/index.html.>`_.

MODE may be used in a generalized way to compare any two fields. For simplicity, field_1 may be thought of in this section as "the forecast", while field_2 may be thought of as "the observation", which is usually a gridded analysis of some sort. The convention of field_1/field_2 is also used in :numref:`MODE_object_attribute`. MODE resolves objects in both the forecast and observed fields. These objects mimic what humans would call "regions of interest". Object attributes are calculated and compared, and are used to associate ("merge") objects within a single field, as well as to "match" objects between the forecast and observed fields. Finally, summary statistics describing the objects and object pairs are produced. These statistics can be used to identify correlations and differences among the objects, leading to insights concerning forecast strengths and weaknesses.

.. _MODE_Scientific-and-statistical:

Scientific and statistical aspects
__________________________________

The methods used by the MODE tool to identify and match forecast and observed objects are briefly described in this section. 

Resolving objects
~~~~~~~~~~~~~~~~~

The process used for resolving objects in a raw data field is called *convolution thresholding*. The raw data field is first convolved with a simple filter function as follows:

.. math:: C(x,y)=\sum_{u, v}\phi(u,v)\ f(x-u, y-v)

In this formula, :math:`f` is the raw data field, :math:`\phi` is the filter function, and :math:`C` is the resulting convolved field. The variables :math:`(x, y)` and :math:`(u, v)` are grid coordinates. The filter function :math:`\phi` is a simple circular filter determined by a radius of influence :math:`R` , and a height :math:`H` :

.. math:: \phi (x,y) = \begin{eqnarray}\begin{cases} H &\text{if } x^2 + y^2\leq R^2\\ 0 &\text{otherwise.} \end{cases}\end{eqnarray}

The parameters :math:`R` and :math:`H` are not independent. They are related by the requirement that the integral of :math:`\phi` over the grid be unity: 

.. math:: \pi R^2 H\text{ = 1.}

Thus, the radius of influence :math:`R` is the only tunable parameter in the convolution process. Once :math:`R` is chosen, :math:`H` is determined by the above equation.

Once the convolved field :math:`C` is in hand, it is thresholded to create a mask field :math:`M` :

.. math:: M(x,y) = \begin{eqnarray}\begin{cases} 1 &\text{if } C(x,y)\ge T\\ 0 &\text{otherwise.} \end{cases}\end{eqnarray}

where :math:`T` is the threshold. The objects are the connected regions where :math:`M = 1` . Finally, the raw data are restored to object interiors to obtain the object field :math:`F` :

.. math:: F(x,y)=M(x,y)f(x,y).

Thus, two parameters — the radius of influence :math:`R`, and the threshold :math:`T` — control the entire process of resolving objects in the raw data field.

An example of the steps involved in resolving objects is shown in :numref:`mode-object_id`. It shows a "raw" precipitation field, where the vertical coordinate represents the precipitation amount. Part (b) shows the convolved field, and part (c) shows the masked field obtained after the threshold is applied. Finally, :numref:`mode-object_id` shows the objects once the original precipitation values have been restored to the interiors of the objects.

.. _mode-object_id:

.. figure:: figure/mode-object_id.png

   Example of an application of the MODE object identification process to a model precipitation field.


Attributes
~~~~~~~~~~

Object attributes are defined both for single objects and for object pairs. One of the objects in a pair is from the forecast field and the other is taken from the observed field. 

**Area** is simply a count of the number of grid squares an object occupies. If desired, a true area (say, in :math:`km^2`) can be obtained by adding up the true areas of all the grid squares inside an object, but in practice this is seldom necessary.

Moments are used in the calculation of several object attributes. If we define :math:`\xi(x,y)` to be 1 for points :math:`(x,y)` inside our object, and zero for points outside, then the first-order moments, :math:`S_x` and :math:`S_y`, are defined as 

.. math:: S_x = \sum_{x,y} x\xi(x,y) {}\ \text{and } {}\ S_y = \sum_{x,y} y\xi(x,y)

Higher order moments are similarly defined and are used in the calculation of some of the other attributes. For example, the **centroid** is a kind of geometric center of an object, and can be calculated from first moments. It allows one to assign a single point location to what may be a large, extended object. 

**Axis Angle**, denoted by :math:`\theta`, is calculated from the second-order moments. It gives information on the orientation or "tilt" of an object. **Curvature** is another attribute that uses moments in its calculation, specifically, third-order moments.

**Aspect Ratio** is computed by fitting a rectangle around an object. The rectangle is aligned so that it has the same axis angle as the object, and the length and width are chosen so as to just enclose the object. We make no claim that the rectangle so obtained is the smallest possible rectangle enclosing the given object. However, this rectangle is much easier to calculate than a smaller enclosing rectangle and serves our purposes just as well. Once the rectangle is determined, the aspect ratio of the object is defined to be the width of the fitted rectangle divided by its length.

Another object attribute defined by MODE is **complexity**. Complexity is defined by comparing the area of an object to the area of its convex hull.

All the attributes discussed so far are defined for single objects. Once these are determined, they can be used to calculate attributes for pairs of objects. One example is the  **centroid difference**. This measure is simply the (vector) difference between the centroids of the two objects. Another example is the  **angle difference**. This is the difference between the axis angles.

Several area measures are also used for pair attributes. **Union Area** is the total area that is in either one (or both) of the two objects. **Intersection Area** is the area that is inside both objects simultaneously. **Symmetric Difference** is the area inside at least one object, but not inside both.

Fuzzy logic
~~~~~~~~~~~

Once object attributes :math:`\alpha_1,\alpha_2,\ldots,\alpha_n` are estimated, some of them are used as input to a fuzzy logic engine that performs the matching and merging steps. **Merging** refers to grouping together objects in a single field, while **matching** refers to grouping together objects in different fields, typically the forecast and observed fields. Interest maps :math:`I_i` are applied to the individual attributes :math:`\alpha_i` to convert them into interest values, which range from zero (representing no interest) to one (high interest). For example, the default interest map for centroid difference is one for small distances, and falls to zero as the distance increases. For other attributes (*e.g.*, intersection area), low values indicate low interest, and high values indicate more interest.

The next step is to define confidence maps :math:`C_i` for each attribute. These maps (again with values ranging from zero to one) reflect how confident we are in the calculated value of an attribute. The confidence maps generally are functions of the entire attribute vector :math:`\alpha = (\alpha_1, \alpha_2, \ldots, \alpha_n)`, in contrast to the interest maps, where each :math:`I_i` is a function only of :math:`\alpha_i`. To see why this is necessary, imagine an electronic anemometer that outputs a stream of numerical values of wind speed and direction. It is typically the case for such devices that when the wind speed becomes small enough, the wind direction is poorly resolved. The wind must be at least strong enough to overcome friction and turn the anemometer. Thus, in this case, our confidence in one attribute (wind direction) is dependent on the value of another attribute (wind speed). In MODE, all of the confidence maps except the map for axis angle are set to a constant value of 1. The axis angle confidence map is a function of aspect ratio, with values near one having low confidence, and values far from one having high confidence.

Next, scalar weights :math:`\boldsymbol{w}_i` are assigned to each attribute, representing an empirical judgment regarding the relative importance of the various attributes. As an example, the initial development of MODE, centroid distance was weighted more heavily than other attributes, because the location of storm systems close to each other in space seemed to be a strong indication (stronger than that given by any other attribute) that they were related.

Finally, all these ingredients are collected into a single number called the total interest, :math:`\boldsymbol{T}`, given by:

.. math:: T(\alpha)=\frac{\sum_{i}w_i C_i(\alpha)I_i(\alpha_i)}{\sum_{i}w_i C_i(\alpha)}

This total interest value is then thresholded, and pairs of objects that have total interest values above the threshold are merged (if they are in the same field) or matched (if they are in different fields).

Another merging method is available in MODE, which can be used instead of, or along with, the fuzzy logic based merging just described. Recall that the convolved field is thresholded to produce the mask field. A second (lower) threshold can be specified so that objects that are separated at the higher threshold but joined at the lower threshold are merged.

Summary statistics
~~~~~~~~~~~~~~~~~~

Once MODE has been run, summary statistics are written to an output file. These files contain information about all single and cluster objects and their attributes. Total interest for object pairs is also output, as are percentiles of intensity inside the objects. The output file is in a simple flat ASCII tabular format (with one header line) and thus should be easily readable by just about any programming language, scripting language, or statistics package. Refer to :numref:`MODE-output` for lists of the statistics included in the MODE output files. Example scripts will be posted on the MET website in the future.

Practical information
_____________________

This section contains a description of how MODE can be configured and run. The MODE tool is used to perform a features-based verification of gridded model data using gridded observations. The input gridded model and observation datasets must be in one of the MET supported gridded file formats. The requirement of having all gridded fields using the same grid specification has been removed with METv5.1. The Grid-Stat tool performs no interpolation when the input model, observation, and climatology datasets must be on a common grid. MET will interpolate these files to a common grid if one is specified. There is a regrid option in the configuration file that allows the user to define the grid upon which the scores will be computed. The gridded analysis data may be based on observations, such as Stage II or Stage IV data for verifying accumulated precipitation, or a model analysis field may be used. However, users are cautioned that it is generally unwise to verify model output using an analysis field produced by the same model.

MODE provides the capability to select a single model variable/level from which to derive objects to be analyzed. MODE was developed and tested using accumulated precipitation. However, the code has been generalized to allow the use of any gridded model and observation field. Based on the options specified in the configuration file, MODE will define a set of simple objects in the model and observation fields. It will then compute an interest value for each pair of objects across the fields using a fuzzy engine approach. Those interest values are thresholded, and any pairs of objects above the threshold will be matched/merged. Through the configuration file, MODE offers a wide range of flexibility in how the objects are defined, processed, matched, and merged.

mode usage
~~~~~~~~~~

The usage statement for the MODE tool is listed below:

.. code-block:: none

  Usage: mode
         fcst_file
         obs_file
         config_file
         [-config_merge merge_config_file]
         [-outdir path]
         [-log file]
         [-v level]
         [-compress level]

The MODE tool has three required arguments and can accept several optional arguments.

Required arguments for mode
^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. The **fcst_file** argument indicates the gridded file containing the model field to be verified.

2. The **obs_file** argument indicates the gridded file containing the gridded observations to be used for the verification of the model.

3. The **config_file** argument indicates the name of the configuration file to be used. The contents of the configuration file are discussed below.

Optional arguments for mode
^^^^^^^^^^^^^^^^^^^^^^^^^^^

4. The **-config_merge merge_config_file** option indicates the name of a second configuration file to be used when performing fuzzy engine merging by comparing the model or observation field to itself. The MODE tool provides the capability of performing merging within a single field by comparing the field to itself. Interest values are computed for each object and all of its neighbors. If an object and its neighbor have an interest value above some threshold, they are merged. The **merge_config_file** controls the settings of the fuzzy engine used to perform this merging step. If a **merge_config_file** is not provided, the configuration specified by the config_file in the previous argument will be used.

5. The **-outdir path** option indicates the directory where output files should be written.

6. The **-log file** option directs output and errors to the specified log file. All messages will be written to that file as well as standard out and error. Thus, users can save the messages without having to redirect the output on the command line. The default behavior is no log file. 

7. The **-v level** option indicates the desired level of verbosity. The contents of "level" will override the default setting of 2. Setting the verbosity to 0 will make the tool run with no log messages, while increasing the verbosity above 1 will increase the amount of logging.

8. The **-compress level** option indicates the desired level of compression (deflate level) for NetCDF variables. The valid level is between 0 and 9. The value of “level” will override the default setting of 0 from the configuration file or the environment variable MET_NC_COMPRESS. Setting the compression level to 0 will make no compression for the NetCDF output. Lower number is for fast compression and higher number is for better compression.

An example of the MODE calling sequence is listed below:

**Example 1:**

.. code-block:: none

  mode sample_fcst.grb \
  sample_obs.grb \
  MODEConfig_grb

In Example 1, the MODE tool will verify the model data in the sample_fcst.grb GRIB file using the observations in the sample_obs.grb GRIB file applying the configuration options specified in the MODEConfig_grb file.

A second example of the MODE calling sequence is presented below:

**Example 2:**

.. code-block:: none

  mode sample_fcst.nc \
  sample_obs.nc \
  MODEConfig_nc

In Example 2, the MODE tool will verify the model data in the sample_fcst.nc NetCDF output of pcp_combine using the observations in the sample_obs.nc NetCDF output of pcp_combine, using the configuration options specified in the MODEConfig_nc file. Since the model and observation files contain only a single field of accumulated precipitation, the MODEConfig_nc file should specify that accumulated precipitation be verified.

.. _MODE-configuration-file:

mode configuration file
~~~~~~~~~~~~~~~~~~~~~~~

The default configuration file for the MODE tool, MODEConfig_default, can be found in the installed share/met/config directory. Another version of the configuration file is provided in scripts/config. We encourage users to make a copy of the configuration files prior to modifying their contents. Descriptions of MODEConfig_default and the required variables for any MODE configuration file are also provided below. While the configuration file contains many entries, most users will only need to change a few for their use. Specific options are described in the following subsections.

Note that environment variables may be used when editing configuration files, as described in :numref:`pb2nc configuration file` for the PB2NC tool.

_____________________

.. code-block:: none

  model          = "WRF";
  desc           = "NA";
  obtype         = "ANALYS";
  regrid         = { ... }
  met_data_dir   = "MET_BASE";
  output_prefix  = "";
  version        = "VN.N";

The configuration options listed above are common to many MET tools and are described in :numref:`Data IO MET Configuration File Options`.


_____________________

.. code-block:: none

  grid_res = 4;

The **grid_res** entry is the nominal spacing for each grid square in kilometers. This entry is not used directly in the code, but subsequent entries in the configuration file are defined in terms of it. Therefore, setting this appropriately will help ensure that appropriate default values are used for these entries.

_____________________

.. code-block:: none

  quilt = FALSE;

The **quilt** entry indicates whether all permutations of convolution radii and thresholds should be run.

• If **FALSE**, the number of forecast and observation convolution radii and thresholds must all match. One configuration of MODE will be run for each group of settings in those lists.

• If **TRUE**, the number of forecast and observation convolution radii must match and the number of forecast and observation convolution thresholds must match. For N radii and M thresholds, NxM configurations of MODE will be run.

_____________________

.. code-block:: none

  fcst = {
     field = {
        name = "APCP";
        level = "A03";
     }
     censor_thresh      = [];
     censor_val         = [];
     conv_radius        = 60.0/grid_res; // in grid squares
     conv_thresh        = >=5.0;
     vld_thresh         = 0.5;
     filter_attr_name   = [];
     filter_attr_thresh = [];
     merge_thresh       = >=1.25;
     merge_flag         = THRESH;
  }
  obs = fcst; 

The **field** entries in the forecast and observation dictionaries specify the model and observation variables and level to be compared. See a more complete description of them in :numref:`Data IO MET Configuration File Options`. In the above example, the forecast settings are copied into the observation dictionary using **obs = fcst;.**

The **censor_thresh** and **censor_val** entries are used to censor the raw data as described in :numref:`Data IO MET Configuration File Options`. Their functionality replaces the **raw_thresh** entry, which is deprecated in met-6.1. Prior to defining objects, it is recommended that the raw fields should be made to look similar to each other. For example, if the model only predicts values for a variable above some threshold, the observations should be thresholded at that same level. The censor thresholds can be specified using symbols. By default, no censor thresholding is applied.

The **conv_radius** entry defines the radius of the circular convolution applied to smooth the raw fields. The radii are specified in terms of grid units. The default convolution radii are defined in terms of the previously defined **grid_res** entry. Multiple convolution radii may be specified as an array (e.g. **conv_radius = [ 5, 10, 15 ];**).

The **conv_thresh** entry specifies the threshold values to be applied to the convolved field to define objects. By default, objects are defined using a convolution threshold of 5.0. Multiple convolution thresholds may be specified as an array (e.g. **conv_thresh = [ >=5.0, >=10.0, >=15.0 ];)**.

Multiple convolution radii and thresholds and processed using the logic defined by the **quilt** entry.

The **vld_thresh** entry must be set between 0 and 1. When performing the circular convolution step if the proportion of bad data values in the convolution area is greater than or equal to this threshold, the resulting convolved value will be bad data. If the proportion is less than this threshold, the convolution will be performed on only the valid data. By default, the **vld_thresh** is set to 0.5.

The **filter_attr_name** and **filter_attr_thresh** entries are arrays of the same length which specify object filtering criteria. By default, no object filtering criteria is defined.

The **filter_attr_name** entry is an array of strings specifying the MODE output header column names for the object attributes of interest, such as **AREA, LENGTH, WIDTH**, and **INTENSITY_50**. In addition, **ASPECT_RATIO** specifies the aspect ratio (width/length), **INTENSITY_101** specifies the mean intensity value, and **INTENSITY_102** specifies the sum of the intensity values.

The **filter_attr_thresh** entry is an array of thresholds for these object attributes. Any simple objects not meeting all of the filtering criteria are discarded.

Note that the **area_thresh** and **inten_perc_thresh** entries from earlier versions of MODE are replaced by these options and are now deprecated. 

The **merge_thresh** entry is used to define larger objects for use in merging the original objects. It defines the threshold value used in the double thresholding merging technique. Note that in order to use this merging technique, it must be requested for both the forecast and observation fields. These thresholds should be chosen to define larger objects that fully contain the originally defined objects. For example, for objects defined as >=5.0, a merge threshold of >=2.5 will define larger objects that fully contain the original objects. Any two original objects contained within the same larger object will be merged. By default, the merge thresholds are set to be greater than or equal to 1.25. Multiple merge thresholds may be specified as an array (e.g. **merge_thresh = [ >=1.0, >=2.0, >=3.0 ];**). The number of **merge_thresh** entries must match the number of **conv_thresh** entries.

The **merge_flag** entry controls what type of merging techniques will be applied to the objects defined in each field. 

• **NONE** indicates that no merging should be applied. 

• **THRESH** indicates that the double thresholding merging technique should be applied. 

• **ENGINE** indicates that objects in each field should be merged by comparing the objects to themselves using a fuzzy engine approach. 

• **BOTH** indicates that both techniques should be used. 

By default, the double thresholding merging technique is applied.

_____________________

.. code-block:: none

  mask_missing_flag = NONE;

The **mask_missing_flag** entry specifies how missing data in the raw model and observation fields will be treated. 

• **NONE** indicates no additional processing is to be done. 

• **FCST** indicates missing data in the observation field should be used to mask the forecast field. 

• **OBS** indicates missing data in the forecast field should be used to mask the observation field. 

• **BOTH** indicates masking should be performed in both directions (i.e., mask the forecast field with the observation field and vice-versa).

Prior to defining objects, it is recommended that the raw fields be made to look similar to each other by assigning a value of BOTH to this parameter. However, by default no masking is performed.


_____________________

.. code-block:: none

  match_flag = MERGE_BOTH;

The **match_flag** entry controls how matching will be performed when comparing objects from the forecast field to objects from the observation field. An interest value is computed for each possible pair of forecast/observation objects. The interest values are then thresholded to define which objects match. If two objects in one field happen to match the same object in the other field, then those two objects could be merged. The **match_flag** entry controls what type of merging is allowed in this context. 

• **NONE** indicates that no matching should be performed between the fields at all. 

• **MERGE_BOTH** indicates that additional merging is allowed in both fields. 

• **MERGE_FCST** indicates that additional merging is allowed only in the forecast field. 

• **NO_MERGE** indicates that no additional merging is allowed in either field, meaning that each object will match at most one object in the other field. 

By default, additional merging is allowed in both fields.

_____________________

.. code-block:: none

  max_centroid_dist = 800/grid_res;

Computing the attributes for all possible pairs of objects can take some time depending on the numbers of objects. The **max_centroid_dist** entry is used to specify how far apart objects should be in order to conclude that they have no chance of matching. No pairwise attributes are computed for pairs of objects whose centroids are farther away than this distance, defined in terms of grid units. Setting this entry to a reasonable value will improve the execution time of the MODE tool. By default, the maximum centroid distance is defined in terms of the previously defined **grid_res** entry.

_____________________

.. code-block:: none

  mask = {
     grid = "";
     grid_flag = NONE; // Apply to NONE, FCST, OBS, or BOTH
     poly = "";
     poly_flag = NONE; // Apply to NONE, FCST, OBS, or BOTH
  }

Defining a **grid** and **poly** masking region is described in :numref:`Data IO MET Configuration File Options`. Applying a masking region when running MODE sets all grid points falling outside of that region to missing data, effectively limiting the area of which objects should be defined.

The **grid_flag** and **poly_flag** entries specify how the grid and polyline masking should be applied:

• **NONE** indicates that the masking grid should not be applied. 

• **FCST** indicates that the masking grid should be applied to the forecast field. 

• **OBS** indicates that the masking grid should be applied to the observation field. 

• **BOTH** indicates that the masking grid should be applied to both fields. 

By default, no masking grid or polyline is applied.


_____________________

.. code-block:: none

  weight = {
     centroid_dist    = 2.0;
     boundary_dist    = 4.0;
     convex_hull_dist = 0.0;
     angle_diff       = 1.0;
     aspect_diff      = 0.0;
     area_ratio       = 1.0;
     int_area_ratio   = 2.0;
     curvature_ratio  = 0.0;
     complexity_ratio = 0.0;
     inten_perc_ratio = 0.0;
     inten_perc_value = 50;
  } 

The **weight** entries listed above control how much weight is assigned to each pairwise attribute when computing a total interest value for object pairs. The weights listed above correspond to the **centroid distance** between the objects, the **boundary distance** (or minimum distance), the **convex hull distance** (or minimum distance between the convex hulls of the objects), the **orientation angle** difference, the **aspect ratio** difference, the **object area ratio** (minimum area divided by maximum area), the **intersection divided by the minimum object area ratio**, the **curvature ratio**, the **complexity ratio**, and the **intensity ratio**. The weights need not sum to any particular value. When the total interest value is computed, the weighted sum is normalized by the sum of the weights listed above.

The **inten_perc_value** entry corresponds to the **inten_perc_ratio**. The **inten_perc_value** should be set between 0 and 102 to define which percentile of intensity should be compared for pairs of objects. 101 and 102 specify the intensity mean and sum, respectively. By default, the 50th percentile, or median value, is chosen.

_____________________

.. code-block:: none

  interest_function = {
     centroid_dist      = ( ... );
     boundary_dist      = ( ... );
     convex_hull_dist   = ( ... );
     angle_diff         = ( ... );
     aspect_diff        = ( ... );
     corner             = 0.8;
     ratio_if           = ( ( 0.0, 0.0 )
                          ( corner, 1.0 )
			  ( 1.0, 1.0 ) );
     area_ratio         = ratio_if;
     int_area_ratio     = ( ... );
     curvature_ratio    = ratio_if;
     complexity_ratio   = ratio_if;
     inten_perc_ratio   = ratio_if;
  }

The set of interest function entries listed above define which values are of interest for each pairwise attribute measured. The interest functions may be defined as a piecewise linear function or as an algebraic expression. A piecewise linear function is defined by specifying the corner points of its graph. An algebraic function may be defined in terms of several built-in mathematical functions. See :numref:`MODE_A-Scientific-and-statistical` for how interest values are used by the fuzzy logic engine. By default, many of these functions are defined in terms of the previously defined **grid_res** entry.


_____________________

.. code-block:: none

  total_interest_thresh = 0.7;

The **total_interest_thresh** entry should be set between **0** and **1**. This threshold is applied to the total interest values computed for each pair of objects. Object pairs that have an interest value that is above this threshold will be matched, while those with an interest value that is below this threshold will remain unmatched. Increasing the threshold will decrease the number of matches while decreasing the threshold will increase the number of matches. By default, the total interest threshold is set to 0.7.


_____________________

.. code-block:: none

  print_interest_thresh = 0.0;

The **print_interest_thresh** entry determines which pairs of object attributes will be written to the output object attribute ASCII file. The user may choose to set the **print_interest_thresh** to the same value as the **total_interest_thresh**, meaning that only object pairs that actually match are written to the output file. By default, the print interest threshold is set to zero, meaning that all object pair attributes will be written as long as the distance between the object centroids is less than the **max_centroid_dist** entry.

_____________________

.. code-block:: none

  fcst_raw_plot = {
     color_table = "MET_BASE/colortables/met_default.ctable";
     plot_min = 0.0;
     plot_max = 0.0;
     colorbar_spacing = 1;
  }
  obs_raw_plot = {
     color_table = "MET_BASE/colortables/met_default.ctable";
     plot_min = 0.0;
     plot_max = 0.0;
     colorbar_spacing = 1;
  }
  object_plot = {
     color_table = "MET_BASE/colortables/mode_obj.ctable";
  }

Specifying dictionaries to define the **color_table, plot_min**, and **plot_max** entries are described in :numref:`Data IO MET Configuration File Options`.

The MODE tool generates a color bar to represent the contents of the colortable that was used to plot a field of data. The number of entries in the color bar matches the number of entries in the color table. The values defined for each color in the color table are also plotted next to the color bar. The **colorbar_spacing** entry is used to define the frequency with which the color table values should be plotted. Setting this entry to 1, as shown above, indicates that every color table value should be plotted. Setting it to an integer, n > 1, indicates that only every n-th color table value should be plotted.


_____________________

.. code-block:: none

  plot_valid_flag = FALSE;

When applied, the **plot_valid_flag entry** indicates that only the region containing valid data after masking is applied should be plotted. 

• **FALSE** indicates the entire domain should be plotted.

• **TRUE** indicates only the region containing valid data after masking should be plotted.

The default value of this flag is FALSE.


_____________________

.. code-block:: none

  plot_gcarc_flag = FALSE;

When applied, the **plot_gcarc_flag** entry indicates that the edges of polylines should be plotted using great circle arcs as opposed to straight lines in the grid. The default value of this flag is FALSE.


_____________________

.. code-block:: none

  ps_plot_flag  = TRUE;
  ct_stats_flag = TRUE;

These flags can be set to TRUE or FALSE to produce additional output, in the form of PostScript plots and contingency table counts and statistics, respectively.


_____________________

.. code-block:: none

  nc_pairs_flag = {
     latlon     = TRUE;
     raw        = TRUE;
     object_raw = TRUE;
     object_id  = TRUE;
     cluster_id = TRUE;
     polylines  = TRUE;
  }

Each component of the pairs information in the NetCDF file can be turned on or off. The old syntax is still supported: **TRUE** means accept the defaults, **FALSE** means no NetCDF output is generated. NetCDF output can also be turned off by setting all the individual dictionary flags to false.


_____________________

.. code-block:: none

  shift_right = 0;

When MODE is run on global grids, this parameter specifies how many grid squares to shift the grid to the right. MODE does not currently connect objects from one side of a global grid to the other, potentially causing objects straddling the "cut" longitude to be separated into two objects. Shifting the grid by integer number of grid units enables the user to control where that longitude cut line occurs.

.. _MODE-output:

mode output
~~~~~~~~~~~

MODE produces output in ASCII, NetCDF, and PostScript formats.

**ASCII output**

The MODE tool creates two ASCII output files. The first ASCII file contains contingency table counts and statistics for comparing the forecast and observation fields. This file consists of 4 lines. The first is a header line containing column names. The second line contains data comparing the two raw fields after any masking of bad data or based on a grid or lat/lon polygon has been applied. The third contains data comparing the two fields after any raw thresholds have been applied. The fourth, and last, line contains data comparing the derived object fields scored using traditional measures.

.. _CTS_output:

.. list-table:: Format of MODE CTS output file.
  :widths: auto
  :header-rows: 2

  * - mode ASCII
    - CONTINGENCY TABLE
    - OUTPUT FORMAT
  * - Column Number
    - MODE CTS Column Name
    - Description
  * - 1
    - VERSION
    - Version number
  * - 2
    - MODEL
    - User provided text string designating model name
  * - 3
    - N_VALID
    - Number of valid data points
  * - 4
    - GRID_RES
    - User provided nominal grid resolution
  * - 5
    - DESC
    - User provided text string describing the verification task
  * - 6
    - FCST_LEAD
    - Forecast lead time in HHMMSS format
  * - 7
    - FCST_VALID
    - Forecast valid start time in YYYYMMDD_HHMMSS format
  * - 8
    - FCST_ACCUM
    - Forecast accumulation time in HHMMSS format
  * - 9
    - OBS_LEAD
    - Observation lead time in HHMMSS format; when field2 is actually an observation, this should be "000000"
  * - 10
    - OBS_VALID
    - Observation valid start time in YYYYMMDD_HHMMSS format
  * - 11
    - OBS_ACCUM
    - Observation accumulation time in HHMMSS format
  * - 12
    - FCST_RAD
    - Forecast convolution radius in grid squares
  * - 13
    - FCST_THR
    - Forecast convolution threshold
  * - 14
    - OBS_RAD
    - Observation convolution radius in grid squares
  * - 15
    - OBS_THR
    - Observation convolution threshold
  * - 16
    - FCST_VAR
    - Forecast variable
  * - 17
    - FCST_UNITS
    - Units for model variable
  * - 18
    - FCST_LEV
    - Forecast vertical level
  * - 19
    - OBS_VAR
    - Observation variable
  * - 20
    - OBS_UNITS
    - Units for observation variable
  * - 21
    - OBS_LEV
    - Observation vertical level
  * - 22
    - OBTYPE
    - User provided observation type
  * - 23
    - FIELD
    - Field type for this line:* RAW for the raw input fields * OBJECT for the resolved object fields
  * - 24
    - TOTAL
    - Total number of matched pairs
  * - 25
    - FY_OY
    - Number of forecast yes and observation yes
  * - 26
    - FY_ON
    - Number of forecast yes and observation no
  * - 27
    - FN_OY
    - Number of forecast no and observation yes
  * - 28
    - FN_ON
    - Number of forecast no and observation no
  * - 29
    - BASER
    - Base rate
  * - 30
    - FMEAN
    - Forecast mean
  * - 31
    - ACC
    - Accuracy
  * - 32
    - FBIAS
    - Frequency Bias
  * - 33
    - PODY
    - Probability of detecting yes
  * - 34
    - PODN
    - Probability of detecting no
  * - 35
    - POFD
    - Probability of false detection
  * - 36
    - FAR
    - False alarm ratio
  * - 37
    - CSI
    - Critical Success Index
  * - 38
    - GSS
    - Gilbert Skill Score
  * - 39
    - HK
    - Hanssen-Kuipers Discriminant
  * - 40
    - HSS
    - Heidke Skill Score
  * - 41
    - ODDS
    - Odds Ratio

This first file uses the following naming convention:

*mode\_PREFIX\_FCST\_VAR\_LVL\_vs\_OBS\_VAR\_LVL\_HHMMSSL\_YYYYMMDD\_HHMMSSV\_HHMMSSA\_cts.txt*

where *PREFIX* indicates the user-defined output prefix, *FCST\_VAR\_LVL* is the forecast variable and vertical level being used, *OBS\_VAR\_LVL* is the observation variable and vertical level being used, *HHMMSSL* indicates the forecast lead time, *YYYYMMDD\_HHMMSSV* indicates the forecast valid time, and *HHMMSSA* indicates the accumulation period. The {\tt cts} string stands for contingency table statistics. The generation of this file can be disabled using the *ct\_stats\_flag* option in the configuration file. This CTS output file differs somewhat from the CTS output of the Point-Stat and Grid-Stat tools. The columns of this output file are summarized in :numref:`CTS_output`.

The second ASCII file the MODE tool generates contains all of the attributes for simple objects, the merged cluster objects, and pairs of objects. Each line in this file contains the same number of columns, though those columns not applicable to a given line contain fill data. The first row of every MODE object attribute file is a header containing the column names. The number of lines in this file depends on the number of objects defined. This file contains lines of 6 types that are indicated by the contents of the **OBJECT_ID** column. The **OBJECT_ID** can take the following 6 forms: **FNN, ONN, FNNN_ONNN, CFNNN, CONNN, CFNNN_CONNN**. In each case, **NNN** is a three-digit number indicating the object index. While all lines have the first 18 header columns in common, these 6 forms for **OBJECT_ID** can be divided into two types - one for single objects and one for pairs of objects. The single object lines **(FNN, ONN, CFNNN**, and **CONNN)** contain valid data in columns 19–39 and fill data in columns 40–51. The object pair lines **(FNNN_ONNN** and **CFNNN_CONNN)** contain valid data in columns 40–51 and fill data in columns 19–39.

These object identifiers are described in :numref:`MODE_object_attribute`. 


.. role:: raw-html(raw)
   :format: html

.. _MODE_object_attribute:
	    
.. list-table:: Object identifier descriptions for MODE object attribute output file.
  :widths: auto
  :header-rows: 2

  * - 
    - mode ASCII OBJECT
    - IDENTIFIER DESCRIPTIONS
  * - Object identifier (object_id)
    - Valid Data Columns
    - Description of valid data
  * - FNNN, ONNN
    - 1-18,19-39
    - Attributes for simple forecast, observation objects
  * - FNNN\_ :raw-html:`<br />`   ONNN
    - 1-18, 40-51
    - Attributes for pairs of simple forecast and observation objects
  * - CFNNN, CONNN
    - 1-18,19-39
    - Attributes for merged cluster objects in forecast, observation fields
  * - CFNNN\_ :raw-html:`<br />` CONNN
    - 1-18, 40-51
    - Attributes for pairs of forecast and observation cluster objects

**A note on terminology:** a cluster (referred to as "composite" in earlier versions) object need not necessarily consist of more than one simple object. A cluster object is by definition any set of one or more objects in one field which match a set of one or more objects in the other field. When a single simple forecast object matches a single simple observation object, they are each considered to be cluster objects as well.

The contents of the columns in this ASCII file are summarized in :numref:`MODE_object_attribute_output`.

.. _MODE_object_attribute_output:

.. list-table:: Format of MODE object attribute output files.
  :widths: auto
  :header-rows: 2

  * - mode ASCII OBJECT
    - ATTRIBUTE OUTPUT FORMAT
    - 
  * - Column
    - MODE Column Name
    - Description
  * - 1
    - VERSION
    - Version number
  * - 2
    - MODEL
    - User provided text string designating model name
  * - 3
    - N_VALID
    - Number of valid data points
  * - 4
    - GRID_RES
    - User provided nominal grid resolution
  * - 5
    - DESC
    - User provided text string describing the verification task
  * - 6
    - FCST_LEAD
    - Forecast lead time in HHMMSS format
  * - 7
    - FCST_VALID
    - Forecast valid start time in YYYYMMDD_HHMMSS format
  * - 8
    - FCST_ACCUM
    - Forecast accumulation time in HHMMSS format
  * - 9
    - OBS_LEAD
    - Observation lead time in HHMMSS format; when field2 is actually an observation, this should be "000000"
  * - 10
    - OBS_VALID
    - Observation valid start time in YYYYMMDD_HHMMSS format
  * - 11
    - OBS_ACCUM
    - Observation accumulation time in HHMMSS format
  * - 12
    - FCST_RAD
    - Forecast convolution radius in grid squares
  * - 13
    - FCST_THR
    - Forecast convolution threshold
  * - 14
    - OBS_RAD
    - Observation convolution radius in grid squares
  * - 15
    - OBS_THR
    - Observation convolution threshold
  * - 16
    - FCST_VAR
    - Forecast variable
  * - 17
    - FCST_UNITS
    - Units for forecast variable
  * - 18
    - FCST_LEV
    - Forecast vertical level
  * - 19
    - OBS_VAR
    - Observation variable
  * - 20
    - OBS_UNITS
    - Units for observation variable
  * - 21
    - OBS_LEV
    - Observation vertical level
  * - 22
    - OBTYPE
    - User provided observation type
  * - 23
    - OBJECT_ID
    - Object numbered from 1 to the number of objects in each field
  * - 24
    - OBJECT_CAT
    - Object category indicating to which cluster object it belongs
  * - 25-26
    - CENTROID_X, _Y
    - Location of the centroid (in grid units)
  * - 27-28
    - CENTROID_LAT, _LON
    - Location of the centroid (in lat/lon degrees)
  * - 29
    - AXIS_ANG
    - Object axis angle (in degrees)
  * - 30
    - LENGTH
    - Length of the enclosing rectangle (in grid units)
  * - 31
    - WIDTH
    - Width of the enclosing rectangle (in grid units)
  * - 32
    - AREA
    - Object area (in grid squares)
  * - 33
    - AREA_THRESH
    - Area of the object containing data values in the raw field that meet the object definition threshold criteria (in grid squares)
  * - 34
    - CURVATURE
    - Radius of curvature of the object defined in terms of third order moments (in grid units)
  * - 35-36
    - CURVATURE_X, _Y
    - Center of curvature (in grid coordinates)
  * - 37
    - COMPLEXITY
    - Ratio of the difference between the area of an object and the area of its convex hull divided by the area of the complex hull (unitless)
  * - 38-42
    - INTENSITY_10, _25, _50, _75, _90
    - 10th, 25th, 50th, 75th, and 90th percentiles of intensity of the raw field within the object (various units)
  * - 43
    - INTENSITY_NN
    - The percentile of intensity chosen for use in the PERCENTILE_INTENSITY_RATIO column (variable units)
  * - 44
    - INTENSITY_SUM
    - Sum of the intensities of the raw field within the object (variable units)
  * - 45
    - CENTROID_DIST
    - Distance between two objects centroids (in grid units)
  * - 46
    - BOUNDARY_DIST
    - Minimum distance between the boundaries of two objects (in grid units)
  * - 47
    - CONVEX_HULL :raw-html:`<br />` \_DIST
    - Minimum distance between the convex hulls of two objects (in grid units)
  * - 48
    - ANGLE_DIFF
    - Difference between the axis angles of two objects (in degrees)
  * - 49
    - ASPECT_DIFF
    - Absolute value of the difference between the aspect ratios of two objects (unitless)
  * - 50
    - AREA_RATIO
    - Ratio of the areas of two objects defined as the lesser of the two divided by the greater of the two (unitless)
  * - 51
    - INTERSECTION :raw-html:`<br />` \_AREA
    - Intersection area of two objects (in grid squares)
  * - 52
    - UNION_AREA
    - Union area of two objects (in grid squares)
  * - 53
    - SYMMETRIC_DIFF
    - Symmetric difference of two objects (in grid squares)
  * - 54
    - INTERSECTION :raw-html:`<br />`  \_OVER_AREA
    - Ratio of intersection area to the lesser of the forecast and observation object areas (unitless)
  * - 55
    - CURVATURE :raw-html:`<br />` \_RATIO
    - Ratio of the curvature of two objects defined as the lesser of the two divided by the greater of the two (unitless)
  * - 56
    - COMPLEXITY :raw-html:`<br />` \_RATIO
    - Ratio of complexities of two objects defined as the lesser of the forecast complexity divided by the observation complexity or its reciprocal (unitless)
  * - 57
    - PERCENTILE :raw-html:`<br />` \_INTENSITY :raw-html:`<br />` \_RATIO
    - Ratio of the nth percentile (INTENSITY_NN column) of intensity of the two objects defined as the lesser of the forecast intensity divided by the observation intensity or its reciprocal (unitless)
  * - 58
    - INTEREST
    - Total interest value computed for a pair of simple objects (unitless)

**NetCDF Output**

The MODE tool creates a NetCDF output file containing the object fields that are defined. The NetCDF file contains gridded fields including indices for the simple forecast objects, indices for the simple observation objects, indices for the matched cluster forecast objects, and indices for the matched cluster observation objects. The NetCDF file also contains lat/lon and x/y data for the vertices of the polygons for the boundaries of the simple forecast and observation objects. The generation of this file can be disabled using the **nc_pairs_flag** configuration file option.

The dimensions and variables included in the mode NetCDF files are described in :numref:`NetCDF_dimensions_for_MODE_output` and :numref:`Variables_contained_in_MODE_NetCDF_output`.

.. _NetCDF_dimensions_for_MODE_output:

.. list-table:: NetCDF dimensions for MODE output.
  :widths: auto
  :header-rows: 2

  * - mode NETCDF DIMENSIONS
    - 
  * - NetCDF Dimension
    - Description
  * - lat
    - Dimension of the latitude (i.e. Number of grid points in the North-South direction)
  * - lon
    - Dimension of the longitude (i.e. Number of grid points in the East-West direction)
  * - fcst_thresh_length
    - Number of thresholds applied to the forecast
  * - obs_thresh_length
    - Number of thresholds applied to the observations
  * - fcst_simp
    - Number of simple forecast objects
  * - fcst_simp_bdy
    - Number of points used to define the boundaries of all of the simple forecast objects
  * - fcst_simp_hull
    - Number of points used to define the hull of all of the simple forecast objects
  * - obs_simp
    - Number of simple observation objects
  * - obs_simp_bdy
    - Number of points used to define the boundaries of all of the simple observation objects
  * - obs_simp_hull
    - Number of points used to define the hull of all of the simple observation objects
  * - fcst_clus
    - Number of forecast clusters
  * - fcst_clus_hull
    - Number of points used to define the hull of all of the cluster forecast objects
  * - obs_clus
    - Number of observed clusters
  * - obs_clus_hull
    - Number of points used to define the hull of all of the cluster observation objects


.. _Variables_contained_in_MODE_NetCDF_output:

.. role:: raw-html(raw)
   :format: html

.. list-table:: Variables contained in MODE NetCDF output.
  :widths: auto
  :header-rows: 2

  * - 
    - mode NETCDF VARIABLES
    - 
  * - NetCDF Variable
    - Dimension
    - Description
  * - lat
    - lat, lon
    - Latitude
  * - lon
    - lat, lon
    - Longitude
  * - fcst_raw
    - lat, lon
    - Forecast raw values
  * - fcst_obj_raw
    - lat, lon
    - Forecast Object Raw Values
  * - fcst_obj_id
    - lat, lon
    - Simple forecast object id number for each grid point
  * - fcst_clus_id
    - lat, lon
    - Cluster forecast object id number for each grid point
  * - obs_raw
    - lat, lon
    - Observation Raw Values
  * - obs_obj_raw
    - lat, lon
    - Observation Object Raw Values
  * - obs_obj_id
    - \-
    - Simple observation object id number for each grid point
  * - obs_clus_id
    - \-
    - Cluster observation object id number for each grid point
  * - fcst_conv_radius
    - \-
    - Forecast convolution radius
  * - obs_conv_radius
    - \-
    - Observation convolution radius
  * - fcst_conv :raw-html:`<br />` \_threshold
    - \-
    - Forecast convolution threshold
  * - obs_conv :raw-html:`<br />` \_threshold
    - \-
    - Observation convolution threshold
  * - n_fcst_simp
    - \-
    - Number of simple forecast objects
  * - n_obs_simp
    - \-
    - Number of simple observation objects
  * - n_clus
    - \-
    - Number of cluster objects
  * - fcst_simp_bdy :raw-html:`<br />` \_start
    - fcst_simp
    - Forecast Simple Boundary Starting Index
  * - fcst_simp_bdy :raw-html:`<br />` \_npts
    - fcst_simp
    - Number of Forecast Simple Boundary Points
  * - fcst_simp_bdy :raw-html:`<br />` \_lat
    - fcst_simp_bdy
    - Forecast Simple Boundary PoLatitude
  * - fcst_simp_bdy :raw-html:`<br />` \_lon
    - fcst_simp_bdy
    - Forecast Simple Boundary PoLongitude
  * - fcst_simp_bdy_x
    - fcst_simp_bdy
    - Forecast Simple Boundary PoX-Coordinate
  * - fcst_simp_bdy_y
    - fcst_simp_bdy
    - Forecast Simple Boundary PoY-Coordinate
  * - fcst_simp_hull :raw-html:`<br />` \_start
    - fcst_simp
    - Forecast Simple Convex Hull Starting Index
  * - fcst_simp_hull :raw-html:`<br />` \_npts
    - fcst_simp
    - Number of Forecast Simple Convex Hull Points
  * - fcst_simp_hull :raw-html:`<br />` \_lat
    - fcst_simp_hull
    - Forecast Simple Convex Hull Point Latitude
  * - fcst_simp_hull :raw-html:`<br />` \_lon
    - fcst_simp_hull
    - Forecast Simple Convex Hull Point Longitude
  * - fcst_simp_hull_x
    - fcst_simp_hull
    - Forecast Simple Convex Hull Point X-Coordinate
  * - fcst_simp_hull_y
    - fcst_simp_hull
    - Forecast Simple Convex Hull Point Y-Coordinate
  * - obs_simp_bdy :raw-html:`<br />` \_start
    - obs_simp
    - Observation Simple Boundary Starting Index
  * - obs_simp_bdy    \_npts
    - obs_simp
    - Number of Observation Simple Boundary Points
  * - obs_simp_bdy :raw-html:`<br />` \_lat
    - obs_simp_bdy
    - Observation Simple Boundary Point Latitude
  * - obs_simp_bdy :raw-html:`<br />` \_lon
    - obs_simp_bdy
    - Observation Simple Boundary Point Longitude
  * - obs_simp_bdy_x
    - obs_simp_bdy
    - Observation Simple Boundary Point X-Coordinate
  * - obs_simp_bdy_y
    - obs_simp_bdy
    - Observation Simple Boundary Point Y-Coordinate
  * - obs_simp_hull :raw-html:`<br />` \_start
    - obs_simp
    - Observation Simple Convex Hull Starting Index
  * - obs_simp_hull :raw-html:`<br />` \_npts
    - obs_simp
    - Number of Observation Simple Convex Hull Points
  * - obs_simp_hull :raw-html:`<br />` \_lat
    - obs_simp_hull
    - Observation Simple Convex Hull Point Latitude
  * - obs_simp_hull :raw-html:`<br />` \_lon
    - obs_simp_hull
    - Observation Simple Convex Hull Point Longitude
  * - obs_simp_hull_x
    - obs_simp_hull
    - Observation Simple Convex Hull Point X-Coordinate
  * - obs_simp_hull_y
    - obs_simp_hull
    - Observation Simple Convex Hull Point Y-Coordinate
  * - fcst_clus_hull :raw-html:`<br />` \_start
    - fcst_clus
    - Forecast Cluster Convex Hull Starting Index
  * - fcst_clus_hull :raw-html:`<br />` \_npts
    - fcst_clus
    - Number of Forecast Cluster Convex Hull Points
  * - fcst_clus_hull :raw-html:`<br />` \_lat
    - fcst_clus_hull
    - Forecast Cluster Convex Hull Point Latitude
  * - fcst_clus_hull :raw-html:`<br />` \_lon
    - fcst_clus_hull
    - Forecast Cluster Convex Hull Point Longitude
  * - fcst_clus_hull_x
    - fcst_clus_hull
    - Forecast Cluster Convex Hull Point X-Coordinate
  * - fcst_clus_hull_y
    - fcst_clus_hull
    - Forecast Cluster Convex Hull Point Y-Coordinate
  * - obs_clus_hull :raw-html:`<br />` \_start
    - obs_clus
    - Observation Cluster Convex Hull Starting Index
  * - obs_clus_hull :raw-html:`<br />` \_npts
    - obs_clus
    - Number of Observation Cluster Convex Hull Points
  * - obs_clus_hull :raw-html:`<br />` \_lat
    - obs_clus_hull
    - Observation Cluster Convex Hull Point Latitude
  * - obs_clus_hull :raw-html:`<br />` \_lon
    - obs_clus_hull
    - Observation Cluster Convex Hull Point Longitude
  * - obs_clus_hull_x
    - obs_clus_hull
    - Observation Cluster Convex Hull Point X-Coordinate
  * - obs_clus_hull_y
    - obs_clus_hull
    - Observation Cluster Convex Hull Point Y-Coordinate
      
**Postscript File**

Lastly, the MODE tool creates a PostScript plot summarizing the features-based approach used in the verification. The PostScript plot is generated using internal libraries and does not depend on an external plotting package. The generation of this PostScript output can be disabled using the **ps_plot_flag** configuration file option.

The PostScript plot will contain 5 summary pages at a minimum, but the number of pages will depend on the merging options chosen. Additional pages will be created if merging is performed using the double thresholding or fuzzy engine merging techniques for the forecast and/or observation fields. Examples of the PostScript plots can be obtained by running the example cases provided with the MET tarball.

The first page of PostScript output contains a great deal of summary information. Six tiles of images provide thumbnail images of the raw fields, matched/merged object fields, and object index fields for the forecast and observation grids. In the matched/merged object fields, matching colors of objects across fields indicate that the corresponding objects match, while within a single field, black outlines indicate merging. Note that objects that are colored royal blue are unmatched. Along the bottom of the page, the criteria used for object definition and matching/merging are listed. Along the right side of the page, total interest values for pairs of simple objects are listed in sorted order. The numbers in this list correspond to the object indices shown in the object index plots.

The second and third pages of the PostScript output file display enlargements of the forecast and observation raw and object fields, respectively.  The fourth page displays the forecast object with the outlines of the observation objects overlaid, and vice versa. The fifth page contains summary information about the pairs of matched cluster objects.

If the double threshold merging or the fuzzy engine merging techniques have been applied, the output from those steps is summarized on additional pages.
