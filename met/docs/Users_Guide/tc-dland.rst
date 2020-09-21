.. _tc-dland:

TC-Dland Tool
=============

Introduction
____________

Many filtering criteria within the MET-TC tools depend on the distinction between when a storm is over land or water. The TC-dland tool was developed to aid in quickly parsing data for filter jobs that only verify over water, threshold verification based on distance to land, and exclusion of forecasts outside a specified time window of landfall. For each grid point in the user-specified grid, it computes the great circle arc distance to the nearest coast line. Compared to the simple Euclidean distances, great circle arc distances are more accurate but take considerably longer to compute. Grid points over water have distances greater than zero while points over land have distances less than zero.

While the TC-dland tool is available to be run, most users will find the precomputed distance to land files distributed with the release sufficient. Therefore, the typical user will not actually need to run this tool.

Input/output format
___________________

The input for the TC-dland tool is a file containing the longitude (degrees east) and latitude (degrees north) of all the coastlines and islands considered to be a significant landmass. The default input is to use all three land data files (**aland.dat, shland.dat, wland.dat**) found in the installed **share/met/tc_data/** directory. The use of all three files produces a global land data file. The **aland.dat** file contains the longitude and latitude distinctions used by NHC for the Atlantic and eastern North Pacific basins, the **shland.dat** contains longitude and latitude distinctions for the Southern Hemisphere (south Pacific and South Indian Ocean), and the **wland.dat** contains the remainder of the Northern Hemisphere (western North Pacific and North Indian Ocean). Users may supply their own input file in order to refine the definition of coastlines and a significant landmass.

The output file from TC-dland is a NetCDF format file containing a gridded field representing the distance to the nearest coastline or island, as specified in the input file. This file is used in the TC-Pairs tool to compute the distance from land for each track point in the adeck and bdeck. As noted in :numref:`met_directory_structure`, precomputed distance to land (NetCDF output from TC-dland) files are available in the release. In the installed **share/met/tc_data** directory: 

**dland_nw_hem_tenth_degree.nc:** TC-dland output from **aland.dat** using a 1/10th degree grid

**dland_global_tenth_degree.nc:** TC-dland output from all three land data files (global coverage) using a 1/10th degree grid.

Practical information
_____________________

This section briefly describes how to run tc_dland. The default grid is set to 1/10th degree Northwest (NW) hemispheric quadrant (over North America) grid.

tc_dland usage
~~~~~~~~~~~~~~

.. code-block:: none

  Usage: tc_dland
         out_file
         [-grid spec]
         [-noll]
         [-land file]
         [-log file]
         [-v level]
         [-compress level]

**tc_dland** has one required argument and accepts several optional ones.

Required arguments for tc_dland
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. The **out_file** argument indicates the NetCDF output file containing the computed distances to land.

Optional arguments for tc_dland
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

2. The **-grid spec** argument overrides the default grid (1/10th NH grid). Spec = **lat_ll lon_ll delta_lat delta_lon n_lat n_lon**

3. The **-noll** argument skips writing the lon/lat variables in the output NetCDF file to reduce the file size.

4. The **-land file** argument overwrites the default land data files (**aland.dat, shland.dat**, and **wland.dat**).

5. The **-log file** argument outputs log messages to the specified file.

6. The **-v level** option indicates the desired level of verbosity. The contents of “level” will override the default setting of 2. Setting the verbosity to 0 will make the tool run with no log messages, while increasing the verbosity above 1 will increase the amount of logging.

7. The **-compress level** option specifies the desired level of compression (deflate level) for NetCDF variables. The valid level is between 0 and 9. Setting the compression level to 0 will make no compression for the NetCDF output. Lower numbers result in minimal compression and faster I/O processing speed; higher numbers result in better compression, but at the expense of I/O processing speed.
