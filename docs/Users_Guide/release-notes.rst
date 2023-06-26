***********************
MET Release Information
***********************

MET Release Notes
=================

When applicable, release notes are followed by the GitHub issue number which describes the bugfix,
enhancement, or new feature (`MET GitHub issues <https://github.com/dtcenter/MET/issues>`_).
Important issues are listed **in bold** for emphasis.

MET Version 11.1.0-rc1 release notes (20230616)
-----------------------------------------------

  .. dropdown:: Repository, build, and test

     * Enhance compile_MET_all.sh to allow the user to specify the compiler and Python versions to load (`#2485 <https://github.com/dtcenter/MET/issues/2485>`_).
     * Enhance compile_MET_all.sh to include the library/app name in the log filename (`#2540 <https://github.com/dtcenter/MET/issues/2540>`_).

  .. dropdown:: Documentation

     * Update the documentation for the HSS and HSS_EC statistics (`#2492 <https://github.com/dtcenter/MET/issues/2492>`_).
     * Update the documentation for the ens, NEP, and NMEP configuration options (`#2513 <https://github.com/dtcenter/MET/issues/2513>`_).

  .. dropdown:: Enhancements

    .. dropdown:: Common Libraries

       * Add support for the Lambert Azimuthal Equal Area grids (`#1693 <https://github.com/dtcenter/MET/issues/1693>`_).
       * Update the default ASCII2NC message_type_map configuration option for Little R input data (`#2487 <https://github.com/dtcenter/MET/issues/2487>`_).
       * Merge older and newer MRMS GRIB2 table files (`#2508 <https://github.com/dtcenter/MET/issues/2508>`_).
       * SonarQube: Further reduce findings for MET-11.1.0-rc1 (`#2521 <https://github.com/dtcenter/MET/issues/2521>`_).

    .. dropdown:: Application Code

       * **Enhance Gen-Vx-Mask shapefile masking to support multiple shapes and specify shape metadata** (`#1060 <https://github.com/dtcenter/MET/issues/1060>`_).
       * **Enhance Multivariate MODE to generate object statistics for each input field requested by the user** (`#1283 <https://github.com/dtcenter/MET/issues/1283>`_).
       * Enhance MODE to bound check interest_function corner points in the range 0 to 1 (`#2545 <https://github.com/dtcenter/MET/issues/2545>`_).

    .. dropdown:: Tropical Cyclone Tools

       * **Create an initial development version of a new TC-Diag tool to support the computation of tropical cyclone diagnostics** (`#2168 <https://github.com/dtcenter/MET/issues/2168>`_).
       * **Enhance TC-Stat to write the RIRW job CTC/CTS output to a .stat output file** (`#2425 <https://github.com/dtcenter/MET/issues/2425>`_).
       * **Enhance TC-Pairs to derive the full circle wind radius from the wind radius quadrants** (`#2532 <https://github.com/dtcenter/MET/issues/2532>`_).
       * Enhance TC-RMW to reorder the dimensions of the NetCDF output to store the gridded dimensions last (`#2523 <https://github.com/dtcenter/MET/issues/2523>`_).
       * Enhance TC-Gen to parse GTWO shapefile lead times from the data rather than assuming them to be 2, 5, and 7 days (`#2552 <https://github.com/dtcenter/MET/issues/2552>`_).

  .. dropdown:: Bugfixes

     * Bugfix: Fix the Clang compilation of MET version 11 (`#2514 <https://github.com/dtcenter/MET/issues/2514>`_).
     * Bugfix: Fix the fill value setting used in the write_tmp_dataplane internal Python embedding script (`#2525 <https://github.com/dtcenter/MET/issues/2525>`_).
     * Bugfix: Fix the TC-Stat RIRW runtime error when computing CTS statistics from an empty contingency table (`#2542 <https://github.com/dtcenter/MET/issues/2542>`_).
     * Bugfix: Fix logic for Python embedding with data censoring and/or conversion (`#2575 <https://github.com/dtcenter/MET/issues/2575>`_).

MET Version 11.1.0-beta2 release notes (20230505)
-------------------------------------------------

**Note** that the 11.1.0-beta2 release was originally created on 20230423 but was recreated on 20230428 and 20230505 to include critical bugfixes.

  .. dropdown:: Documentation

     * Improve documentation on Python Embedding for point observations (`#2303 <https://github.com/dtcenter/MET/issues/2303>`_).
     * Create dropdown menus for Appendix A (`#2460 <https://github.com/dtcenter/MET/issues/2460>`_).
     * Clarify MET Compile Time Python requirements (`#2490 <https://github.com/dtcenter/MET/issues/2490>`_).

  .. dropdown:: Enhancements

     * Enhance the MET point processing tools to read the Python 'point_data' variable instead of just 'met_point_data' (`#2285 <https://github.com/dtcenter/MET/issues/2285>`_).
     * SonarQube: Further reduce bugs for MET-11.1.0-beta2 (`#2474 <https://github.com/dtcenter/MET/issues/2474>`_).
     * SonarQube: Replace all instances of NULL with nullptr (`#2504 <https://github.com/dtcenter/MET/issues/2504>`_).
     * SonarQube: Remove code that will never be executed (`#2506 <https://github.com/dtcenter/MET/issues/2506>`_).

  .. dropdown:: Bugfixes

     * Bugfix: Correct the branch name for the SonarQube scanning nightly (`#2401 <https://github.com/dtcenter/MET/issues/2401>`_).
     * Bugfix: Fix support for the YYYYMMDD format in NetCDF level timestrings (`#2482 <https://github.com/dtcenter/MET/issues/2482>`_).
     * Bugfix: AERONET the lat/lon is not changed with different station ID (`#2493 <https://github.com/dtcenter/MET/issues/2493>`_).
     * Bugfix: dtype in Python embedding example script and appendixF correction (`#2518 <https://github.com/dtcenter/MET/issues/2518>`_).
     * Bugfix: write_tmp_dataplane uses fill_value unrecognized by MET (`#2525 <https://github.com/dtcenter/MET/issues/2525>`_).
     * **Bugfix: Resolve compilation problems due to need for \-std=c++11** (`#2531 <https://github.com/dtcenter/MET/issues/2531>`_).

MET Version 11.1.0-beta1 release notes (20230228)
-------------------------------------------------

  .. dropdown:: Repository, build, and test

     * Add modulefiles for supported systems to the repository (`#2415 <https://github.com/dtcenter/MET/issues/2415>`_).
     * Add LICENSE.md to the repository (`#2461 <https://github.com/dtcenter/MET/issues/2461>`_).
     * Update the copyright year to 2023 and increase the version number to 11.1.0 (`#2469 <https://github.com/dtcenter/MET/issues/2469>`_).

  .. dropdown:: Documentation 

     * Enhance the Release Notes by adding dropdown menus (`#2146 <https://github.com/dtcenter/MET/issues/2146>`_).

  .. dropdown:: Enhancements 

     * Convert the python list to the numpy array for the python embedding at the base class (`#2386 <https://github.com/dtcenter/MET/issues/2386>`_).
     * Refine Python runtime environment (`#2388 <https://github.com/dtcenter/MET/issues/2388>`_).
     * Upgrade to using Python 3.10.4 (`#2421 <https://github.com/dtcenter/MET/issues/2421>`_).
     * **Enhance TC-Pairs to disable the output of consensus track members** (`#2429 <https://github.com/dtcenter/MET/issues/2429>`_).

  .. dropdown:: Bugfixes 

     * Bugfix: Fix the MET CF-Compliant NetCDF library code to Polar Stereographic data from NSIDC Sea Ice Edge NetCDF files (`#2218 <https://github.com/dtcenter/MET/issues/2218>`_).
     * Bugfix: Remove override keyword to avoid C++11 dependency (`#2380 <https://github.com/dtcenter/MET/issues/2380>`_).
     * Bugfix: Fix ASCII2NC to not compute AOD 550 if other inputs are negative values (`#2383 <https://github.com/dtcenter/MET/issues/2383>`_).
     * Bugfix: Fix PB2NC to report accurate total observation counts in log messages (`#2387 <https://github.com/dtcenter/MET/issues/2387>`_).
     * Bugfix: Update the MET flowchart for version 11.0.0 (`#2389 <https://github.com/dtcenter/MET/issues/2389>`_).
     * Bugfix: Fix issues with the met_compile_all.sh script and associated tar files (`#2390 <https://github.com/dtcenter/MET/issues/2390>`_).
     * Bugfix: Correct definitions of NCEP grid numbers 172 and 220 (`#2399 <https://github.com/dtcenter/MET/issues/2399>`_).
     * Bugfix: Address MET-11.0.0 SonarQube Blocker Bugs (`#2402 <https://github.com/dtcenter/MET/issues/2402>`_).
     * Bugfix: Refine fix for handling empty configuration files (`#2408 <https://github.com/dtcenter/MET/issues/2408>`_).
     * Bugfix: Fix time interpolation of monthly climatology data between December 15 and January 15 (`#2412 <https://github.com/dtcenter/MET/issues/2412>`_).
     * Bugfix: Fix ASCII2NC to handle missing NDBC buoy location information (`#2426 <https://github.com/dtcenter/MET/issues/2426>`_).
     * Bugfix: Fix the MET vx_pointdata_python library to handle MET_PYTHON_EXE for python embedding of point observations (`#2428 <https://github.com/dtcenter/MET/issues/2428>`_).
     * Bugfix: Refine the regrid dictionary's data conversion and censoring operations and fix climo time matching logic for a single monthly climo file (`#2437 <https://github.com/dtcenter/MET/issues/2437>`_).
     * Bugfix: Fix the creation of the MET User's Guide PDF (`#2449 <https://github.com/dtcenter/MET/issues/2449>`_).
     * Bugfix: Fix inconsistent ASCII2NC AIRNOW location lookup logic (`#2452 <https://github.com/dtcenter/MET/issues/2452>`_).

MET Version 11.0.0 release notes (20221209)
-------------------------------------------

  .. dropdown:: Repository, build, and test

     * **Restructure the contents of the MET repository so that it matches the existing release tarfiles** (`#1920 <https://github.com/dtcenter/MET/issues/1920>`_).
     * **Add initial files to create the MET compilation environment in the dtcenter/met-base Docker image** (`dtcenter/METbaseimage#1 <https://github.com/dtcenter/METbaseimage/issues/1>`_).
     * Restructure the MET Dockerfiles to create images based on the new METbaseimage (`#2196 <https://github.com/dtcenter/MET/issues/2196>`_).
     * Enhance METbaseimage to support NetCDF files using groups in the enhanced data model (`dtcenter/METbaseimage#6 <https://github.com/dtcenter/METbaseimage/issues/6>`_).
     * Add .zenodo.json file to add metadata about releases (`#2198 <https://github.com/dtcenter/MET/issues/2198>`_).
     * Update the SonarQube version used for routine software scans (`#2270 <https://github.com/dtcenter/MET/issues/2270>`_).
     * Fix OpenMP compilation error for GCC 9.3.0/9.4.0 (`#2106 <https://github.com/dtcenter/MET/issues/2106>`_).
     * Fix oom() compile time linker error (`#2238 <https://github.com/dtcenter/MET/issues/2238>`_).
     * Fix MET-11.0.0-beta3 linker errors (`#2281 <https://github.com/dtcenter/MET/issues/2281>`_).
     * Fix GHA documentation workflow (`#2282 <https://github.com/dtcenter/MET/issues/2282>`_).
     * Fix GHA warnings and update the version of actions (i.e. actions/checkout@v3) (`#2297 <https://github.com/dtcenter/MET/issues/2297>`_).

  .. dropdown:: Documentation

     * Create outline for the MET Contributor's Guide (`#1774 <https://github.com/dtcenter/MET/issues/1774>`_).
     * Document PB2NC's handling of quality markers (`#2278 <https://github.com/dtcenter/MET/issues/2278>`_).
     * Move release notes into its own chapter in the User's Guide (`#2298 <https://github.com/dtcenter/MET/issues/2298>`_).

  .. dropdown:: Bugfixes

     * Fix regression test differences in pb2nc and ioda2nc output (`#2102 <https://github.com/dtcenter/MET/issues/2102>`_).
     * Fix support for reading rotated lat/lon grids from CF-compliant NetCDF files (`#2115 <https://github.com/dtcenter/MET/issues/2115>`_).
     * Fix support for reading rotated lat/lon grids from GRIB1 files (grid type 10) (`#2118 <https://github.com/dtcenter/MET/issues/2118>`_).
     * Fix support for int64 NetCDF variable types (`#2123 <https://github.com/dtcenter/MET/issues/2123>`_).
     * Fix Stat-Analysis to aggregate the ECNT ME and RMSE values correctly (`#2170 <https://github.com/dtcenter/MET/issues/2170>`_).
     * Fix NetCDF library code to process scale_factor and add_offset attributes independently (`#2187 <https://github.com/dtcenter/MET/issues/2187>`_).
     * Fix Ensemble-Stat to work with different missing members for two or more variables (`#2208 <https://github.com/dtcenter/MET/issues/2208>`_).
     * Fix truncated station_id name in the output from IODA2NC (`#2216 <https://github.com/dtcenter/MET/issues/2216>`_).
     * Fix Stat-Analysis aggregation of the neighborhood statistics line types (`#2271 <https://github.com/dtcenter/MET/issues/2271>`_).
     * Fix Point-Stat and Ensemble-Stat GRIB table lookup logic for python embedding of point observations (`#2286 <https://github.com/dtcenter/MET/issues/2286>`_).
     * Fix ascii2nc_airnow_hourly test in unit_ascii2nc.xml (`#2306 <https://github.com/dtcenter/MET/issues/2306>`_).
     * Fix TC-Stat parsing of TCMPR lines (`#2309 <https://github.com/dtcenter/MET/issues/2309>`_).
     * Fix ASCII2NC logic for reading AERONET v3 data (`#2370 <https://github.com/dtcenter/MET/issues/2370>`_).

  .. dropdown:: Enhancements

    .. dropdown:: NetCDF

      * **Enhance MET's NetCDF library interface to support level strings that include coordinate variable values instead of just indexes** (`#1815 <https://github.com/dtcenter/MET/issues/1815>`_).
      * Enhance MET to handle NC strings when processing CF-Compliant NetCDF files (`#2042 <https://github.com/dtcenter/MET/issues/2042>`_).
      * Enhance MET to handle CF-compliant time strings with an offset defined in months or years (`#2155 <https://github.com/dtcenter/MET/issues/2155>`_).
      * Refine NetCDF level string handling logic to always interpret @ strings as values (`#2225 <https://github.com/dtcenter/MET/issues/2225>`_).

    .. dropdown:: GRIB
		  
        * Add support for reading National Blend Model GRIB2 data (`#2055 <https://github.com/dtcenter/MET/issues/2055>`_).
        * Update the GRIB2 MRMS table in MET (`#2081 <https://github.com/dtcenter/MET/issues/2081>`_).

    .. dropdown::Python

        * Reimplement the pntnc2ascii.R utility Rscript in Python (`#2085 <https://github.com/dtcenter/MET/issues/2085>`_).
        * Add more error checking for python embedding of point observations (`#2202 <https://github.com/dtcenter/MET/issues/2202>`_).
        * **Add a Python helper script/function to transform point_data objects to met_point_data objects for Python Embedding** (`#2302 <https://github.com/dtcenter/MET/issues/2302>`_).

    .. dropdown:: METplus-Internal

        * MET: Replace fixed length character arrays with strings (`dtcenter/METplus-Internal#14 <https://github.com/dtcenter/METplus-Internal/issues/14>`_).
        * MET: Add a timestamp to the log output at the beginning and end of each MET tool run (`dtcenter/METplus-Internal#18 <https://github.com/dtcenter/METplus-Internal/issues/18>`_).
        * MET: Add the user ID and the command line being executed to the log output at beginning and end of each MET tool run (`dtcenter/METplus-Internal#19 <https://github.com/dtcenter/METplus-Internal/issues/19>`_).
        * MET: Enhance MET to have better signal handling for shutdown events (`dtcenter/METplus-Internal#21 <https://github.com/dtcenter/METplus-Internal/issues/21>`_).

    .. dropdown:: Common Libraries

        * **Define new grid class to store semi-structured grid information (e.g. lat or lon vs level or time)** (`#1954 <https://github.com/dtcenter/MET/issues/1954>`_).
        * Refine warning/error messages when parsing thresholds (`#2211 <https://github.com/dtcenter/MET/issues/2211>`_).
        * Remove namespace specification from header files (`#2227 <https://github.com/dtcenter/MET/issues/2227>`_).
        * Update MET version number to 11.0.0 (`#2132 <https://github.com/dtcenter/MET/issues/2132>`_).
        * Store unspecified accumulation interval as 0 rather than bad data (`#2250 <https://github.com/dtcenter/MET/issues/2250>`_).
        * Add sanity check to error out when both is_u_wind and is_v_wind are set to true (`#2357 <https://github.com/dtcenter/MET/issues/2357>`_).

    .. dropdown:: Statistics

        * **Add Anomaly Correlation Coefficient to VCNT Line Type** (`#2022 <https://github.com/dtcenter/MET/issues/2022>`_).
        * **Allow 2x2 HSS calculations to include user-defined EC values** (`#2147 <https://github.com/dtcenter/MET/issues/2147>`_).
        * **Add the fair CRPS statistic to the ECNT line type in a new CRPS_EMP_FAIR column** (`#2206 <https://github.com/dtcenter/MET/issues/2206>`_).
        * **Add MAE to the ECNT line type from Ensemble-Stat and for HiRA** (`#2325 <https://github.com/dtcenter/MET/issues/2325>`_).
        * **Add the Mean Absolute Difference (SPREAD_MD) to the ECNT line type** (`#2332 <https://github.com/dtcenter/MET/issues/2332>`_).
        * **Add new bias ratio statistic to the ECNT line type from Ensemble-Stat and for HiRA** (`#2058 <https://github.com/dtcenter/MET/issues/2058>`_).

    .. dropdown:: Configuration and masking

        * Define the Bukovsky masking regions for use in MET (`#1940 <https://github.com/dtcenter/MET/issues/1940>`_).
        * **Enhance Gen-Vx-Mask by adding a new poly_xy masking type option** (`#2152 <https://github.com/dtcenter/MET/issues/2152>`_).
        * Add M_to_KFT and KM_to_KFT functions to ConfigConstants (`#2180 <https://github.com/dtcenter/MET/issues/2180>`_).
        * Update map data with more recent NaturalEarth definitions (`#2207 <https://github.com/dtcenter/MET/issues/2207>`_).

    .. dropdown:: Point Pre-Processing Tools

        * **Enhance IODA2NC to support IODA v2.0 format** (`#2068 <https://github.com/dtcenter/MET/issues/2068>`_).
        * **Add support for EPA AirNow ASCII data in ASCII2NC** (`#2142 <https://github.com/dtcenter/MET/issues/2142>`_).
        * Add a sum option to the time summaries computed by the point pre-processing tools (`#2204 <https://github.com/dtcenter/MET/issues/2204>`_).
        * Add "station_ob" to metadata_map as a message_type metadata variable for ioda2nc (`#2215 <https://github.com/dtcenter/MET/issues/2215>`_).
        * **Enhance ASCII2NC to read NDBC buoy data** (`#2276 <https://github.com/dtcenter/MET/issues/2276>`_).
        * Print ASCII2NC warning message about python embedding support not being compiled (`#2277 <https://github.com/dtcenter/MET/issues/2277>`_).

    .. dropdown:: Point-Stat, Grid-Stat, Stat-Analysis

        * Add support for point-based climatologies for use in SEEPS (`#1941 <https://github.com/dtcenter/MET/issues/1941>`_).
        * **Enhance Point-Stat to compute SEEPS for point observations and write new SEEPS and SEEPS_MPR STAT line types** (`#1942 <https://github.com/dtcenter/MET/issues/1942>`_).
        * **Enhance Grid-Stat to compute SEEPS for gridded observations and write the SEEPS STAT line type** (`#1943 <https://github.com/dtcenter/MET/issues/1943>`_).
        * Sort mask.sid station lists to check their contents more efficiently (`#1950 <https://github.com/dtcenter/MET/issues/1950>`_).
        * **Enhance Stat-Analysis to aggregate SEEPS_MPR and SEEPS line types** (`#2339 <https://github.com/dtcenter/MET/issues/2339>`_).
        * Relax Point-Stat and Ensemble-Stat logic for the configuration of message_type_group_map (`#2362 <https://github.com/dtcenter/MET/issues/2362>`_).
        * Fix Point-Stat and Grid-Stat logic for processing U/V winds with python embedding (`#2366 <https://github.com/dtcenter/MET/issues/2366>`_).

    .. dropdown:: Ensemble Tools

        * **Remove ensemble post-processing from the Ensemble-Stat tool** (`#1908 <https://github.com/dtcenter/MET/issues/1908>`_).
        * Eliminate Gen-Ens-Prod warning when parsing the nbhrd_prob dictionary (`#2224 <https://github.com/dtcenter/MET/issues/2224>`_).

    .. dropdown:: Tropical Cyclone Tools

        * **Enhance TC-Pairs to read hurricane model diagnostic files (e.g. SHIPS) and TC-Stat to filter the new data** (`#392 <https://github.com/dtcenter/MET/issues/392>`_).
        * **Enhance TC-Pairs consensus logic to compute the spread of the location, wind speed, and pressure** (`#2036 <https://github.com/dtcenter/MET/issues/2036>`_).
        * Enhance TC-RMW to compute tangential and radial winds (`#2072 <https://github.com/dtcenter/MET/issues/2072>`_).
        * Refine TCDIAG output from TC-Pairs as needed (`#2321 <https://github.com/dtcenter/MET/issues/2321>`_).
        * Rename the TCDIAG SOURCE column as DIAG_SOURCE (`#2337 <https://github.com/dtcenter/MET/issues/2337>`_).

    .. dropdown:: Miscellaneous

        * Enhance MTD to process time series with non-uniform time steps, such as monthly data (`#1971 <https://github.com/dtcenter/MET/issues/1971>`_).
        * Refine Grid-Diag output variable names when specifying two input data sources (`#2232 <https://github.com/dtcenter/MET/issues/2232>`_).
        * Add tmp_dir configuration option to the Plot-Point-Obs tool (`#2237 <https://github.com/dtcenter/MET/issues/2237>`_).

MET Upgrade Instructions
========================

MET Version 11.1.0 upgrade instructions
---------------------------------------

* If compiling support for PYTHON (:numref:`compiling_python_support`), in addition to $MET_PYTHON_CC and $MET_PYTHON_LD, set **$MET_PYTHON_BIN_EXE** to specify the desired python executable to be used (`#2428 <https://github.com/dtcenter/MET/issues/2428>`_).

* If running TC-Pairs to generate consensus tracks, update your TC-Pairs configuration file to include the new **write_members** option (`#2429 <https://github.com/dtcenter/MET/issues/2429>`_).

MET Version 11.0.0 upgrade instructions
---------------------------------------

* Ensemble post-processing has been fully removed from Ensemble-Stat in version 11.0.0. It can be performed using the Gen-Ens-Prod tool.
