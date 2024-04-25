***********************
MET Release Information
***********************

MET Release Notes
=================

When applicable, release notes are followed by the GitHub issue number which describes the bugfix,
enhancement, or new feature (`MET GitHub issues <https://github.com/dtcenter/MET/issues>`_).
Important issues are listed **in bold** for emphasis.

MET Version 12.0.0-beta4 Release Notes (20240417)
-------------------------------------------------

  .. dropdown:: Repository, build, and test

     * Add GitHub action to run SonarQube for MET pull requests and feature branches (`#2379 <https://github.com/dtcenter/MET/issues/2379>`_).
     * Move namespace specifications below include directives (`#2696 <https://github.com/dtcenter/MET/issues/2696>`_).
     * Enhance GitHub action compilation options and testing workflows (`#2815 <https://github.com/dtcenter/MET/issues/2815>`_).
     * Fix the SonarQube findings for MET version 12.0.0 (`#2673 <https://github.com/dtcenter/MET/issues/2673>`_).
     * SonarQube: Replace "enum" to "enum class" (`#2830 <https://github.com/dtcenter/MET/issues/2830>`_).

  .. dropdown:: Bugfixes

     * **Bugfix: Fix the TC-Diag and TC-RMW tools to correctly handle the range and azimuth settings in range/azimuth grids** (`#2833 <https://github.com/dtcenter/MET/issues/2833>`_).

  .. dropdown:: Enhancements

     * **Refine configuration options for defining bins in the verification of probabilistic forecasts** (`#2280 <https://github.com/dtcenter/MET/issues/2280>`_).
     * **Add new wind direction verification statistics for RMSE, Bias, and MAE** (`#2395 <https://github.com/dtcenter/MET/issues/2395>`_).
     * Add new ECNT statistics that incorporate observational uncertainty as advised in Ferro (2017) (`#2583 <https://github.com/dtcenter/MET/issues/2583>`_).
     * Enhance ASCII2NC to support IABP/IPAB Arctic and Antarctic drifting buoy observations (`#2654 <https://github.com/dtcenter/MET/issues/2654>`_).
     * Enhance Multivariate MODE to read input data only once rather than multiple times (`#2707 <https://github.com/dtcenter/MET/issues/2707>`_).
     * Enhance the calculation of RPSS to support starting from probabilistic data (`#2786 <https://github.com/dtcenter/MET/issues/2786>`_).
     * Add convex hull to MODE output (`#2819 <https://github.com/dtcenter/MET/issues/2819>`_).

MET Version 12.0.0-beta3 Release Notes (20240207)
-------------------------------------------------

  .. dropdown:: Repository, build, and test

     * Enhance METbaseimage to install SciPy Python package needed by the MET TC-Diag tool (`METbaseimage#20 <https://github.com/dtcenter/METbaseimage/issues/20>`_).
     * Remove the SonarQube token from the properties file (`#2757 <https://github.com/dtcenter/MET/issues/2757>`_).
     * Repository cleanup of stale code and configuration consistency (`#2776 <https://github.com/dtcenter/MET/issues/2776>`_).
     * Add new example installation configuration files for Intel compiler users (`#2785 <https://github.com/dtcenter/MET/issues/2785>`_).
     * Update GitHub actions workflows to switch from node 16 to node 20 (`#2796 <https://github.com/dtcenter/MET/issues/2796>`_).

  .. dropdown:: Bugfixes

     * Bugfix: Fix support for NSIDC v4 Climate Data Record data on Polar Stereographic grids in CF-compliant NetCDF files (`#2652 <https://github.com/dtcenter/MET/issues/2652>`_).
     * Bugfix: Fix Python embedding failure when providing a single point observation (`#2755 <https://github.com/dtcenter/MET/issues/2755>`_).
     * Bugfix: Fix MET to compile without the optional `--enable-python` configuration option (`#2760 <https://github.com/dtcenter/MET/issues/2760>`_).
     * Bugfix: Fix the parsing of level values for GRIB2 template 4.48 data (`#2782 <https://github.com/dtcenter/MET/issues/2782>`_).

  .. dropdown:: Enhancements

     * **Add support for native WRF output files already on pressure levels** (`#2547 <https://github.com/dtcenter/MET/issues/2547>`_).
     * Enhance ASCII2NC to read ISMN point observations of soil moisture and temperature (`#2701 <https://github.com/dtcenter/MET/issues/2701>`_).
     * **Major enhancements to multivariate MODE** (`#2745 <https://github.com/dtcenter/MET/issues/2745>`_).
     * Enhance TC-Diag to use tc_diag_driver version 0.11.0 (`#2769 <https://github.com/dtcenter/MET/issues/2769>`_).
     * Switch from writing temporary Python files in NetCDF to JSON and NumPy serialization (`#2772 <https://github.com/dtcenter/MET/issues/2772>`_).
     * Revise the use of temporary files in PB2NC (`#2792 <https://github.com/dtcenter/MET/issues/2792>`_).
     * Enhance MET to make warnings messages about time differences configurable (`#2801 <https://github.com/dtcenter/MET/issues/2801>`_).
     * Enhance Stat-Analysis to apply the `-set_hdr` option to filter jobs (`#2805 <https://github.com/dtcenter/MET/issues/2805>`_).
     * Enhance MET to parse LAEA grids from the MET NetCDF file format (`#2809 <https://github.com/dtcenter/MET/issues/2809>`_).

MET Version 12.0.0-beta2 Release Notes (20231117)
-------------------------------------------------

  .. dropdown:: Repository, build, and test

     * Enhance METbaseimage to compile the ecKit and Atlas libraries (`METbaseimage#13 <https://github.com/dtcenter/METbaseimage/issues/13>`_).
     * Enhance METbaseimage to install the YAML Python package (`METbaseimage#15 <https://github.com/dtcenter/METbaseimage/issues/15>`_).
     * **Enhance MET to compile and link against the Proj library** (`#2669 <https://github.com/dtcenter/MET/issues/2669>`_).
     * **Enhance MET to compile and link against the Atlas and ecKit libraries** (`#2574 <https://github.com/dtcenter/MET/issues/2574>`_).
     * **Enhance "compile_MET_all.sh" to support the new Intel oneAPI compilers and upgrade dependent library versions as needed** (`#2611 <https://github.com/dtcenter/MET/issues/2611>`_).
     * Upgrade SonarQube server version from 9.8 to 10.2 (`#2689 <https://github.com/dtcenter/MET/issues/2689>`_).
     * Update the token for upgraded SonarQube server (`#2702 <https://github.com/dtcenter/MET/issues/2702>`_).

  .. dropdown:: Bugfixes

     * Bugfix: Correct the usage statement for Point2Grid (`#2666 <https://github.com/dtcenter/MET/issues/2666>`_).
     * Bugfix: Investigate unexpected number of derived HPBL observations in PB2NC (`#2687 <https://github.com/dtcenter/MET/issues/2687>`_).
     * Bugfix: Fix the Point-Stat CNT header line typo causing duplicate "SI_BCL" column names (`#2730 <https://github.com/dtcenter/MET/issues/2730>`_).

  .. dropdown:: Enhancements

     * Documentation: Make Headers Consistent in All MET Guides (`#2716 <https://github.com/dtcenter/MET/issues/2716>`_).
     * Document the use of temporary files in MET and reduce it as much as reasonably possible (`#2690 <https://github.com/dtcenter/MET/issues/2690>`_).
     * **Eliminate the use of temporary files in the vx_config library** (`#2691 <https://github.com/dtcenter/MET/issues/2691>`_).
     * **Add support for NetCDF files following the UGRID convention** (`#2231 <https://github.com/dtcenter/MET/issues/2231>`_).
     * Enhance TC-Pairs to include storm diagnostics in consensus track output (`#2476 <https://github.com/dtcenter/MET/issues/2476>`_).
     * Refine TC-Pairs consensus diagnostics configuration options (`#2699 <https://github.com/dtcenter/MET/issues/2699>`_).
     * **Enhance TC-Diag to actually compute and write diagnostics** (`#2550 <https://github.com/dtcenter/MET/issues/2550>`_).
     * **Enhance MODE to use OpenMP to make the convolution step faster** (`#2724 <https://github.com/dtcenter/MET/issues/2724>`_).
     * Enhance Multivariate MODE to change the default "merge_flag" setting to NONE (`#2708 <https://github.com/dtcenter/MET/issues/2708>`_).
     * **Enhance Multivariate MODE to support differing numbers of forecast and observation input fields** (`#2706 <https://github.com/dtcenter/MET/issues/2706>`_).
     * Fix the SonarQube findings for MET v12.0 (`#2673 <https://github.com/dtcenter/MET/issues/2673>`_).

MET Version 12.0.0-beta1 Release Notes (20230915)
-------------------------------------------------

  .. dropdown:: Repository, build, and test

     * Refine the METbaseimage to compile dependent libraries from a single tar file (`METbaseimage#9 <https://github.com/dtcenter/METbaseimage/issues/9>`_).
     * Update METbaseimage to complete the transition to the Debian 12 (bookworm) base image (`METbaseimage#12 <https://github.com/dtcenter/METbaseimage/issues/12>`_).
     * Update the ``install_met_env.generic`` configuration file (`#2643 <https://github.com/dtcenter/MET/issues/2643>`_).
     * Switch SonarQube server (mandan to needham) (`#2650 <https://github.com/dtcenter/MET/issues/2650>`_).
     * Update GitHub issue and pull request templates to reflect the current development workflow details (`#2659 <https://github.com/dtcenter/MET/issues/2659>`_).
     * Update the unit test diff logic to handle SEEPS, SEEPS_MPR, and MODE CTS line type updates (`#2665 <https://github.com/dtcenter/MET/issues/2665>`_).

  .. dropdown:: Bugfixes

     * Bugfix: Refine support for coordinate dimensions in CF-compliant NetCDF files (`#2638 <https://github.com/dtcenter/MET/issues/2638>`_).
     * Bugfix: Fix logic for computing the 100-th percentile (`#2644 <https://github.com/dtcenter/MET/issues/2644>`_).

  .. dropdown:: Enhancements

     * Refine TC-Diag logic for handling missing data (`#2609 <https://github.com/dtcenter/MET/issues/2609>`_).
     * **Update ioda2nc to support version 3 IODA files** (`#2640 <https://github.com/dtcenter/MET/issues/2640>`_).
     * **Enhance MODE CTS output file to include missing categorical statistics, including SEDI** (`#2648 <https://github.com/dtcenter/MET/issues/2648>`_).
     * **Enhance MET to compile and link against the Proj library** (`#2669 <https://github.com/dtcenter/MET/issues/2669>`_).
     * Change the default setting for the model string from "WRF" to "FCST" in the default MET configuration files (`#2682 <https://github.com/dtcenter/MET/issues/2682>`_).

MET Upgrade Instructions
========================

MET Version 12.0.0 Upgrade Instructions
---------------------------------------

* MET Version 12.0.0 introduces one new required and two new optional dependencies:

  * The required `Proj <https://proj.org/>`_ library dependency was added in the 12.0.0-beta1 development cycle (`#2669 <https://github.com/dtcenter/MET/issues/2669>`_).
  * The optional `Atlas <https://sites.ecmwf.int/docs/atlas/>`_ library dependency was added in the 12.0.0-beta2 development cycle (`#2574 <https://github.com/dtcenter/MET/issues/2574>`_).
  * The optional `ecKit <https://github.com/ecmwf/eckit>`_ library dependency was added in the 12.0.0-beta2 development cycle (`#2574 <https://github.com/dtcenter/MET/issues/2574>`_).

* Note that the `#2833 <https://github.com/dtcenter/MET/issues/2833>`_ bugfix affects all previously generated output from the TC-Diag and TC-RMW tools.
