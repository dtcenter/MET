MET release notes
_________________

When applicable, release notes are followed by the GitHub issue number which
describes the bugfix, enhancement, or new feature:
https://github.com/dtcenter/MET/issues

Version |version| release notes (|release_date|)
------------------------------------------------

- Bugfixes:
   - All changes included in the `met-9.0.1 <https://github.com/dtcenter/MET/milestone/64?closed=1>`_, `met-9.0.2 <https://github.com/dtcenter/MET/milestone/65?closed=1>`_, and `met-9.0.3 <https://github.com/dtcenter/MET/milestone/66?closed=1>`_ bugfix releases.

- Output format changes:
   - Add new CNT line type columns (ANOM_CORR_UNCNTR) for the uncentered anomaly correlation without the mean error (`#1399 <http://github.com/dtcenter/MET/issues/1399>`_).
   - Add new RPS line type column (RPS_COMP) for the complement of RPS (`#1280 <http://github.com/dtcenter/MET/issues/1280>`_).

- Configuration file changes:
   - Add various set_attr configuration options to override metadata read from input gridded data files (`#1020 <http://github.com/dtcenter/MET/issues/1020>`_).
   - Enhance the regrid dictionary to support data censoring and conversion (`#1293 <http://github.com/dtcenter/MET/issues/1293>`_).
   - For Grid-Diag, replace model entry with desc (`#1391 <http://github.com/dtcenter/MET/issues/1391>`_). 
   - For TC-Pairs, add new basin_map configuration option (`#1390 <http://github.com/dtcenter/MET/issues/1390>`_).
   - For TC-RMW, add many track filtering configuration options (`#1315 <http://github.com/dtcenter/MET/issues/1315>`_).

- Documentation changes:
   - Migrate this User's Guide from Lyx to Sphinx and publish the result (`#1217 <http://github.com/dtcenter/MET/issues/1217>`_, `#1321 <http://github.com/dtcenter/MET/issues/1321>`_, `#1322 <http://github.com/dtcenter/MET/issues/1322>`_, `#1323 <http://github.com/dtcenter/MET/issues/1323>`_, `#1325 <http://github.com/dtcenter/MET/issues/1325>`_).
   - Correct Uniform Fractions Skill Score documentation (`#1431 <http://github.com/dtcenter/MET/issues/1431>`_).
   - Clarify Point2Grid documentation about Gaussian methods (`#1413 <http://github.com/dtcenter/MET/issues/1413>`_).
   - Resolve Point-Stat compiler warning messages (`#1435 <http://github.com/dtcenter/MET/issues/1435>`_).

- Build process changes:
   - Fix the compilation of the make_mapfiles development utility (`#1364 <http://github.com/dtcenter/MET/issues/1364>`_).
   - Run valgrind to search for memory issues (`#816 <http://github.com/dtcenter/MET/issues/816>`_).
   - Fix valgrind warnings (`#1387 <http://github.com/dtcenter/MET/issues/1387>`_).
   - Fix uninitialized ConcatString variable (`#1386 <http://github.com/dtcenter/MET/issues/1386>`_).
   - Enhance the GRIB2 library to print a warning message about inconsistent usage of the -D__64BIT__ compilation flag (`#1416 <http://github.com/dtcenter/MET/issues/1416>`_).
   - Enhance various log message, error messages, and usage statements (`#1350 <http://github.com/dtcenter/MET/issues/1350>`_, `#1347 <http://github.com/dtcenter/MET/issues/1347>`_, `#1339 <http://github.com/dtcenter/MET/issues/1339>`_, `#1338 <http://github.com/dtcenter/MET/issues/1338>`_, `#1333 <http://github.com/dtcenter/MET/issues/1333>`_).
   - Remove MET_BASE references from Rscripts (`#1289 <http://github.com/dtcenter/MET/issues/1289>`_).

- Fortify changes:
   - Address Fortify findings for Point2Grid (`#1352 <http://github.com/dtcenter/MET/issues/1352>`_).
   - Address Fortify findings for met-9.0.2 (`#1359 <http://github.com/dtcenter/MET/issues/1359>`_).
   - Address Fortify findings for met-9.1_beta2 (`#1417 <http://github.com/dtcenter/MET/issues/1417>`_).

- Enhancements to existing tools:
   - Common utility libraries
      - Enhance the regrid dictionary with data censoring and conversion (`#1293 <http://github.com/dtcenter/MET/issues/1293>`_).
      - Make the ConcatString class more efficient (`#1358 <http://github.com/dtcenter/MET/issues/1358>`_).
      - Make the StringArray class more efficient (`#1357 <http://github.com/dtcenter/MET/issues/1357>`_).
      - Fix parsing of CF-compliant time stamps from CDO (`#1331 <http://github.com/dtcenter/MET/issues/1331>`_).
   - Python library
      - Update Python embedding scripts (`#1265 <http://github.com/dtcenter/MET/issues/1265>`_).
      - Enhance the parsing of ASCII file lists with Python embedding (`#1432 <http://github.com/dtcenter/MET/issues/1432>`_).
   - Tropical cyclone library
      - Alert users of the automatic renaming of AVN to GFS in MET tropical cyclone tools (`#1444 <http://github.com/dtcenter/MET/issues/1444>`_).
   - Plot-Data-Plane
      - Update the formatting of the colorbar values to show more significant digits (`#1423 <http://github.com/dtcenter/MET/issues/1423>`_).
   - Gen-Vx-Mask
      - Enhance Gen-Vx-Mask with python embedding and named input grids (`#1292 <http://github.com/dtcenter/MET/issues/1292>`_).
   - Grid-Diag
      - Enhancements, refinement, and testing of the Grid-Diag tool (`#1391 <http://github.com/dtcenter/MET/issues/1391>`_, `#1279 <http://github.com/dtcenter/MET/issues/1279>`_).
   - Point2Grid
      - Enhance Point2Grid to process multiple fields (`#1396 <http://github.com/dtcenter/MET/issues/1396>`_).
      - Enhance Point2Grid to enable Gaussian filtering for GOES16/17 data (`#1291 <http://github.com/dtcenter/MET/issues/1291>`_).
   - Point-Stat and Grid-Stat
      - Add new CNT line type columns (ANOM_CORR_UNCNTR) for the uncentered anomaly correlation without the mean error (`#1399 <http://github.com/dtcenter/MET/issues/1399>`_).
      - Add new RPS line type column (RPS_COMP) for the complement of RPS (`#1280 <http://github.com/dtcenter/MET/issues/1280>`_).
   - Point-Stat
      - Update Point-Stat HiRA configuration options for computing RPS (`#1400 <http://github.com/dtcenter/MET/issues/1400>`_).
   - Grid-Stat
      - Update Grid-Stat attributes written to the NetCDF matched pairs file (`#1324 <http://github.com/dtcenter/MET/issues/1324>`_).
   - Ensemble-Stat
      - Fix Ensemble-Stat runtime error when obs_thresh results in 0 pairs being retained (`#1397 <http://github.com/dtcenter/MET/issues/1397>`_).
      - Enhance Ensemble-Stat log messages (`#1440 <http://github.com/dtcenter/MET/issues/1440>`_).
   - Series-Analysis
      - Enhance Series-Analysis to print a warning message about a common misconfiguration (`#1372 <http://github.com/dtcenter/MET/issues/1372>`_).
      - Add Series-Analysis verbosity level warning (`#1382 <http://github.com/dtcenter/MET/issues/1382>`_).
   - TC-Pairs
      - Add TC-Pairs new basin_map configuration option (`#1390 <http://github.com/dtcenter/MET/issues/1390>`_).
   - TC-Gen
      - Fix TC-Gen BEST track genesis event definition (`#1447 <http://github.com/dtcenter/MET/issues/1447>`_).
      - Fix TC-Gen to correctly apply the genesis event definition criteria from the configuration file to BEST and operational tracks (`#1427 <http://github.com/dtcenter/MET/issues/1427>`_).
      - Add TC-Gen warning for duplicate genesis events (`#1380 <http://github.com/dtcenter/MET/issues/1380>`_).
      - Enhance TC-Gen to support file lists for teh -genesis and -track command line options (`#1442 <http://github.com/dtcenter/MET/issues/1442>`_).
   - TC-RMW
      - Add many TC-RMW track filtering configuration options (`#1315 <http://github.com/dtcenter/MET/issues/1315>`_).
