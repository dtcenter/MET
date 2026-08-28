***********************
MET Release Information
***********************

MET Release Notes
=================

When applicable, release notes are followed by the GitHub issue number which describes the bugfix,
enhancement, or new feature (`MET GitHub issues <https://github.com/dtcenter/MET/issues>`_).
Important issues are listed **in bold** for emphasis.

MET Version 12.2.2 Release Notes (20260828)
-------------------------------------------

  .. dropdown:: Bugfixes

     * Fix OpenMP 4.5 compilation errors from the GNU 8.5.0 compiler
       (`#3359 <https://github.com/dtcenter/MET/issues/3359>`_).
     * Fix Grid-Stat to correct the timing information of the SEEPS data written to the NetCDF matched pairs output file
       (`#3362 <https://github.com/dtcenter/MET/issues/3362>`_).
     * Fix NetCDF CF convention support false_easting and false_northing for lambert conformal projections
       (`#3374 <https://github.com/dtcenter/MET/issues/3374>`_).
     * Correctly handle invalid latitude values from an MPAS file
       (`#3381 <https://github.com/dtcenter/MET/issues/3381>`_).
     * Refine RPS library code to use centered probability bins
       (`#3396 <https://github.com/dtcenter/MET/issues/3396>`_).
     * Enable MPAS UGRID support to parse forecast lead time from diag files
       (`#3410 <https://github.com/dtcenter/MET/issues/3410>`_).
     * Fix Ensemble-Stat to skip observations when the requested observation error table lookup fails
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
