

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cstdio>
#include <cmath>

#include "indent.h"
#include "vx_log.h"
#include "empty_string.h"
#include "bool_to_string.h"
#include "is_bad_data.h"

#include "config_file.h"


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

Dictionary::assign(*this);

Debug = c.Debug;

return;

}


////////////////////////////////////////////////////////////////////////


void MetConfig::dump(ostream & out, int depth) const

{

Indent prefix(depth);

out << prefix << "Filename ... ";

Filename.dump(out, depth + 1);

out << prefix << "Debug    = "   << bool_to_string(Debug) << "\n";

out << prefix << "Entries ...\n";

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

Filename.add(name);

configdebug = (Debug ? 1 : 0);

if ( (configin = fopen(bison_input_filename, "r")) == NULL )  {

   mlog << Error
        << "\nMetConfig::read(const char *) -> "
        << "unable to open input file \"" << name << "\"\n\n";

   exit ( 1 );

}

int parse_status;

parse_status = configparse();

if ( parse_status != 0 )  {

   return ( false );

}

if ( DS.n_elements() != 1 )  {

   mlog << Error 
        << "\nMetConfig::read(const char *) -> "
        << "should be only one dictionary left after parsing! ...("
        << DS.n_elements() << ")\n\n";

   DS.dump(cout);

   exit ( 1 );

}

   //
   //  done
   //

bison_input_filename = (const char *) 0;

dict_stack = (DictionaryStack *) 0;

LineNumber = 1;

Column     = 1;

is_lhs     = true;

return ( true );

}


////////////////////////////////////////////////////////////////////////


const DictionaryEntry * MetConfig::lookup(const char * name)

{

const DictionaryEntry * e = (const DictionaryEntry *) 0;

e = Dictionary::lookup(name);

LastLookupStatus = (e != 0);

return ( e );

}


////////////////////////////////////////////////////////////////////////


bool MetConfig::lookup_bool(const char * name, bool error_out)

{

const DictionaryEntry * Entry = lookup(name);
bool is_correct_type = false;

if ( Entry )  is_correct_type = (Entry->type() == BooleanType);

LastLookupStatus = is_correct_type;

if ( !Entry || !is_correct_type )  {

   if ( error_out )  {

      mlog << Error
           << "\nMetConfig::lookup_bool() -> "
           << "lookup failed for name \"" << name << "\"\n\n";

      exit ( 1 );

   }

   return ( false );

}

return ( Entry->b_value() );

}


////////////////////////////////////////////////////////////////////////


int MetConfig::lookup_int(const char * name, bool error_out)

{

const DictionaryEntry * Entry = lookup(name);
bool is_correct_type = false;

if ( Entry )  is_correct_type = (Entry->type() == IntegerType);

LastLookupStatus = is_correct_type;

if ( !Entry || !is_correct_type )  {

   if ( error_out )  {

      mlog << Error
           << "\nMetConfig::lookup_int() -> "
           << "lookup failed for name \"" << name << "\"\n\n";

      exit ( 1 );

   }

   return ( bad_data_int );

}

return ( Entry->i_value() );

}


////////////////////////////////////////////////////////////////////////


double MetConfig::lookup_double(const char * name, bool error_out)

{

const DictionaryEntry * Entry = lookup(name);
bool is_correct_type = false;

if ( Entry )  is_correct_type = (Entry->type() == FloatType);

LastLookupStatus = is_correct_type;

if ( !Entry || !is_correct_type )  {

   if ( error_out )  {

      mlog << Error
           << "\nMetConfig::lookup_double() -> "
           << "lookup failed for name \"" << name << "\"\n\n";

      exit ( 1 );

   }

   return ( bad_data_double );

}

return ( Entry->d_value() );

}


////////////////////////////////////////////////////////////////////////


NumArray MetConfig::lookup_num_array(const char * name, bool error_out)

{

const DictionaryEntry * Entry = lookup(name);
const Dictionary * Dict = (const Dictionary *) 0;
ConfigObjectType Type = no_config_object_type;
bool is_correct_type = false;
NumArray array;

if ( Entry )  is_correct_type = (Entry->type() == ArrayType);

LastLookupStatus = is_correct_type;

if ( !Entry || !is_correct_type )  {

   if ( error_out )  {

      mlog << Error
           << "\nMetConfig::lookup_num_array() -> "
           << "lookup failed for name \"" << name << "\"\n\n";

      exit ( 1 );

   }

   return ( array );

}

   //
   //  Retrieve the array dictionary
   //

Dict = Entry->array_value();

   //
   //  Check the array type.
   //  Error out if array contains unexpected type.
   //

if ( Dict->n_entries() > 0 )  {

   Type = (*Dict)[0]->type();
  
   if( Type != IntegerType && Type != FloatType )  {

      mlog << Error
           << "\nMetConfig::lookup_num_array() -> "
           << "array \"" << name
           << "\" does not contain numeric entries.\n\n";

      exit ( 1 );

   }
}

   //
   //  Populate the NumArray with integers or doubles
   //

for (int i=0; i<Dict->n_entries(); i++)  {

   if( Type == IntegerType)  array.add((*Dict)[i]->i_value());
   else                      array.add((*Dict)[i]->d_value());

}

return ( array );

}


////////////////////////////////////////////////////////////////////////


ConcatString MetConfig::lookup_string(const char * name, bool error_out)

{

const DictionaryEntry * Entry = lookup(name);
bool is_correct_type = false;

if ( Entry )  is_correct_type = (Entry->type() == StringType);

LastLookupStatus = is_correct_type;

if ( !Entry || !is_correct_type )  {

   if ( error_out )  {

      mlog << Error
           << "\nMetConfig::lookup_string() -> "
           << "lookup failed for name \"" << name << "\"\n\n";

      exit ( 1 );

   }

   ConcatString s;

   return ( s );

}

return ( *(Entry->string_value()) );

}


////////////////////////////////////////////////////////////////////////


StringArray MetConfig::lookup_string_array(const char * name, bool error_out)

{

const DictionaryEntry * Entry = lookup(name);
const Dictionary * Dict = (const Dictionary *) 0;
bool is_correct_type = false;
StringArray array;

if ( Entry )  is_correct_type = (Entry->type() == ArrayType);

LastLookupStatus = is_correct_type;

if ( !Entry || !is_correct_type )  {

   if ( error_out )  {

      mlog << Error
           << "\nMetConfig::lookup_string_array() -> "
           << "lookup failed for name \"" << name << "\"\n\n";

      exit ( 1 );

   }

   return ( array );

}

   //
   //  Retrieve the array dictionary
   //
   
Dict = Entry->array_value();

   //
   //  Check the array type.
   //  Error out if array contains unexpected type.
   //

if ( Dict->n_entries() > 0 )  {

   if( (*Dict)[0]->type() != StringType )  {
  
      mlog << Error
           << "\nMetConfig::lookup_string_array() -> "
           << "array \"" << name
           << "\" does not contain StringType.\n\n";

      exit ( 1 );

   }
}

   //
   //  Populate the StringArray
   //
   
for (int i=0; i<Dict->n_entries(); i++)  {

   array.add(*(*Dict)[i]->string_value());
   
}

return ( array );

}


////////////////////////////////////////////////////////////////////////


SingleThresh MetConfig::lookup_thresh(const char * name, bool error_out)

{

const DictionaryEntry * Entry = lookup(name);
bool is_correct_type = false;

if ( Entry )  is_correct_type = (Entry->type() == ThresholdType);

LastLookupStatus = is_correct_type;

if ( !Entry || !is_correct_type )  {

   if ( error_out )  {

      mlog << Error
           << "\nMetConfig::lookup_thresh() -> "
           << "lookup failed for name \"" << name << "\"\n\n";

      exit ( 1 );

   }

   SingleThresh s;

   return ( s );

}

return ( *(Entry->thresh_value()) );

}


////////////////////////////////////////////////////////////////////////


ThreshArray MetConfig::lookup_thresh_array(const char * name, bool error_out)

{

const DictionaryEntry * Entry = lookup(name);
const Dictionary * Dict = (const Dictionary *) 0;
bool is_correct_type = false;
ThreshArray array;

if ( Entry )  is_correct_type = (Entry->type() == ArrayType);

LastLookupStatus = is_correct_type;

if ( !Entry || !is_correct_type )  {

   if ( error_out )  {

      mlog << Error
           << "\nMetConfig::lookup_thresh_array() -> "
           << "lookup failed for name \"" << name << "\"\n\n";

      exit ( 1 );

   }

   return ( array );

}

   //
   //  Retrieve the array dictionary
   //

Dict = Entry->array_value();

   //
   //  Check the array type.
   //  Error out if array contains unexpected type.
   //

if ( Dict->n_entries() > 0 )  {

   if( (*Dict)[0]->type() != ThresholdType )  {

      mlog << Error
           << "\nMetConfig::lookup_thresh_array() -> "
           << "array \"" << name
           << "\" does not contain ThreshType.\n\n";

      exit ( 1 );

   }
}

   //
   //  Populate the ThreshArray
   //

for (int i=0; i<Dict->n_entries(); i++)  {

   array.add(*(*Dict)[i]->thresh_value());

}

return ( array );

}


////////////////////////////////////////////////////////////////////////
