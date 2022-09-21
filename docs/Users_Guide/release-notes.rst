MET Release Notes
=================

When applicable, release notes are followed by the GitHub issue number which describes the bugfix,
enhancement, or new feature (`MET GitHub issues <https://github.com/dtcenter/MET/issues>`_).
Important issues are listed **in bold** for emphasis.

MET Version 11.0.0-beta3 release notes (20220921)
-------------------------------------------------

.. warning:: **Ensemble post-processing was added to Gen-Ens-Prod in version 10.1.0 and will be *removed* from Ensemble-Stat in version 11.0.0!**

* Repository and build:

   * Add initial files to create the MET compilation environment in the dtcenter/met-base Docker image (`dtcenter/METbaseimage#1 <https://github.com/dtcenter/METbaseimage/issues/1>`_).
   * Update the METbaseimage to install Python 3.8.6 from source (`dtcenter/METbaseimage#3 <https://github.com/dtcenter/METbaseimage/issues/3>`_).
   * Restructure the MET Dockerfiles to create images based on the new METbaseimage (`#2196 <https://github.com/dtcenter/MET/issues/2196>`_).
   * Add .zenodo.json file to add metadata about releases (`#2198 <https://github.com/dtcenter/MET/issues/2198>`_).

* Bugfixes:

   * Fix the truncated station_id name in the output from IODA2NC (`#2216 <https://github.com/dtcenter/MET/issues/2216>`_).
   * Fix oom() compile time linker error (`#2238 <https://github.com/dtcenter/MET/issues/2238>`_).
   * Store unspecified accumulation interval as 0 rather than bad data (`#2250 <https://github.com/dtcenter/MET/issues/2250>`_).

* Enhancements:

   * **Remove ensemble post-processing from the Ensemble-Stat tool** (`#1908 <https://github.com/dtcenter/MET/issues/1908>`_).
   * **Enhance Point-Stat to compute SEEPS for point observations and write new SEEPS and SEEPS_MPR STAT line types** (`#1942 <https://github.com/dtcenter/MET/issues/1942>`_).
   * **Add the fair CRPS statistic to the ECNT line type in a new CRPS_EMP_FAIR column** (`#2206 <https://github.com/dtcenter/MET/issues/2206>`_).
   * Update map data with more recent NaturalEarth definitions (`#2207 <https://github.com/dtcenter/MET/issues/2207>`_).
   * Define new grid class to store semi-structured grid information (e.g. lat or lon vs level or time) (`#1954 <https://github.com/dtcenter/MET/issues/1954>`_).
   * Add support for EPA AirNow ASCII data in ASCII2NC (`#2142 <https://github.com/dtcenter/MET/issues/2142>`_).
   * Add tmp_dir configuration option to the Plot-Point-Obs tool (`#2237 <https://github.com/dtcenter/MET/issues/2237>`_).
   * Refine NetCDF level string handling logic to always interpret @ strings as values (`#2225 <https://github.com/dtcenter/MET/issues/2225>`_).
   * Add support for reading National Blend Model GRIB2 data (`#2055 <https://github.com/dtcenter/MET/issues/2055>`_).

MET Version 11.0.0-beta2 release notes (20220809)
-------------------------------------------------

* Bugfixes:

   * Fix Ensemble-Stat to work with different missing members for two or more variables (`#2208 <https://github.com/dtcenter/MET/issues/2208>`_).

* Enhancements:

   * **Enhance MET's NetCDF library interface to support level strings that include coordinate variable values instead of just indexes** (`#1815 <https://github.com/dtcenter/MET/issues/1815>`_).
   * **Enhance MTD to process time series with non-uniform time steps, such as monthly data** (`#1971 <https://github.com/dtcenter/MET/issues/1971>`_).
   * Define the Bukovsky masking regions for use in MET (`#1940 <https://github.com/dtcenter/MET/issues/1940>`_).
   * Update the GRIB2 MRMS table in MET (`#2081 <https://github.com/dtcenter/MET/issues/2081>`_).
   * Add more error checking for python embedding of point observations (`#2202 <https://github.com/dtcenter/MET/issues/2202>`_).
   * Add a sum option to the time summaries computed by the point pre-processing tools (`#2204 <https://github.com/dtcenter/MET/issues/2204>`_).
   * Refine warning/error messages when parsing thresholds (`#2211 <https://github.com/dtcenter/MET/issues/2211>`_).
   * Add "station_ob" to metadata_map as a message_type metadata variable for ioda2nc (`#2215 <https://github.com/dtcenter/MET/issues/2215>`_).
   * MET: Add the user ID and the command line being executed to the log output at beginning and end of each MET tool run (`dtcenter/METplus-Internal#19 <https://github.com/dtcenter/METplus-Internal/issues/19>`_).
   * MET: Enhance MET to have better signal handling for shutdown events (`dtcenter/METplus-Internal#21 <https://github.com/dtcenter/METplus-Internal/issues/21>`_).

MET Version 11.0.0-beta1 release notes (20220622)
-------------------------------------------------

* Repository and build:

   * **Restructure the contents of the MET repository so that it matches the existing release tarfiles** (`#1920 <https://github.com/dtcenter/MET/issues/1920>`_).
   * Fix the OpenMP compilation error for GCC 9.3.0/9.4.0 (`#2106 <https://github.com/dtcenter/MET/issues/2106>`_).
   * Update the MET version number to 11.0.0 (`#2132 <https://github.com/dtcenter/MET/issues/2132>`_).

* Bugfixes:

   * Fix regression test differences in pb2nc and ioda2nc output (`#2102 <https://github.com/dtcenter/MET/issues/2102>`_).
   * Fix support for reading rotated lat/lon grids from CF-compliant NetCDF files (`#2115 <https://github.com/dtcenter/MET/issues/2115>`_).
   * Fix support for reading rotated lat/lon grids from GRIB1 files (grid type 10) (`#2118 <https://github.com/dtcenter/MET/issues/2118>`_).
   * Fix support for int64 NetCDF variable types (`#2123 <https://github.com/dtcenter/MET/issues/2123>`_).
   * Fix Stat-Analysis to aggregate the ECNT ME and RMSE values correctly (`#2170 <https://github.com/dtcenter/MET/issues/2170>`_).
   * Fix NetCDF library code to process scale_factor and add_offset attributes independently (`#2187 <https://github.com/dtcenter/MET/issues/2187>`_).

* Enhancements:

   * Sort mask.sid station lists to check their contents more efficiently (`#1950 <https://github.com/dtcenter/MET/issues/1950>`_).
   * Add Anomaly Correlation Coefficient to VCNT Line Type (`#2022 <https://github.com/dtcenter/MET/issues/2022>`_).
   * Enhance TC-RMW to compute tangential and radial winds (`#2072 <https://github.com/dtcenter/MET/issues/2072>`_).
   * Allow 2x2 HSS calculations to include user-defined EC values (`#2147 <https://github.com/dtcenter/MET/issues/2147>`_).
   * Enhance Gen-Vx-Mask by adding a new poly_xy masking type option (`#2152 <https://github.com/dtcenter/MET/issues/2152>`_).
   * Add M_to_KFT and KM_to_KFT functions to ConfigConstants (`#2180 <https://github.com/dtcenter/MET/issues/2180>`_).
   * MET: Replace fixed length character arrays with strings (`dtcenter/METplus-Internal#14 <https://github.com/dtcenter/METplus-Internal/issues/14>`_).
