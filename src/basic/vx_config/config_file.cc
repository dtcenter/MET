// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


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


using namespace std;


////////////////////////////////////////////////////////////////////////


   //
   //  external linkage
   //


extern int configparse();

extern stringstream * configin;

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

ConfigStream.clear();

Dictionary::clear();

Debug = false;

return;

}


////////////////////////////////////////////////////////////////////////


void MetConfig::assign(const MetConfig & c)

{

clear();

Filename = c.Filename;

ConfigStream << c.ConfigStream.str();

Dictionary::assign(c);

Debug = c.Debug;

return;

}


////////////////////////////////////////////////////////////////////////


void MetConfig::debug_dump(int depth) const
{
   dump(cout, depth);
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
      if ( LastLookupStatus ) tmp_dir = _e->string_value();
      else                    tmp_dir = default_tmp_dir;
   }

   return tmp_dir;
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

   return n;
}

////////////////////////////////////////////////////////////////////////

int MetConfig::output_precision()

{

int n = lookup_int(conf_key_output_precision, false);

if ( !LastLookupStatus )  n = default_precision;

return n;

}


////////////////////////////////////////////////////////////////////////


bool MetConfig::read(const char * filename)

{

set_buffer_from_file(filename);

return parse_buffer();

}


////////////////////////////////////////////////////////////////////////


bool MetConfig::read_string(const char * config_string)

{

set_buffer_from_string(config_string);

return parse_buffer();

}


////////////////////////////////////////////////////////////////////////


void MetConfig::set_buffer_from_file(const char * filename)

{

if ( empty(filename) )  {

   mlog << Error << "\nMetConfig::set_buffer_from_file(const char *) -> "
        << "empty filename!\n\n";

   exit ( 1 );

}

   // Open input config file

ifstream configfilein;

met_open(configfilein, filename);

if ( ! configfilein )  {

   mlog << Error << "\nMetConfig::read(const char *) -> "
        << "unable to open input file \"" << filename << "\". Please specify \"file_type = FileType_<type>;\" at the configration file\n\n";

   exit ( 1 );

}

   // Initialize stream and load contents 

ConfigStream.clear();

string line;

while ( getline(configfilein, line) )  {

   recursive_envs(line);

   ConfigStream << line << "\n";

}

   // Close the input file
 
configfilein.close();

bison_input_filename = filename;

Filename.add(filename);

return;

}


////////////////////////////////////////////////////////////////////////


void MetConfig::set_buffer_from_string(const char * config_string)

{

string line(config_string);

recursive_envs(line);

ConfigStream.clear();

ConfigStream << line << "\n";

bison_input_filename = "config_string";

Filename.add("config_string");

return;

}


////////////////////////////////////////////////////////////////////////


bool MetConfig::parse_buffer()

{

configin = &ConfigStream;

DictionaryStack DS(*this);

dict_stack = &DS;

LineNumber = 1;

Column     = 1;

is_lhs     = true;

configdebug = (Debug ? 1 : 0);

int parse_status = configparse();

if ( parse_status != 0 )  {

   return false;

}

if ( DS.n_elements() != 1 )  {

   mlog << Error << "\nMetConfig::parse_buffer(const char *) -> "
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

bison_input_filename = (const char *) nullptr;

dict_stack = (DictionaryStack *) nullptr;

LineNumber = 1;

Column     = 1;

is_lhs     = true;

set_exit_on_warning();

return true;

}


////////////////////////////////////////////////////////////////////////


const DictionaryEntry * MetConfig::lookup(const char * name)

{

const DictionaryEntry * _e = (const DictionaryEntry *) nullptr;

_e = Dictionary::lookup(name);

LastLookupStatus = (_e != nullptr);

return _e;

}


////////////////////////////////////////////////////////////////////////


const DictionaryEntry * MetConfig::lookup(const char * name, const ConfigObjectType expected_type)

{

const DictionaryEntry * _e = (const DictionaryEntry *) nullptr;

_e = Dictionary::lookup(name);

if ( !_e || (_e->type() != expected_type) )  {

   LastLookupStatus = false;

   _e = nullptr;

}

return _e;

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

size_t pos1;
size_t pos2;
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

if ( pos1 == string::npos )  return false;

   //
   //  replace the environment variable
   //

if ( (pos2 = line.find('}', pos1)) == string::npos )  {

   mlog << Error << "\nreplace_env(string &) -> "
        << "can't find closing bracket in string \"" << line << "\"\n\n";

   exit ( 1 );

}

string env;
char * env_value = nullptr;

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

return true;

}


////////////////////////////////////////////////////////////////////////
