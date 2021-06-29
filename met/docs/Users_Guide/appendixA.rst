.. _appendixA:

Appendix A FAQs & How do I ... ?
================================

Frequently Asked Questions
__________________________

File_IO
~~~~~~~

**Q. File_IO - How do I improve the speed of MET Tools using Gen_Vx_Mask?**

A. The main reason to use gen_vx_mask is to make the MET
statistics tools (i.e. point_stat or grid_stat) run
faster. It can be slow to figure out which points are
inside/outside a polyline region if the polyline contains
thousands of points. In that case, run gen_vx_mask to
create the mask once rather than having to recreate it
each time a MET statistics tool is run. But if the
polyline only contains a small number of points,
running gen_vx_mask first would only save a second or two.
		 
In the usage statement for gen_vx_mask, the "mask_file"
is an ASCII Lat/Lon polyline file or gridded data file
defining the masking region.

So pass a gridded data file for the nest as the
"mask_file" rather than having to create a set of
lat/lon points. 

Here's an example of that, using data from the MET tarball:

  .. code-block:: ini

		  ${MET_BUILD_BASE}/bin/gen_vx_mask \
		  data/sample_fcst/2005080700/wrfprs_ruc13_12.tm00_G212 \
		  data/sample_fcst/2009123112/arw-fer-gep1/d01_2009123112_02400.grib \
		  mask.nc -name MY_MASK
 
If the result contains slightly different matched pair
counts (621, 619 and 617).
There could be a couple of answers.

1.
The polyline mask for the different grids is producing
slightly different results and the differences lie
along the boundary of the mask.

2.
There are some missing data values somewhere in the
forecast and observations
causing slightly different matched pairs.
		
To investigate this, run a configuration of point_stat to
dump out the MPR
lines for those three runs. Then take a closer look at
them to see where the
differences lie. Identifying the stations where the
differences occur is the
first step in finding an explanation.

**File_IO - How do I use map_data? Use China as an example.**

A. This example starts with a 0.5 degree GFS and completes
the following steps:

1.
Use the regrid_data_plane tool to regrid 2m temperature
to a smaller domain centered on China:

  .. code-block:: ini
				  
		  ${MET_BUILD_BASE}/bin/regrid_data_plane \ 
		  gfs_2012040900_F012.grib \ 
		  'latlon 160 80 15.0 60.0 0.5 0.5' \ 
		  china_tmp_2m.nc \ 
		  -field 'name="TMP"; level="Z2";'

2.
Run plot_data_plane to plot with the default map background:

  .. code-block:: ini
				
		${MET_BUILD_BASE}/bin/plot_data_plane 
		china_tmp_2m.nc china_tmp_2m.ps \ 
		'name="TMP_Z2"; level="(*,*)";'

3.
Re-run but pointing only to the admin_China_data:

  .. code-block:: ini
		
		${MET_BUILD_BASE}/bin/plot_data_plane 
		china_tmp_2m.nc china_tmp_2m_admin.ps \ 
		'name="TMP_Z2"; level="(*,*)"; 
		map_data = { source = [ 
		{ file_name = 
		"${MET_BUILD_BASE}/data/map/admin_by_country/admin_China_data"; 
		} 
		]; 
		}'
				
An arbitrary number of map_data "file_name" entries
can be listed. However, using "country_data" doesn't
look very good with the "admin_China_data".
		
To apply this to any MET tool runs, just cut-and-paste
the "map_data" section listed above into the appropriate
config file. That will overwrite the default settings it
reads from the ConfigMapData file. Alternatively, update
the default map data files in that ConfigMapData file.

**Q. FILE_IO - What is another way to understand the Number of Matched Pairs?**

A.  In this example the dimension of the grid is 37x37. Thus, up to
    1369 matched pairs are possible. However, if the forecast or
    observation contains bad data at a point, that matched pair is
    not included in the calculations. Use the ncview tool to look at
    an example netCDF file. If the forecast field contains missing data
    around the edge of the domain, then that is a reason there may be
    992 matched pairs instead of 1369.

**Q.  FILE_IO - What is required for formatting files in NetCDF?**

A.  In order to use gridded NetCDF files in MET, the files need to
    look like the output of the pcp_combine tool.
    Listed below is the header from one of the NetCDF files from
    pcp_combine created by the METv5.1 test scripts. Here are the
    required parts.

    1.
    Dimensions should be named "lat" and "lon".
    
    2.
    The "lat" and "lon" variable are **NOT** required.
    
    3.
    Gridded variables (e.g. APCP_12) must use the "lat" and "lon" dimensions.
    
    4.
    Gridded variables should include the attributes listed in the example
    (for timing info, only the init_time_ut, valid_time_ut, and
    accum_time_sec are actually used. "ut" stands for unix time,
    the number of seconds since Jan 1, 1970).
    
    5.
    Global attributes should include the grid/projection information.

**Q. FILE_IO - How do I choose a Time Slice in a NetCDF file?**

A.  When processing NetCDF files, the level information needs to be
    specified to tell MET which 2D slice of data to use. There is
    currently no way to explicitly define which time slice to use
    other than selecting the time index.
    
    Let's use plot_data_plane as an example:

      .. code-block:: ini
		      
		      ${MET_BUILD_BASE}/bin/plot_data_plane \ 
		      MERGE_20161201_20170228.nc \ 
		      obs.ps \ 
		      'name="APCP"; level="(5,*,*)";'

    Since these indices are 0-based, this will select the 6-th
    time slice of the APCP data and plot it.

**Q. FILE_IO - How do I use the UNIX Time Conversion?**

A.  Regarding the timing information in the NetCDF variable attributes...

      .. code-block:: ini
		      
		      APCP_24:init_time_ut = 1306886400 ;
		      
    “ut” stands for UNIX time, which is the number of seconds
    since Jan 1, 1970. It is a convenient way of storing timing
    information since it is easy to add/subtract. The UNIX date command
    can be used to convert back/forth between unix time and time strings:

    1.
    Convert unix time to ymd_hms date

      .. code-block:: ini
		      
    		      date -ud '1970-01-01 UTC '1306886400' seconds' +%Y%m%d_%H%M%S 20110601_000000
 
    2.
    Convert ymd_hms to unix date

      .. code-block:: ini
		      
		      date -ud ''2011-06-01' UTC '00:00:00'' +%s 1306886400
		  
    Regarding TRMM data, it may be easier to work with the binary data and
    use the trmmbin2nc.R script described on this page:
    http://www.dtcenter.org/met/users/downloads/observation_data.php

    Follow the TRMM binary links to either the 3 or 24-hour accumulations,
    save the files, and run them through that script. That is the faster
    and easier than trying to get an ASCII dump. That Rscript can also
    subset the TRMM data if needed. Look for the section of it titled: 

    3.
    Output domain specification 

    Define the lat/lon's that needs to be included in the output.

**Q. How does fixed-width output format work?**

A.  MET does not use the Fortran-like fixed width format in its
    ASCII output file. Instead, the column widths are adjusted for each
    run to insert at least one space between adjacent columns. The header
    columns of the MET output contain user-defined strings which may be
    of arbitrary length. For example, columns such as MODEL, OBTYPE, and
    DESC may be set by the user to any string value. Additionally, the
    amount of precision written is also configurable. The
    "output_precision" config file entry can be changed from its default
    value of 5 decimal places... up to 12 decimal places. That too would
    impact the column widths of the output.
    
    Due to these issues, it is not possible to select a reasonable fixed
    width for each column ahead of time. The AsciiTable class in MET does
    a lot of work to line up the output columns, making sure there's
    at least one space between them.
    
    If a fixed-width format is needed, the easiest option would be
    writing a script to post-process the MET output into the fixed-width
    format that is needed or that the code expects.

 **Q. How does scientific notation work?**

 A.  By default, the ascii output files created by MET make use of
     scientific notation when appropriate. The formatting of the
     numbers that the AsciiTable class writes is handled by a call
     to printf. The "%g" formatting option which can result in
     scientific notation: http://www.cplusplus.com/reference/cstdio/printf/

     It has been recommended that a configuration option be added to
     MET to disable the use of scientific notation. That enhancement
     is planned for a future release.
    
**Q. Why was the MET written largely in C++ instead of FORTRAN?**

A. MET relies upon the object-oriented aspects of C++, particularly in
   using the MODE tool. Due to time and budget constraints, it also makes
   use of a pre-existing forecast verification library that was developed
   at NCAR.


**Q. Why is PrepBUFR used?**

A. The first goal of MET was to replicate the capabilities of existing verification packages and make these capabilities available to both the DTC and the public. 

**Q. Why is GRIB used?**

A. Forecast data from both WRF cores can be processed into GRIB format, and it is a commonly accepted output format for many NWP models.

**Q. Is GRIB2 supported?**

A. Yes, forecast output in GRIB2 format can be read by MET. Be sure to compile the GRIB2 code by setting the appropriate configuration file options (see Chapter 2). 

**Q. How does MET differ from the previously mentioned existing verification packages?**

A. MET is an actively maintained, evolving software package that is being made freely available to the public through controlled version releases.

**Q. How does the MODE tool differ from the Grid-Stat tool?**

A. They offer different ways of viewing verification. The Grid-Stat tool provides traditional verification statistics, while MODE provides specialized spatial statistics.

**Q. Will the MET work on data in native model coordinates?**

A. No - it will not. In the future, we may add options to allow additional model grid coordinate systems.

**Q. How do I get help if my questions are not answered in the User's Guide?**

A. First, look on our `MET User's Guide website <https://dtcenter.org/community-code/model-evaluation-tools-met>`_. If that doesn't answer your question, then email: met_help@ucar.edu.

**Q. Where are the graphics?**

A. Currently, very few graphics are included. The plotting tools (plot_point_obs, plot_data_plane, and plot_mode_field) can help you visualize your raw data. Also, ncview can be used with the NetCDF output from MET tools to visualize results. Further graphics support will be made available in the future on the MET website.

**Q. How do I find the version of the tool I am using?**

A. Type the name of the tool followed by **-version**. For example, type “pb2nc **-version**”.

**Q. What are MET's conventions for latitude, longitude, azimuth and bearing angles?**

A. MET considers north latitude and east longitude positive. Latitudes have range from :math:`-90^\circ` to :math:`+90^\circ`. Longitudes have range from :math:`-180^\circ` to :math:`+180^\circ`. Plane angles such as azimuths and bearing (example: horizontal wind direction) have range :math:`0^\circ` to :math:`360^\circ` and are measured clockwise from the north.

.. _Troubleshooting:   
   
Troubleshooting
_______________

The first place to look for help with individual commands is this user's guide or the usage statements that are provided with the tools. Usage statements for the individual MET tools are available by simply typing the name of the executable in MET's *bin/* directory. Example scripts available in the MET's *scripts/* directory show examples of how one might use these commands on example datasets. Here are suggestions on other things to check if you are having problems installing or running MET.

**MET won't compile**

* Have you specified the locations of NetCDF, GNU Scientific Library, and BUFRLIB, and optional additional libraries using corresponding MET\_ environment variables prior to running configure?

* Have these libraries been compiled and installed using the same set of compilers used to build MET?

* Are you using NetCDF version 3.4 or version 4? Currently, only NetCDF version 3.6 can be used with MET.

**Grid_stat won't run**

* Are both the observational and forecast datasets on the same grid?

**MODE won't run**

* If using precipitation, do you have the same accumulation periods for both the forecast and observations? (If you aren't sure, run pcp_combine.)

* Are both the observation and forecast datasets on the same grid?

**Point-Stat won't run**

* Have you run pb2nc first on your PrepBUFR observation data?

**Error while loading shared libraries**

* Add the lib dir to your LD_LIBRARY_PATH. For example, if you receive the following error: “./mode_analysis: error while loading shared libraries: libgsl.so.19: cannot open shared object file: No such file or directory”, you should add the path to the gsl lib (for example, */home/user/MET/gsl-2.1/lib*) to your LD_LIBRARY_PATH.

**General troubleshooting**

* For configuration files used, make certain to use empty square brackets (e.g. [ ]) to indicate no stratification is desired. Do NOT use empty double quotation marks inside square brackets (e.g. [“”]).

* Have you designated all the required command line arguments?

* Try rerunning with a higher verbosity level. Increasing the verbosity level to 4 or 5 prints much more diagnostic information to the screen. 

Where to get help
_________________

If none of the above suggestions have helped solve your problem, help is available through: met_help@ucar.edu

How to contribute code
______________________

If you have code you would like to contribute, we will gladly consider your contribution. Please send email to: met_help@ucar.edu
