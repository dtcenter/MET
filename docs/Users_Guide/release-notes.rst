MET Release Notes
=================

When applicable, release notes are followed by the GitHub issue number which describes the bugfix,
enhancement, or new feature (`MET GitHub issues <https://github.com/dtcenter/MET/issues>`_).
Important issues are listed **in bold** for emphasis.

MET Version 11.0.0-beta1 release notes (20220622)
-------------------------------------------------

* Repository and build:

   * **Restructure the contents of the MET repository so that it matches the existing release tarfiles** (`#1920 <https://github.com/dtcenter/MET/issues/1920>`_).
   * Fix the OpenMP compilation error for GCC 9.3.0/9.4.0 (`#2106 <https://github.com/dtcenter/MET/issues/2106>`_).
   * Update the MET version number to 11.0.0 (`#2132 <https://github.com/dtcenter/MET/issues/2132>`_).

* Bugfixes:

   * Fix regression test differences in pb2nc and ioda2nc output (`#2102 <https://github.com/dtcenter/MET/issues/2102>`_).
   * Fix support for reading rotated lat/lon grids from CF-compliant NetCDF files (`#2115 <https://github.com/dtcenter/MET/issues/2115>`_).
   * Fix support for reading rotated lat/lon grids from GRIB1 files (grid type 10) (`#2118 <https://github.com/dtcenter/MET/issues/2118>`_).
   * Fix support for int64 NetCDF variable types (`#2123 <https://github.com/dtcenter/MET/issues/2123>`_).
   * Fix Stat-Analysis to aggregate the ECNT ME and RMSE values correctly (`#2170 <https://github.com/dtcenter/MET/issues/2170>`_).
   * Fix NetCDF library code to process scale_factor and add_offset attributes independently (`#2187 <https://github.com/dtcenter/MET/issues/2187>`_).

* Enhancements:

   * Sort mask.sid station lists to check their contents more efficiently (`#1950 <https://github.com/dtcenter/MET/issues/1950>`_).
   * Add Anomaly Correlation Coefficient to VCNT Line Type (`#2022 <https://github.com/dtcenter/MET/issues/2022>`_).
   * Enhance TC-RMW to compute tangential and radial winds (`#2072 <https://github.com/dtcenter/MET/issues/2072>`_).
   * Allow 2x2 HSS calculations to include user-defined EC values (`#2147 <https://github.com/dtcenter/MET/issues/2147>`_).
   * Enhance Gen-Vx-Mask by adding a new poly_xy masking type option (`#2152 <https://github.com/dtcenter/MET/issues/2152>`_).
   * Add M_to_KFT and KM_to_KFT functions to ConfigConstants (`#2180 <https://github.com/dtcenter/MET/issues/2180>`_).
