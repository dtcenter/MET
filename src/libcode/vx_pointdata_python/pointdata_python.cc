// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "pointdata_python.h"
#include "pointdata_from_array.h"
#include "vx_python3_utils.h"

#include "vx_math.h"
#include "vx_log.h"

using namespace std;


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class MetPythonPointDataFile
   //


////////////////////////////////////////////////////////////////////////


MetPythonPointDataFile::MetPythonPointDataFile()

{

python_init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


MetPythonPointDataFile::~MetPythonPointDataFile()

{

close();

}


////////////////////////////////////////////////////////////////////////


MetPythonPointDataFile::MetPythonPointDataFile(const MetPythonPointDataFile &)

{

mlog << Error << "\nMetPythonPointDataFile::MetPythonPointDataFile(const MetPythonPointDataFile &) -> "
     << "should never be called!\n\n";

exit ( 1 );

}


////////////////////////////////////////////////////////////////////////


MetPythonPointDataFile & MetPythonPointDataFile::operator=(const MetPythonPointDataFile &)

{

mlog << Error << "\nMetPythonPointDataFile::operator=(const MetPythonPointDataFile &) -> "
     << "should never be called!\n\n";

exit ( 1 );

}


////////////////////////////////////////////////////////////////////////


void MetPythonPointDataFile::python_init_from_scratch()

{

PythonCommand.clear();

close();

return;

}


////////////////////////////////////////////////////////////////////////


void MetPythonPointDataFile::close()

{

met_data.clear();

   //
   //  Don't reset the Type field
   //  Don't reset the PythonCommand
   //

return;

}


////////////////////////////////////////////////////////////////////////


bool MetPythonPointDataFile::open(const char *cur_command, bool use_xarray)

{

close();

const char *method_name = "MetPythonPointDataFile::open() ";

   //
   //  Store the PythonCommand that is being run
   //

PythonCommand = cur_command;

bool status = python_point_data(PythonCommand, met_data, nullptr);

met_data.get_hdr_cnt();
met_data.get_obs_cnt();
MetPointHeader *hdr_data = met_data.get_header_data();
MetPointObsData *obs_data = met_data.get_point_obs_data();

print_met_data(obs_data, hdr_data, method_name);


   //
   //  done
   //

return status;

}


////////////////////////////////////////////////////////////////////////


void MetPythonPointDataFile::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "File = ";

   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


MetPointDataPython *MetPythonPointDataFile::get_met_point_data()

{

return &met_data;

}


////////////////////////////////////////////////////////////////////////

