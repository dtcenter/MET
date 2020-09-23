// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////


#ifndef  __MET_DATAPLANE_FROM_NUMPY_ARRAY_H__
#define  __MET_DATAPLANE_FROM_NUMPY_ARRAY_H__


////////////////////////////////////////////////////////////////////////


#include "data_plane.h"
#include "var_info_python.h"
#include "vx_grid.h"

#include "python3_dict.h"
#include "python3_numpy.h"


extern "C" {

#include "Python.h"

}


////////////////////////////////////////////////////////////////////////


extern bool dataplane_from_numpy_array(Python3_Numpy & np, const Python3_Dict & attrs, DataPlane & dp_out, Grid & grid_out, VarInfoPython & vinfo);


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_DATAPLANE_FROM_NUMPY_ARRAY_H__  */


////////////////////////////////////////////////////////////////////////

