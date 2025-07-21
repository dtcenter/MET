***********************
MET Release Information
***********************

MET Release Notes
=================

When applicable, release notes are followed by the GitHub issue number which describes the bugfix,
enhancement, or new feature (`MET GitHub issues <https://github.com/dtcenter/MET/issues>`_).
Important issues are listed **in bold** for emphasis.

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

MET Version 12.0.0 Upgrade Instructions
---------------------------------------

.. note:: Note that the `#2833 <https://github.com/dtcenter/MET/issues/2833>`_
          bugfix affects all previously generated output from the TC-Diag and
          TC-RMW tools.

.. note:: Note that the GRIB table updates in `#2780 <https://github.com/dtcenter/MET/issues/2780>`_
          modified some existing units strings for consistency and clarity.

.. dropdown:: Compilation dependencies

   MET Version 12.0.0 introduces one new required and two new optional dependencies:

   * The `Proj <https://proj.org/>`_ library is required
     (`#2669 <https://github.com/dtcenter/MET/issues/2669>`_).
   * The `Atlas <https://sites.ecmwf.int/docs/atlas/>`_ and
     `ecKit <https://github.com/ecmwf/eckit>`_ libraries are optional and only
     required when configuring MET with the ``--enable-ugrid`` option
     (`#2574 <https://github.com/dtcenter/MET/issues/2574>`_).

.. dropdown:: Configuration file changes since version 11.1.0

   MET Version 12.0.0 adds, modifies, or removes the following configuration options:

   * Configuration of all tools:

     * Adds new **time_offset_warning** option in ``ConfigConstants``
       that can be overridden in tool-specific configuration files.

   * Point-Stat, Grid-Stat, Ensemble-Stat, and Series-Analysis configurations:

     * Enables **climo_mean** and **climo_stdev** to be defined separately
       within the **fcst** and **obs** dictionaries.

   * Point-Stat and Ensemble-Stat configurations:

     * Adds new **obtype_as_group_val_flag** option.
     * Adds new **point_weight_flag** option.

   * TC-Diag configuration:

     * Adds new **override_diags** option in the **domain_info** dictionary.
     * Adds new **one_time_per_file_flag** option.
     * Renames **nc_rng_azi_flag** option as **nc_cyl_grid_flag**.
     * Replaces **output_prefix** option with **output_base_format**.

   * MODE configuration:

     * Adds new **multivar_intensity_compare_fcst** and
       **multivar_intensity_compare_obs** options.
     * Adds new **multivar_name** and **multivar_level** options
       within the **fcst** and **obs** dictionaries.

   * PB2NC configuration:

     * Removes unused **quality_mark_thresh** option.
     * Adds new **obs_quality_inc** and **obs_quality_exc** options.

   * IODA2NC configuration:

     * Updates default **metadata_map** setting.

   * Unstructured Grid input files:

     * Adds new ``UGridConfig_lfric`` and ``UGridConfig_mpas``
       to define model-specific metadata information.

.. dropdown:: Output line type changes since version 11.1.0

   MET Version 12.0.0 adds or modifies the following ASCII output line types:

   * Matched Pair (**MPR**) line type from Point-Stat:

     * Renames existing CLIMO_MEAN, CLIMO_STDEV, and CLIMO_CDF columns
       as OBS_CLIMO_MEAN, OBS_CLIMO_STDEV, OBS_CLIMO_CDF, respectively.
     * Adds new FCST_CLIMO_MEAN and FCST_CLIMO_STDEV columns to the
       end of the line.

   * Vector Partial Sums (**VL1L2**) line type from Point-Stat and Grid-Stat:

     * Adds new TOTAL_DIR, DIR_ME, DIR_MAE, and DIR_MSE columns to
       the end of the line containing wind direction statistics.

   * Vector Anomaly Partial Sums (**VAL1L2**) line type from Point-Stat and Grid-Stat:

     * Adds new TOTAL_DIR, DIRA_ME, DIRA_MAE, and DIRA_MSE columns to
       the end of the line containing wind direction statistics.

   * Vector Continuous Statisics (**VCNT**) line type from Point-Stat and Grid-Stat:

     * Adds new TOTAL_DIR, DIR_ME, DIR_ME_BCL, DIR_ME_BCU, DIR_MAE,
       DIR_MAE_BCL, DIR_MAE_BCU, DIR_MSE, DIR_MSE_BCL, DIR_MSE_BCU,
       DIR_RMSE, DIR_RMSE_BCL, and DIR_RMSE_BCU columns to the end
       of the line containing wind direction statistics.
     * Note that no bootstrap confidence intervals are currently
       being computed so the BCL and BCU columns all contain NA.

   * Stable Equitable Error in Probability Space (**SEEPS**) line type
     from Point-Stat and Grid-Stat:

     * Renames existing S12, S13, S21, S23, S31, and S32 columns
       as ODFL, ODFH, OLFD, OLFH, OHFD, and OHFL, respectively,
       where O and F are for observation and forecast and
       D, L, and H are for dry, light, and heavy precipitation.

   * Observation Rank (**ORANK**) line type from Ensemble-Stat:

     * Renames existing CLIMO_MEAN and CLIMO_STDEV columns
       as OBS_CLIMO_MEAN and OBS_CLIMO_STDEV, respectively.
     * Adds new FCST_CLIMO_MEAN and FCST_CLIMO_STDEV columns to the
       end of the line.

   * Ensemble Continuous Statistics (**ECNT**) line type from Ensemble-Stat:

     * Adds new IGN_CONV_OERR and IGN_CORR_OERR columns to the
       end of the line.

   * MODE Continuous Statistics (**MODE CTS**) line type from MODE:

     * Adds new SEDS, EDI, SEDI, and BAGSS columns to the end of the line.
