MET release notes
_________________

When applicable, release notes are followed by the GitHub issue number which
describes the bugfix, enhancement, or new feature:
`MET GitHub issues. <https://github.com/dtcenter/MET/issues>`_

MET Version |version| release notes (|release_date|)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* Bugfixes:

   * **Fix Stat-Analysis aggregation of the neighborhood statistics line types** (`#2271 <http://github.com/dtcenter/MET/issues/2271>`_)
   * Fix support for int64 NetCDF variable types (`#2123 <http://github.com/dtcenter/MET/issues/2123>`_)
   * Fix regression test differences in pb2nc and ioda2nc output (`#2102 <http://github.com/dtcenter/MET/issues/2102>`_)

MET Version 10.0.1 release notes (2021-12-01)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* Bugfixes:

   * **Fix MTD to compute the CDIST_TRAVELLED value correctly** (`#1976 <http://github.com/dtcenter/MET/issues/1976>`_)
   * **Fix MADIS2NC to handle the 2016 changes to its format** (`#1936 <http://github.com/dtcenter/MET/issues/1936>`_).
   * Fix TC-Stat event equalization logic to include any model name requested using -amodel (`#1932 <http://github.com/dtcenter/MET/issues/1932>`_).
   * Fix Ensemble-Stat failure when verifying against gridded ECMWF GRIB1 files (`#1879 <http://github.com/dtcenter/MET/issues/1879>`_).
   * Fix python embedding when using a named grid with MET_PYTHON_EXE set (`#1798 <http://github.com/dtcenter/MET/issues/1798>`_).
   * Fix the plot_tcmpr.R script to support specifying a directory with -lookin (`#1872 <http://github.com/dtcenter/MET/issues/1872>`_).
   * Fix the plot_tcmpr.R script to plot extra-tropical cyclone tracks not verified against BEST tracks (`#1801 <http://github.com/dtcenter/MET/issues/1801>`_).
   * Fix the Plot-Point-Obs documentation to remove the duplicate configuration section (`#1789 <http://github.com/dtcenter/MET/issues/1789>`_).

MET Version 10.0.0 release notes (2021-05-10)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* Repository and build:
  
   * **Migrate GitHub respository from the NCAR to DTCenter organization** (`#1462 <http://github.com/dtcenter/MET/issues/1462>`_).
   * **Switch to consistent vX.Y.Z version numbering, from v10.0 to v10.0.0** (`#1590 <http://github.com/dtcenter/MET/issues/1590>`_).
   * Switch from tagging releases as met-X.Y.Z to vX.Y.Z instead (`#1541 <http://github.com/dtcenter/MET/issues/1541>`_).
   * Add a GitHub pull request template (`#1516 <http://github.com/dtcenter/MET/issues/1516>`_).
   * Resolve warnings from autoconf (`#1498 <http://github.com/dtcenter/MET/issues/1498>`_).
   * Restructure nightly builds (`#1510 <http://github.com/dtcenter/MET/issues/1510>`_).
   * Update the MET unit test logic by unsetting environment variables after each test to provide a clean environment for the next (`#1624 <http://github.com/dtcenter/MET/issues/1624>`_).
   * Run the nightly build as the shared met_test user (`#1116 <http://github.com/dtcenter/MET/issues/1116>`_).
   * Correct the time offset for tests in unit_plot_data_plane.xml (`#1677 <http://github.com/dtcenter/MET/issues/1677>`_).
   * Enhance the sample plotting R-script to read output from different versions of MET (`#1653 <http://github.com/dtcenter/MET/issues/1653>`_).
   * Update the default configuration options to compile the development code with the debug (-g) option and the production code without it (`#1788 <http://github.com/dtcenter/MET/issues/1788>`_).
   * Update MET to compile using GCC version 10 (`#1552 <https://github.com/dtcenter/MET/issues/1552>`_).
   * Update MET to compile using PGI version 20 (`#1317 <https://github.com/dtcenter/MET/issues/1317>`_).
     
* Documentation:

   * **Migrate the MET documentation to Read the Docs** (`#1649 <http://github.com/dtcenter/MET/issues/1649>`_).
   * Enhance and update documentation (`#1459 <http://github.com/dtcenter/MET/issues/1459>`_ and `#1460 <http://github.com/dtcenter/MET/issues/1460>`_, and `#1731 <http://github.com/dtcenter/MET/issues/1731>`_).
   * Enhance the python embedding documentation (`#1468 <http://github.com/dtcenter/MET/issues/1468>`_).
   * Document the supported grid definition templates (`#1469 <http://github.com/dtcenter/MET/issues/1469>`_).
   * Update comments at the top of each MET config file directing users to the MET User's Guide (`#1598 <http://github.com/dtcenter/MET/issues/1598>`_).
   * Migrate content from README and README_TC in data/config to the MET User's Guide (`#1474 <http://github.com/dtcenter/MET/issues/1474>`_).
   * Add version selector to the Sphinx documentation page (`#1461 <http://github.com/dtcenter/MET/issues/1461>`_).
   * Make bolding consistent across the documentation (`#1458 <http://github.com/dtcenter/MET/issues/1458>`_).
   * Implement hanging indents for references (`#1457 <http://github.com/dtcenter/MET/issues/1457>`_).
   * Correct typos and spelling errors (`#1456 <http://github.com/dtcenter/MET/issues/1456>`_).
   * Update the Grid-Diag documentation to clarify the -data command line option (`#1611 <http://github.com/dtcenter/MET/issues/1611>`_).
   * Documentation updates to correct typos and apply consistent formatting (`#1455 <http://github.com/dtcenter/MET/issues/1455>`_).
   * Correct the definition of H_RATE and PODY in MET User's Guide Appendix C (`#1631 <http://github.com/dtcenter/MET/issues/1631>`_).

* Library code:

   * Bugfixes:

      * Apply the GRIB ensemble filtering option (GRIB_ens) whenever specified by the user (`#1604 <http://github.com/dtcenter/MET/issues/1604>`_).
      * Fix the set_attr_accum option to set the accumulation time instead of the lead time (`#1646 <http://github.com/dtcenter/MET/issues/1646>`_).
      * Fix ASCII file list parsing logic (`#1484 <http://github.com/dtcenter/MET/issues/1484>`_ and `#1508 <http://github.com/dtcenter/MET/issues/1508>`_).
      * Fix parsing error for floating point percentile thresholds, like ">SFP33.3" (`#1716 <http://github.com/dtcenter/MET/issues/1716>`_).

   * Python embedding enhancements:

      * Note that the netCDF4 Python package is now required in place of the pickle package!
      * **Replace the pickle format for temporary python files with NetCDF for gridded data** (`#1319 <http://github.com/dtcenter/MET/issues/1319>`_, `#1697 <http://github.com/dtcenter/MET/issues/1697>`_).
      * **Replace the pickle format for temporary python files with ASCII for point observations in ascii2nc and matched pair data in Stat-Analysis** (`#1319 <http://github.com/dtcenter/MET/issues/1319>`_, `#1700 <http://github.com/dtcenter/MET/issues/1700>`_).
      * **Complete support for Python XArray embedding** (`#1534 <http://github.com/dtcenter/MET/issues/1534>`_).
      * Treat gridded fields of entirely missing data as missing files and fix python embedding to call common data processing code (`#1494 <http://github.com/dtcenter/MET/issues/1494>`_).
      * Clarify error messages for Xarray python embedding (`#1472 <http://github.com/dtcenter/MET/issues/1472>`_).
      * Add support for Gaussian grids with python embedding (`#1477 <http://github.com/dtcenter/MET/issues/1477>`_).
      * Correct error messages from python embedding (`#1473 <http://github.com/dtcenter/MET/issues/1473>`_).
      * Enhance to support the "grid" being defined as a named grid or specification string (`#1471 <http://github.com/dtcenter/MET/issues/1471>`_).
      * Enhance to parse python longlong variables as integers to make the python embedding scripts less particular (`#1747 <http://github.com/dtcenter/MET/issues/1747>`_).
      * Fix the read_ascii_mpr.py python embedding script to pass all 37 columns of MPR data to Stat-Analysis (`#1620 <http://github.com/dtcenter/MET/issues/1620>`_).
      * Fix the read_tmp_dataplane.py python embedding script to handle the fill value correctly (`#1753 <http://github.com/dtcenter/MET/issues/1753>`_).

   * Miscellaneous:

      * **Enhance support for rotated latlon grids and update related documentation** (`#1574 <http://github.com/dtcenter/MET/issues/1574>`_).
      * Parse the -v and -log options prior to application-specific command line options (`#1527 <http://github.com/dtcenter/MET/issues/1527>`_).
      * Update GRIB1/2 table entries for the MXUPHL, MAXREF, MAXUVV, and MAXDVV variables (`#1658 <http://github.com/dtcenter/MET/issues/1658>`_).
      * Update the Air Force GRIB tables to reflect current AF usage (`#1519 <http://github.com/dtcenter/MET/issues/1519>`_).
      * Enhance the DataLine::get_item() error message to include the file name, line number, and column (`#1429 <http://github.com/dtcenter/MET/issues/1429>`_).
   	* Add support for climatological probabilities for complex CDP thresholds, like >=CDP33&&<=CDP67 (`#1705 <http://github.com/dtcenter/MET/issues/1705>`_).
      * Update the NCL-derived color tables (`#1568 <http://github.com/dtcenter/MET/issues/1568>`_).

   * NetCDF library:

      * Enhance to support additional NetCDF data types (`#1492 <http://github.com/dtcenter/MET/issues/1492>`_ and `#1493 <http://github.com/dtcenter/MET/issues/1493>`_).
      * Add support for the NetCDF-CF conventions time bounds option (`#1657 <http://github.com/dtcenter/MET/issues/1657>`_).
      * Extend CF-compliant NetCDF file support when defining the time dimension as a time string (`#1755 <http://github.com/dtcenter/MET/issues/1755>`_).
      * Error out when reading CF-compliant NetCDF data with incomplete grid definition (`#1454 <http://github.com/dtcenter/MET/issues/1454>`_).
      * Reformat and simplify the magic_str() printed for NetCDF data files (`#1655 <http://github.com/dtcenter/MET/issues/1655>`_).
      * Parse the "init_time" and "valid_time" attributes from MET NetCDF input files (`#1346 <http://github.com/dtcenter/MET/issues/1346>`_).

   * Statistics computations:

      * **Modify the climatological Brier Score computation to match the NOAA/EMC VSDB method** (`#1684 <http://github.com/dtcenter/MET/issues/1684>`_).
      * **Add support for the Hersbach CRPS algorithm by add new columns to the ECNT line type** (`#1450 <http://github.com/dtcenter/MET/issues/1450>`_).
      * Enhance MET to derive the Hersbach CRPSCL_EMP and CRPSS_EMP statistics from a single deterministic reference model (`#1685 <http://github.com/dtcenter/MET/issues/1685>`_).
      * Correct the climatological CRPS computation to match the NOAA/EMC VSDB method (`#1451 <http://github.com/dtcenter/MET/issues/1451>`_).
      * Refine log messages when verifying probabilities (`#1502 <http://github.com/dtcenter/MET/issues/1502>`_).

* Application code:

   * ASCII2NC Tool:

      * Fix to handle bad records in little_r format (`#1737 <http://github.com/dtcenter/MET/issues/1737>`_).
      * Create empty output files for zero input observations instead of erroring out (`#1630 <http://github.com/dtcenter/MET/issues/1630>`_).

   * MADIS2NC Tool:

      * Clarify various error messages (`#1409 <http://github.com/dtcenter/MET/issues/1409>`_).

   * PB2NC Tool:

      * Fix intermittent segfault when deriving PBL (`#1715 <http://github.com/dtcenter/MET/issues/1715>`_).

   * Point2Grid Tool:

      * **Support additional NetCDF point observation data sources** (`#1345 <http://github.com/dtcenter/MET/issues/1345>`_, `#1509 <http://github.com/dtcenter/MET/issues/1509>`_, and `#1511 <http://github.com/dtcenter/MET/issues/1511>`_).
      * Support the 2-dimensional time variable in Himawari data files (`#1580 <http://github.com/dtcenter/MET/issues/1580>`_).
      * Create empty output files for zero input observations instead of erroring out (`#1630 <http://github.com/dtcenter/MET/issues/1630>`_).
      * Improve the point2grid runtime performance (`#1421 <http://github.com/dtcenter/MET/issues/1421>`_).
      * Process point observations by variable name instead of GRIB code (`#1408 <http://github.com/dtcenter/MET/issues/1408>`_).

   * GIS Tools:

      * Fix memory corruption bug in the gis_dump_dbf utility which causes it to abort at runtime (`#1777 <http://github.com/dtcenter/MET/issues/1777>`_).

   * Plot-Point-Obs Tool:

      * **Overhaul Plot-Point-Obs to make it highly configurable** (`#213 <http://github.com/dtcenter/MET/issues/213>`_, `#1528 <http://github.com/dtcenter/MET/issues/1528>`_, and `#1052 <http://github.com/dtcenter/MET/issues/1052>`_).
      * Support regridding option in the config file (`#1627 <http://github.com/dtcenter/MET/issues/1627>`_).

   * Point-Stat Tool:

      * **Add mpr_column and mpr_thresh configuration options to filter out matched pairs based on large fcst, obs, and climo differences** (`#1575 <http://github.com/dtcenter/MET/issues/1575>`_).
      * **Print the rejection code reason count log messages at verbosity level 2 for zero matched pairs** (`#1644 <http://github.com/dtcenter/MET/issues/1644>`_).
      * **Add detailed log messages when discarding observations** (`#1588 <http://github.com/dtcenter/MET/issues/1588>`_).
      * Update log messages (`#1514 <http://github.com/dtcenter/MET/issues/1514>`_).
      * Enhance the validation of masking regions to check for non-unique masking region names (`#1439 <http://github.com/dtcenter/MET/issues/1439>`_).
      * Fix Point-Stat runtime error for some CF-complaint NetCDF files (`#1782 <http://github.com/dtcenter/MET/issues/1782>`_).

   * Grid-Stat Tool:

      * **Add mpr_column and mpr_thresh configuration options to filter out matched pairs based on large fcst, obs, and climo differences** (`#1575 <http://github.com/dtcenter/MET/issues/1575>`_).
      * Correct the climatological CDF values in the NetCDF matched pairs output files and correct the climatological probability values for climatgological distribution percentile (CDP) threshold types (`#1638 <http://github.com/dtcenter/MET/issues/1638>`_).

   * Stat-Analysis Tool:

      * **Process multiple output thresholds and write multiple output line types in a single aggregate_stat job** (`#1735 <http://github.com/dtcenter/MET/issues/1735>`_).
      * Skip writing job output to the logfile when the -out_stat option is provided (`#1736 <http://github.com/dtcenter/MET/issues/1736>`_).
      * Add -fcst_init_inc/_exc and -fcst_valid_inc/_exc job command filtering options to Stat-Analysis (`#1135 <http://github.com/dtcenter/MET/issues/1135>`_).
      * Add -column_exc job command option to exclude lines based on string values (`#1733 <http://github.com/dtcenter/MET/issues/1733>`_).
      * Fix Stat-Analysis failure when aggregating ECNT lines (`#1706 <http://github.com/dtcenter/MET/issues/1706>`_).

   * Grid-Diag Tool:

      * Fix bug when reading the same variable name from multiple data sources (`#1694 <http://github.com/dtcenter/MET/issues/1694>`_).

   * MODE Tool:

      * **Update the MODE AREA_RATIO output column to list the forecast area divided by the observation area** (`#1643 <http://github.com/dtcenter/MET/issues/1643>`_).
      * **Incremental development toward the Multivariate MODE tool** (`#1282 <http://github.com/dtcenter/MET/issues/1282>`_, `#1284 <http://github.com/dtcenter/MET/issues/1284>`_, and `#1290 <http://github.com/dtcenter/MET/issues/1290>`_).

   * TC-Pairs Tool:

      * Fix to report the correct number of lines read from input track data files (`#1725 <http://github.com/dtcenter/MET/issues/1725>`_).
      * Fix to read supported RI edeck input lines and ignore unsupported edeck probability line types (`#1768 <http://github.com/dtcenter/MET/issues/1768>`_).

   * TC-Stat Tool:

      * Add -column_exc job command option to exclude lines based on string values (`#1733 <http://github.com/dtcenter/MET/issues/1733>`_).

   * TC-Gen Tool:

      * **Overhaul the genesis matching logic, add the development and operational scoring algorithms, and add many config file options** (`#1448 <http://github.com/dtcenter/MET/issues/1448>`_).
      * Add config file options to filter data by initialization time (init_inc and init_exc) and hurricane basin (basin_mask) (`#1626 <http://github.com/dtcenter/MET/issues/1626>`_).
      * Add the genesis matched pair (GENMPR) output line type (`#1597 <http://github.com/dtcenter/MET/issues/1597>`_).
      * Add a gridded NetCDF output file with counts for genesis events and track points (`#1430 <http://github.com/dtcenter/MET/issues/1430>`_).
      * Enhance the matching logic and update several config options to support its S2S application (`#1714 <http://github.com/dtcenter/MET/issues/1714>`_).
      * Fix lead window filtering option (`#1465 <http://github.com/dtcenter/MET/issues/1465>`_).

   * IODA2NC Tool:

      * **Add the new ioda2nc tool** (`#1355 <http://github.com/dtcenter/MET/issues/1355>`_).
