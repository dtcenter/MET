***********************
MET Release Information
***********************

MET Release Notes
=================

When applicable, release notes are followed by the GitHub issue number which describes the bugfix,
enhancement, or new feature (`MET GitHub issues <https://github.com/dtcenter/MET/issues>`_).
Important issues are listed **in bold** for emphasis.

MET Version 13.0.0-beta2 Release Notes (20260508)
-------------------------------------------------

  .. dropdown:: Enhancements


     * **Support @value notation and multiple vertical levels for UGRID NetCDF files**
       (`#3254 <https://github.com/dtcenter/MET/issues/3254>`_).
     * **Enhance Gen-Ens-Prod to support the Ensemble Agreement Scale (EAS) algorithm for calculating probabilities**
       (`#3294 <https://github.com/dtcenter/MET/issues/3294>`_).
     * Refine Regrid-Data-Plane to print warnings about missing fields rather than erroring out
       (`#3336 <https://github.com/dtcenter/MET/issues/3336>`_).
     * Improve the Python embedding handling of gridded data attributes since JSON does not serialize user defined objects like "cartopy.crs.LambertConformal"
       (`#3373 <https://github.com/dtcenter/MET/issues/3373>`_).

  .. dropdown:: Bugfixes

     * Fix ASCII2NC to handle a wider range of NDBC bad data values
       (`#3342 <https://github.com/dtcenter/MET/issues/3342>`_).
     * Fix support for the "file_type" option in the PCP-Combine "sum" command
       (`#3353 <https://github.com/dtcenter/MET/issues/3353>`_).
     * Fix OpenMP 4.5 compilation errors from the GNU 8.5.0 compiler
       (`#3359 <https://github.com/dtcenter/MET/issues/3359>`_).
     * Fix Grid-Stat to correct the timing information of the SEEPS data written to the NetCDF matched pairs output file
       (`#3362 <https://github.com/dtcenter/MET/issues/3362>`_).
     * Fix TC-RMW to run on BEST tracks (and enhance TC-RMW to more flexibly match track points to gridded data)
       (`#3370 <https://github.com/dtcenter/MET/issues/3370>`_).
     * Fix NetCDF CF convention support to "false_easting" and "false_northing" for lambert conformal projections
       (`#3374 <https://github.com/dtcenter/MET/issues/3374>`_).

  .. dropdown:: Repository, build, and test

     * Update MET's development environment to better support RRFS GRIB2 files
       (`#3337 <https://github.com/dtcenter/MET/issues/3337>`_).
     * Update MET's compilation script to support upgraded versions of the dependent libraries
       (`#3377 <https://github.com/dtcenter/MET/issues/3377>`_).

  .. dropdown:: METbaseimage testing environment

     * Update METbaseimage to use Python version 3.14
       (`#61 <https://github.com/dtcenter/METbaseimage/issues/61>`_).

MET Version 13.0.0-beta1 Release Notes (20260205)
-------------------------------------------------

  .. dropdown:: Enhancements

     * Minimize the use of temporary files in Stat-Analysis
       (`#2698 <https://github.com/dtcenter/MET/issues/2698>`_).
     * Resolve runtime differences for different GNU/Intel optimization levels for PBL heights in PB2NC
       (`#3110 <https://github.com/dtcenter/MET/issues/3110>`_).
     * **Enhance Grid-Diag to compute mutual information**
       (`#3171 <https://github.com/dtcenter/MET/issues/3171>`_).
     * Enhance MET Python embedding and grid specification strings to support LAEA grids
       (`#3230 <https://github.com/dtcenter/MET/issues/3230>`_).
     * Resolve several SonarQube Reliability issues in MET's develop branch
       (`#3253 <https://github.com/dtcenter/MET/issues/3253>`_).
     * Refine handling of missing data for orographic corrections
       (`#3270 <https://github.com/dtcenter/MET/issues/3270>`_).
     * Enhance the MET tools to return consistent exit codes
       (`#3278 <https://github.com/dtcenter/MET/issues/3278>`_).
     * Enhance the "GEOG_MATCH" interpolation method to print a WARNING about missing topography and land/sea mask inputs
       (`#3285 <https://github.com/dtcenter/MET/issues/3285>`_).
     * Enhance Point2Grid to make the default output value configurable
       (`#3297 <https://github.com/dtcenter/MET/issues/3297>`_).
     * **Refine the logic for setting the default masking "FULL" grid in the MET tools**
       (`#3298 <https://github.com/dtcenter/MET/issues/3298>`_).
     * Enhance PB2NC and IODA2NC to set the "quality_mark_thresh" configuration option as an actual threshold
       (`#3307 <https://github.com/dtcenter/MET/issues/3307>`_).

  .. dropdown:: Bugfixes

     * Fix the logic to apply "set_attr_grid" before "ShiftRight"
       (`#3255 <https://github.com/dtcenter/MET/issues/3255>`_).
     * Fix ASCII2NC hang when run with an empty input file
       (`#3266 <https://github.com/dtcenter/MET/issues/3266>`_).
     * Fix support for the "set_attr_grid" config option when defining the verification domain
       (`#3293 <https://github.com/dtcenter/MET/issues/3293>`_).
     * Fix dependency checks and compile flag defaults in compile_MET_all.sh
       (`#3317 <https://github.com/dtcenter/MET/issues/3317>`_).

  .. dropdown:: Repository, build, and test

     * Deprecate and remove the "--enable-mode-graphics" configuration option and corresponding "plot_mode_field" utility
       (`#3322 <https://github.com/dtcenter/MET/issues/3322>`_).

  .. dropdown:: METbaseimage testing environment

     * Replace deprecated pip install arguments
       (`METbaseimage #47 <https://github.com/dtcenter/METbaseimage/issues/47>`_).
     * Create new hardened and streamlined base image for METviewer
       (`METbaseimage #52 <https://github.com/dtcenter/METbaseimage/issues/52>`_).

MET Upgrade Instructions
========================

This section summarizes and highlights important changes to MET since version 12.2.0, including:

  - Adding new or deprecating existing **tools**.
  - Modifying **command line options**.
  - Modifying **configuration file options**.
  - Modifying existing **output file formats**, such as new or modified ASCII or NetCDF file
    formats.
  - Changing existing **output data values** generated by MET, typically due to fixing bugs that
    were computing incorrect output.
  - Any other relevant details needed to upgrade from the MET version 12.1.0 to 12.2.0.

MET Version 13.0.0 Upgrade Instructions
---------------------------------------

.. dropdown:: New or deprecated tools

   MET version 13.0.0 adds or removes the following tools:

   * The plot_mode_field utility to visualize the NetCDF output generated by the MODE tool
     has been deprecated and removed to reduce MET's external library dependencies, including
     the Cairo, Freetype, and Pixman libraries and the Ghostscript Fonts package. Users are
     encouarged to visualize the NetCDF output from MODE using the plot_data_plane utility
     and/or using functionality provided in `METplotpy <https://github.com/dtcenter/METplotpy>`_.

.. dropdown:: Command line option changes

   MET version 13.0.0 adds, modifies, or removes the following command line options:

   * Point2Grid adds a new "-default_value" command line option to override the default
     output grid value of bad data (NA or -9999).
   
.. dropdown:: Configuration file changes

   MET version 13.0.0 adds, modifies, or removes the following configuration options:

   * The PB2NC and IODA2NC "quality_mark_thresh" configuration file option can now be set as a
     threshold string or integer, as before.
     
     * When processing GDAS surface observations with PB2NC, consider setting
       "quality_mark_thresh = <=2||==9;" to retain high quality surface observations
       (1 and 2) plus those ignored by the data assimilation system (9).

     * While IODA2NC did previously set "quality_mark_thresh = 0;" in the configuration file,
       it was not actually applied by IODA2NC and no quality mark filtering was performed.
       Changing the default to "quality_mark_thresh = NA;", which always evaluates to true,
       maintains that earlier default behavior. However, changing this setting now applies
       the expected filtering logic.

   * Setting the mask grid to "FULL" has been removed from the default configuration files for
     Point-Stat, Grid-Stat, Pair-Stat, and Ensemble-Stat to produce more intuitive behavior.
     However, if no spatial masking region is requested in the configuration file, then the
     "FULL" grid will automatically be added, as noted in :numref:`config_options-mask`.

   * Grid-Diag configuration file

     * The "mask.grid" and "mask.poly" entries have changed from strings to arrays of strings
       to support the processing of multiple masking regions.

     * The new "output_flag" entry is a dictionary specifying the desired output types.

   * Gen-Ens-Prod configuration file

     * The "eas_prob" dictionary is added to configure the Ensemble Agreement Scale (EAS) algorithm.

     * The "ensemble_flag.eas" and "ensemble_flag.eas_width" entries are added to enable the
       writing of EAS output fields.

.. dropdown:: Output format changes - NONE

.. dropdown:: Output format changes - NONE

   MET version 13.0.0 adds or modifies the following output file formats:

   * Grid-Diag output format

     * The new "mask" dimension, "mask_name" variable, and "mask_size" variable are added
       to support the processing of mulitple masking regions.

     * Existing histogram variables are modified to include the "mask" dimension.

     * New information theory variables are added for "entropy", "joint_entropy", and "mutual_information".

   * Gen-Ens-Prod output format

     * Adds new output variables with names include "EAS" and "EAS_WIDTH" for the Ensemble
       Agreement Scale algorithm.

.. dropdown:: Output data changes - NONE

   MET version 13.0.0 modifies existing output data values in the following ways:

   * None

.. dropdown:: Additional upgrade instructions - NONE

   Recommendations when upgrading to MET version 13.0.0:

   * None
