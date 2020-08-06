MET release notes
_________________

When applicable, release notes are followed by the GitHub issue number which
describes the bugfix, enhancement, or new feature:
https://github.com/NCAR/MET/issues

Version 9.1 BETA3 release notes
-------------------------------

- **Date: 20200729**

- Bugfixes:
   - Fix TC-Gen to correctly apply the genesis event definition criteria from
     the configuration file to BEST and operational tracks (`#1427 <http://github.com/NCAR/MET/issues/1427>`_).

- Enhancements to Existing Tools:
   - Enhance Grid-Diag to support multiple input file lists, improve the log
     messages, and write additional output variables and attributes (`#1391 <http://github.com/NCAR/MET/issues/1391>`_).
   - Enhance the parsing of ASCII file lists with Python embedding (`#1432 <http://github.com/NCAR/MET/issues/1432>`_).
   - Update the formatting of the colorbar values in plot_data_plane to show
     more significant digits (`#1423 <http://github.com/NCAR/MET/issues/1423>`_).
   - Enhance Series-Analysis to print a warning message about a common
     misconfiguration (`#1372 <http://github.com/NCAR/MET/issues/1372>`_).
   - Enhance the GRIB2 library to print a warning message about inconsistent
     usage of the -D__64BIT__ compilation flag (`#1416 <http://github.com/NCAR/MET/issues/1416>`_).
   - Continued reduction of Fortify findings (`#1417 <http://github.com/NCAR/MET/issues/1417>`_).
   - Various documentation updates (`#1325 <http://github.com/NCAR/MET/issues/1325>`_, `#1431 <http://github.com/NCAR/MET/issues/1431>`_).

Version 9.1 BETA2 release notes
-------------------------------

- **Date: 20200714**

- Bugfixes:
   - All changes included in the met-9.0.2 and met-9.0.3 bugfix releases:
     https://github.com/NCAR/MET/milestone/65?closed=1
     https://github.com/NCAR/MET/milestone/66?closed=1

- Enhancements to Existing Tools:
   - Update HiRA configuration options in Point-Stat for computing RPS (`#1400 <http://github.com/NCAR/MET/issues/1400>`_).
   - Add new anomaly correlation without the mean error (ANOM_CORR_UNCNTR) to
     the CNT line type (`#1399 <http://github.com/NCAR/MET/issues/1399>`_).
   - Fix Ensemble-Stat runtime error when obs_thresh results in 0 pairs being
     retained (`#1397 <http://github.com/NCAR/MET/issues/1397>`_).
   - Enhance point2grid to process multiple fields (`#1396 <http://github.com/NCAR/MET/issues/1396>`_).
   - Add new basin_map configuration option for TC-Pairs (`#1390 <http://github.com/NCAR/MET/issues/1390>`_).
   - Fix valgrind warnings (`#1387 <http://github.com/NCAR/MET/issues/1387>`_).
   - Fix uninitialized ConcatString variable (`#1386 <http://github.com/NCAR/MET/issues/1386>`_).
   - Add Series-Analysis verbosity level warning (`#1382 <http://github.com/NCAR/MET/issues/1382>`_).
   - Add TC-Gen warning for duplicate genesis events (`#1380 <http://github.com/NCAR/MET/issues/1380>`_).
   - Fix the compilation of the make_mapfiles development utility (`#1364 <http://github.com/NCAR/MET/issues/1364>`_).
   - Address Fortify findings from met-9.0.2 (`#1359 <http://github.com/NCAR/MET/issues/1359>`_).
   - Make the ConcatString class more efficient (`#1358 <http://github.com/NCAR/MET/issues/1358>`_).
   - Make the StringArray class more efficient (`#1357 <http://github.com/NCAR/MET/issues/1357>`_).
   - Address Fortify findings for point2grid (`#1352 <http://github.com/NCAR/MET/issues/1352>`_).
   - Enhance various log message, error messages, and usage statements (`#1350 <http://github.com/NCAR/MET/issues/1350>`_, `#1347 <http://github.com/NCAR/MET/issues/1347>`_, `#1339 <http://github.com/NCAR/MET/issues/1339>`_, `#1338 <http://github.com/NCAR/MET/issues/1338>`_, `#1333 <http://github.com/NCAR/MET/issues/1333>`_).
   - Fix parsing of CF-compliant time stamps from CDO (`#1331 <http://github.com/NCAR/MET/issues/1331>`_).
   - Update attributes in the Grid-Stat NetCDF matched pairs file (`#1324 <http://github.com/NCAR/MET/issues/1324>`_).
   - Updates to the MET User's Guide (`#1321 <http://github.com/NCAR/MET/issues/1321>`_).
   - Add track filtering to the TC-RMW tool (`#1315 <http://github.com/NCAR/MET/issues/1315>`_).
   - Update Python embedding scripts (`#1265 <http://github.com/NCAR/MET/issues/1265>`_).
   - Add various set_attr configuration options (`#1020 <http://github.com/NCAR/MET/issues/1020>`_).
   - Run valgrind to search for memory issues (`#816 <http://github.com/NCAR/MET/issues/816>`_).

Version 9.1 BETA1 release notes
-------------------------------

- **Date: 20200626**

- Bugfixes:
   - All changes included in the met-9.0.1 bugfix release:
     https://github.com/NCAR/MET/milestone/64?closed=1

- Enhancements to Existing Tools:
   - Initial version of Sphinx documentation (`#1217 <http://github.com/NCAR/MET/issues/1217>`_).
   - Add the complement of RPS to the existing RPS line type (`#1280 <http://github.com/NCAR/MET/issues/1280>`_).
   - Remove MET_BASE references from Rscripts (`#1289 <http://github.com/NCAR/MET/issues/1289>`_).
   - Enhance Point2Grid to enable Gaussian filtering for GOES16/17 data (`#1291 <http://github.com/NCAR/MET/issues/1291>`_).
   - Enhance Gen-Vx-Mask with python embedding and named input grids (`#1292 <http://github.com/NCAR/MET/issues/1292>`_).
   - Enhance the regrid dictionary with data censoring and conversion (`#1293 <http://github.com/NCAR/MET/issues/1293>`_).

