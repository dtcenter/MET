***********************
MET Release Information
***********************

MET Release Notes
=================

When applicable, release notes are followed by the GitHub issue number which describes the bugfix,
enhancement, or new feature (`MET GitHub issues <https://github.com/dtcenter/MET/issues>`_).
Important issues are listed **in bold** for emphasis.

MET Version 12.2.3 Release Notes (TBD)
--------------------------------------

  .. dropdown:: Bugfixes

     * Fix NetCDF CF convention support to read Stage IV precipitation files
       (`#3358 <https://github.com/dtcenter/MET/issues/3358>`_).
     * Fix PB2NC so that derived HPBL output does not depend on which other stations are processed
       (`#3450 <https://github.com/dtcenter/MET/issues/3450>`_).
     * Fix CNT statistics derived from SAL1L2 lines and from climatology bin mean partial sums
       (`#3478 <https://github.com/dtcenter/MET/issues/3478>`_).

  .. dropdown:: Repository, build, and test

     * Update the SonarQube scanner so that the scan runs on Java 21 as required by the upgraded SonarQube server
       (`#3470 <https://github.com/dtcenter/MET/issues/3470>`_).

MET Version 12.2.2 Release Notes (20260828)
-------------------------------------------

  .. dropdown:: Bugfixes

     * Fix OpenMP 4.5 compilation errors from the GNU 8.5.0 compiler
       (`#3359 <https://github.com/dtcenter/MET/issues/3359>`_).
     * Fix Grid-Stat to correct the timing information of the SEEPS data written to the NetCDF matched pairs output file
       (`#3362 <https://github.com/dtcenter/MET/issues/3362>`_).
     * Fix NetCDF CF convention to support 'false_easting' and 'false_northing' for lambert conformal projections
       (`#3374 <https://github.com/dtcenter/MET/issues/3374>`_).
     * Correctly handle invalid latitude values from an MPAS file
       (`#3381 <https://github.com/dtcenter/MET/issues/3381>`_).
     * Refine RPS library code to use centered probability bins
       (`#3396 <https://github.com/dtcenter/MET/issues/3396>`_).
     * Enable MPAS UGRID support to parse forecast lead time from diag files
       (`#3410 <https://github.com/dtcenter/MET/issues/3410>`_).
     * Fix Ensemble-Stat to skip observations when the observation error table lookup fails
       (`#3429 <https://github.com/dtcenter/MET/issues/3429>`_).

MET Version 12.2.1 Release Notes (20260227)
-------------------------------------------

  .. dropdown:: Bugfixes

     * Fix support for the "set_attr_grid" config option when defining the verification domain
       (`#3293 <https://github.com/dtcenter/MET/issues/3293>`_).
     * Fix dependency checks and compile flag defaults in "compile_MET_all.sh"
       (`#3317 <https://github.com/dtcenter/MET/issues/3317>`_).
     * Fix ASCII2NC to handle a wider range of NDBC bad data values
       (`#3342 <https://github.com/dtcenter/MET/issues/3342>`_).
     * Fix support for the "file_type" option in the PCP-Combine "sum" command
       (`#3353 <https://github.com/dtcenter/MET/issues/3353>`_).

  .. dropdown:: METbaseimage testing environment

     * Update METbaseimage to Python 3.14
       (`METbaseimage#61 <https://github.com/dtcenter/METbaseimage/issues/61>`_).

  .. note:: MET version 12.2.1 is built upon METbaseimage version 3.4.9 in which the
            Python version was upgraded from 3.12 to 3.14. This change only impacts
            MET's Docker development and testing environment. MET version 12.1.2 should
            continue working well with Python version 3.12.

MET Version 12.2.0 Release Notes (20251113)
-------------------------------------------

  .. dropdown:: Enhancements

     * Extend time range support with NetCDF-CF files
       (`#2836 <https://github.com/dtcenter/MET/issues/2836>`_).
     * Enhance ASCII2NC to recursively search input directories for specific file types for use with ISMN data
       (`#3148 <https://github.com/dtcenter/MET/issues/3148>`_).
     * Enhance RMW-Analysis to filter individual track points rather than entire tracks
       (`#3168 <https://github.com/dtcenter/MET/issues/3168>`_).
     * Enhance madis2nc so that it writes variable names
       (`#3172 <https://github.com/dtcenter/MET/issues/3172>`_).
     * **Enhance Point-Stat and Ensemble-Stat to apply orographic corrections when verifying surface temperatures and heights (e.g. cloud base) against point observations**
       (`#3174 <https://github.com/dtcenter/MET/issues/3174>`_).
     * Enhance Pair-Stat to support data censoring and conversion configuration options
       (`#3186 <https://github.com/dtcenter/MET/issues/3186>`_).
     * Add CVE scanning to the release-docker-images.yml workflow
       (`#3198 <https://github.com/dtcenter/MET/issues/3198>`_).
     * **Support multiple vertical level variables for CF NetCDF input**
       (`#3226 <https://github.com/dtcenter/MET/issues/3226>`_).
     * Add new pre-defined grid definitions to support the computation of CBS scores
       (`#3232 <https://github.com/dtcenter/MET/issues/3232>`_).
     * Refine default lapse rates for orographic corrections
       (`#3258 <https://github.com/dtcenter/MET/issues/3258>`_).
     * Refine handling of missing data for orographic corrections
       (`#3270 <https://github.com/dtcenter/MET/issues/3270>`_).
     * Enhance the MET tools to return consistent exit codes
       (`#3278 <https://github.com/dtcenter/MET/issues/3278>`_).

  .. dropdown:: Bugfixes

     * Bugfix: Fix compilation on non-GCC compilers by replacing #include <bits/stdc++.h>
       (`#3175 <https://github.com/dtcenter/MET/issues/3175>`_).
     * Bugfix: Fix the AW_MEAN regridding method when OMP_NUM_THREADS is set very large
       (`#3206 <https://github.com/dtcenter/MET/issues/3206>`_).
     * Bugfix: Fix STAT-Analysis handling of zero-length vectors when aggregating MPR inputs and writing WDIR or VCNT outputs
       (`#3210 <https://github.com/dtcenter/MET/issues/3210>`_).
     * Bugfix: Python embedding does not work for MET version 12.1.0 with Python version 3.10
       (`#3219 <https://github.com/dtcenter/MET/issues/3219>`_).
     * Bugfix: Fix segfault when reading climatological data from a CF-compliant NetCDF file fails
       (`#3235 <https://github.com/dtcenter/MET/issues/3235>`_).
     * Update the logic to apply "set_attr_grid" before "ShiftRight"
       (`#3255 <https://github.com/dtcenter/MET/issues/3255>`_).
     * Fix ASCII2NC hang when run with an empty input file
       (`#3266 <https://github.com/dtcenter/MET/issues/3266>`_).

  .. dropdown:: Repository, build, and test

     * Confirm that the METplus difference logic handles MTD output file
       (`#977 <https://github.com/dtcenter/MET/issues/977>`_).
     * Switch MET's R-based differencing logic over to using the Python-based METplus implementation
       (`#2718 <https://github.com/dtcenter/MET/issues/2718>`_).

  .. dropdown:: METbaseimage testing environment

     * Add logic to regenerate previously tagged versions based on a schedule
       (`METbaseimage#18 <https://github.com/dtcenter/METbaseimage/issues/18>`_).
     * Add a new GHA development workflow and enhance workflows to scan Docker images for CVEs #33
       (`METbaseimage#33 <https://github.com/dtcenter/METbaseimage/issues/33>`_).
     * METbaseimage: Address Critical CVEs from Grype scans of the Docker images
       (`METplus-Internal#61 <https://github.com/dtcenter/METplus-Internal/issues/61>`_).
     * Enhance the "Create Release Docker Images" METplus workflows to determine and build the most recent bugfix versions
       (`METplus-Internal#64 <https://github.com/dtcenter/METplus-Internal/issues/64>`_).

MET Upgrade Instructions
========================

This section summarizes and highlights important changes to MET since version 12.1.0, including:

  - Adding new or modifying existing **configuration file options**.
  - Modifying existing **output file formats**, such as new or modified ASCII or NetCDF file
    formats.
  - Changing existing **output data values** generated by MET, typically due to fixing bugs that
    were computing incorrect output.
  - Any other relevant details needed to upgrade from the MET version 12.1.0 to 12.2.0.

MET Version 12.2.3 Upgrade Instructions
---------------------------------------

.. dropdown:: Output data changes

   MET version 12.2.3 modifies existing output data values in the following ways:

   * Gridded data read from NetCDF files that follow the CF convention and use a polar
     stereographic grid mapping may change. As described in MET
     `#3358 <https://github.com/dtcenter/MET/issues/3358>`_, MET now reads data stored from
     north to south (negative y spacing), such as the NOAA Stage IV NetCDF precipitation
     files, and uses the "standard_parallel" attribute to define the grid's scale latitude.
     Grids and output for affected files, such as their orientation, may differ from previous
     versions.

   * PBL heights (HPBL) derived by PB2NC may change. As described in MET
     `#3450 <https://github.com/dtcenter/MET/issues/3450>`_, the PBL derived for a station
     previously could depend on which other stations were processed in the same run, because
     unused input records from one station's profile carried over into the next, and deep
     profiles read past the levels actually stored. The PBL derived for each station now
     depends only on that station's own observations.

   * Stat-Analysis summary jobs that use the "-derive" option to compute CNT statistics from
     SAL1L2 input lines now write output. As described in MET
     `#3478 <https://github.com/dtcenter/MET/issues/3478>`_, the anomaly partial sums were
     previously not read from SAL1L2 lines, so the derived anomaly statistics (e.g. ANOM_CORR,
     ANOM_CORR_UNCNTR, RMSFA, and RMSOA) were always bad data and no output was written.

   * The FOBAR and FOABAR columns of the SL1L2 and SAL1L2 output lines written by Point-Stat,
     Grid-Stat, and Pair-Stat for the climatology bin means (VX_MASK ending in "_BIN_MEAN")
     now contain the mean across bins. As described in MET
     `#3478 <https://github.com/dtcenter/MET/issues/3478>`_, these columns were previously
     always zero, so statistics derived from them, such as RMSE, PR_CORR, and ANOM_CORR,
     were incorrect.

MET Version 12.2.0 Upgrade Instructions
---------------------------------------

.. dropdown:: Configuration file changes

   MET version 12.2.0 adds, modifies, or removes the following configuration options:

   * Ensemble-Stat configuration:
 
     * Adds support for **land_mask** and **topo_mask** configuration dictionaries
       previously only supported in Point-Stat.
   
   * Ensemble-Stat and Point-Stat configurations:

     * Adds new **lapse_rate_correction** configuration dictionary to use topography
       differences to correct forecast or observation values of surface temperature.

     * Adds new **msl_agl_correction** configuration dictionary to convert between
       height values defined relative to mean-sea-level or above-ground-level.

.. dropdown:: Output format changes - NONE

   MET version 12.2.0 adds or modifies the following output file formats:

   * None

.. dropdown:: Output data changes - NONE

   MET version 12.2.0 modifies existing output data values in the following ways:

   * None

.. dropdown:: Additional upgrade instructions - NONE

   Recommendations when upgrading to MET version 12.2.0:

   * None
