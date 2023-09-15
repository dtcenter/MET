***********************
MET Release Information
***********************

MET Release Notes
=================

When applicable, release notes are followed by the GitHub issue number which describes the bugfix,
enhancement, or new feature (`MET GitHub issues <https://github.com/dtcenter/MET/issues>`_).
Important issues are listed **in bold** for emphasis.

MET Version 12.0.0-beta1 release notes (20230915)
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

MET Version 12.0.0 upgrade instructions
---------------------------------------

* MET Version 12.0.0 introduces three new required dependencies:

  * The `Proj <https://proj.org/>`_ library dependency was added in the beta1 development cycle (`#2669 <https://github.com/dtcenter/MET/issues/2669>`_).
  * The `Atlas <https://sites.ecmwf.int/docs/atlas/>`_ library dependency will be added in the beta2 development cycle (`#2574 <https://github.com/dtcenter/MET/issues/2574>`_).
  * The `ecKit <https://github.com/ecmwf/eckit>`_ library dependency will be added in the beta2 development cycle (`#2574 <https://github.com/dtcenter/MET/issues/2574>`_).
