// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2018
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#ifndef  __MET_DATAPLANE_FROM_XARRAY_H__
#define  __MET_DATAPLANE_FROM_XARRAY_H__


////////////////////////////////////////////////////////////////////////


#include "data_plane.h"


extern "C" {

#include "Python.h"

}


////////////////////////////////////////////////////////////////////////


extern DataPlane dataplane_from_xarray(PyObject * data_array, PyObject * atts_dict);


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_DATAPLANE_FROM_XARRAY_H__  */


////////////////////////////////////////////////////////////////////////


