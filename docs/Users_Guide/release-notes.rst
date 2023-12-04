***********************
MET Release Information
***********************

MET Release Notes
=================

When applicable, release notes are followed by the GitHub issue number which describes the bugfix,
enhancement, or new feature (`MET GitHub issues <https://github.com/dtcenter/MET/issues>`_).
Important issues are listed **in bold** for emphasis.

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
