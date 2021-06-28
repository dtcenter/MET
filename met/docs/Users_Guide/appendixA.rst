.. _appendixA:

Appendix A FAQs & How do I ... ?
================================

Frequently Asked Questions
__________________________

**File_IO**

.. panels::

   .. dropdown:: File_IO - How to improve the speed of MET Tools with Gen_Vx_Mask

		 The main reason to use gen_vx_mask is to make the MET
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

		 ${MET_BUILD_BASE}/bin/gen_vx_mask \
		 data/sample_fcst/2005080700/wrfprs_ruc13_12.tm00_G212 \
		 data/sample_fcst/2009123112/arw-fer-gep1/d01_2009123112_02400.grib \
		 mask.nc -name MY_MASK
 
		If the result contains slightly different matched pair counts (621, 619 and 617).
		There could be a couple of answers.

		#. The polyline mask for the different grids is producing slightly different results
		and the differences lie along the boundary of the mask.
		#. There are some missing data values somewhere in the forecast and observations
		causing slightly different matched pairs.
		
		To investigate this, run a configuration of point_stat to dump out the MPR
		lines for those three runs. Then take a closer look at them to see where the
		differences lie. Identifying the stations where the differences occur is the
		first step in finding an explanation.

   .. dropdown:: :fa:`eye,mr-1` test with eye

		 hidden content
	 

**Q. Why was the MET written largely in C++ instead of FORTRAN?**

A. MET relies upon the object-oriented aspects of C++, particularly in using the MODE tool. Due to time and budget constraints, it also makes use of a pre-existing forecast verification library that was developed at NCAR.

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
