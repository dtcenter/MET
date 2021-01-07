MET release notes
_________________

When applicable, release notes are followed by the GitHub issue number which
describes the bugfix, enhancement, or new feature: `MET Git-Hub issues. <https://github.com/dtcenter/MET/issues>`_

Version |version| release notes (|release_date|)
------------------------------------------------

Version `10.0-beta2 <https://github.com/dtcenter/MET/projects/24>`_ release notes (20201207)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* Repository and build:

   * Switch from tagging releases as met-X.Y.Z to vX.Y.Z instead (`#1541 <http://github.com/dtcenter/MET/issues/1541>`_).
   * Switch to consistent vX.Y.Z version numbering, from v10.0 to v10.0.0 (`#1590 <http://github.com/dtcenter/MET/issues/1590>`_).
   * Run the nightly build as the shared met_test user (`#1116 <http://github.com/dtcenter/MET/issues/1116>`_).

* Documentation:
  
   * Update comments at the top of each MET config file directing users to the MET User's Guide (`#1598 <http://github.com/dtcenter/MET/issues/1598>`_).
   * Migrate content from README and README_TC in data/config to the MET User's Guide (`#1474 <http://github.com/dtcenter/MET/issues/1474>`_).
   * Add version selector to the Sphinx documentation page (`#1461 <http://github.com/dtcenter/MET/issues/1461>`_).
   * Make bolding consistent across the documentation (`#1458 <http://github.com/dtcenter/MET/issues/1458>`_).
   * Implement hanging indents for references (`#1457 <http://github.com/dtcenter/MET/issues/1457>`_).
   * Correct typos and spelling errors (`#1456 <http://github.com/dtcenter/MET/issues/1456>`_).

* Library code:
  
   * Enhance support for rotated latlon grids and update related documentation (`#1574 <http://github.com/dtcenter/MET/issues/1574>`_).
   * Update the NCL-derived color tables (`#1568 <http://github.com/dtcenter/MET/issues/1568>`_).
   * Parse the -v and -log options prior to application-specific command line options (`#1527 <http://github.com/dtcenter/MET/issues/1527>`_).
   * Treat gridded fields of entirely missing data as missing files and fix python embedding to call common data processing code (`#1494 <http://github.com/dtcenter/MET/issues/1494>`_).
  
* Application code:
  
   * Overhaul the plot_point_obs tool to make it highly configurable (`#213 <http://github.com/dtcenter/MET/issues/213>`_, `#1528 <http://github.com/dtcenter/MET/issues/1528>`_, and `#1052 <http://github.com/dtcenter/MET/issues/1052>`_).
   * Add new ioda2nc tool (`#1355 <http://github.com/dtcenter/MET/issues/1355>`_).
   * Incremental development toward the Multivariate MODE tool (`#1282 <http://github.com/dtcenter/MET/issues/1282>`_, `#1284 <http://github.com/dtcenter/MET/issues/1284>`_, and `#1290 <http://github.com/dtcenter/MET/issues/1290>`_).

Version `10.0-beta1 <https://github.com/dtcenter/MET/projects/20>`_ release notes (20201022)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* Bugfixes since the version 9.1 release:
  
   * Clarify madis2nc error messages (`#1409 <http://github.com/dtcenter/MET/issues/1409>`_).
     
   * Fix tc_gen lead window filtering option (`#1465 <http://github.com/dtcenter/MET/issues/1465>`_).
     
   * Clarify error messages for Xarray python embedding (`#1472 <http://github.com/dtcenter/MET/issues/1472>`_).
     
   * Add support for Gaussian grids with python embedding (`#1477 <http://github.com/dtcenter/MET/issues/1477>`_).
     
   * Fix ASCII file list parsing logic (`#1484 <http://github.com/dtcenter/MET/issues/1484>`_ and `#1508 <http://github.com/dtcenter/MET/issues/1508>`_).

* Repository and build:
  
   * Migrate GitHub respository from the NCAR to DTCenter organization (`#1462 <http://github.com/dtcenter/MET/issues/1462>`_).
     
   * Add a GitHub pull request template (`#1516 <http://github.com/dtcenter/MET/issues/1516>`_).
     
   * Resolve warnings from autoconf (`#1498 <http://github.com/dtcenter/MET/issues/1498>`_).
     
   * Restructure nightly builds (`#1510 <http://github.com/dtcenter/MET/issues/1510>`_).

* Documentation:
  
   * Enhance and update documentation (`#1459 <http://github.com/dtcenter/MET/issues/1459>`_ and `#1460 <http://github.com/dtcenter/MET/issues/1460>`_).

* Library code:
  
   * Refine log messages when verifying probabilities (`#1502 <http://github.com/dtcenter/MET/issues/1502>`_).
     
   * Enhance NetCDF library code to support additional data types (`#1492 <http://github.com/dtcenter/MET/issues/1492>`_ and `#1493 <http://github.com/dtcenter/MET/issues/1493>`_).

* Application code:
  
   * Update point_stat log messages (`#1514 <http://github.com/dtcenter/MET/issues/1514>`_).
     
   * Enhance point2grid to support additional NetCDF point observation data sources (`#1345 <http://github.com/dtcenter/MET/issues/1345>`_, `#1509 <http://github.com/dtcenter/MET/issues/1509>`_, and `#1511 <http://github.com/dtcenter/MET/issues/1511>`_).

