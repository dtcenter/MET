.. _masking:

***************
Spatial Masking
***************

Verification over a particular region or area of interest may be performed using "masking". Defining a masking region is simply selecting the desired set of grid points to be used. The Gen-Vx-Mask tool automates this process and replaces the Gen-Poly-Mask and Gen-Circle-Mask tools from previous releases. It may be run to create a bitmap verification masking region to be used by many of the statistical tools. This tool enables the user to generate a masking region once for a domain and apply it to many cases. It supports multiple methods for defining regional spatial masks, as described below. In addition, Gen-Vx-Mask can be run iteratively, passing the output from one run as input to the next, to combine multiple masking regions and define a complex area of interest.

Gen-Vx-Mask Tool
================

The Gen-Vx-Mask tool may be run to create a bitmap verification masking region to be used by the MET statistics tools. This tool enables the user to generate a masking region once for a domain and apply it to many cases. While the MET statistics tools can define some masking regions on the fly using pre-defined grids and polylines, doing so can be slow, especially for complex polylines containing hundreds of vertices. Using the Gen-Vx-Mask tool to create a bitmap masking region before running the other MET tools will make them run more efficiently.

gen_vx_mask Usage
-----------------

The usage statement for the Gen-Vx-Mask tool is shown below:

.. code-block:: none

  Usage: gen_vx_mask
         input_grid
         mask_file
         out_file
         -type str
         [-input_field string]
         [-mask_field string]
         [-complement]
         [-union | -intersection | -symdiff]
         [-thresh string]
         [-height n]
         [-width n]
         [-shapeno n]
         [-shape_str name string]
         [-value n]
         [-name string]
         [-log file]
         [-v level]
         [-compress level]

gen_vx_mask has four required arguments and can take optional ones. Note that **-type string** (masking type) was optional in prior versions but is now required.

Required Arguments for gen_vx_mask
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. The **input_grid** is a named grid, the path to a gridded data file, or an explicit grid specification string (see :numref:`App_B-grid_specification_strings`) which defines the grid for which a mask is to be defined. If set to a gen_vx_mask output file, automatically read mask data as the **input_field**.

2. The **mask_file** defines the masking information, see below.

• For "poly", "poly_xy", "box", "circle", and "track" masking, specify an ASCII Lat/Lon file. Refer to :ref:`Types_of_masking_gen_vx_mask` for details on how to construct the ASCII Lat/Lon file for each type of mask.

• For "grid" masking, specify a named grid, the path to a gridded data file, or an explicit grid specification.

• For "data" masking, specify a gridded data file.

• For "solar_alt", "solar_azi", and "solar_time" masking, specify a gridded data file or a time string in YYYYMMDD[_HH[MMSS]] UTC format.

• For "lat" and "lon" masking, no "mask_file" is needed, simply repeat "input_grid".

• For "shape" masking, specify a shapefile (suffix ".shp").

3. The **out_file** is the output NetCDF mask file to be written.

4. The **-type string** is a comma-separated list of masking types to be applied. The application will print an error message and exit if "-type string" is not specified at least once on the command line. Use multiple times for multiple mask types. See a list of supported masking types described below.

.. note::

   While multiple **-type** mask types can be requested in a single run, all requested masking types must use the same **mask_file** setting.

Optional Arguments for gen_vx_mask
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

5. The **-input_field string** option initializes the "input_grid" with values from this field.

6. The **-mask_field string** option defines the field from "mask_file" to be used for "data" masking. Use multiple times for multiple mask types.

7. The **-complement** option can be used to compute the complement of the area defined by "mask_file".

8. The **-union | -intersection | -symdiff** options specify how to combine multiple binary masks. Applies to masks read from the "input_field" and those generated during the current run.

9. The **-thresh string** option is a comma-separated list of thresholds to be applied. Use multiple times for multiple mask types.

• For "circle" and "track" masking, threshold the distance (km).

• For "data" masking, threshold the values of "mask_field".

• For "solar_alt" and "solar_azi" masking, threshold the computed solar values (deg).

• For "solar_time" masking, threshold the solar time (hr).

• For "lat" and "lon" masking, threshold the latitude and longitude values (deg).

10. The **-height n** and **-width n** options specify the dimensions in grid units for "box" masking.

11. The **-shapeno n** option is only used for shapefile masking. See the description of shapefile masking below.

12. The **-shape_str name string** option is only used for shapefile masking. See the description of shapefile masking below.

13. The **-value n** option overrides the default output mask data value (1).

14. The **-name string** option specifies the output variable name for the mask.

15. The **-log file** option writes log messages to the specified log file. All messages will be written to that file as well as standard out and error. Thus, users can save the messages without having to redirect the output on the command line. The default behavior is no log file.

16. The **-v level** option indicates the desired level of verbosity. The value of "level" will override the default setting of 2. Setting the verbosity to 0 will make the tool run with no log messages, while increasing the verbosity will increase the amount of logging.

17. The **-compress level** option indicates the desired level of compression (deflate level) for NetCDF variables. The valid level is between 0 and 9. The value of "level" will override the default setting of 0 from the configuration file or the environment variable MET_NC_COMPRESS. Setting the compression level to 0 will make no compression for the NetCDF output. Lower number is for fast compression and higher number is for better compression.

.. _Types_of_masking_gen_vx_mask:

Types of Masking Available in gen_vx_mask
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
The Gen-Vx-Mask tool supports the following types of masking region definition selected using the **-type** command line option:

1. Polyline (**poly**) masking reads an input ASCII file containing Lat/Lon locations, connects the first and last points, and selects grid points whose Lat/Lon location falls inside that polyline in Lat/Lon space. This option is useful when defining geographic subregions of a domain.

2. Polyline XY (**poly_xy**) masking reads an input ASCII file containing Lat/Lon locations. It converts the polyline Lat/Lon locations into grid X/Y space and connects the first and last points. It selects grid points whose X/Y location falls inside that polyline in X/Y space. This option is useful when defining geographic subregions of a domain.

3. Box (**box**) masking reads an input ASCII file containing Lat/Lon locations and draws a box around each point. The height and width of the box is specified by the **-height** and **-width** command line options in grid units. For a square, only one of **-height** or **-width** needs to be used.

4. Circle (**circle**) masking reads an input ASCII file containing Lat/Lon locations and for each grid point, computes the minimum great-circle arc distance in kilometers to those points. If the **-thresh** command line option is not used, the minimum distance value for each grid point will be written to the output. If it is used, only those grid points whose minimum distance meets the threshold criteria will be selected. This option is useful when defining areas within a certain radius of radar locations.

5. Track (**track**) masking reads an input ASCII file containing Lat/Lon locations and for each grid point, computes the minimum great-circle arc distance in kilometers to the track defined by those points. The first and last track points are not connected. As with **circle** masking the output for each grid point depends on the use of the **-thresh** command line option. This option is useful when defining the area within a certain distance of a hurricane track.

6. Grid (**grid**) masking reads an input gridded data file, extracts the field specified using its grid definition, and selects grid points falling inside that grid. This option is useful when using a model nest to define the corresponding area of the parent domain.

7. Data (**data**) masking reads an input gridded data file, extracts the field specified using the **-mask_field** command line option, thresholds the data using the **-thresh** command line option, and selects grid points which meet that threshold criteria. The option is useful when thresholding topography to define a mask based on elevation or when threshold land use to extract a particular category.

8. Solar altitude (**solar_alt**) and solar azimuth (**solar_azi**) masking computes the solar altitude and azimuth values in degrees at each grid point for the time defined by the **mask_file** setting. **mask_file** may either be set to an explicit time string in YYYYMMDD[_HH[MMSS]] UTC format or to a gridded data file. If set to a gridded data file, the **-mask_field** command line option specifies the field of data whose valid time should be used. If the **-thresh** command line option is not used, the raw solar altitude or azimuth degrees for each grid point will be written to the output. If it is used, the resulting binary mask field will be written. This option is useful when defining a day/night mask.

9. Solar time (**solar_time**) masking computes the solar time in decimal hours at each grid point for the for the time defined by the **mask_file** setting, as described above. The solar hours of the day range from 0 to 24, with a value of 12 indicating solar noon. Note that solar time is based only on longitude. If the **-thresh** command line option is not used, the raw solar time hours will be written to the output.

10. Latitude (**lat**) and longitude (**lon**) masking computes the latitude and longitude value at each grid point. This logic only requires the definition of the grid, specified by the **input_file**. Technically, the **mask_file** is not needed, but a value must be specified for the command line to parse correctly. Users are advised to simply repeat the **input_file** setting twice. If the **-thresh** command line option is not used, the raw latitude or longitude values for each grid point will be written to the output. This option is useful when defining latitude or longitude bands over which to compute statistics.

11. Shapefile (**shape**) masking uses closed polygons taken from an ESRI shapefile to define the masking region. Gen-Vx-Mask reads the shapefile with the ".shp" suffix and extracts the latitude and longitudes of the vertices. The shapefile must consist of closed polygons rather than polylines, points, or any of the other data types that shapefiles support. When the **-shape_str** command line option is used, Gen-Vx-Mask also reads metadata from the corresponding dBASE file with the ".dbf" suffix.

    Shapefiles usually contain more than one polygon, and the user must select which of these shapes should be used. The **-shapeno n** and **-shape_str name string** command line options enable the user to select one or more polygons from the shapefile. For **-shape n**, **n** is a comma-separated list of integer shape indices to be used. Note that these values are zero-based. So the first polygon in the shapefile is shape number 0, the second polygon in the shapefile is shape number 1, etc. For example, **-shapeno 0,1,2** uses the first three shapes in the shapefile. When multiple shapes are specified, the mask is defined as their union. So all grid points falling inside at least one of the specified shapes are included in the mask.

    For the user's convenience, some utilities that perform human-readable screen dumps of shapefile contents are provided with MET. The **gis_dump_shp**, **gis_dump_shx**, and **gis_dump_dbf** tools enable the user to examine the contents of these shapefiles. In particular, the **gis_dump_dbf** tool prints the name and values of the metadata for each record. The **-shape_str** command line option filters the shapes using the attributes listed in the **gis_dump_dbf** output, and requires two arguments. The **name** argument is set to any valid shapefile attribute, and the **string** argument is a comma-separated list of values to be matched. An example of using **-shape_str** is **-shape_str CONTINENT Europe**, which will match all "CONTINENT" attribues that have the string "Europe" in them. Strings that contain embedded whitespace should be enclosed in single quotes. Also note that case insensitive matching is used. For example, when using a global country outline shapefile, **-shape_str NAME 'united kingdom,united states of america'** matches the "NAME" attributes that have both "United Kingdom" and "United States of America" in them. If **-shape_str** is used multiple times, only shapes matching all the named attributes will be used. For example, **-shape_str CONTINENT Europe -shape_str NAME Spain,Portugal** will only match shapes where the "CONTINENT" attrinute contains "Europe "and the "NAME" attribute contains "Spain" or "Portugal". If a user wishes, they can combine both the **-shape_str** and **-shapeno** options. In this case, the union of all matches from the shapefile will be used.

The polyline, polyline XY, box, circle, and track masking methods all read an ASCII file containing Lat/Lon locations. Those files must contain a string, which defines the name of the masking region, followed by a series of whitespace-separated latitude (degrees north) and longitude (degree east) values.

Logic for gen_vx_mask
^^^^^^^^^^^^^^^^^^^^^

The Gen-Vx-Mask tool performs three main steps, described below.

1. Determine the input grid definition.

• Read the **input_grid** to determine the grid over which the mask should be defined.

• By default, initialize the input field value at each grid point to zero.

• If the **-input_field** option was specified, initialize each input field value using the values from that field.

• If the **input_grid** is the output from a previous run of Gen-Vx-Mask, automatically initialize each input field value with the previously-generated mask value.

2. Process each of the requested masking regions.

• For each **-type** mask type option requested, process the **mask_file** setting.

• Read the **mask_file**, process it based on the **-type** setting (as described above), and define the masking region value for each grid point to specify whether or not it is included in the mask.

• By default, store the mask value as 1 unless the **-value** option was specified to override that default value.

• If the **-complement** option was specified, select the opposite of the masking area.

• Apply logic to combine the newly generated masking region with those defined by previous **-type** mask type options to create a **mask_field**.

  • By default, compute the **-union** of multiple masks, unless **-intersection** or **-symdiff** were specified to override this default.

3. Apply logic to combine the input field and current masking region and write the **out_file**.

• By default, the output value at each grid point is set to the value of current masking region if included in the mask, or the value of **input_field** if not included.

• If the **-union, -intersection**, or **-symdiff** option was specified, apply that logic to the input field and current masking region values at each grid point to determine the output value.

• Write the output value for each grid point to the **out_file**.

Examples for gen_vx_mask
^^^^^^^^^^^^^^^^^^^^^^^^

An example of defining the northwest hemisphere of the earth, as defined by latitudes >= 0 and longitudes < 0, in a single run is shown below:

.. code-block:: none

  gen_vx_mask G004 G004 northwest_hemisphere.nc \
  -type lat,lon -thresh ge0,lt0 \
  -intersection -name nw_hemisphere


The Gen-Vx-Mask tool to be run iteratively on its own output using different **mask_file** settings to generate complex masking areas. The **-union, -intersection**, and **-symdiff** options control the logic for combining the input field and current mask values at each grid point. For example, one could define a complex masking region by selecting grid points with an elevation greater than 1000 meters within a Contiguous United States geographic region by doing the following:

• Run Gen-Vx-Mask to apply data masking by thresholding a field of topography greater than 1000 meters.

• Run Gen-Vx-Mask a second time on the output from the first call and applying polyline masking to define the geographic area of interest. Use the **-intersection** option to only select grid points whose value is non-zero in both the input field and the current mask.

An example of this Gen-Vx-Mask calling sequence is shown below:

.. code-block:: none

  gen_vx_mask fcst.grib fcst.grib TOPO_mask.nc \
  -type data \
  -mask_field 'name="TOPO"; level="L0";' \
  -thresh '>1000'

  gen_vx_mask TOPO_mask.nc CONUS.poly TOPO_CONUS_mask.nc \
  -type poly \
  -intersection -name TOPO_CONUS_mask


Here, Gen-Vx-Mask uses the **data** masking type to read topography data (**TOPO**) from a GRIB file and thresholds the values **>1000** to define a topography mask. The second run of Gen-Vx-Mask uses the **poly** masking type to read the ASCII Lat/Lon file named **CONUS.poly** and select all grid points within that region to define a polyline mask. When reading its own output, Gen-Vx-Mask automatically reads the topography mask as the **input_field** and applies the **intersection** logic to combine it with the polyline mask, selecting grid points where both conditions are true. The resulting complex mask is written to the output NetCDF file named **TOPO_CONUS_mask.nc**.

Feature-Relative Methods
========================

This section contains a description of several methods that may be used to perform feature-relative (or event -based) evaluation. The methodology pertains to examining the environment surrounding a particular feature or event such as a tropical, extra-tropical cyclone, convective cell, snow-band, etc. Several approaches are available for these types of investigations including applying masking described above (e.g. circle or box) or using the FORCE interpolation method in the regrid configuration option (see :numref:`config_options`). These methods generally require additional scripting, including potentially storm-track identification, outside of MET to be paired with the features of the MET tools. METplus may be used to execute this type of analysis.  Please refer to the `METplus User's Guide <https://metplus.readthedocs.io/>`_.
