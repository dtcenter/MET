// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <iostream>
#include <stdlib.h>
#include <string.h>

#include "vx_log.h"
#include "vx_math.h"

#include "vx_python3_utils.h"
#include "python_line.h"


////////////////////////////////////////////////////////////////////////


static const char generic_python_wrapper [] = "generic_python";
static const char generic_pickle_wrapper [] = "generic_pickle";

static const char write_pickle_wrapper   [] = "MET_BASE/wrappers/point_write_pickle.py";

static const char list_name              [] = "point_data";

static const char pickle_output_filename [] = "out.pickle";


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class PyLineDataFile
   //


////////////////////////////////////////////////////////////////////////


PyLineDataFile::PyLineDataFile()

{

index = -1;

N = -1;

main_list = 0;

}


////////////////////////////////////////////////////////////////////////


PyLineDataFile::~PyLineDataFile()

{

index = -1;

N = -1;

main_list = 0;

}


////////////////////////////////////////////////////////////////////////








