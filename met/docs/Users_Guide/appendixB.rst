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

Grids
_____

All of NCEP's pre-defined grids that reside on one of the projections listed above are implemented in MET. The user may specify one of these NCEP grids in the configuration files as "GNNN" where NNN is the 3-digit NCEP grid number. Defining a new masking grid in MET would involve modifying the vx_data_grids library and recompiling.

Please see NCEP's website for a description and plot of these pre-defined grids: 

http://www.nco.ncep.noaa.gov/pmb/docs/on388/tableb.html.

Polylines for NCEP Regions
__________________________

Many of NCEP's pre-defined verification regions are implemented in MET as lat/lon polyline files. The user may specify one of these NCEP verification regions in the configuration files by pointing to the lat/lon polyline file in the installed share/met/poly directory. Users may also easily define their own lat/lon polyline files.

See NCEP's website for a description and plot of these pre-defined verification regions: http://www.emc.ncep.noaa.gov/mmb/research/nearsfc/nearsfc.verf.html 

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
