.. _data_io:

************
MET Data I/O
************

Data must often be preprocessed prior to using it for verification. Several MET tools exist for this purpose. In addition to preprocessing observations, some plotting utilities for data checking are also provided and described at the end of this section. Both the input and output file formats are described in this section. :numref:`Input data formats` and :numref:`Intermediate data formats` are primarily concerned with re-formatting input files into the intermediate files required by some MET modules. These steps are represented by the first three columns in the MET flowchart depicted in :numref:`overview-figure`. Output data formats are described in :numref:`Output data formats`. Common configuration files options are described in :numref:`Configuration File Details`. Description of software modules used to reformat the data may now be found in :numref:`reformat_point`  and :numref:`reformat_grid`.

.. _Input data formats:

Input data formats
==================

The MET package can handle multiple gridded input data formats: GRIB version 1, GRIB version 2, and NetCDF files following the Climate and Forecast (CF) conventions, containing WRF output post-processed using wrf_interp, or produced by the MET tools themselves. MET supports standard NCEP, USAF, UKMet Office and ECMWF GRIB tables along with custom, user-defined GRIB tables and the extended PDS including ensemble member metadata. See :numref:`Configuration File Details` for more information. Point observation files may be supplied in either PrepBUFR, ASCII, or MADIS format. Note that MET does not require the Unified Post-Processor to be used, but does require that the input GRIB data be on a standard, de-staggered grid on pressure or regular levels in the vertical. While the Grid-Stat, Wavelet-Stat, MODE, and MTD tools can be run on a gridded field at virtually any level, the Point-Stat tool can only be used to verify forecasts at the surface or on pressure or height levels. MET does not interpolate between native model vertical levels.

When comparing two gridded fields with the Grid-Stat, Wavelet-Stat, Ensemble-Stat, MODE, MTD, or Series-Analysis tools, the input model and observation datasets must be on the same grid. MET will regrid files according to user specified options. Alternatively, outside of MET, the copygb and wgrib2 utilities are recommended for re-gridding GRIB1 and GRIB2 files, respectively. To preserve characteristics of the observations, it is generally preferred to re-grid the model data to the observation grid, rather than vice versa.

Input point observation files in PrepBUFR format are available through NCEP. The PrepBUFR observation files contain a wide variety of point-based observation types in a single file in a standard format. However, some users may wish to use observations not included in the standard PrepBUFR files. For this reason, prior to performing the verification step in the Point-Stat tool, the PrepBUFR file is reformatted with the PB2NC tool. In this step, the user can select various ways of stratifying the observation data spatially, temporally, and by type. The remaining observations are reformatted into an intermediate NetCDF file. The ASCII2NC tool may be used to convert ASCII point observations that are not available in the PrepBUFR files into this common NetCDF point observation format. Several other MET tools, described below, are also provided to reformat point observations into this common NetCDF point observation format prior to passing them as input to the Point-Stat or Ensemble-Stat verification tools.

Tropical cyclone forecasts and observations are typically provided in a specific ATCF (Automated Tropical Cyclone Forecasting) ASCII format, in A-deck, B-deck, and E-deck files.

Requirements for CF Compliant NetCDF
------------------------------------

The MET tools use following attributes and variables for input CF Compliant NetCDF data.

1. The global attribute "Conventions".

2. The "`standard_name <https://cfconventions.org/Data/cf-conventions/cf-conventions-1.9/cf-conventions.html#standard-name>`_" and "`units <https://cfconventions.org/Data/cf-conventions/cf-conventions-1.9/cf-conventions.html#units>`_" attributes for coordinate variables. The "`axis <https://cfconventions.org/Data/cf-conventions/cf-conventions-1.9/cf-conventions.html#time-axis-ex>`_" attribute ("T" or "time") must exist as the time variable if the "standard_name" attribute does not exist.

3. The "`coordinates <https://cfconventions.org/Data/cf-conventions/cf-conventions-1.9/cf-conventions.html#coordinate-types>`_" attribute for the data variables. It contains the coordinate variable names.

4. The "`grid_mapping <https://cfconventions.org/Data/cf-conventions/cf-conventions-1.9/cf-conventions.html#appendix-grid-mappings>`_" attribute for the data variables for projections and the matching grid mapping variable (optional for the latitude_longitude projection).

5. The gridded data should be evenly spaced horizontally and vertically.

6. (Optional) the "`forecast_reference_time <https://cfconventions.org/Data/cf-conventions/cf-conventions-1.9/cf-conventions.html#scalar-coordinate-variables>`_" variable for init_time.

MET processes the CF-Compliant gridded NetCDF files with the projection information. The CF-Compliant NetCDF is defined by the global attribute "Conventions" whose value begins with "CF-" ("CF-<Version_number>"). The global attribute "Conventions" is mandatory. MET accepts the variation of this attribute ("conventions" and "CONVENTIONS"). The value should be started with "CF-" and followed by the version number. MET accepts the attribute value that begins with "CF " ("CF" and a space instead of a hyphen) or "COARDS".

The grid mapping variable contains the projection information. The grid mapping variable can be found by looking at the variable attribute "grid_mapping" from the data variables. The "standard_name" attribute is used to filter out the coordinate variables like time, latitude, and longitude variables. The value of the "grid_mapping" attribute is the name of the grid mapping variable. Four projections are supported with grid mapping variables: latitude_longitude, lambert_conformal_conic, polar_stereographic, and geostationary. In case of the latitude_longitude projection, the latitude and longitude variable names should be the same as the dimension names and the "units" attribute should be valid.

Here are examples for the grid mapping variable ("edr" is the data variable):

**Example 1: grid mapping for latitude_longitude projection**

.. code-block:: none

    float edr(time, z, lat, lon) ;
            edr:units = "m^(2/3) s^-1" ;
            edr:long_name = "Median eddy dissipation rate" ;
            edr:coordinates = "lat lon" ;
            edr:_FillValue = -9999.f ;
            edr:grid_mapping = "grid_mapping" ;
    int grid_mapping ;
            grid_mapping:grid_mapping_name = "latitude_longitude" ;
            grid_mapping:semi_major_axis = 6371000. ;
            grid_mapping:inverse_flattening = 0 ;


**Example 2: grid mapping for lambert_conformal_conic projection**

.. code-block:: none

    float edr(time, z, y, x) ;
            edr:units = "m^(2/3) s^-1" ;
            edr:long_name = "Eddy dissipation rate" ;
            edr:coordinates = "lat lon" ;
            edr:_FillValue = -9999.f ;
            edr:grid_mapping = "grid_mapping" ;
    int grid_mapping ;
            grid_mapping:grid_mapping_name = "lambert_conformal_conic" ;
            grid_mapping:standard_parallel = 25. ;
            grid_mapping:longitude_of_central_meridian = -95. ;
            grid_mapping:latitude_of_projection_origin = 25. ;
            grid_mapping:false_easting = 0 ;
            grid_mapping:false_northing = 0 ;
            grid_mapping:GRIB_earth_shape = "spherical" ;
            grid_mapping:GRIB_earth_shape_code = 0 ;

When the grid mapping variable is not available, MET detects the latitude_longitude projection in following order:

1. the lat/lon projection from the dimensions

2. the lat/lon projection from the "coordinates" attribute from the data variable

3. the lat/lon projection from the latitude and longitude variables by the "standard_name" attribute

MET is looking for variables with the same name as the dimension and checking the "units" attribute to find the latitude and longitude variables. The valid "units" strings are listed in the table below. MET accepts the variable "tlat" and "tlon" if the dimension names are "nlat" and "nlon”.

If there are no latitude and longitude variables from dimensions, MET gets coordinate variable names from the "coordinates" attribute. The matching coordinate variables should have the proper "units" attribute.

MET gets the time, latitude, and longitude variables by looking at the standard name: "time", "latitude", and "longitude" as the last option.

MET gets the valid time from the time variable and the "forecast_reference_time" variable for the init_time. If the time variable does not exist, it can come from the file name. MET supports only two cases:

1. TRMM_3B42_3hourly_filename (3B42.<yyyymmdd>.<hh>.7.G3.nc)

2. TRMM_3B42_daily_filename (3B42_daily.<yyyy>.<mm>.<dd>.7.G3.nc)

.. list-table:: Valid strings for the "units" attribute.
  :widths: auto
  :header-rows: 1

  * - time
    - latitude
    - longitude
  * - "seconds since YYYY-MM-DD HH:MM:SS",
      "minutes since YYYY-MM-DD HH:MM:SS",
      "hours since YYYY-MM-DD HH:MM:SS",
      "days since YYYY-MM-DD HH:MM:SS",
      "months since YYYY-MM-DD HH:MM:SS",
      "years since YYYY-MM-DD HH:MM:SS",
      Accepts "Y", "YY", "YYY", "M", "D", "HH", and "HH:MM".
      "HH:MM:SS" is optional
    - "degrees_north",
      "degree_north",
      "degree_N",
      "degrees_N",
      "degreeN",
      "degreesN"
    - "degrees_east",
      "degree_east",
      "degree_E",
      "degrees_E",
      "degreeE",
      "degreesE"

Performance with NetCDF input data
----------------------------------

There is no limitation on the NetCDF file size. The size of the data variables matters more than the file size. The NetCDF API loads the metadata first upon opening the NetCDF file. It's similar for accessing data variables. There are two API calls: getting the metadata and getting the actual data. The memory is allocated and consumed at the second API call (getting the actual data).

The dimensions of the data variables matter. MET requests the NetCDF data needs based on: 1) loading and processing a data plane, and 2) loading and processing the next data plane. This means an extra step for slicing with one more dimension in the NetCDF input data. The performance is quite different if the compression is enabled with high resolution data. NetCDF does compression per variable. The variables can have different compression levels (0 to 9).  A value of 0 means no compression, and 9 is the highest level of compression possible. The number for decompression is the same between one more and one less dimension NetCDF input files (combined VS separated). The difference is the amount of data to be decompressed which requires more memory. For example, let's assume the time dimension is 30. NetCDF data with one less dimension (no time dimension) does decompression 30 times for nx by ny dataset. NetCDF with one more dimension does compression 30 times for 30 by nx by ny dataset and slicing for target time offset. So it's better to have multiple NetCDF files with one less dimension than a big file with bigger variable data if compressed. If the compression is not enabled, the file size will be much bigger requiring more disk space.

.. _Intermediate data formats:

Intermediate data formats
=========================

MET uses NetCDF as an intermediate file format. The MET tools which write gridded output files write to a common gridded NetCDF file format. The MET tools which write point output files write to a common point observation NetCDF file format.

.. _Output data formats:

Output data formats
===================

The MET package currently produces output in the following basic file formats: STAT files, ASCII files, NetCDF files, PostScript plots, and png plots from the Plot-Mode-Field utility.

The STAT format consists of tabular ASCII data that can be easily read by many analysis tools and software packages. MET produces STAT output for the Grid-Stat, Point-Stat, Ensemble-Stat, Wavelet-Stat, and TC-Gen tools. STAT is a specialized ASCII format containing one record on each line. However, a single STAT file will typically contain multiple line types. Several header columns at the beginning of each line remain the same for each line type. However, the remaining columns after the header change for each line type. STAT files can be difficult for a human to read as the quantities represented for many columns of data change from line to line.

For this reason, ASCII output is also available as an alternative for these tools. The ASCII files contain exactly the same output as the STAT files but each STAT line type is grouped into a single ASCII file with a column header row making the output more human-readable. The configuration files control which line types are output and whether or not the optional ASCII files are generated.

The MODE tool creates two ASCII output files as well (although they are not in a STAT format). It generates an ASCII file containing contingency table counts and statistics comparing the model and observation fields being compared. The MODE tool also generates a second ASCII file containing all of the attributes for the single objects and pairs of objects. Each line in this file contains the same number of columns, and those columns not applicable to a given line type contain fill data. Similarly, the MTD tool writes one ASCII output file for 2D objects attributes and four ASCII output files for 3D object attributes.

The TC-Pairs and TC-Stat utilities produce ASCII output, similar in style to the STAT files, but with TC relevant fields.

Many of the tools generate gridded NetCDF output. Generally, this output acts as input to other MET tools or plotting programs. The point observation preprocessing tools produce NetCDF output as input to the statistics tools. Full details of the contents of the NetCDF files is found in :numref:`Data format summary` below.

The MODE, Wavelet-Stat and plotting tools produce PostScript plots summarizing the spatial approach used in the verification. The PostScript plots are generated using internal libraries and do not depend on an external plotting package. The MODE plots contain several summary pages at the beginning, but the total number of pages will depend on the merging options chosen. Additional pages will be created if merging is performed using the double thresholding or fuzzy engine merging techniques for the forecast and observation fields. The number of pages in the Wavelet-Stat plots depend on the number of masking tiles used and the dimension of those tiles. The first summary page is followed by plots for the wavelet decomposition of the forecast and observation fields. The generation of these PostScript output files can be disabled using command line options.

Users can use the optional plotting utilities Plot-Data-Plane, Plot-Point-Obs, and Plot-Mode-Field to produce graphics showing forecast, observation, and MODE object files.

.. _Data format summary:

Data format summary
===================

The following is a summary of the input and output formats for each of the tools currently in MET. The output listed is the maximum number of possible output files. Generally, the type of output files generated can be controlled by the configuration files and/or the command line options:

#. **PB2NC Tool**

    * **Input**: PrepBUFR point observation file(s) and one configuration file.

    * **Output**: One NetCDF file containing the observations that have been retained.

#. **ASCII2NC Tool**

    * **Input**: ASCII point observation file(s) that has (have) been formatted as expected, and optional configuration file.

    * **Output**: One NetCDF file containing the reformatted observations.

#. **MADIS2NC Tool**

    * **Input**: MADIS point observation file(s) in NetCDF format.

    * **Output**: One NetCDF file containing the reformatted observations.


#. **LIDAR2NC Tool**

    * **Input**: One CALIPSO satellite HDF file.

    * **Output**: One NetCDF file containing the reformatted observations.

#. **IODA2NC Tool**

    * **Input**: IODA observation file(s) in NetCDF format.

    * **Output**: One NetCDF file containing the reformatted observations.

#. **Point2Grid Tool**

    * **Input**: One NetCDF file in the common point observation format.

    * **Output**: One NetCDF file containing a gridded representation of the point observations.

#. **Pcp-Combine Tool**

    * **Input**: Two or more gridded model or observation files (in GRIB format for "sum" command, or any gridded file for "add", "subtract", and "derive" commands) containing data (often accumulated precipitation) to be combined.

    * **Output**: One NetCDF file containing output for the requested operation(s).

#. **Regrid-Data-Plane Tool**

    * **Input**: One gridded model or observation field and one gridded field to provide grid specification if desired.

    * **Output**: One NetCDF file containing the regridded data field(s).

#. **Shift-Data-Plane Tool**

    * **Input**: One gridded model or observation field.

    * **Output**: One NetCDF file containing the shifted data field.

#. **MODIS-Regrid Tool**

    * **Input**: One gridded model or observation field and one gridded field to provide grid specification.

    * **Output**: One NetCDF file containing the regridded data field.

#. **Gen-VX-Mask Tool**

    * **Input**: One gridded model or observation file and one file defining the masking region (varies based on masking type).

    * **Output**: One NetCDF file containing a bitmap for the resulting masking region.

#. **Point-Stat Tool**

    * **Input**: One gridded model file, at least one NetCDF file in the common point observation format, and one configuration file.

    * **Output**: One STAT file containing all of the requested line types and several ASCII files for each line type requested.

#. **Grid-Stat Tool**

    * **Input**: One gridded model file, one gridded observation file, and one configuration file.

    * **Output**: One STAT file containing all of the requested line types, several ASCII files for each line type requested, and one NetCDF file containing the matched pair data and difference field for each verification region and variable type/level being verified.

#. **Ensemble Stat Tool**

    * **Input**: An arbitrary number of gridded model files, one or more gridded and/or point observation files, and one configuration file. Point and gridded observations are both accepted.

    * **Output**: One NetCDF file containing requested ensemble forecast information. If observations are provided, one STAT file containing all requested line types, several ASCII files for each line type requested, and one NetCDF file containing gridded observation ranks.

#. **Wavelet-Stat Tool**

    * **Input**: One gridded model file, one gridded observation file, and one configuration file.

    * **Output**: One STAT file containing the "ISC" line type, one ASCII file containing intensity-scale information and statistics, one NetCDF file containing information about the wavelet decomposition of forecast and observed fields and their differences, and one PostScript file containing plots and summaries of the intensity-scale verification.

#. **GSID2MPR Tool**

    * **Input**: One or more binary GSI diagnostic files (conventional or radiance) to be reformatted.

    * **Output**: One ASCII file in matched pair (MPR) format.

#. **GSID2ORANK Tool**

    * **Input**: One or more binary GSI diagnostic files (conventional or radiance) to be reformatted.

    * **Output**: One ASCII file in observation rank (ORANK) format.

#. **Stat-Analysis Tool**

    * **Input**: One or more STAT files output from the Point-Stat, Grid-Stat, Ensemble Stat, Wavelet-Stat, or TC-Gen tools and, optionally, one configuration file containing specifications for the analysis job(s) to be run on the STAT data.

    * **Output**: ASCII output of the analysis jobs is printed to the screen unless redirected to a file using the "-out" option or redirected to a STAT output file using the "-out_stat" option.

#. **Series-Analysis Tool**

    * **Input**: An arbitrary number of gridded model files and gridded observation files and one configuration file.

    * **Output**: One NetCDF file containing requested output statistics on the same grid as the input files.

#. **Grid-Diag Tool**

    * **Input**: An arbitrary number of gridded data files and one configuration file.

    * **Output**: One NetCDF file containing individual and joint histograms of the requested data.

#. **MODE Tool**

    * **Input**: One gridded model file, one gridded observation file, and one or two configuration files.

    * **Output**: One ASCII file containing contingency table counts and statistics, one ASCII file containing single and pair object attribute values, one NetCDF file containing object indices for the gridded simple and cluster object fields, and one PostScript plot containing a summary of the features-based verification performed.

#. **MODE-Analysis Tool**

    * **Input**: One or more MODE object statistics files from the MODE tool and, optionally, one configuration file containing specification for the analysis job(s) to be run on the object data.

    * **Output**: ASCII output of the analysis jobs will be printed to the screen unless redirected to a file using the "-out" option.

#. **MODE-TD Tool**

    * **Input**: Two or more gridded model files, two or more gridded observation files, and one configuration file.

    * **Output**: One ASCII file containing 2D object attributes, four ASCII files containing 3D object attributes, and one NetCDF file containing object indices for the gridded simple and cluster object fields.

#. **TC-Dland Tool**

    * **Input**: One or more files containing the longitude (Degrees East) and latitude (Degrees North) of all the coastlines and islands considered to be a significant landmass.

    * **Output**: One NetCDF format file containing a gridded field representing the distance to the nearest coastline or island, as specified in the input file.

#. **TC-Pairs Tool**

    * **Input**: At least one A-deck or E-deck file and one B-deck ATCF format file containing output from a tropical cyclone tracker and one configuration file. The A-deck files contain forecast tracks, the E-deck files contain forecast probabilities, and the B-deck files are typically the NHC Best Track Analysis but could also be any ATCF format reference.

    * **Output**: ASCII output with the suffix .tcst.

#. **TC-Stat Tool**

    * **Input**: One or more TCSTAT output files output from the TC-Pairs tool and, optionally, one configuration file containing specifications for the analysis job(s) to be run on the TCSTAT data.

    * **Output**: ASCII output of the analysis jobs will be printed to the screen unless redirected to a file using the "-out" option.

#. **TC-Gen Tool**

    * **Input**: One or more Tropical Cyclone genesis format files, one or more verifying operational and BEST track files in ATCF format, and one configuration file.

    * **Output**: One STAT file containing all of the requested line types, several ASCII files for each line type requested, and one gridded NetCDF file containing counts of track points.

#. **TC-RMW Tool**

    * **Input**: One or more gridded data files, one ATCF track file defining the storm location, and one configuration file.

    * **Output**: One gridded NetCDF file containing the requested model fields transformed into cylindrical coordinates.

#. **RMW-Analysis Tool**

    * **Input**: One or more NetCDF output files from the TC-RMW tool and one configuration file.

    * **Output**: One NetCDF file for results aggregated across the filtered set of input files.

#. **Plot-Point-Obs Tool**

    * **Input**: One NetCDF file containing point observation from the ASCII2NC, PB2NC, MADIS2NC, or LIDAR2NC tool.

    * **Output**: One postscript file containing a plot of the requested field.

#. **Plot-Data-Plane Tool**

    * **Input**: One gridded data file to be plotted.

    * **Output**: One postscript file containing a plot of the requested field.

#. **Plot-MODE-Field Tool**

    * **Input**: One or more MODE output files to be used for plotting and one configuration file.

    * **Output**: One PNG file with the requested MODE objects plotted. Options for objects include raw, simple or cluster and forecast or observed objects.

#. **GIS-Util Tools**

    * **Input**: ESRI shape files ending in .dbf, .shp, or .shx.

    * **Output**: ASCII description of their contents printed to the screen.

.. _Configuration File Details:
  
Configuration File Details
==========================

Part of the strength of MET is the leveraging of capability across tools. There are several configuration options that are common to many of the tools.

Many of the MET tools use a configuration file to set parameters. This prevents the command line from becoming too long and cumbersome and makes the output easier to duplicate.


The configuration file details are described in :ref:`config_options` and :ref:`config_options_tc`.
