.. _data_io:

MET Data I/O
============

Data must often be preprocessed prior to using it for verification. Several MET tools exist for this purpose. In addition to preprocessing observations, some plotting utilities for data checking are also provided and described at the end of this section. Both the input and output file formats are described in this section. :numref:`Input data formats` and :numref:`Intermediate data formats` are primarily concerned with re-formatting input files into the intermediate files required by some MET modules. These steps are represented by the first three columns in the MET flowchart depicted in :numref:`overview-figure`. Output data formats are described in :numref:`Output data formats`. Common configuration files options are described in :numref:`Configuration File Details`. Description of software modules used to reformat the data may now be found in :numref:`reformat_point`  and :numref:`reformat_grid`.

.. _Input data formats:

Input data formats
__________________

The MET package can handle gridded input data in one of four formats: GRIB version 1, GRIB version 2, NetCDF files following the Climate and Forecast (CF) conventions, and NetCDF files produced by the MET tools themselves. MET supports standard NCEP, USAF, UKMet Office and ECMWF grib tables along with custom, user-defined GRIB tables and the extended PDS including ensemble member metadata. See :numref:`Configuration File Details` for more information. Point observation files may be supplied in either PrepBUFR, ASCII, or MADIS format. Note that MET does not require the Unified Post-Processor to be used, but does require that the input GRIB data be on a standard, de-staggered grid on pressure or regular levels in the vertical. While the Grid-Stat, Wavelet-Stat, MODE, and MTD tools can be run on a gridded field at virtually any level, the Point-Stat tool can only be used to verify forecasts at the surface or on pressure or height levels. MET does not interpolate between native model vertical levels.

When comparing two gridded fields with the Grid-Stat, Wavelet-Stat, Ensemble-Stat, MODE, MTD, or Series-Analysis tools, the input model and observation datasets must be on the same grid. MET will regrid files according to user specified options. Alternately, outside of MET, the copygb and wgrib2 utilities are recommended for re-gridding GRIB1 and GRIB2 files, respectively. To preserve characteristics of the observations, it is generally preferred to re-grid the model data to the observation grid, rather than vice versa.

Input point observation files in PrepBUFR format are available through NCEP. The PrepBUFR observation files contain a wide variety of point-based observation types in a single file in a standard format. However, some users may wish to use observations not included in the standard PrepBUFR files. For this reason, prior to performing the verification step in the Point-Stat tool, the PrepBUFR file is reformatted with the PB2NC tool. In this step, the user can select various ways of stratifying the observation data spatially, temporally, and by type. The remaining observations are reformatted into an intermediate NetCDF file. The ASCII2NC tool may be used to convert ASCII point observations that are not available in the PrepBUFR files into this NetCDF format for use by the Point-Stat verification tool. Users with METAR or RAOB data from MADIS can convert these observations into NetCDF format with the MADIS2NC tool, then use them with the Point-Stat or Ensemble-Stat verification tools.

Tropical cyclone forecasts and observations are typically provided in a specific ASCII format, in A Deck and B Deck files.

.. _Intermediate data formats:

Intermediate data formats
_________________________

MET uses NetCDF as an intermediate file format. The MET tools which write gridded output files write to a common gridded NetCDF file format. The MET tools which write point output files write to a common point observation NetCDF file format.

.. _Output data formats:

Output data formats
___________________

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
___________________

The following is a summary of the input and output formats for each of the tools currently in MET. The output listed is the maximum number of possible output files. Generally, the type of output files generated can be controlled by the configuration files and/or the command line options:

#. **PB2NC Tool**

    * **Input**: One PrepBUFR point observation file and one configuration file.

    * **Output**: One NetCDF file containing the observations that have been retained.

#. **ASCII2NC Tool**

    * **Input**: One or more ASCII point observation file(s) that has (have) been formatted as expected, and optional configuration file.

    * **Output**: One NetCDF file containing the reformatted observations.

#. **MADIS2NC Tool**

    * **Input**: One MADIS point observation file.

    * **Output**: One NetCDF file containing the reformatted observations.


#. **LIDAR2NC Tool**

    * **Input**: One CALIPSO satellite HDF file

    * **Output**: One NetCDF file containing the reformatted observations.

#. **Point2Grid Tool**

    * **Input**: One NetCDF file containing point observation from the ASCII2NC, PB2NC, MADIS2NC, or LIDAR2NC tool.

    * **Output**: One NetCDF file containing a gridded representation of the point observations.

#. **Pcp-Combine Tool**

    * **Input**: Two or more gridded model or observation files (in GRIB format for “sum” command, or any gridded file for “add”, “subtract”, and “derive” commands) containing data (often accumulated precipitation) to be combined.

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

    * **Input**: One gridded model file, at least one point observation file in NetCDF format (as the output of the PB2NC, ASCII2NC, MADIS2NC, or LIDAR2NC tool), and one configuration file.

    * **Output**: One STAT file containing all of the requested line types and several ASCII files for each line type requested.

#. **Grid-Stat Tool**

    * **Input**: One gridded model file, one gridded observation file, and one configuration file.

    * **Output**: One STAT file containing all of the requested line types, several ASCII files for each line type requested, and one NetCDF file containing the matched pair data and difference field for each verification region and variable type/level being verified.

#. **Ensemble Stat Tool**

    * **Input**: An arbitrary number of gridded model files, one or more gridded and/or point observation files, and one configuration file. Point and gridded observations are both accepted.

    * **Output**: One NetCDF file containing requested ensemble forecast information. If observations are provided, one STAT file containing all requested line types, several ASCII files for each line type requested, and one NetCDF file containing gridded observation ranks.

#. **Wavelet-Stat Tool**

    * **Input**: One gridded model file, one gridded observation file, and one configuration file.

    * **Output**: One STAT file containing the “ISC” line type, one ASCII file containing intensity-scale information and statistics, one NetCDF file containing information about the wavelet decomposition of forecast and observed fields and their differences, and one PostScript file containing plots and summaries of the intensity-scale verification.

#. **GSID2MPR Tool**

    * **Input**: One or more binary GSI diagnostic files (conventional or radiance) to be reformatted.

    * **Output**: One ASCII file in matched pair (MPR) format.

#. **GSID2ORANK Tool**

    * **Input**: One or more binary GSI diagnostic files (conventional or radiance) to be reformatted.

    * **Output**: One ASCII file in observation rank (ORANK) format.

#. **Stat-Analysis Tool**

    * **Input**: One or more STAT files output from the Point-Stat, Grid-Stat, Ensemble Stat, Wavelet-Stat, or TC-Gen tools and, optionally, one configuration file containing specifications for the analysis job(s) to be run on the STAT data.

    * **Output**: ASCII output of the analysis jobs is printed to the screen unless redirected to a file using the “-out” option or redirected to a STAT output file using the “-out_stat” option.

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

    * **Output**: ASCII output of the analysis jobs will be printed to the screen unless redirected to a file using the “-out” option.

#. **MODE-TD Tool**

    * **Input**: Two or more gridded model files, two or more gridded observation files, and one configuration file.

    * **Output**: One ASCII file containing 2D object attributes, four ASCII files containing 3D object attributes, and one NetCDF file containing object indices for the gridded simple and cluster object fields.

#. **TC-Dland Tool**

    * **Input**: One or more files containing the longitude (Degrees East) and latitude (Degrees North) of all the coastlines and islands considered to be a significant landmass.

    * **Output**: One NetCDF format file containing a gridded field representing the distance to the nearest coastline or island, as specified in the input file.

#. **TC-Pairs Tool**

    * **Input**: At least one A-deck and one B-deck ATCF format file containing output from a tropical cyclone tracker and one configuration file. The A-deck files contain forecast tracks while the B-deck files are typically the NHC Best Track Analysis but could also be any ATCF format reference.

    * **Output**: ASCII output with the suffix .tcstat.

#. **TC-Stat Tool**

    * **Input**: One or more TCSTAT output files output from the TC-Pairs tool and, optionally, one configuration file containing specifications for the analysis job(s) to be run on the TCSTAT data.

    * **Output**: ASCII output of the analysis jobs will be printed to the screen unless redirected to a file using the “-out” option.

#. **TC-Gen Tool**

    * **Input**: One or more Tropical Cyclone genesis format files, one or more verifying operational and BEST track files in ATCF format, and one configuration file.

    * **Output**: One STAT file containing all of the requested line types and several ASCII files for each line type requested.

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
__________________________

Part of the strength of MET is the leveraging of capability across tools. There are several config options that are common to many of the tools. They are described in this section.

Many of the MET tools use a configuration file to set parameters. This prevents the command line from becoming too long and cumbersome and makes the output easier to duplicate.

Settings common to multiple tools are described in the following sections while those specific to individual tools are explained in the sections for those tools. In addition, these configuration settings are described in the **share/met/config/README** file and the **share/met/config/README-TC** file for the MET-Tropical Cyclone tools.

.. _Data IO MET Configuration File Options:

MET Configuration File Options
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The information listed below may also be found in the **data/config/README** file.

.. highlight:: none
.. literalinclude:: ../../data/config/README

.. _Data IO MET-TC Configuration File Options:

MET-TC Configuration File Options
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The information listed below may also be found in the **data/config/README_TC** file.

.. highlight:: none
.. literalinclude:: ../../data/config/README_TC
