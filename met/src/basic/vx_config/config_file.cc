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


static void recursive_envs(const char * infile, const char * outfile);

static void recursive_envs(string &);

static bool replace_env(string &);


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


ConcatString MetConfig::get_tmp_dir()
{
   ConcatString tmp_dir;

   // Use the MET_TMP_DIR environment variable, if set.
   if(!get_env("MET_TMP_DIR", tmp_dir)) {
      const DictionaryEntry * _e = lookup(conf_key_tmp_dir);
      if ( LastLookupStatus ) tmp_dir = *(_e->string_value());
      else                    tmp_dir = default_tmp_dir;
   }

   return ( tmp_dir );
}


////////////////////////////////////////////////////////////////////////

int MetConfig::nc_compression()
{
   ConcatString cs;
   int n = 0;

   // Use the MET_NC_COMPRESS environment variable, if set.
   if(get_env("MET_NC_COMPRESS", cs)) {
      n = atoi(cs.c_str());
   }
   else {
      n = lookup_int(conf_key_nc_compression, false);
      if ( !LastLookupStatus )  n = default_nc_compression;
   }

   return ( n );
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

   mlog << Error << "\nMetConfig::read(const char *) -> "
        << "empty filename!\n\n";

   exit ( 1 );

}

DictionaryStack DS(*this);
ConcatString temp_filename = get_tmp_dir();

temp_filename << "/" << make_temp_file_name("met_config", 0);

recursive_envs(name, temp_filename.c_str());

bison_input_filename = (const char *) temp_filename.c_str();

dict_stack = &DS;

LineNumber = 1;

Column     = 1;

is_lhs     = true;

Filename.add(bison_input_filename);

configdebug = (Debug ? 1 : 0);

if ( (configin = met_fopen(bison_input_filename, "r")) == NULL )  {

   mlog << Error << "\nMetConfig::read(const char *) -> "
        << "unable to open input file \"" << bison_input_filename << "\"\n\n";

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

if ( DS.n_elements() != 1 )  {

   mlog << Error << "\nMetConfig::read(const char *) -> "
        << "should be only one dictionary left after parsing! ...("
        << DS.n_elements() << ")\n\n";

   DS.dump(cout);
   DS.dump_config_format(cout);

   mlog << Error << "\n"
        << "parse failed!\n\n";

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

unlink(temp_filename.c_str());

return ( true );

}


////////////////////////////////////////////////////////////////////////


bool MetConfig::read_string(const char * s)

{

if ( empty(s) )  {

   mlog << Error << "\nMetConfig::read_string(const char *) -> "
        << "empty input string!\n\n";

   exit ( 1 );

}

   //
   //  write temporary config file
   //  default to the current directory
   //

ofstream out;
ConcatString temp_filename = get_tmp_dir();

temp_filename << "/" << make_temp_file_name("config", ".temp");

 out.open(temp_filename.c_str());

if ( ! out )  {

   mlog << Error << "\nMetConfig::read_string(const char *) -> "
        << "unable to open temp file \"" << temp_filename << "\".\n"
        << "Set MET_TMP_DIR to specify a temporary directory.\n\n";

   exit ( 1 );

}

out << s << '\n';

out.close();

bool status = read(temp_filename.c_str());

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

if ( !_e || (_e->type() != expected_type) )  {

   LastLookupStatus = false;

   _e = 0;

}

return ( _e );

}


////////////////////////////////////////////////////////////////////////


void recursive_envs(const char * infile, const char * outfile)

{

ifstream in;
ofstream out;

met_open(in, infile);

if ( ! in )  {

   mlog << Error << "\nrecursive_envs() -> "
        << "unable to open input file \"" << infile << "\"\n\n";

   exit ( 1 );

}

met_open(out, outfile);


if ( ! out )  {

   mlog << Error << "\nrecursive_envs() -> "
        << "unable to open output file \"" << outfile << "\"\n\n";

   in.close();
   exit ( 1 );

}

string line;

while ( getline(in, line) )  {

   recursive_envs(line);

   out << line << '\n';

}

   //
   //  done
   //

in.close();
out.close();

return;

}


////////////////////////////////////////////////////////////////////////


void recursive_envs(string & line)

{

while ( 1 )  {

   if ( ! replace_env(line) )  break;

}

return;

}


////////////////////////////////////////////////////////////////////////


bool replace_env(string & line)

{

size_t pos1, pos2;
string out;

   //
   //  eliminate any trailing C++-style comments
   //

if ( (pos1 = line.find("//")) != string::npos )  {

   line.erase(pos1);

}

   //
   //  look for environment variables
   //

pos1 = line.find("${");

if ( pos1 == string::npos )  return ( false );

   //
   //  replace the environment variable
   //

if ( (pos2 = line.find('}', pos1)) == string::npos )  {

   mlog << Error << "\nreplace_env(string &) -> "
        << "can't find closing bracket in string \"" << line << "\"\n\n";

   exit ( 1 );

}

string env;
char * env_value = 0;

env = line.substr(pos1 + 2, pos2 - pos1 - 2);

env_value = getenv(env.c_str());

if ( ! env_value )  {

   mlog << Error << "\nreplace_env() -> "
        << "unable to get value for environment variable \""
        << (env.c_str()) << "\"\n\n";

   exit ( 1 );

}

out = line.substr(0, pos1);

out += env_value;

out += line.substr(pos2 + 1);

   //
   //  done
   //

line = out;

return ( true );

}


////////////////////////////////////////////////////////////////////////
