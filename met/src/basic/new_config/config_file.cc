

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cstdio>
#include <cmath>

#include "indent.h"
#include "empty_string.h"
#include "bool_to_string.h"

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

Dict.clear();

Debug = false;

return;

}


////////////////////////////////////////////////////////////////////////


void MetConfig::assign(const MetConfig & c)

{

clear();

Filename = c.Filename;

Dict = c.Dict;

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

Dict.dump(out, depth + 1);


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


const DictionaryEntry *  MetConfig::lookup(const char * name) const

{

const DictionaryEntry * e = (const DictionaryEntry *) 0;

e = Dict.lookup(name);

return ( e );

}


////////////////////////////////////////////////////////////////////////


bool MetConfig::read(const char * name)

{

if ( empty(name) )  {

   cerr << "\n\n  MetConfig::read(const char *) -> empty filename!\n\n";

   exit ( 1 );

}

// clear();

DictionaryStack DS(Dict);

bison_input_filename = name;

dict_stack = &DS;

Filename.add(name);

configdebug = (Debug ? 1 : 0);

if ( (configin = fopen(bison_input_filename, "r")) == NULL )  {

   cerr << "\n\n  MetConfig::read(const char *) -> unable to open input file \"" << name << "\"\n\n";

   exit ( 1 );

}

int parse_status;

parse_status = configparse();

if ( parse_status != 0 )  {

   // cerr << "\n\n  " << program_name << " -> parse status is nonzero!\n\n";
   // 
   // exit ( 1 );

   return ( false );

}

if ( DS.n_elements() != 1 )  {

   cout << "\n\n  MetConfig::read(const char *) -> should be only one dictionary left after parsing! ...("
        << DS.n_elements() << ")\n\n";

   DS.dump(cout);

   exit ( 1 );

}

   //
   //  done
   //

bison_input_filename = (const char *) 0;

dict_stack = (DictionaryStack *) 0;

return ( true );

}


////////////////////////////////////////////////////////////////////////





