// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "pointdata_python.h"
#include "pointdata_from_array.h"
#include "vx_python3_utils.h"

#include "vx_math.h"
#include "vx_log.h"


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


bool MetPythonPointDataFile::open(const char * cur_command, bool use_xarray)

{

close();

ConcatString full_path, file_name;
int i, file_argc;
char **file_argv = (char **) nullptr; // allocated
StringArray sa;
const char *method_name = "MetPythonPointDataFile::open() ";

   //
   //  Store the PythonCommand that is being run
   //

PythonCommand = cur_command;

   //
   //  parse and store argc and argv
   //

sa = PythonCommand.split(" ");

file_argc = sa.n_elements();

if ( file_argc > 0 )  {
   file_argv = new char * [ file_argc ];
   char a_var_name[512+1];

   for ( i=0; i<sa.n_elements(); i++ )  {
      int buf_len = sa[i].length();
      snprintf(a_var_name, 512, "file_argv[%d]", i);
      file_argv[i] = m_strcpy2(sa[i].c_str(), method_name, a_var_name);
   }
}

   //
   //  Build the path and store the file name
   //

full_path = sa[0];

file_name = full_path;

file_name.chomp(".py");   //  remove possible ".py" suffix from script filename

MaskFilters *filters = 0;
bool status = python_point_data(file_name.c_str(), file_argc, file_argv,
                                met_data, filters);

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

