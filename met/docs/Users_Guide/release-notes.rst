MET release notes
_________________

When applicable, release notes are followed by the GitHub issue number which
describes the bugfix, enhancement, or new feature: `MET GitHub issues. <https://github.com/dtcenter/MET/issues>`_

Version |version| release notes (|release_date|)
------------------------------------------------


Version `10.0.0-beta5 <https://github.com/dtcenter/MET/projects/33>`_ release notes (20210426)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* Bugfixes:

   * Fix Grid-Diag bug when reading the same variable name from multiple data sources (`#1694 <http://github.com/dtcenter/MET/issues/1694>`_).
   * Fix Stat-Analysis failure when aggregating ECNT lines (`#1706 <http://github.com/dtcenter/MET/issues/1706>`_).
   * Fix intermittent PB2NC segfault when deriving PBL (`#1715 <http://github.com/dtcenter/MET/issues/1715>`_).
   * Fix parsing error for floating point percentile thresholds, like ">SFP33.3" (`#1716 <http://github.com/dtcenter/MET/issues/1716>`_).
   * Fix ascii2nc to handle bad records in little_r format (`#1737 <http://github.com/dtcenter/MET/issues/1737>`_).

* Documentation:

   * Migrate the MET documetation to Read the Docs (`#1649 <http://github.com/dtcenter/MET/issues/1649>`_).

* Library code:

   * Miscellaneous:

      * Add support for climatological probabilities for complex CDP thresholds, like >=CDP33&&<=CDP67 (`#1705 <http://github.com/dtcenter/MET/issues/1705>`_).

   * NetCDF library:

      * Extend CF-compliant NetCDF file support when defining the time dimension as a time string (`#1755 <http://github.com/dtcenter/MET/issues/1755>`_).

   * Python embedding:

      * Replace the pickle format for temporary python files with NetCDF for gridded data (`#1319 <http://github.com/dtcenter/MET/issues/1319>`_, `#1697 <http://github.com/dtcenter/MET/issues/1697>`_).
      * Replace the pickle format for temporary python files with ASCII for point observations in ascii2nc and matched pair data in Stat-Analysis (`#1319 <http://github.com/dtcenter/MET/issues/1319>`_, `#1700 <http://github.com/dtcenter/MET/issues/1700>`_).
      * Enhance python embedding to support the "grid" being defined as a named grid or specification string (`#1471 <http://github.com/dtcenter/MET/issues/1471>`_).
      * Enhance the python embedding library code to parse python longlong variables as integers to make the python embedding scripts less particular (`#1747 <http://github.com/dtcenter/MET/issues/1747>`_).
      * Fix the read_ascii_mpr.py python embedding script to pass all 37 columns of MPR data to Stat-Analysis (`#1620 <http://github.com/dtcenter/MET/issues/1620>`_).
      * Fix the read_tmp_dataplane.py python embedding script to handle the fill value correctly (`#1753 <http://github.com/dtcenter/MET/issues/1753>`_).

* Application code:

   * Point-Stat and Grid-Stat Tools:

      * Enhance Point-Stat and Grid-Stat by adding mpr_column and mpr_thresh configuration options to filter out matched pairs based on large fcst, obs, and climo differences (`#1575 <http://github.com/dtcenter/MET/issues/1575>`_).

   * Stat-Analysis Tool:

      * Enhance Stat-Analysis to process multiple output thresholds and write multiple output line types in a single aggregate_stat job (`#1735 <http://github.com/dtcenter/MET/issues/1735>`_).
      * Enhance Stat-Analysis to skip writing job output to the logfile when the -out_stat option is provided (`#1736 <http://github.com/dtcenter/MET/issues/1736>`_).
      * Enhance Stat-Analysis by adding a -column_exc job command option to exclude lines based on string values (`#1733 <http://github.com/dtcenter/MET/issues/1733>`_).

   * MET-TC Tools:
   
      * Fix TC-Pairs to report the correct number of lines read from input track data files (`#1725 <http://github.com/dtcenter/MET/issues/1725>`_).
      * Enhance TC-Stat by adding a -column_exc job command option to exclude lines based on string values (`#1733 <http://github.com/dtcenter/MET/issues/1733>`_).
      * Enhance the TC-Gen matching logic and update several config options to support its S2S application (`#1714 <http://github.com/dtcenter/MET/issues/1714>`_).

Version `10.0.0-beta4 <https://github.com/dtcenter/MET/projects/26>`_ release notes (20210302)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* Bugfixes:

   * Fix the set_attr_accum option to set the accumulation time instead of the lead time (`#1646 <http://github.com/dtcenter/MET/issues/1646>`_).
   * Correct the time offset for tests in unit_plot_data_plane.xml (`#1677 <http://github.com/dtcenter/MET/issues/1677>`_).

* Repository and build:

   * Enhance the sample plotting R-script to read output from different versions of MET (`#1653 <http://github.com/dtcenter/MET/issues/1653>`_).

* Library code:

   * Miscellaneous:

      * Update GRIB1/2 table entries for the MXUPHL, MAXREF, MAXUVV, and MAXDVV variables (`#1658 <http://github.com/dtcenter/MET/issues/1658>`_).
      * Update the Air Force GRIB tables to reflect current AF usage (`#1519 <http://github.com/dtcenter/MET/issues/1519>`_).
      * Enhance the DataLine::get_item() error message to include the file name, line number, and column (`#1429 <http://github.com/dtcenter/MET/issues/1429>`_).

   * NetCDF library:

      * Add support for the NetCDF-CF conventions time bounds option (`#1657 <http://github.com/dtcenter/MET/issues/1657>`_).
      * Error out when reading CF-compliant NetCDF data with incomplete grid definition (`#1454 <http://github.com/dtcenter/MET/issues/1454>`_).
      * Reformat and simplify the magic_str() printed for NetCDF data files (`#1655 <http://github.com/dtcenter/MET/issues/1655>`_).

   * Statistics computations:

      * Add support for the Hersbach CRPS algorithm by add new columns to the ECNT line type (`#1450 <http://github.com/dtcenter/MET/issues/1450>`_).
      * Enhance MET to derive the Hersbach CRPSCL_EMP and CRPSS_EMP statistics from a single deterministic reference model (`#1685 <http://github.com/dtcenter/MET/issues/1685>`_).
      * Correct the climatological CRPS computation to match the NOAA/EMC VSDB method (`#1451 <http://github.com/dtcenter/MET/issues/1451>`_).
      * Modify the climatological Brier Score computation to match the NOAA/EMC VSDB method (`#1684 <http://github.com/dtcenter/MET/issues/1684>`_).

* Application code:

   * ASCII2NC and Point2Grid:

      * Enhance ascii2nc and point2grid to gracefully process zero input observations rather than erroring out (`#1630 <http://github.com/dtcenter/MET/issues/1630>`_).

   * Point-Stat Tool:

      * Enhance the validation of masking regions to check for non-unique masking region names (`#1439 <http://github.com/dtcenter/MET/issues/1439>`_).
      * Print the Point-Stat rejection code reason count log messages at verbosity level 2 for zero matched pairs (`#1644 <http://github.com/dtcenter/MET/issues/1644>`_).
      * Add detailed log messages to Point-Stat when discarding observations (`#1588 <http://github.com/dtcenter/MET/issues/1588>`_).

   * Stat-Analysis Tool:

      * Add -fcst_init_inc/_exc and -fcst_valid_inc/_exc job command filtering options to Stat-Analysis (`#1135 <http://github.com/dtcenter/MET/issues/1135>`_).

   * MODE Tool:

      * Update the MODE AREA_RATIO output column to list the forecast area divided by the observation area (`#1643 <http://github.com/dtcenter/MET/issues/1643>`_).

Version `10.0.0-beta3 <https://github.com/dtcenter/MET/projects/25>`_ release notes (20210127)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* Bugfixes:

   * Correct the climatological CDF values in the Grid-Stat NetCDF matched pairs output files, and correct the climatological probability values for climatgological distribution percentile (CDP) threshold types (`#1638 <http://github.com/dtcenter/MET/issues/1638>`_).
   * Apply the GRIB ensemble filtering option (GRIB_ens) whenever specified by the user (`#1604 <http://github.com/dtcenter/MET/issues/1604>`_).

* Repository and build:

   * Update the MET unit test logic by unsetting environment variables after each test to provide a clean environment for the next (`#1624 <http://github.com/dtcenter/MET/issues/1624>`_).

* Documentation:
  
   * Update the Grid-Diag documentation to clarify the -data command line option (`#1611 <http://github.com/dtcenter/MET/issues/1611>`_).
   * Documentation updates to correct typos and apply consistent formatting (`#1455 <http://github.com/dtcenter/MET/issues/1455>`_).
   * Correct the definition of H_RATE and PODY in MET User's Guide Appendix C (`#1631 <http://github.com/dtcenter/MET/issues/1631>`_).

* Library code:
  
   * When reading MET NetCDF files, parse the "init_time" and "valid_time" attributes (`#1346 <http://github.com/dtcenter/MET/issues/1346>`_).
   * Python embedding enhancements:

     * Complete support for Python XArray embedding (`#1534 <http://github.com/dtcenter/MET/issues/1534>`_).
     * Correct error messages from Python embedding (`#1473 <http://github.com/dtcenter/MET/issues/1473>`_).
  
* Application code:

   * Enhance Plot-Point-Obs to support regridding in the config file (`#1627 <http://github.com/dtcenter/MET/issues/1627>`_).
   * Update ASCII2NC and Point2Grid to create empty output files for zero input observations instead of erroring out (`#1630 <http://github.com/dtcenter/MET/issues/1630>`_).
   * Point2Grid Tool:

     * Improve the Point2Grid runtime performance (`#1421 <http://github.com/dtcenter/MET/issues/1421>`_).
     * Process point observations by variable name instead of GRIB code (`#1408 <http://github.com/dtcenter/MET/issues/1408>`_).
     * Support the 2-dimensional time variable in Himawari data files (`#1580 <http://github.com/dtcenter/MET/issues/1580>`_).

   * TC-Gen Tool:

     * Overhaul the Tropical Cyclone genesis matching logic, add the development and operational scoring algorithms, and add many config file options (`#1448 <http://github.com/dtcenter/MET/issues/1448>`_).
     * Add config file options to filter data by initialization time (init_inc and init_exc) and hurricane basin (basin_mask) (`#1626 <http://github.com/dtcenter/MET/issues/1626>`_).
     * Add the genesis matched pair (GENMPR) output line type (`#1597 <http://github.com/dtcenter/MET/issues/1597>`_).
     * Add a gridded NetCDF output file with counts for genesis events and track points (`#1430 <http://github.com/dtcenter/MET/issues/1430>`_).

Version `10.0.0-beta2 <https://github.com/dtcenter/MET/projects/24>`_ release notes (20201207)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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

Version `10.0.0-beta1 <https://github.com/dtcenter/MET/projects/20>`_ release notes (20201022)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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
