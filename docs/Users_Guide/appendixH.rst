.. _appendixH:

*****************************
Appendix H Unstructured Grids
*****************************

Introduction
============

METv12.0.0+ includes limited support for using datasets on unstructured grids (UGRIDs) in the PointStat and GridStat tools. This support includes verifying UGRID forecasts against point observations (PointStat), verifying UGRID forecasts regridded to a structured grid against gridded observations on a structured grid (GridStat), and verifying UGRID forecasts against UGRID observations on the same UGRID (GridStat). The implementation of UGRID support in METv12.0.0+ provides a mechanism for a user to describe the topology of their unstructured grid. Thus far, development to support datasets on unstructured grids has been driven by the LFRic (“elfrick”) NWP model (:ref:`Adams et al. 2019 <Adams-2019>`), and the Model for Prediction Across Scales (MPAS) NWP model (:ref:`Skamarock et al., 2012 <Skamarock-2012>`). However, the support for unstructured grids was implemented in such a way that additional unstructured grids can be utilized, with the appropriate mapping of the user's topology to the elements that MET requires.

Unstructured Grid Files
=======================

Support for UGRID files in NetCDF format is provided. MET will attempt to auto-detect UGRID files. If the file is CF-compliant and the “Conventions” global attribute is “UGRID” or “MPAS”, or if the global attribute “mesh_spec” is present, MET recognizes the file as a UGRID file. If the user is not using a supported UGRID, or a UGRID file that does not contain the required global attributes for autodetection, the user should set the **file_type** configuration entry to **NETCDF_UGRID**. In some cases, the UGRID topology is provided in an ancillary file separate from forecast variables. This is controlled via the **ugrid_coordinates_file** configuration entry. This file is checked first, and then the UGRID data file. If the same elements are found in both files, the information from the data file takes precedence, and thus it is recommended to leave **ugrid_coordinates_file** unset if all required elements are found in the data file. If both a data and coordinates file are provided, the shape of the latitude and longitude dimensions are cross-referenced as a limited enforcement measure that the coordinates file and data file belong to the same UGRID.

Required Unstructured Grid Metadata
===================================

To correctly parse the UGRID topology, MET needs the following information that must be contained within the UGRID coordinates file or the UGRID data file. It is often the case that the variable names for these elements differ between UGRIDs. If the user is not using one of the supported UGRIDs, they will need to define the **ugrid_metadata_map** in a custom configuration file that they provide on the command line using the **-ugrid_config** command line argument, which tells MET the variable names that correspond to the following elements:

.. _table_ugrid_metadata:

.. list-table:: Required UGRID Metadata
  :widths: auto
  :header-rows: 1

  * - Metadata Map Key
    - Description
    - Required
  * - dim_face
    - Dimension name for UGRID cells
    - Required
  * - lat_face
    - Coordinate variable name for the latitude of each UGRID cell
    - Required
  * - lon_face
    - Coordinate variable name for the longitude of each UGRID cell
    - Required
  * - dim_time
    - Dimension name for the time coordinate
    - Required
  * - time
    - Coordinate variable for valid time
    - Required
  * - init_time
    - Coordinate variable for initialization time (used to compute the lead time)
    - Optional
  * - dim_vert
    - Dimension name for the vertical coordinate, if present
    - Optional
  * - vert_face
    - Coordinate variable name for the vertical coordinate
    - Optional
  * - area_face
    - Variable name for the area of each UGRID cell (e.g. "areaCell" for MPAS), in m\ :sup:`2` or km\ :sup:`2` as given by its "units" attribute (m\ :sup:`2` if missing). It is used as the weight for the AW_MEAN interpolation method (see :ref:`ugrid_interpolation`), the AREA and AREA_CNTR grid weighting (**grid_weight_flag**), and the AW_MEAN regridding method. See :ref:`ugrid_limitations` for the behavior without it.
    - Optional

Unstructured Grid Configuration Entries
=======================================

In the PointStat and GridStat config files, a user can control aspects of the UGRID capability. Since the UGRID support in MET is optional, these items are not included in the default MET config files. If the user requires modification for any of the following, they must add them to their MET config file.

ugrid_dataset
-------------

If the UGRID dataset is supported by MET, then this item is set to the string identifying the UGRID. Currently supported ugrid_dataset options include “lfric”, and “mpas”. If this item is not set, the user must provide a separate UGRID configuration file on the command line using the **-ugrid_config** command line argument.

ugrid_max_distance_km
---------------------

For PointStat, this is the maximum allowable distance in kilometers for a point observation to be matched to the nearest UGRID cell center. If left unspecified, each point observation will be matched to the closest UGRID cell, regardless of how far apart they are. If no UGRID cells are within the specified distance, the observation will not be used for verification. For GridStat, this is the distance from each forecast grid point to search for observation grid cells to include when interpolating to the forecast grid point locations.

The same distance applies to the interpolation and smoothing methods which use more than one UGRID cell (see :ref:`ugrid_interpolation`). Cells farther than **ugrid_max_distance_km** are excluded and count as missing data for the **interp.vld_thresh** threshold. Setting this distance is recommended when the UGRID covers only part of the observation domain. Otherwise, observations outside of the UGRID are matched to the cells along its edge.

ugrid_coordinates_file
----------------------

The absolute path to the ancillary NetCDF file that describes the UGRID topology. This file is checked for the required UGRID metadata first, and the data file is checked second and takes precedence over this file if the same elements are found in both files.

ugrid_metadata_map
------------------

The mapping dictionary which allows a user to specify the variable names of the required UGRID topology elements that are required by MET. For "lfric" and "mpas", **ugrid_metadata_map** comes pre-configured with MET and a user can simply set **ugrid_dataset**. Otherwise, the user must create a separate UGRID configuration file containing the **ugrid_metadata_map** containing the elements from :numref:`table_ugrid_metadata` and provide it on the command line using the **-ugrid_config** command line argument.

.. _ugrid_interpolation:

Unstructured Grid Interpolation
===============================

The cells of a UGRID are not arranged in rows and columns, so the interpolation and smoothing methods that use more than one point are computed from the N cells closest to the target location instead of the N x N box of grid points used for a structured grid. The target location is the point observation for PointStat and the center of each UGRID cell for GridStat smoothing. The closest cells are found from the cell centers using a k-d tree and are limited by **ugrid_max_distance_km**.

The number of cells, N, is the number of points defined by the **interp.type.width** and **interp.shape** entries, as it is for a structured grid. For example, a width of 3 with a SQUARE shape uses the 9 closest cells, and this number is written to the INTERP_PNTS output column. The BILIN method uses the width x width closest cells, which is 4 since the width of BILIN is always 2. The **interp.vld_thresh** entry applies to the N cells.

.. list-table:: Interpolation Methods for Unstructured Grids
  :widths: auto
  :header-rows: 1

  * - Method
    - Description
    - PointStat
    - GridStat
  * - NEAREST
    - Value of the closest cell
    - Yes
    - Yes
  * - MIN, MAX, MEDIAN
    - Minimum, maximum, or median of the N closest cells
    - Yes
    - Yes
  * - UW_MEAN
    - Unweighted average of the N closest cells
    - Yes
    - Yes
  * - AW_MEAN
    - Average weighted by the cell area (**area_face**). Without the cell area, the cells are weighted equally (same as UW_MEAN) and a warning is printed.
    - Yes
    - Yes
  * - DW_MEAN
    - Average weighted by distance\ :sup:`-2`. If the target location is within 1 meter of a cell center, the value of that cell is used.
    - Yes
    - No
  * - BILIN
    - Bilinear interpolation (see below)
    - Yes
    - No
  * - BEST
    - Value of the N closest cells closest to the observation value
    - Yes
    - No
  * - HiRA (NBRHD)
    - The N closest cells are used as the HiRA neighborhood (**hira** dictionary)
    - Yes
    - No

For BILIN, MET fits the bilinear surface v = a + b*x + c*y + d*x*y through the closest cells by least squares, where x and y are the east and north distances (km) of each cell center from the target location on the local tangent plane, and uses the value of the surface at the target location (a). For the 4 corners of a grid box aligned to the east and north directions, this is the standard bilinear interpolation. If the cross term (d) can not be determined, for example with only 3 valid cells, a plane v = a + b*x + c*y is fitted instead, which is the barycentric interpolation for 3 cells. If neither can be fitted (for example, when the cells lie along a line), the result is bad data. Since the closest cells do not always surround the target location, the fitted surface may extrapolate. The result is therefore limited to the range of the cell values, as the bilinear interpolation of a structured grid is.

DW_MEAN and BILIN are not supported for GridStat smoothing since the closest cell of each cell is the cell itself, and BEST is not supported since there is no observation value to match. The LS_FIT, UPPER_LEFT, UPPER_RIGHT, LOWER_RIGHT, LOWER_LEFT, GAUSSIAN, MAXGAUSS, and GEOG_MATCH methods rely on the structure of a regular grid and are not supported for UGRIDs. Requesting an unsupported method results in an error when the configuration is processed.

The following PointStat example uses the 9 closest cells within 20 km of each observation:

.. code-block:: none

  interp = {
     vld_thresh = 1.0;
     shape      = SQUARE;
     type = [
        { method = NEAREST; width = 1; },
        { method = UW_MEAN; width = 3; },
        { method = AW_MEAN; width = 3; },
        { method = BILIN;   width = 2; }
     ];
  }

  ugrid_dataset          = "mpas";
  ugrid_max_distance_km  = 20;

.. _ugrid_limitations:

Unstructured Grid Limitations
=============================

We anticipate expanding the UGRID capabilities in MET in the near future. Until then, users are encouraged to be mindful of the following limitations:

1. In GridStat, verifying directly on a UGRID (**regrid.to_grid = NONE**) is supported only when the forecast and observation are on the same UGRID, and with the following restrictions:

   * The NetCDF matched pairs output is not supported. Set **nc_pairs_flag = FALSE**, since it is enabled by default.
   * The Fourier decomposition (**fourier**) is not supported. Leave **fourier.wave_1d_beg** and **fourier.wave_1d_end** empty (the default).
   * The neighborhood statistics (NBRCTC, NBRCTS, NBRCNT), gradients (GRAD), and distance maps (DMAP) rely on the structure of a regular grid and are not supported. Set them to NONE in **output_flag** (the default).

   GridStat checks these settings after determining the verification grid and exits with an error, before writing any output, if any of them is requested for a UGRID.

   Otherwise, the UGRID (either forecast, obs, or both) must be regridded to a structured verification grid prior to verification being performed. In most cases, gridded observations are on a structured grid and so the users are encouraged to regrid their UGRID forecast to the structured observation grid. The user is referred to :ref:`appendixB` for guidance on defining a structured verification grid, being mindful that the regridding can be slow for large verification grids. For Point-Stat, evaluation occurs by interpolating UGRID data directly to observation point locations and does not require regridding.

2. Data at cell edges are currently not supported, only those variables which have data at the cell centers are supported. Users should note in particular that wind components that are typically derived using data at cell edges are currently unsupported.

3. The interpolation and smoothing methods supported for UGRIDs are listed in :ref:`ugrid_interpolation`. In PointStat, when the land/sea mask (**land_mask**) or topography (**topo_mask**) is enabled, only the NEAREST method is supported for both **interp** and **topo_mask.interp**, since the surface interpolation does not yet use the closest cells.

4. The area of the UGRID cells is available only when **area_face** is defined in the **ugrid_metadata_map** (see :numref:`table_ugrid_metadata`). Without it:

   * **grid_weight_flag** = AREA or AREA_CNTR results in an error.
   * The AW_MEAN regridding method from a UGRID results in an error.
   * The AW_MEAN interpolation method uses equal weights (same as UW_MEAN) and prints a warning.

5. The cell area of a UGRID is the area of the cell itself, so the centered variants are not distinct: **grid_weight_flag** = AREA_CNTR is identical to AREA. The AW_MEAN_CNTR regridding method is not supported from a UGRID and results in an error, since it computes the extent of each input grid box from the neighboring grid indices, which are not neighboring cells on a UGRID. Use AW_MEAN instead. AW_MEAN_CNTR is not an interpolation method for PointStat or GridStat and is rejected for UGRIDs (see :ref:`ugrid_interpolation`).
