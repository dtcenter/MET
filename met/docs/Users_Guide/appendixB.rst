.. _appendixB:

Appendix B Map Projections, Grids, and Polylines
================================================

Map Projections
_______________

The following map projections are currently supported in MET:

* Lambert Conformal Projection

* Polar Stereographic Projection (Northern)

* Polar Stereographic Projection (Southern)

* Mercator Projection

* Lat/Lon Projection

* Rotated Lat/Lon Projection

* Gaussian Projection

Grid Specification Strings
__________________________

Several configuration file and command line options support the definition of grids as a grid specification string. A description of the that string for each of the supported grid types is provided below.

To specify a Lambert Grid, the syntax is

.. code-block:: none

  lambert Nx Ny lat_ll lon_ll lon_orient D_km R_km standard_lat_1 [ standard_lat_2 ] N|S

Here, **Nx** and **Ny** are the number of points in, respectively, the **x** and **y** grid directions. These two numbers give the overall size of the grid. **lat_ll** and **lon_ll** are the latitude and longitude, in degrees, of the lower left point of the grid. North latitude and east longitude are considered positive. **lon_orient** is the orientation longitude of the grid. It's the meridian of longitude that's parallel to one of the vertical grid directions. **D_km** and **R_km** are the grid resolution and the radius of the Earth, both in kilometers. **standard_lat_1** and **standard_lat_2** are the standard parallels of the Lambert projection. If the two latitudes are the same, then only one needs to be given. **N|S** means to write either **N** or **S** depending on whether the Lambert projection is from the north pole or the south pole.

As an example of specifying a Lambert grid, suppose you have a northern hemisphere Lambert grid with 614 points in the x direction and 428 points in the y direction. The lower left corner of the grid is at latitude :math:`12.190^\circ` north and longitude :math:`133.459^\circ` west. The orientation longitude is :math:`95^\circ` west. The grid spacing is :math:`12.19058^\circ` km. The radius of the Earth is the default value used in many grib files: 6367.47 km. Both standard parallels are at :math:`25^\circ` north. To specify this grid in the config file, you would write

.. code-block:: none
        
  To grid = "lambert 614 428 12.190 -133.459 -95.0 12.19058 6367.47 25.0 N";

For a Polar Stereographic grid, the syntax is

.. code-block:: none
        
  stereo Nx Ny lat_ll lon_ll lon_orient D_km R_km lat_scale N|S

Here, **Nx, Ny, lat_ll, lon_ll, lon_orient, D_km** and **R_km** have the same meaning as in the Lambert case. **lat_scale** is the latitude where the grid scale **D_km** is true, while **N|S** means to write either **N** or **S** depending on whether the stereographic projection is from the north pole or the south pole.

For Plate Carrée (i.e. Lat/Lon) grids, the syntax is

.. code-block:: none

  latlon Nx Ny lat_ll lon_ll delta_lat delta_lon

The parameters **Nx, Ny, lat_ll** and **lon_ll** are as before. **delta_lat** and **delta_lon** are the latitude and longitude increments of the grid-i.e., the change in latitude or longitude between one grid point and an adjacent grid point.

For a Rotated Plate Carrée (i.e. Rotated Lat/Lon) grids, the syntax is

.. code-block:: none

  rotlatlon Nx Ny lat_ll lon_ll delta_lat delta_lon true_lat_sp true_lon_sp aux_rotation

The parameters **Nx, Ny, lat_ll, lon_ll, delta_lat,** and **delta_lon** are as before. **true_lat_sp** and **true_lon_sp** are the latitude and longitude for the south pole. **aux_rotation** is the auxilary rotation in degrees.

For a Mercator grid, the syntax is

.. code-block:: none
        
  mercator Nx Ny lat_ll lon_ll lat_ur lon_ur

The parameters **Nx, Ny, lat_ll** and **lon_ll** are again as before, while **lat_ur** and **lon_ur** are the latitude and longitude of the upper right corner of the grid.

For a Gaussian grid, the syntax is

.. code-block:: none
        
  gaussian lon_zero Nx Ny

The parameters **Nx** and **Ny** are as before, while **lon_zero** defines the first longitude.

Grids
_____

The majority of NCEP's pre-defined grids that reside on one of the projections listed above are implemented in MET. The user may specify one of these NCEP grids in the configuration files as "GNNN" where NNN is the 3-digit NCEP grid number. Defining a new masking grid in MET would involve modifying the vx_data_grids library and recompiling.

Please see `NCEP's website for a description and plot of these predefined grids <http://www.nco.ncep.noaa.gov/pmb/docs/on388/tableb.html>`_.

Polylines for NCEP Regions
__________________________

Many of NCEP's pre-defined verification regions are implemented in MET as lat/lon polyline files. The user may specify one of these NCEP verification regions in the configuration files by pointing to the lat/lon polyline file in the installed *share/met/poly* directory. Users may also easily define their own lat/lon polyline files.

See `NCEP's website for a description and plot of these predefined verification regions <http://www.emc.ncep.noaa.gov/mmb/research/nearsfc/nearsfc.verf.html>`_. 

The NCEP verification regions that are implemented in MET as lat/lon polylines are listed below:

* APL.poly for the Appalachians

* ATC.poly for the Arctic Region

* CAM.poly for Central America

* CAR.poly for the Caribbean Sea

* ECA.poly for Eastern Canada

* GLF.poly for the Gulf of Mexico

* GMC.poly for the Gulf of Mexico Coast

* GRB.poly for the Great Basin

* HWI.poly for Hawaii

* LMV.poly for the Lower Mississippi Valley

* MDW.poly for the Midwest

* MEX.poly for Mexico

* NAK.poly for Northern Alaska

* NAO.poly for Northern Atlantic Ocean

* NEC.poly for the Northern East Coast

* NMT.poly for the Northern Mountain Region

* NPL.poly for the Northern Plains

* NPO.poly for the Northern Pacific Ocean

* NSA.poly for Northern South America

* NWC.poly for Northern West Coast

* PRI.poly for Puerto Rico and Islands

* SAK.poly for Southern Alaska

* SAO.poly for the Southern Atlantic Ocean

* SEC.poly for the Southern East Coast

* SMT.poly for the Southern Mountain Region

* SPL.poly for the Southern Plains

* SPO.poly for the Southern Pacific Ocean

* SWC.poly for the Southern West Coast

* SWD.poly for the Southwest Desert

* WCA.poly for Western Canada

* EAST.poly for the Eastern United States (consisting of APL, GMC, LMV, MDW, NEC, and SEC)

* WEST.poly for the Western United States (consisting of GRB, NMT, NPL, NWC, SMT, SPL, SWC, and SWD)

* CONUS.poly for the Continental United States (consisting of EAST and WEST)
