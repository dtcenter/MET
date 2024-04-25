.. _appendixH:

*****************************
Appendix H Unstructured Grids
*****************************

Introduction
============

METv12.0.0+ includes limited support for using datasets on unstructured grids (UGRIDs) in the PointStat and GridStat tools. This support includes verifying UGRID forecasts against point observations (PointStat), and verifying UGRID forecasts regridded to a structured grid against gridded observations on a structured grid (GridStat). The implementation of UGRID support in METv12.0.0+ is based on the UGRID Conventions (https://ugrid-conventions.github.io/ugrid-conventions/). These conventions define various elements for common UGRID topologies. Thus far, development to support datasets on unstructured grids has been driven by the LFRIc (“elfrick”) NWP model, and the Model for Prediction Across Scales (MPAS) NWP model. However, the support for unstructured grids was implemented in such a way that additional unstructured grids can be utilized, provided the topology can be related to the UGRID conventions.

Unstructured Grid Files
=======================

Support for UGRID files in NetCDF format is provided. MET will attempt to auto-detect UGRID files. If the file is CF-compliant and the “Conventions” global attribute is “UGRID” or “MPAS”, or if the global attribute “mesh_spec” is present, MET recognizes the file as a UGRID file. If the user is not using a supported UGRID, or a UGRID file that does not contain the required global attributes for autodetection, the user should set the **file_type** configuration entry to **NETCDF_UGRID**. In some cases, the UGRID topology is provided in an ancillary file separate from forecast variables. This is controlled via the **ugrid_coordinates_file** configuration entry. This file is checked first, and then the UGRID data file. If the same elements are found in both files, the information from the data file takes precedence, and thus it is recommended to leave **ugrid_coordinates_file** unset if all required elements are found in the data file. If both a data and coordinates file are provided, the shape of the latitude and longitude dimensions are cross-referenced as a limited enforcement measure that the coordinates file and data file belong to the same UGRID.

Required Unstructured Grid Metadata
===================================

To correctly parse the UGRID topology, MET needs the following information that must be contained within the UGRID coordinates file or the UGRID data file. It is often the case that the variable names for these elements differ between UGRIDs. If the user is not using one of the supported UGRIDs, they will need to define the **ugrid_metadata_map** in a custom configuration file that they provide on the command line using the **-ugrid_config** command line argument, which tells MET the variable names the correspond to the following elements:

Dimension name for UGRID cells (required)
UGRID convention element name: dim_face
Coordinate variable name for the latitude of each UGRID cell (required)
UGRID convention element name: lat_face
Coordinate variable name for the longitude of each UGRID cell (required)
UGRID convention element name: lon_face
Dimension name for the time coordinate (required)
UGRID convention element name: dim_time
Coordinate variable name for the time coordinate (required)
UGRID convention element name: time
Dimension name for the vertical coordinate, if present (optional)
UGRID convention element name: dim_vert
Coordinate variable name for the vertical coordinate, if present (optional)
UGRID convention element name: vert_face

Unstructured Grid Configuration Elements
========================================

In the PointStat and GridStat config files, a user can control aspects of the UGRID capability.

**ugrid_dataset [optional]**
If the UGRID dataset is supported by MET, then this item is set to the string identifying the UGRID. Currently supported ugrid_dataset options include “lfric”, and “mpas”. If this item is not set, the user must provide a separate UGRID configuration file on the command line using the **-ugrid_config** command line argument.

**ugrid_max_distance_km [default: 0 km]**
For PointStat, this is the distance from each UGRID cell center that PointStat will search outward to collect observations to use for verification. The default is 0 km, which by default will use the closest observation to the UGRID cell for verification. For GridStat, this is the distance from each UGRID forecast cell center to search for observation grid cells to include when interpolating to the UGRID forecast cell locations. Currently, only the nearest point observation or gridded observation cell is used.

**ugrid_coordinates_file [optional]**
The absolute path to the ancillary NetCDF file that describes the UGRID topology. This file is checked for the required UGRID metadata first, and the data file is checked second and takes precedence over this file if the same elements are found in both files.

**ugrid_metadata_map [required if not using a supported UGRID]**
The mapping dictionary which allows a user to specify the variable names of the required UGRID topology elements that are required by MET. For "lfric" and "mpas", **ugrid_metadata_map** comes pre-configured with MET and a user can simply set **ugrid_dataset**. Otherwise, the user must create a separate UGRID configuration file containing **ugrid_metadata_map** and provide it on the command line using the **-ugrid_config** command line argument.

Unstructured Grid Limitations
=============================

We anticipate expanding the UGRID capabilities in MET in the near future. Until then, users are encouraged to be mindful of the following limitations:

1. In GridStat, there is no support for verifying directly on a UGRID. The UGRID (either forecast, obs, or both) must be regridded to a structured verification grid prior to verification being performed. It is recommended that the user define a custom verification grid tailored to their UGRID characteristics, being mindful that the regridding can be slow for large verification grids.

2. Data at cell edges and nodes are currently not supported, only those variables which have data at the cell centers are supported. Users should note in particular that wind components that are typically derived using data at cell edges are currently unsupported.

3. No aggregation methods of point observations within the **ugrid_max_distance_km** are supported except NEAREST, and no aggregation methods of gridded observations within the **ugrid_max_distance_km** are supported except NEAREST.
