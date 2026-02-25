***********************
MET Release Information
***********************

MET Release Notes
=================

When applicable, release notes are followed by the GitHub issue number which describes the bugfix,
enhancement, or new feature (`MET GitHub issues <https://github.com/dtcenter/MET/issues>`_).
Important issues are listed **in bold** for emphasis.

MET Version 12.1.2 Release Notes (20260226)
-------------------------------------------

  .. dropdown:: Bugfixes

     * Fix segfault when reading climatological data from a CF-compliant NetCDF file fails
       (`#3235 <https://github.com/dtcenter/MET/issues/3235>`_).
     * Fix the logic to apply "set_attr_grid" before "ShiftRight"
       (`#3255 <https://github.com/dtcenter/MET/issues/3255>`_).
     * Fix ASCII2NC hang when run with an empty input file
       (`#3266 <https://github.com/dtcenter/MET/issues/3266>`_).
     * Fix ASCII2NC to handle a wider range of NDBC bad data values
       (`#3342 <https://github.com/dtcenter/MET/issues/3342>`_).

  .. dropdown:: METbaseimage testing environment

     * Update the METbaseimage to use Python 3.14
       (`METbaseimage#61 <https://github.com/dtcenter/METbaseimage/issues/61>`_).

  .. note:: MET version 12.1.2 is built upon METbaseimage version 3.4.9 in which the
            Python version was upgraded from 3.12 to 3.14. This change only impacts
            MET's Docker development and testing environment. MET version 12.1.2 should
            continue working well with Python version 3.12.

MET Version 12.1.1 Release Notes (20250827)
-------------------------------------------

  .. dropdown:: Bugfixes

     * Python embedding does not work for MET version 12.1.0 with Python version 3.10
       (`#3219 <https://github.com/dtcenter/MET/issues/3219>`_).

MET Version 12.1.0 Release Notes (20250730)
-------------------------------------------

  .. dropdown:: Bugfixes

     * Fix Grid-Stat segfault when SEEPS is the only NetCDF output type requested
       (`#3020 <https://github.com/dtcenter/MET/issues/3020>`_).
     * Fix incorrect polar stereographic projection handling for sea ice dataset
       (`#3023 <https://github.com/dtcenter/MET/issues/3023>`_).
     * Fix the PARUSR BUFRLIB failure when PB2NC is compiled with :code:`-O2` optimization
       (`#3054 <https://github.com/dtcenter/MET/issues/3054>`_).
     * Fix memory management issues by replacing variable length arrays with STL vectors and arrays
       (`#3075 <https://github.com/dtcenter/MET/issues/3075>`_).
     * Fix intermittent configuration string parsing :code:`yyerror` failure
       (`#3077 <https://github.com/dtcenter/MET/issues/3077>`_).
     * Fix compilation script logic for successful dependent library compilations
       (`#3092 <https://github.com/dtcenter/MET/issues/3092>`_).
     * Fix ASCII2NC failure for tab-delimited input files
       (`#3095 <https://github.com/dtcenter/MET/issues/3095>`_).
     * Refine the MET GRIB2 library logic to find a single GRIB2 table match rather multiple ones
       (`#3107 <https://github.com/dtcenter/MET/issues/3107>`_).
     * Fix the MET NetCDF library to support variables with more that 4 dimensions
       (`#3112 <https://github.com/dtcenter/MET/issues/3112>`_).
     * Correct NetCDF dimension range checking to use inclusive inequalities
       (`#3164 <https://github.com/dtcenter/MET/issues/3164>`_).
     * Fix MET-12.1.0-rc1 Intel compilation warnings and errors
       (`#3165 <https://github.com/dtcenter/MET/issues/3165>`_).
     * Fix compilation on non-GCC compilers by replacing :code:`#include <bits/stdc++.h>`
       (`#3175 <https://github.com/dtcenter/MET/issues/3175>`_).
     * Fix the parsing logic when setting the file_type config option for the forecast or observation but not the climatology
       (`#3179 <https://github.com/dtcenter/MET/issues/3179>`_).
     * Fix the AW_MEAN regridding method when OMP_NUM_THREADS is set very large
       (`#3206 <https://github.com/dtcenter/MET/issues/3206>`_).

  .. dropdown:: Enhancements

     * Enhance ASCII2NC to read USCRN point observations
       (`#1019 <https://github.com/dtcenter/MET/issues/1019>`_).
     * Enhance Multivariate MODE to support multiple convolution radii and thresholds
       (`#2709 <https://github.com/dtcenter/MET/issues/2709>`_).
     * Enhance MET library code to support reading WRF subgrid files
       (`#2794 <https://github.com/dtcenter/MET/issues/2794>`_).
     * Enhance MET library code to support additional vertical level types in WRF files
       (`#2818 <https://github.com/dtcenter/MET/issues/2818>`_).
     * Enhance multivariate MODE to support Python embedding inputs
       (`#2940 <https://github.com/dtcenter/MET/issues/2940>`_).
     * Enhance Gen-Vx-Mask to support a new local solar time masking region option
       (`#2966 <https://github.com/dtcenter/MET/issues/2966>`_).
     * **Create a new Pair-Stat tool to compute statistics for already paired forecast and observation data**
       (`#3006 <https://github.com/dtcenter/MET/issues/3006>`_).
     * Develop a class that supports reading both FCST and OBS from a IODA file to use with the new Pair-Stat tool
       (`#3007 <https://github.com/dtcenter/MET/issues/3007>`_).
     * Enhance the Grid-Stat GRAD line type with additional gradient vector-based statistics to measure sharpness
       (`#3024 <https://github.com/dtcenter/MET/issues/3024>`_).
     * Enhance Series-Analysis to compute statistics from the GRAD line type
       (`#3030 <https://github.com/dtcenter/MET/issues/3030>`_).
     * Improve MET library logging for NetCDF level dimension matches
       (`#3038 <https://github.com/dtcenter/MET/issues/3038>`_).
     * Enhance the unstructured grid library to correctly parse the LFRic model initialization time
       (`#3056 <https://github.com/dtcenter/MET/issues/3056>`_).
     * **Enhance Pair-Stat to support IODA and Python inputs and support temporal filtering**
       (`#3059 <https://github.com/dtcenter/MET/issues/3059>`_).
     * **Enhance the MET statistics tools to read and process range/azimuth data from the TC-RMW and RMW-Analysis tools**
       (`#3064 <https://github.com/dtcenter/MET/issues/3064>`_).
     * **Add support for the CTRACK benchmarking tool and instrument the Ensemble-Stat tool to report metrics**
       (`#3065 <https://github.com/dtcenter/MET/issues/3065>`_).

       * Enhance the benchmarking logic to support multiple runs and instrument the Grid-Diag tool to report metrics
         (`#3109 <https://github.com/dtcenter/MET/issues/3109>`_).
       * Add the :code:`--enable-profiler` configuration option to make the compilation of CTRACK support optional
         (`#3125 <https://github.com/dtcenter/MET/issues/3125>`_).
 
     * Refine the vector level matching logic when reading NetCDF output generated by other MET tools
       (`#3087 <https://github.com/dtcenter/MET/issues/3087>`_).
     * **Enhance the MET tools to fully support writing gridded range/azimuth data to NetCDF output files**
       (`#3096 <https://github.com/dtcenter/MET/issues/3096>`_).
     * Update references to the Gulf of America
       (`#3098 <https://github.com/dtcenter/MET/issues/3098>`_).
     * Enhance PB2NC to write unit and description strings for derived variables
       (`#3099 <https://github.com/dtcenter/MET/issues/3099>`_).
     * Resolve Python deprecation warnings introduced during the switch to Python 3.12
       (`#3106 <https://github.com/dtcenter/MET/issues/3106>`_).
     * **Enhance MET using OpenMP parallelization in both the library and application code**
       (`#3120 <https://github.com/dtcenter/MET/issues/3120>`_).

       * Apply OpenMP to the vx_regrid library and initialize OpenMP for all applications
         (`#3131 <https://github.com/dtcenter/MET/issues/3131>`_).
       * Apply OpenMP to the vx_util library
         (`#3132 <https://github.com/dtcenter/MET/issues/3132>`_).
       * Apply OpenMP to remaining MET library code
         (`#3134 <https://github.com/dtcenter/MET/issues/3134>`_).
       * Apply OpenMP to parallelize loops over grid dimensions in MET Applications
         (`#3140 <https://github.com/dtcenter/MET/issues/3140>`_).
       * Apply OpenMP to parallelize loops over grid dimensions in the remaining MET applications
         (`#3145 <https://github.com/dtcenter/MET/issues/3145>`_).

  .. dropdown:: Documentation

     * Update the MET User's Guide to specify the data types in each output line type table
       (`#3032 <https://github.com/dtcenter/MET/issues/3032>`_).
     * Enhance the MET User's Guide to better describe ASCII file lists
       (`#3066 <https://github.com/dtcenter/MET/issues/3066>`_).
     * Enhance the MET User's Guide by adding linkable sub-sections to the configuration file overview chapters
       (`#3149 <https://github.com/dtcenter/MET/issues/3149>`_).
     * Documentation: Use subprojects in Read The Docs
       (`METplus#2771 <https://github.com/dtcenter/METplus/issues/2771>`_ and
       `PR MET#3113 <https://github.com/dtcenter/MET/issues/3113>`_).

  .. dropdown:: Repository, build, and test

     * Create a new test utility to search a 3D cloud of points for the nearest neighbor to a specified (lat, lon, elev) location
       (`#3012 <https://github.com/dtcenter/MET/issues/3012>`_).
     * Create a new development environment file for Seneca for the Intel oneAPI compilers
       (`#3047 <https://github.com/dtcenter/MET/issues/3047>`_).
     * Decouple the MET unit tests from the MET installation location
       (`#3051 <https://github.com/dtcenter/MET/issues/3051>`_).
     * Update installation configuration and modulefiles for Python 3.12
       (`#3119 <https://github.com/dtcenter/MET/issues/3119>`_).
     * Add CVE scanning to the release-docker-images.yml workflow
       (`#3198 <https://github.com/dtcenter/MET/issues/3198>`_).

  .. dropdown:: METbaseimage testing environment

     * Update the METbaseimage to use Python 3.12.0
       (`METbaseimage#30 <https://github.com/dtcenter/METbaseimage/issues/30>`_,
       `PR MET#3091 <https://github.com/dtcenter/MET/pull/3091>`_, and
       `PR MET#3105 <https://github.com/dtcenter/MET/pull/3105>`_)
     * Restructure the METbaseimage repository to match the common METplus repository branching logic
       (`METbaseimage#32 <https://github.com/dtcenter/METbaseimage/issues/32>`_).
     * METbaseimage: Address Critical CVEs from Grype scans of the Docker images
       (`METplus-Internal#61 <https://github.com/dtcenter/METplus-Internal/issues/61>`_).
     * Enhance METbaseimage to push vX.Y-latest images to Docker Hub
       (`METbaseimage#37 <https://github.com/dtcenter/METbaseimage/issues/37>`_).

MET Version 12.0.3 Release Notes (20250721)
-------------------------------------------

  .. dropdown:: Bugfixes

     * **Address Critical CVEs from Grype scans of the Docker images**
       (`METplus-Internal#61 <https://github.com/dtcenter/METplus-Internal/issues/61>`_).
     * Fix compilation script logic for successful dependent library compilations
       (`#3092 <https://github.com/dtcenter/MET/issues/3092>`_).
     * Fix ASCII2NC failure for tab-delimited input files
       (`#3095 <https://github.com/dtcenter/MET/issues/3095>`_).
     * Refine the MET GRIB2 library logic to find a single GRIB2 table match rather than multiple ones 
       (`#3107 <https://github.com/dtcenter/MET/issues/3107>`_).
     * Fix the MET NetCDF library to support variables with more that 4 dimensions
       (`#3112 <https://github.com/dtcenter/MET/issues/3112>`_).

  .. dropdown:: Enhancements

     * Add CVE scanning to the release-docker-images.yml workflow
       (`#3198 <https://github.com/dtcenter/MET/issues/3198>`_).

MET Version 12.0.2 Release Notes (20250214)
-------------------------------------------

  .. dropdown:: Bugfixes

     * Fix memory management issues by replacing variable length arrays with STL vectors and arrays
       (`#3075 <https://github.com/dtcenter/MET/issues/3075>`_).
     * Fix intermittent configuration string parsing "yyerror" failure
       (`#3077 <https://github.com/dtcenter/MET/issues/3077>`_).

MET Version 12.0.1 Release Notes (20250131)
-------------------------------------------

  .. dropdown:: Bugfixes

     * Fix the PARUSR BUFRLIB failure when PB2NC is compiled with "-O2" optimization
       (`#3054 <https://github.com/dtcenter/MET/issues/3054>`_).

  .. dropdown:: Repository, build, and test

     * Decouple the MET unit tests from the MET installation location
       (`#3051 <https://github.com/dtcenter/MET/issues/3051>`_).

MET Version 12.0.0 Release Notes (20241218)
-------------------------------------------

  .. dropdown:: Bugfixes

     * Fix the Grid-Stat configuration file to support the MET_SEEPS_GRID_CLIMO_NAME option
       (`#2601 <https://github.com/dtcenter/MET/issues/2601>`_).
     * Refine support for coordinate dimensions in CF-compliant NetCDF files
       (`#2638 <https://github.com/dtcenter/MET/issues/2638>`_).
     * Fix logic for computing the 100-th percentile
       (`#2644 <https://github.com/dtcenter/MET/issues/2644>`_).
     * Fix support for NSIDC v4 Climate Data Record data on Polar Stereographic grids in CF-compliant NetCDF files
       (`#2652 <https://github.com/dtcenter/MET/issues/2652>`_).
     * Correct the usage statement for Point2Grid
       (`#2666 <https://github.com/dtcenter/MET/issues/2666>`_).
     * Investigate unexpected number of derived HPBL observations in PB2NC
       (`#2687 <https://github.com/dtcenter/MET/issues/2687>`_).
     * Fix the wind-based diagnostics computations in TC-Diag
       (`#2729 <https://github.com/dtcenter/MET/issues/2729>`_).
     * Fix the Point-Stat CNT header line typo causing duplicate "SI_BCL" column names
       (`#2730 <https://github.com/dtcenter/MET/issues/2730>`_).
     * Fix Python embedding failure when providing a single point observation'
       (`#2755 <https://github.com/dtcenter/MET/issues/2755>`_).
     * Fix MET to compile without the optional `--enable-python` configuration option
       (`#2760 <https://github.com/dtcenter/MET/issues/2760>`_).
     * Fix the parsing of level values for GRIB2 template 4.48 data
       (`#2782 <https://github.com/dtcenter/MET/issues/2782>`_).
     * **Fix the TC-Diag and TC-RMW tools to correctly handle the range and azimuth settings in range/azimuth grids**
       (`#2833 <https://github.com/dtcenter/MET/issues/2833>`_).
     * **Fix TC-RMW to correct the tangential and radial wind computations**
       (`#2841 <https://github.com/dtcenter/MET/issues/2841>`_).
     * Fix Ensemble-Stat's handling of climo data when verifying ensemble-derived probabilities
       (`#2856 <https://github.com/dtcenter/MET/issues/2856>`_).
     * **Fix Point2Grid's handling of the -qc option for ADP input files**
       (`#2867 <https://github.com/dtcenter/MET/issues/2867>`_).
     * Fix Stat-Analysis errors for jobs using the -dump_row option and the -line_type option with VCNT, RPS, DMAP, or SSIDX
       (`#2888 <https://github.com/dtcenter/MET/issues/2888>`_).
     * Fix inconsistent handling of point observation valid times processed through Python embedding
       (`#2897 <https://github.com/dtcenter/MET/issues/2897>`_).
     * Fix Point2Grid processing of GFS Ocean data input
       (`#2936 <https://github.com/dtcenter/MET/issues/2936>`_).
     * **Fix contingency table statistic bugs in the CTS and NBRCTS line types for BAGSS, SEDI CI's, ORSS, and ORSS CI's**
       (`#2958 <https://github.com/dtcenter/MET/issues/2958>`_).
     * Fix the grid dimensions used for `point2grid_cice_set_attr_grid` unit test
       (`#2968 <https://github.com/dtcenter/MET/issues/2968>`_).
     * Fix MTD to run on any MET-supported grid projection
       (`#2979 <https://github.com/dtcenter/MET/issues/2979>`_).
     * Fix Grid-Stat segfault when SEEPS is the only NetCDF output type requested
       (`#3020 <https://github.com/dtcenter/MET/issues/3020>`_).

  .. dropdown:: Enhancements

     * **Enhance Series-Analysis to read its own output and incrementally update output statistics over time**
       (`#1371 <https://github.com/dtcenter/MET/issues/1371>`_).
     * Enhance the `set_attr_grid` processing logic to support input files lacking a grid definition
       (`#1729 <https://github.com/dtcenter/MET/issues/1729>`_).
     * **Add support for NetCDF files following the UGRID convention**
       (`#2231 <https://github.com/dtcenter/MET/issues/2231>`_).
     * **Add support for new point_weight_flag to the Point-Stat and Ensemble-Stat tools**
       (`#2279 <https://github.com/dtcenter/MET/issues/2279>`_).
     * **Refine configuration options for defining bins in the verification of probabilistic forecasts**
       (`#2280 <https://github.com/dtcenter/MET/issues/2280>`_).
     * Allow observation anomaly replacement in Anomaly Correlation Coefficient (ACC) calculation
       (`#2308 <https://github.com/dtcenter/MET/issues/2308>`_).
     * Enhance TC-Pairs to include storm diagnostics in consensus track output
       (`#2476 <https://github.com/dtcenter/MET/issues/2476>`_).
     * **Add support for native WRF output files already on pressure levels**
       (`#2547 <https://github.com/dtcenter/MET/issues/2547>`_).
     * **Enhance TC-Diag to actually compute and write diagnostics**
       (`#2550 <https://github.com/dtcenter/MET/issues/2550>`_).
     * Refine TC-Diag logic for handling missing data
       (`#2609 <https://github.com/dtcenter/MET/issues/2609>`_).
     * **Update ioda2nc to support version 3 IODA files**
       (`#2640 <https://github.com/dtcenter/MET/issues/2640>`_).
     * **Enhance MODE CTS output file to include missing categorical statistics, including SEDI**
       (`#2648 <https://github.com/dtcenter/MET/issues/2648>`_).
     * **Enhance MET to compile and link against the Proj library**
       (`#2669 <https://github.com/dtcenter/MET/issues/2669>`_).
     * Fix the SonarQube findings for MET v12.0
       (`#2673 <https://github.com/dtcenter/MET/issues/2673>`_).
     * Change the default setting for the model string from "WRF" to "FCST" in the default MET configuration files
       (`#2682 <https://github.com/dtcenter/MET/issues/2682>`_).
     * Document the use of temporary files in MET and reduce it as much as reasonably possible
       (`#2690 <https://github.com/dtcenter/MET/issues/2690>`_).
     * **Eliminate the use of temporary files in the vx_config library**
       (`#2691 <https://github.com/dtcenter/MET/issues/2691>`_).
     * Refine TC-Pairs consensus diagnostics configuration options
       (`#2699 <https://github.com/dtcenter/MET/issues/2699>`_).
     * Enhance ASCII2NC to read ISMN point observations of soil moisture and temperature
       (`#2701 <https://github.com/dtcenter/MET/issues/2701>`_).
     * **Enhance Multivariate MODE to support differing numbers of forecast and observation input fields**
       (`#2706 <https://github.com/dtcenter/MET/issues/2706>`_).
     * Enhance Multivariate MODE to change the default "merge_flag" setting to NONE
       (`#2708 <https://github.com/dtcenter/MET/issues/2708>`_).
     * Documentation: Make Headers Consistent in All MET Guides
       (`#2716 <https://github.com/dtcenter/MET/issues/2716>`_).
     * **Enhance MODE to use OpenMP to make the convolution step faster**
       (`#2724 <https://github.com/dtcenter/MET/issues/2724>`_).
     * **Major enhancements to multivariate MODE**
       (`#2745 <https://github.com/dtcenter/MET/issues/2745>`_).
     * Enhance TC-Diag to use tc_diag_driver version 0.11.0
       (`#2769 <https://github.com/dtcenter/MET/issues/2769>`_).
     * Switch from writing temporary Python files in NetCDF to JSON and NumPy serialization
       (`#2772 <https://github.com/dtcenter/MET/issues/2772>`_).
     * Revise the use of temporary files in PB2NC
       (`#2792 <https://github.com/dtcenter/MET/issues/2792>`_).
     * Enhance MET to make warnings messages about time differences configurable
       (`#2801 <https://github.com/dtcenter/MET/issues/2801>`_).
     * Enhance Stat-Analysis to apply the `-set_hdr` option to filter jobs
       (`#2805 <https://github.com/dtcenter/MET/issues/2805>`_).
     * Enhance MET to parse LAEA grids from the MET NetCDF file format
       (`#2809 <https://github.com/dtcenter/MET/issues/2809>`_).
     * **Add new wind direction verification statistics for RMSE, Bias, and MAE**
       (`#2395 <https://github.com/dtcenter/MET/issues/2395>`_).
     * Add new ECNT statistics that incorporate observational uncertainty as advised in Ferro (2017)
       (`#2583 <https://github.com/dtcenter/MET/issues/2583>`_).
     * Update ndbc_stations.xml after 7-character buoy ids are introduced in Aug/Sept 2023
       (`#2631 <https://github.com/dtcenter/MET/issues/2631>`_).
     * Enhance ASCII2NC to support IABP/IPAB Arctic and Antarctic drifting buoy observations
       (`#2654 <https://github.com/dtcenter/MET/issues/2654>`_).
     * Enhance Multivariate MODE to read input data only once rather than multiple times
       (`#2707 <https://github.com/dtcenter/MET/issues/2707>`_).
     * Enhance the calculation of RPSS to support starting from probabilistic data
       (`#2786 <https://github.com/dtcenter/MET/issues/2786>`_).
     * Add convex hull to MODE output (`#2819 <https://github.com/dtcenter/MET/issues/2819>`_).
     * **Add new wind direction verification statistics for RMSE, Bias, and MAE**
       (`#2395 <https://github.com/dtcenter/MET/issues/2395>`_).
     * Document UGRID configuration options added to Point-Stat and Grid-Stat
       (`#2748 <https://github.com/dtcenter/MET/issues/2748>`_
     * Update GRIB tables in MET based on wgrib2 versions 3.1.4 and 3.4.0
       (`#2780 <https://github.com/dtcenter/MET/issues/2780>`_).
     * Refine Point-Stat Warning message about fcst/obs level mismatch
       (`#2795 <https://github.com/dtcenter/MET/issues/2795>`_).
     * Enhance MET to parse the set_attr options prior reading data from gridded data files
       (`#2826 <https://github.com/dtcenter/MET/issues/2826>`_).
     * **Add new -ugrid_config command line option for unstructured grid inputs to Grid-Stat and Point-Stat**
       (`#2842 <https://github.com/dtcenter/MET/issues/2842>`_).
     * Enhance Point2Grid to support modified quality control settings for smoke/dust AOD data in GOES-16/17 as of April 16, 2024
       (`#2853 <https://github.com/dtcenter/MET/issues/2853>`_).
     * **Enhance Point2Grid to support a wider variety of input tripolar datasets**
       (`#2857 <https://github.com/dtcenter/MET/issues/2857>`_).
     * Test NOAA Unstructured grids in MET-12.0.0
       (`#2860 <https://github.com/dtcenter/MET/issues/2860>`_).
     * Enhance Ensemble-Stat and Gen-Ens-Prod to omit warning messages for the MISSING keyword
       (`#2870 <https://github.com/dtcenter/MET/issues/2870>`_).
     * Add new Python functionality to convert MET NetCDF observation data to a Pandas DataFrame
       (`#2781 <https://github.com/dtcenter/MET/issues/2781>`_).
     * Enhance Point2Grid to filter quality control strings with config file options
       (`#2880 <https://github.com/dtcenter/MET/issues/2880>`_).
     * Refine SEEPS processing logic and output naming conventions
       (`#2882 <https://github.com/dtcenter/MET/issues/2882>`_).
     * Enhance PCP-Combine to allow missing data
       (`#2883 <https://github.com/dtcenter/MET/issues/2883>`_).
     * **Enhance MET to calculate weighted contingency table counts and statistics**
       (`#2887 <https://github.com/dtcenter/MET/issues/2887>`_).
     * Enhance the OBTYPE header column for MPR and ORANK line types
       (`#2893 <https://github.com/dtcenter/MET/issues/2893>`_).
     * Enhance TC-Stat to support the -set_hdr job command option
       (`#2911 <https://github.com/dtcenter/MET/issues/2911>`_).
     * Refine ERROR messages written by PB2NC
       (`#2912 <https://github.com/dtcenter/MET/issues/2912>`_).
     * **Enhance MET to support separate climatology datasets for both the forecast and observation inputs**
       (`#2924 <https://github.com/dtcenter/MET/issues/2924>`_).
     * Refine PB2NC warning messages about changing Bufr center times
       (`#2938 <https://github.com/dtcenter/MET/issues/2938>`_).
     * Eliminate Point2Grid warning when no valid output data is found
       (`#3000 <https://github.com/dtcenter/MET/issues/3000>`_).

  .. dropdown:: Documentation

     * Remove the double-quotes around keywords
       (`#2023 <https://github.com/dtcenter/MET/issues/2023>`_).
     * Enhance MTD documentation so that tables 21.3 and 21.4 have units
       (`#2750 <https://github.com/dtcenter/MET/issues/2750>`_).
     * Documentation: Provide instructions for compiling MET with the C++11 standard
       (`#2949 <https://github.com/dtcenter/MET/issues/2949>`_).
     * Update documentation about parsing grid information from CF-compliant NetCDF files
       (`#3009 <https://github.com/dtcenter/MET/issues/3009>`_).
     * Update the MET User's Guide to specify the data types in each output line type table
       (`#3032 <https://github.com/dtcenter/MET/issues/3032>`_).

  .. dropdown:: Repository, build, and test

     * Add GitHub action to run SonarQube for MET pull requests and feature branches
       (`#2379 <https://github.com/dtcenter/MET/issues/2379>`_).
     * **Enhance MET to compile and link against the Atlas and ecKit libraries**
       (`#2574 <https://github.com/dtcenter/MET/issues/2574>`_).
     * **Enhance "compile_MET_all.sh" to support the new Intel oneAPI compilers and upgrade dependent library versions as needed**
       (`#2611 <https://github.com/dtcenter/MET/issues/2611>`_).
     * Update the ``install_met_env.generic`` configuration file
       (`#2643 <https://github.com/dtcenter/MET/issues/2643>`_).
     * Switch SonarQube server (mandan to needham)
       (`#2650 <https://github.com/dtcenter/MET/issues/2650>`_).
     * Update GitHub issue and pull request templates to reflect the current development workflow details
       (`#2659 <https://github.com/dtcenter/MET/issues/2659>`_).
     * Update the unit test diff logic to handle SEEPS, SEEPS_MPR, and MODE CTS line type updates
       (`#2665 <https://github.com/dtcenter/MET/issues/2665>`_).
     * **Enhance MET to compile and link against the Proj library**
       (`#2669 <https://github.com/dtcenter/MET/issues/2669>`_).
     * Fix the SonarQube findings for MET version 12.0.0
       (`#2673 <https://github.com/dtcenter/MET/issues/2673>`_).
     * Upgrade SonarQube server version from 9.8 to 10.2
       (`#2689 <https://github.com/dtcenter/MET/issues/2689>`_).
     * Move namespace specifications below include directives
       (`#2696 <https://github.com/dtcenter/MET/issues/2696>`_).
     * Update the token for upgraded SonarQube server
       (`#2702 <https://github.com/dtcenter/MET/issues/2702>`_).
     * Reimplement and enhance the Perl-based (unit.pl) unit test control script in Python
       (`#2717 <https://github.com/dtcenter/MET/issues/2717>`_).
     * Update compilation script and configuration files as needed for supported platforms
       (`#2753 <https://github.com/dtcenter/MET/issues/2753>`_).
     * Remove the SonarQube token from the properties file
       (`#2757 <https://github.com/dtcenter/MET/issues/2757>`_).
     * Repository cleanup of stale code and configuration consistency
       (`#2776 <https://github.com/dtcenter/MET/issues/2776>`_).
     * Add new example installation configuration files for Intel compiler users
       (`#2785 <https://github.com/dtcenter/MET/issues/2785>`_).
     * Update GitHub actions workflows to switch from node 16 to node 20
       (`#2796 <https://github.com/dtcenter/MET/issues/2796>`_).
     * Enhance GitHub action compilation options and testing workflows
       (`#2815 <https://github.com/dtcenter/MET/issues/2815>`_).
     * SonarQube: Replace "enum" to "enum class"
       (`#2830 <https://github.com/dtcenter/MET/issues/2830>`_).
     * Update tag used for the release checksum action
       (`#2929 <https://github.com/dtcenter/MET/issues/2929>`_).
     * Enhance the ``unit.py`` MET testing script to allow for expected failures
       (`#2937 <https://github.com/dtcenter/MET/issues/2937>`_).
     * Modify configure.ac to define C++17 as the default compilation standard
       (`#2948 <https://github.com/dtcenter/MET/issues/2948>`_).

  .. dropdown:: METplus-Internal issues

     METplus-Internal is a private repository where internal issues related to
     security are tracked.

     * MET: Enhance the MET testing framework to provide a mechanism for expected failure
       (`METplus-Internal#23 <https://github.com/dtcenter/METplus-Internal/issues/23>`_).

  .. dropdown:: METbaseimage testing environment

     METbaseimage is a public repository that provides the Docker environment
     in which automated testing of MET is performed.

     * Refine METbaseimage to compile dependent libraries from a single tar file
       (`METbaseimage#9 <https://github.com/dtcenter/METbaseimage/issues/9>`_).
     * Update METbaseimage to complete the transition to the Debian 12 (bookworm) base image
       (`METbaseimage#12 <https://github.com/dtcenter/METbaseimage/issues/12>`_).
     * Enhance METbaseimage to compile the ecKit and Atlas libraries
       (`METbaseimage#13 <https://github.com/dtcenter/METbaseimage/issues/13>`_).
     * Enhance METbaseimage to install the YAML Python package
       (`METbaseimage#15 <https://github.com/dtcenter/METbaseimage/issues/15>`_).
     * Enhance METbaseimage to install SciPy Python package needed by the MET TC-Diag tool
       (`METbaseimage#20 <https://github.com/dtcenter/METbaseimage/issues/20>`_).
     * Fix METbaseimage environment to correct the ncdump runtime linker error
       (`METbaseimage#24 <https://github.com/dtcenter/METbaseimage/issues/24>`_).
     * Update METbaseimage to use newer versions of Atlas and ecKit
       (`METbaseimage#27 <https://github.com/dtcenter/METbaseimage/issues/27>`_).

MET Upgrade Instructions
========================

This section summarizes and highlights important changes to MET since version 12.0.0, including:

  - Adding new or modifying existing **configuration file options**.
  - Modifying existing **output file formats**, such as new or modified ASCII or NetCDF file
    formats.
  - Changing existing **output data values** generated by MET, typically due to fixing bugs that
    were computing incorrect output.
  - Any other relevant details needed to upgrade from the MET version 12.0.0 to 12.1.0.

MET Version 12.1.0 Upgrade Instructions
---------------------------------------

.. dropdown:: Configuration file changes

   MET version 12.1.0 adds, modifies, or removes the following configuration options:

   * Pair-Stat configuration:

     * Adds new **Pair-Stat** tool with a new configuration file.

   * MODE configuration:

     * Supports **multiple convolution radii and thresholds** for Multivariate MODE,
       but does not require changes to existing MODE configuration files.

   * Series-Analysis configuration:

     * Adds a new **output_stats.grad** entry to enable gradient statistic computation.
     * Adds a new **gradient** dictionary to configure gradient statistic computation.

.. dropdown:: Output format changes

   MET version 12.1.0 adds or modifies the following output file formats:

   * Gradient (**GRAD**) line type:

     * Adds 4 new columns to the end of the GRAD line type:

       * FGMAG, OGMAG, MAG_RMSE, LAPLACE_RMSE

     * Grid-Stat writes the updated GRAD line type output.
     * Series-Analysis adds gradient statistic variables to its NetCDF output, if requested.
 
   * TC-RMW and RMW-Analysis add standard global attributes for FileOrigins, MET_version, and MET_tool.
   * RMW-Analysis adds TrackLat_mean and TrackLon_mean variables to report the average track location.

.. dropdown:: Output data changes

   MET version 12.1.0 modifies existing output data values in the following ways:

   * NetCDF library code:

     * Improves handling of leap years and corrects time values parsed from NetCDF files that
       define times using a **monthly** offset.

   * IODA2NC tool:

     * Corrects time values parsed from input files that store time using the **int64** data type
       for which variable overflow was occurring.

.. dropdown:: Additional upgrade instructions

   Recommendations when upgrading to MET version 12.1.0:

   * Users are *strongly encouraged* to **set the OMP_NUM_THREADS environment variable**, as described in
     :numref:`omp_num_threads`, to take advantage of the dramatic increase in OpenMP use, from 2 
     parallelized loops in version 12.0.0 to 151 in 12.1.0. The parallelization primarily targets more
     efficient looping over grid dimensions. Users should expect improvements in runtimes, particularly
     when processing dense grids with a large number of threads. If not set, look for log messages with
     advice on how to set it.

   * The **range/azimuth grids** created by the TC-RMW, RMW-Analysis, and TC-Diag tools and described in
     :numref:`range/azimuth_grid` are now fully supported in MET. For example, Point-Stat, Grid-Stat
     and Series-Analysis can read range/azimuth inputs, define matched pairs, and compute statistics.
     Verifying gridded analysis on regular grids can be automatically regridded on the fly to match the
     range/azimuth grid of the model data, or vice-versa.

   * The addition of the **CTRACK** benchmarking tool is intended as a developer utility to help identify
     bottlenecks and track runtime efficiency improvements. It is disabled by default at compilation time
     and users, in general, should NOT use the :code:`--enable-profiler` configuration option to enable it.

