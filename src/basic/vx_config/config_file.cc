// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <cstdio>
#include <cmath>

#include "indent.h"
#include "vx_log.h"
#include "empty_string.h"
#include "bool_to_string.h"
#include "is_bad_data.h"
#include "temp_file.h"

#include "config_file.h"
#include "config_constants.h"


////////////////////////////////////////////////////////////////////////


   //
   //  external linkage
   //


extern int configparse();

extern FILE * configin;

extern int configdebug;

extern const char * bison_input_filename;

extern DictionaryStack * dict_stack;

extern int LineNumber;

extern int Column;

extern bool is_lhs;

extern void start_string_scan  (const char *);
extern void finish_string_scan ();


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class MetConfig
   //


////////////////////////////////////////////////////////////////////////


MetConfig::MetConfig()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


MetConfig::~MetConfig()

{

clear();

}


////////////////////////////////////////////////////////////////////////


MetConfig::MetConfig(const MetConfig & c)

{

init_from_scratch();

assign(c);

}


////////////////////////////////////////////////////////////////////////


MetConfig::MetConfig(const char * _filename)

{

init_from_scratch();

read( _filename );

}


////////////////////////////////////////////////////////////////////////


void MetConfig::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void MetConfig::clear()

{

Filename.clear();

Dictionary::clear();

Debug = false;

return;

}


////////////////////////////////////////////////////////////////////////


void MetConfig::assign(const MetConfig & c)

{

clear();

Filename = c.Filename;

Dictionary::assign(c);

Debug = c.Debug;

return;

}


////////////////////////////////////////////////////////////////////////


void MetConfig::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Filename ... \n";

Filename.dump(out, depth + 1);

out << prefix << "\n";

out << prefix << "Debug    = "   << bool_to_string(Debug) << "\n";

out << prefix << "Config File Entries ...\n";

Dictionary::dump(out, depth + 1);


   //
   //  done
   //

out.flush();

return;

}


////////////////////////////////////////////////////////////////////////


void MetConfig::set_debug(bool tf)

{

Debug = tf;

return;

}


////////////////////////////////////////////////////////////////////////


void MetConfig::set_exit_on_warning()

{

bool b = lookup_bool(conf_key_exit_on_warning, false);

if ( LastLookupStatus )  mlog.set_exit_on_warning(b);

return;

}


////////////////////////////////////////////////////////////////////////


int MetConfig::output_precision()

{

int n = lookup_int(conf_key_output_precision, false);

if ( !LastLookupStatus )  n = default_precision;

return ( n );

}


////////////////////////////////////////////////////////////////////////


bool MetConfig::read(const char * name)

{

if ( empty(name) )  {

   mlog << Error
        << "\nMetConfig::read(const char *) -> "
        << "empty filename!\n\n";

   exit ( 1 );

}

// clear();

DictionaryStack DS(*this);

bison_input_filename = name;

dict_stack = &DS;

LineNumber = 1;

Column     = 1;

is_lhs     = true;

Filename.add(bison_input_filename);

configdebug = (Debug ? 1 : 0);

if ( (configin = fopen(bison_input_filename, "r")) == NULL )  {

   mlog << Error
        << "\nMetConfig::read(const char *) -> "
        << "unable to open input file \"" << name << "\"\n\n";

   exit ( 1 );

}

int parse_status;

parse_status = configparse();

if ( configin )  {

   fclose(configin);
   configin = (FILE *) 0;
 
}

if ( parse_status != 0 )  {

   return ( false );

}

// DS.pop_dict("base");

if ( DS.n_elements() != 1 )  {

   mlog << Error 
        << "\nMetConfig::read(const char *) -> "
        << "should be only one dictionary left after parsing! ...("
        << DS.n_elements() << ")\n\n";

   DS.dump(cout);
   DS.dump_config_format(cout);

   mlog << Error 
        << "\n"
        << "  parse failed!\n"
        << "\n";

   exit ( 1 );

}

   //
   //  done
   //

patch_parents();

bison_input_filename = (const char *) 0;

dict_stack = (DictionaryStack *) 0;

LineNumber = 1;

Column     = 1;

is_lhs     = true;

set_exit_on_warning();

return ( true );

}


////////////////////////////////////////////////////////////////////////


bool MetConfig::read_string(const char * s)

{

if ( empty(s) )  {

   mlog << Error
        << "\n\n  MetConfig::read_string(const char *) -> empty input string!\n\n";

   exit ( 1 );

}

ofstream out;
const ConcatString temp_filename = make_temp_file_name("config", ".temp");

out.open(temp_filename);

if ( ! out )  {

   mlog << Error
        << "\n\n  MetConfig::read_string(const char *) -> unable to open temp file \""
        << temp_filename << "\"\n\n";

   exit ( 1 );

}

out << s << '\n';

out.close();

bool status = read(temp_filename);

remove_temp_file(temp_filename);

return ( status );

}


////////////////////////////////////////////////////////////////////////


const DictionaryEntry * MetConfig::lookup(const char * name)

{

const DictionaryEntry * _e = (const DictionaryEntry *) 0;

_e = Dictionary::lookup(name);

LastLookupStatus = (_e != 0);

return ( _e );

}


////////////////////////////////////////////////////////////////////////


const DictionaryEntry * MetConfig::lookup(const char * name, const ConfigObjectType expected_type)

{

const DictionaryEntry * _e = (const DictionaryEntry *) 0;

_e = Dictionary::lookup(name);

if ( !e || (_e->type() != expected_type) )  {

   LastLookupStatus = false;

   _e = 0;

}

return ( _e );

}


////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////
