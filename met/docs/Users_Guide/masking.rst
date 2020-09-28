.. _masking:

Regional Verification using Spatial Masking
===========================================

Verification over a particular region or area of interest may be performed using “masking”. Defining a masking region is simply selecting the desired set of grid points to be used. The Gen-Vx-Mask tool automates this process and replaces the Gen-Poly-Mask and Gen-Circle-Mask tools from previous releases. It may be run to create a bitmap verification masking region to be used by many of the statistical tools. This tool enables the user to generate a masking region once for a domain and apply it to many cases. It has been enhanced to support additional types of masking region definition (e.g. tropical-cyclone track over water only). An iterative approach may be used to define complex areas by combining multiple masking regions together.

Gen-Vx-Mask tool
________________

The Gen-Vx-Mask tool may be run to create a bitmap verification masking region to be used by the MET statistics tools. This tool enables the user to generate a masking region once for a domain and apply it to many cases. While the MET statistics tools can define some masking regions on the fly using polylines, doing so can be slow, especially for complex polylines containing hundreds of vertices. Using the Gen-Vx-Mask tool to create a bitmap masking region before running the other MET tools will make them run more efficiently.

gen_vx_mask usage
~~~~~~~~~~~~~~~~~

The usage statement for the Gen-Vx-Mask tool is shown below:

.. code-block:: none

  Usage: gen_vx_mask
         input_grid
         mask_file
         out_file
         [-type str]
         [-input_field string]
         [-mask_field string]
         [-complement]
         [-union | -intersection | -symdiff]
         [-thresh string]
         [-height n]
         [-width n]
         [-shapeno n]
         [-value n]
         [-name string]
         [-log file]
         [-v level]
         [-compress level]

gen_vx_mask has three required arguments and can take optional ones.

Required arguments for gen_vx_mask
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. The **input_file** argument is a gridded data file which specifies the grid definition for the domain over which the masking bitmap is to be defined. If output from gen_vx_mask, automatically read mask data as the **input_field**.

2. The **mask_file** argument defines the masking information, see below.

• For “poly”, “box”, “circle”, and “track” masking, specify an ASCII Lat/Lon file.

• For “grid” and “data” masking, specify a gridded data file.

• For “solar_alt” and “solar_azi” masking, specify a gridded data file or a time string in YYYYMMDD[_HH[MMSS]] format.

• For “lat” and “lon” masking, no “mask_file” needed, simply repeat the path for “input_file”.

• For “shape” masking, specify an ESRI shapefile (.shp).

3. The **out_file** argument is the output NetCDF mask file to be written.

Optional arguments for gen_vx_mask
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

4. The **-type string** option can be used to override the default masking type (poly). See description of supported types below.

5. The **-input_field string** option can be used to read existing mask data from “input_file”.

6. The **-mask_field string** option can be used to define the field from “mask_file” to be used for “data” masking.

7. The **-complement** option can be used to compute the complement of the area defined by “mask_file”.

8. The **-union | -intersection | -symdiff** option can be used to specify how to combine the masks from “input_file” and “mask_file”.

9. The **-thresh string** option can be used to define the threshold to be applied.

• For “circle” and “track” masking, threshold the distance (km).

• For “data” masking, threshold the values of “mask_field”.

• For “solar_alt” and “solar_azi” masking, threshold the computed solar values.

• For “lat” and “lon” masking, threshold the latitude and longitude values. 

10. The **-height n** and **-width n** options set the size in grid units for “box”masking.

11. The **-shapeno n** option is only used for shapefile masking. (See description of shapefile masking below).

12. The **-value n** option can be used to override the default output mask data value (1).

13. The **-name string** option can be used to specify the output variable name for the mask.

14. The **-log file** option directs output and errors to the specified log file. All messages will be written to that file as well as standard out and error. Thus, users can save the messages without having to redirect the output on the command line. The default behavior is no log file. 

15. The **-v level** option indicates the desired level of verbosity. The value of "level" will override the default setting of 2. Setting the verbosity to 0 will make the tool run with no log messages, while increasing the verbosity will increase the amount of logging.

16. The **-compress level** option indicates the desired level of compression (deflate level) for NetCDF variables. The valid level is between 0 and 9. The value of “level” will override the default setting of 0 from the configuration file or the environment variable MET_NC_COMPRESS. Setting the compression level to 0 will make no compression for the NetCDF output. Lower number is for fast compression and higher number is for better compression.

The Gen-Vx-Mask tool supports the following types of masking region definition selected using the **-type** command line option:

1. Polyline (**poly**) masking reads an input ASCII file containing Lat/Lon locations, connects the first and last points, and selects grid points falling inside that polyline. This option is useful when defining geographic subregions of a domain.

2. Box (**box**) masking reads an input ASCII file containing Lat/Lon locations and draws a box around each point. The height and width of the box is specified by the **-height** and **-width** command line options in grid units. For a square, only one of **-height** or **-width** needs to be used.

3. Circle (**circle**) masking reads an input ASCII file containing Lat/Lon locations and for each grid point, computes the minimum great-circle arc distance in kilometers to those points. If the **-thresh** command line option is not used, the minimum distance value for each grid point will be written to the output. If it is used, only those grid points whose minimum distance meets the threshold criteria will be selected. This option is useful when defining areas within a certain radius of radar locations.

4. Track (**track**) masking reads an input ASCII file containing Lat/Lon locations and for each grid point, computes the minimum great-circle arc distance in kilometers to the track defined by those points. The first and last track points are not connected. As with **circle** masking the output for each grid point depends on the use of the **-thresh** command line option. This option is useful when defining the area within a certain distance of a hurricane track.

5. Grid (**grid**) masking reads an input gridded data file, extracts the field specified using its grid definition, and selects grid points falling inside that grid. This option is useful when using a model nest to define the corresponding area of the parent domain.

6. Data (**data**) masking reads an input gridded data file, extracts the field specified using the **-mask_field** command line option, thresholds the data using the **-thresh** command line option, and selects grid points which meet that threshold criteria. The option is useful when thresholding topography to define a mask based on elevation or when threshold land use to extract a particular category.

7. Solar altitude (**solar_alt**) and solar azimuth (**solar_azi**) masking computes the solar altitude and azimuth values at each grid point for the time defined by the **mask_file** setting. **mask_file** may either be set to an explicit time string in YYYYMMDD[_HH[MMSS]] format or to a gridded data file. If set to a gridded data file, the **-mask_field** command line option specifies the field of data whose valid time should be used. If the **-thresh** command line option is not used, the raw solar altitude or azimuth value for each grid point will be written to the output. If it is used, the resulting binary mask field will be written. This option is useful when defining a day/night mask.

8. Latitude (**lat**) and longitude (**lon**) masking computes the latitude and longitude value at each grid point. This logic only requires the definition of the grid, specified by the **input_file**. Technically, the **mask_file** is not needed, but a value must be specified for the command line to parse correctly. Users are advised to simply repeat the **input_file** setting twice. If the **-thresh** command line option is not used, the raw latitude or longitude values for each grid point will be written to the output. This option is useful when defining latitude or longitude bands over which to compute statistics.

9. Shapefile (**shape**) masking uses a closed polygon taken from an ESRI shapefile to define the masking region. Gen-Vx-Mask reads the shapefile with the ".shp" suffix and extracts the latitude and longitudes of the vertices. The other types of shapefiles (index file, suffix “.shx”, and dBASE file, suffix “.dbf”) are not currently used. The shapefile must consist of closed polygons rather than polylines, points, or any of the other data types that shapefiles support. Shapefiles usually contain more than one polygon, and the **-shape n** command line option enables the user to select one polygon from the shapefile. The integer **n** tells which shape number to use from the shapefile. Note that this value is zero-based, so that the first polygon in the shapefile is polygon number 0, the second polygon in the shapefile is polygon number 1, etc. For the user's convenience, some utilities that perform human-readable screen dumps of shapefile contents are provided. The gis_dump_shp, gis_dump_shx and gis_dump_dbf tools enable the user to examine the contents of her shapefiles. As an example, if the user knows the name of the particular polygon but not the number of the polygon in the shapefile, the user can use the gis_dump_dbf utility to examine the names of the polygons in the shapefile. The information written to the screen will display the corresponding polygon number.

The polyline, box, circle, and track masking methods all read an ASCII file containing Lat/Lon locations. Those files must contain a string, which defines the name of the masking region, followed by a series of whitespace-separated latitude (degrees north) and longitude (degree east) values.

The Gen-Vx-Mask tool performs three main steps, described below.

1. Determine the **input_field** and grid definition.

• Read the **input_file** to determine the grid over which the mask should be defined.

• By default, initialize the **input_field** at each grid point to a value of zero.

• If the **-input_field** option was specified, initialize the **input_field** at each grid point to the value of that field.

• If the **input_file** is the output from a previous run of Gen-Vx-Mask, automatically initialize each grid point with the **input_field** value.

2. Determine the **mask_field**.

• Read the **mask_file**, process it based on the **-type** setting (as described above), and define the **mask_field** value for each grid point to specify whether or not it is included in the mask.

• By default, store the mask value as 1 unless the **-value** option was specified to override that default value.

• If the **-complement** option was specified, the opposite of the masking area is selected.

3. Apply logic to combine the **input_field** and **mask_field** and write the **out_file**.

• By default, the output value at each grid point is set to the value of **mask_field** if included in the mask, or the value of **input_field** if not included.

• If the **-union, -intersection**, or **-symdiff** option was specified, apply that logic to the **input_field** and **mask_field** values at each grid point to determine the output value.

• Write the output value for each grid point to the **out_file**.

This three step process enables the Gen-Vx-Mask tool to be run iteratively on its own output to generate complex masking areas. Additionally, the **-union, -intersection**, and **-symdiff** options control the logic for combining the input data value and current mask value at each grid point. For example, one could define a complex masking region by selecting grid points with an elevation greater than 1000 meters within a specified geographic region by doing the following:

• Run the Gen-Vx-Mask tool to apply data masking by thresholding a field of topography greater than 1000 meters. 

• Rerun the Gen-Vx-Mask tool passing in the output of the first call and applying polyline masking to define the geographic area of interest. 

  – Use the **-intersection** option to only select grid points whose value is non-zero in both the input field and the current mask.

An example of the gen_vx_mask calling sequence is shown below:

.. code-block:: none

  gen_vx_mask sample_fcst.grb \
  CONUS.poly CONUS_poly.nc

In this example, the Gen-Vx-Mask tool will read the ASCII Lat/Lon file named **CONUS.poly** and apply the default polyline masking method to the domain on which the data in the file **sample_fcst.grib** resides. It will create a NetCDF file containing a bitmap for the domain with a value of 1 for all grid points inside the CONUS polyline and a value of 0 for all grid points outside. It will write an output NetCDF file named **CONUS_poly.nc**.

Feature-Relative Methods
________________________

This section contains a description of several methods that may be used to perform feature-relative (or event -based) evaluation. The methodology pertains to examining the environment surrounding a particular feature or event such as a tropical, extra-tropical cyclone, convective cell, snow-band, etc. Several approaches are available for these types of investigations including applying masking described above (e.g. circle or box) or using the “FORCE” interpolation method in the regrid configuration option (see :numref:`Data IO MET Configuration File Options`). These methods generally require additional scripting, including potentially storm-track identification, outside of MET to be paired with the features of the MET tools. METplus may be used to execute this type of analysis.  Please refer to the `METplus User's Guide <https://dtcenter.github.io/METplus/>`__.
