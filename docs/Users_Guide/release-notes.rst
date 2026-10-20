***********************
MET Release Information
***********************

MET Release Notes
=================

When applicable, release notes are followed by the GitHub issue number which describes the bugfix,
enhancement, or new feature (`MET GitHub issues <https://github.com/dtcenter/MET/issues>`_).
Important issues are listed **in bold** for emphasis.

MET Version 13.0.0-beta3 Release Notes (TBD)
---------------------------------------------

  .. dropdown:: Enhancements

     * **Generalize support for automated derivation of wind speed and direction and rotation of winds from grid to earth-relative**
       (`#1518 <https://github.com/dtcenter/MET/issues/1518>`_).
     * **Enhance area-weighted regridding and grid weighting to support centered grid boxes**
       (`#3178 <https://github.com/dtcenter/MET/issues/3178>`_).
     * **Add power spectra calculation capability to the Grid-Diag tool in MET**
       (`#3304 <https://github.com/dtcenter/MET/issues/3304>`_).
     * Add lookup tables for data types of MET output
       (`#3384 <https://github.com/dtcenter/MET/issues/3384>`_).
     * Enhance ``@value`` NetCDF dimension lookup logic to support negative values
       (`#3406 <https://github.com/dtcenter/MET/issues/3406>`_).
     * Improve Ensemble-Stat to more efficiently apply observation error corrections and perturbations
       (`#3426 <https://github.com/dtcenter/MET/issues/3426>`_).
     * Rename MET configuration options to avoid names like ``warning`` and ``error``
       (`#3455 <https://github.com/dtcenter/MET/issues/3455>`_).

  .. dropdown:: Bugfixes

     * Fix failures and warnings when compiling with clang
       (`#3291 <https://github.com/dtcenter/MET/issues/3291>`_).
     * Fix NetCDF CF convention support to read Stage IV precipitation files
       (`#3358 <https://github.com/dtcenter/MET/issues/3358>`_).
     * Fix MPAS UGRID support to correctly handle invalid latitude values
       (`#3381 <https://github.com/dtcenter/MET/issues/3381>`_).
     * Refine RPS library code to use centered probability bins
       (`#3396 <https://github.com/dtcenter/MET/issues/3396>`_).
     * Enable MPAS UGRID support to parse forecast lead time from diag files
       (`#3410 <https://github.com/dtcenter/MET/issues/3410>`_).
     * Fix Ensemble-Stat to skip observations when the observation error table lookup fails
       (`#3429 <https://github.com/dtcenter/MET/issues/3429>`_).
     * Fix PB2NC so that derived HPBL output does not depend on which other stations are processed
       (`#3450 <https://github.com/dtcenter/MET/issues/3450>`_).
     * Fix Stat-Analysis CNT statistics derived from SAL1L2 lines and from climatology bin mean partial sums
       (`#3478 <https://github.com/dtcenter/MET/issues/3478>`_).

  .. dropdown:: Repository, build, and test

     * Update the configuration files and modulefiles
       (`#3393 <https://github.com/dtcenter/MET/issues/3393>`_).
     * Remove R dependency from the MET unit test logic
       (`#3395 <https://github.com/dtcenter/MET/issues/3395>`_).
     * Upgrade MET to use METbaseimage version 3.5
       (`#3407 <https://github.com/dtcenter/MET/issues/3407>`_).
     * Enhance the documentation by correcting broken links and typos
       (`#3416 <https://github.com/dtcenter/MET/issues/3416>`_).
     * Fix SonarQube findings for MET version 13.0.0
       (`#3443 <https://github.com/dtcenter/MET/issues/3443>`_).
     * Update the SonarQube scanner so that the scan runs on Java 21 as required by the upgraded SonarQube server
       (`#3470 <https://github.com/dtcenter/MET/issues/3470>`_).
     * Automate link checking with Sphinx
       (`METplus #3247 <https://github.com/dtcenter/METplus/issues/3247>`_).
     * Resolve blockers from the AF SonarQube quality gate scanner
       (`METplus-Internal #68 <https://github.com/dtcenter/METplus-Internal/issues/68>`_).

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


  .. note::

     When using the **compile_MET_all.sh** script with this developmental
     release, download the appropriate dependency bundle, **tar_files.met-base-develop.tgz**, using 
     *wget https://dtcenter.ucar.edu/dfiles/code/METplus/MET/installation/tar_files.met-base-develop.tgz*
     instead of **tar_files.tgz**. The standard **tar_files.tgz** is intended
     for stable releases and may not contain compatible versions of the
     required libraries.

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
  - Any other relevant details needed to upgrade from the MET version 12.2.0 to 13.0.0.

MET Version 13.0.0 Upgrade Instructions
---------------------------------------

.. dropdown:: New or deprecated tools

   MET version 13.0.0 adds or removes the following tools:

   * The plot_mode_field utility to visualize the NetCDF output generated by the MODE tool
     has been deprecated and removed to reduce MET's external library dependencies, including
     the Cairo, Freetype, and Pixman libraries and the Ghostscript Fonts package. Users are
     encouraged to visualize the NetCDF output from MODE using the plot_data_plane utility
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

     * The "power_spectrum" dictionary is added to configure how missing data values should
       be handled when computing power spectra.

     * The new "output_flag" entry is a dictionary specifying the desired output types.

   * Gen-Ens-Prod configuration file

     * The "eas_prob" dictionary is added to configure the Ensemble Agreement Scale (EAS) algorithm.

     * The "ensemble_flag.eas" and "ensemble_flag.eas_width" entries are added to enable the
       writing of EAS output fields.

   * Ensemble-Stat configuration file

     * The "obs_error" dictionary is renamed to "obs_err", as described in
       :numref:`config_options-obs_err`, to avoid false positives when log files are scanned
       for words like "error". The old name is still supported but prints a warning and should
       be replaced. This applies both to the top-level entry and to entries set separately
       in each "obs.field" entry.

   * ConfigConstants configuration file

     * The "u_wind_field_name", "v_wind_field_name", "wind_speed_field_name", and "wind_direction_field_name"
       entries are comma-separated lists of field names to search when reading wind data needed to derive winds
       or rotate them from being grid-relative to earth-relative. See :numref:`PS_wind_rotation_derivation`
       for a description of wind rotation and derivation and :numref:`config_wind_field_names` for the
       corresponding configuration options.

.. dropdown:: Output format changes

   MET version 13.0.0 adds or modifies the following output file formats:

   * Grid-Diag output format

     * The new "mask" dimension, "mask_name" variable, and "mask_size" variable are added
       to support the processing of multiple masking regions.

     * Existing histogram variables are modified to include the "mask" dimension.

     * New information theory variables are added for "entropy", "joint_entropy", and
       "mutual_information".

     * A new power spectrum "wavenumber" dimension is added, along with "wavenumber" and
       "wavelength" variables. "power_spectrum" variables are written for each input,
       and "error_power_spectrum" variables contain the differences for multiple inputs.

   * Gen-Ens-Prod output format

     * Adds new output variables with names including "EAS" and "EAS_WIDTH" for the Ensemble
       Agreement Scale algorithm.

.. dropdown:: Output data changes

   MET version 13.0.0 modifies existing output data values in the following ways:

   * Ranked probability statistics found in the RPS line type and written by Ensemble-Stat and the
     HiRA algorithm of Point-Stat are modified slightly. As described in MET
     `#3396 <https://github.com/dtcenter/MET/issues/3396>`_, improvements to the RPS computation
     algorithm result in minor changes to the statistics.

   * Output regridded from Lambert Azimuthal Equal Area (LAEA) grids using the AW_MEAN
     interpolation method, or masked using the "AREA" grid weight option, changes slightly.
     As described in MET `#3178 <https://github.com/dtcenter/MET/issues/3178>`_, LAEA grid box
     areas were previously computed from a grid box centered on each grid point, unlike all
     other grid types, which used the grid box whose lower-left corner is that point. LAEA now
     follows the same convention as the other grid types. See the additional upgrade
     instructions below for details.

   * Output regridded to lat/lon, rotated lat/lon, and Mercator grids using the AW_MEAN
     interpolation method changes along the western edge of regional grids, the eastern edge
     of Mercator grids, and the first column of global grids. As described in MET
     `#3178 <https://github.com/dtcenter/MET/issues/3178>`_, input grid points within half an
     output grid box of those edges were previously discarded rather than assigned to the
     nearest output grid box.

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

.. dropdown:: Additional upgrade instructions

   Recommendations when upgrading to MET version 13.0.0:

   * The "MET_OBS_ERROR_TABLE" environment variable is renamed to "MET_OBS_ERR_TABLE", and the
     default *share/met/table_files/obs_error_table.txt* file is renamed to *obs_err_table.txt*,
     as described in :numref:`met_obs_err_table`. The old environment variable name is still
     supported but prints a warning and should be replaced. User-defined observation error
     table files whose names begin with "obs_error" are still read.

   * MET version 13.0.0 adds the AW_MEAN_CNTR interpolation method and the AREA_CNTR grid weight
     option, which compute the area of the grid box **centered** on each grid point. The existing
     AW_MEAN and AREA options compute the area of the grid box whose **lower-left corner** is that
     point. Previously, Lambert Azimuthal Equal Area (LAEA) grids were the one exception to that
     convention, computing centered areas even for AW_MEAN and AREA. That inconsistency has been
     resolved, so AW_MEAN and AREA now behave the same way for LAEA grids as for all others.

   * Users regridding from or weighting LAEA grids who wish to reproduce output from MET version
     12.2.0 and earlier should switch from AW_MEAN to AW_MEAN_CNTR, and from AREA to AREA_CNTR.
     Note that AW_MEAN_CNTR is not simply the old behavior renamed. It also divides the weight of
     each input grid box among the output grid boxes it overlaps, so results will not match
     exactly. AW_MEAN_CNTR is the recommended method for area-weighted regridding regardless of
     grid type.

   * Users of all other grid types are unaffected by the LAEA change, since their grid box areas
     were already computed from the lower-left corner.
