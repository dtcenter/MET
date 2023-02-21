***********************
MET Release Information
***********************

MET Release Notes
=================

When applicable, release notes are followed by the GitHub issue number which describes the bugfix,
enhancement, or new feature (`MET GitHub issues <https://github.com/dtcenter/MET/issues>`_).
Important issues are listed **in bold** for emphasis.

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

   .. grid:: NetCDF

      * **Enhance MET's NetCDF library interface to support level strings that include coordinate variable values instead of just indexes** (`#1815 <https://github.com/dtcenter/MET/issues/1815>`_).
      * Enhance MET to handle NC strings when processing CF-Compliant NetCDF files (`#2042 <https://github.com/dtcenter/MET/issues/2042>`_).
      * Enhance MET to handle CF-compliant time strings with an offset defined in months or years (`#2155 <https://github.com/dtcenter/MET/issues/2155>`_).
      * Refine NetCDF level string handling logic to always interpret @ strings as values (`#2225 <https://github.com/dtcenter/MET/issues/2225>`_).

   * GRIB:

      * Add support for reading National Blend Model GRIB2 data (`#2055 <https://github.com/dtcenter/MET/issues/2055>`_).
      * Update the GRIB2 MRMS table in MET (`#2081 <https://github.com/dtcenter/MET/issues/2081>`_).

   * Python:

      * Reimplement the pntnc2ascii.R utility Rscript in Python (`#2085 <https://github.com/dtcenter/MET/issues/2085>`_).
      * Add more error checking for python embedding of point observations (`#2202 <https://github.com/dtcenter/MET/issues/2202>`_).
      * **Add a Python helper script/function to transform point_data objects to met_point_data objects for Python Embedding** (`#2302 <https://github.com/dtcenter/MET/issues/2302>`_).

   * METplus-Internal:

      * MET: Replace fixed length character arrays with strings (`dtcenter/METplus-Internal#14 <https://github.com/dtcenter/METplus-Internal/issues/14>`_).
      * MET: Add a timestamp to the log output at the beginning and end of each MET tool run (`dtcenter/METplus-Internal#18 <https://github.com/dtcenter/METplus-Internal/issues/18>`_).
      * MET: Add the user ID and the command line being executed to the log output at beginning and end of each MET tool run (`dtcenter/METplus-Internal#19 <https://github.com/dtcenter/METplus-Internal/issues/19>`_).
      * MET: Enhance MET to have better signal handling for shutdown events (`dtcenter/METplus-Internal#21 <https://github.com/dtcenter/METplus-Internal/issues/21>`_).

   * Common Libraries:

      * **Define new grid class to store semi-structured grid information (e.g. lat or lon vs level or time)** (`#1954 <https://github.com/dtcenter/MET/issues/1954>`_).
      * Refine warning/error messages when parsing thresholds (`#2211 <https://github.com/dtcenter/MET/issues/2211>`_).
      * Remove namespace specification from header files (`#2227 <https://github.com/dtcenter/MET/issues/2227>`_).
      * Update MET version number to 11.0.0 (`#2132 <https://github.com/dtcenter/MET/issues/2132>`_).
      * Store unspecified accumulation interval as 0 rather than bad data (`#2250 <https://github.com/dtcenter/MET/issues/2250>`_).
      * Add sanity check to error out when both is_u_wind and is_v_wind are set to true (`#2357 <https://github.com/dtcenter/MET/issues/2357>`_).

   * Statistics:

      * **Add Anomaly Correlation Coefficient to VCNT Line Type** (`#2022 <https://github.com/dtcenter/MET/issues/2022>`_).
      * **Allow 2x2 HSS calculations to include user-defined EC values** (`#2147 <https://github.com/dtcenter/MET/issues/2147>`_).
      * **Add the fair CRPS statistic to the ECNT line type in a new CRPS_EMP_FAIR column** (`#2206 <https://github.com/dtcenter/MET/issues/2206>`_).
      * **Add MAE to the ECNT line type from Ensemble-Stat and for HiRA** (`#2325 <https://github.com/dtcenter/MET/issues/2325>`_).
      * **Add the Mean Absolute Difference (SPREAD_MD) to the ECNT line type** (`#2332 <https://github.com/dtcenter/MET/issues/2332>`_).
      * **Add new bias ratio statistic to the ECNT line type from Ensemble-Stat and for HiRA** (`#2058 <https://github.com/dtcenter/MET/issues/2058>`_).

   * Configuration and masking:

      * Define the Bukovsky masking regions for use in MET (`#1940 <https://github.com/dtcenter/MET/issues/1940>`_).
      * **Enhance Gen-Vx-Mask by adding a new poly_xy masking type option** (`#2152 <https://github.com/dtcenter/MET/issues/2152>`_).
      * Add M_to_KFT and KM_to_KFT functions to ConfigConstants (`#2180 <https://github.com/dtcenter/MET/issues/2180>`_).
      * Update map data with more recent NaturalEarth definitions (`#2207 <https://github.com/dtcenter/MET/issues/2207>`_).

   * Point Pre-Processing Tools:

      * **Enhance IODA2NC to support IODA v2.0 format** (`#2068 <https://github.com/dtcenter/MET/issues/2068>`_).
      * **Add support for EPA AirNow ASCII data in ASCII2NC** (`#2142 <https://github.com/dtcenter/MET/issues/2142>`_).
      * Add a sum option to the time summaries computed by the point pre-processing tools (`#2204 <https://github.com/dtcenter/MET/issues/2204>`_).
      * Add "station_ob" to metadata_map as a message_type metadata variable for ioda2nc (`#2215 <https://github.com/dtcenter/MET/issues/2215>`_).
      * **Enhance ASCII2NC to read NDBC buoy data** (`#2276 <https://github.com/dtcenter/MET/issues/2276>`_).
      * Print ASCII2NC warning message about python embedding support not being compiled (`#2277 <https://github.com/dtcenter/MET/issues/2277>`_).

   * Point-Stat, Grid-Stat, Stat-Analysis:

      * Add support for point-based climatologies for use in SEEPS (`#1941 <https://github.com/dtcenter/MET/issues/1941>`_).
      * **Enhance Point-Stat to compute SEEPS for point observations and write new SEEPS and SEEPS_MPR STAT line types** (`#1942 <https://github.com/dtcenter/MET/issues/1942>`_).
      * **Enhance Grid-Stat to compute SEEPS for gridded observations and write the SEEPS STAT line type** (`#1943 <https://github.com/dtcenter/MET/issues/1943>`_).
      * Sort mask.sid station lists to check their contents more efficiently (`#1950 <https://github.com/dtcenter/MET/issues/1950>`_).
      * **Enhance Stat-Analysis to aggregate SEEPS_MPR and SEEPS line types** (`#2339 <https://github.com/dtcenter/MET/issues/2339>`_).
      * Relax Point-Stat and Ensemble-Stat logic for the configuration of message_type_group_map (`#2362 <https://github.com/dtcenter/MET/issues/2362>`_).
      * Fix Point-Stat and Grid-Stat logic for processing U/V winds with python embedding (`#2366 <https://github.com/dtcenter/MET/issues/2366>`_).

   * Ensemble Tools:

      * **Remove ensemble post-processing from the Ensemble-Stat tool** (`#1908 <https://github.com/dtcenter/MET/issues/1908>`_).
      * Eliminate Gen-Ens-Prod warning when parsing the nbhrd_prob dictionary (`#2224 <https://github.com/dtcenter/MET/issues/2224>`_).

   * Tropical Cyclone Tools:

      * **Enhance TC-Pairs to read hurricane model diagnostic files (e.g. SHIPS) and TC-Stat to filter the new data** (`#392 <https://github.com/dtcenter/MET/issues/392>`_).
      * **Enhance TC-Pairs consensus logic to compute the spread of the location, wind speed, and pressure** (`#2036 <https://github.com/dtcenter/MET/issues/2036>`_).
      * Enhance TC-RMW to compute tangential and radial winds (`#2072 <https://github.com/dtcenter/MET/issues/2072>`_).
      * Refine TCDIAG output from TC-Pairs as needed (`#2321 <https://github.com/dtcenter/MET/issues/2321>`_).
      * Rename the TCDIAG SOURCE column as DIAG_SOURCE (`#2337 <https://github.com/dtcenter/MET/issues/2337>`_).

   * Miscellaneous:

      * Enhance MTD to process time series with non-uniform time steps, such as monthly data (`#1971 <https://github.com/dtcenter/MET/issues/1971>`_).
      * Refine Grid-Diag output variable names when specifying two input data sources (`#2232 <https://github.com/dtcenter/MET/issues/2232>`_).
      * Add tmp_dir configuration option to the Plot-Point-Obs tool (`#2237 <https://github.com/dtcenter/MET/issues/2237>`_).

MET Upgrade Instructions
========================

* Ensemble post-processing has been fully removed from Ensemble-Stat in version 11.0.0. It can be performed using the Gen-Ens-Prod tool.
