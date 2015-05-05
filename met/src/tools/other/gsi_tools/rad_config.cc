

////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "rad_config.h"


////////////////////////////////////////////////////////////////////////


static const char data_threshold_key [] = "data_threshold";


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class GsiRadConfig
   //


////////////////////////////////////////////////////////////////////////


GsiRadConfig::GsiRadConfig()

{

init_from_scratch();

}


////////////////////////////////////////////////////////////////////////


GsiRadConfig::~GsiRadConfig()

{

clear();

}


////////////////////////////////////////////////////////////////////////


GsiRadConfig::GsiRadConfig(const GsiRadConfig & c)

{

// init_from_scratch();
// 
// assign(c);

cerr << "\n\n  GsiRadConfig::GsiRadConfig(const GsiRadConfig &) -> should never be called!\n\n";

exit ( 1 );

}


////////////////////////////////////////////////////////////////////////


GsiRadConfig & GsiRadConfig::operator=(const GsiRadConfig & c)

{

// if ( this == &c )  return ( * this );
// 
// assign(c);

cerr << "\n\n  GsiRadConfig::operator=(const GsiRadConfig &) -> should never be called!\n\n";

exit ( 1 );


return ( * this );

}


////////////////////////////////////////////////////////////////////////


void GsiRadConfig::init_from_scratch()

{

clear();

return;

}


////////////////////////////////////////////////////////////////////////


void GsiRadConfig::clear()

{

Config.clear();

data_threshold.clear();

return;

}


////////////////////////////////////////////////////////////////////////


bool GsiRadConfig::read(const char * filename)

{

clear();

const bool status = Config.read(filename);

if ( ! status )  return ( status );

get_config_data();

return ( status );

}


////////////////////////////////////////////////////////////////////////


void GsiRadConfig::get_config_data()

{

const DictionaryEntry * e = Config.lookup(data_threshold_key);

if ( !e )  {

   cerr << "\n\n  GsiRadConfig::get_config_data() -> lookup failed for key \"" << data_threshold_key << "\"\n\n";

   exit ( 1 );

}

if ( e->type() != ThresholdType )  {

   cerr << "\n\n  GsiRadConfig::get_config_data() -> bad object type for key \"" << data_threshold_key << "\"\n\n";

   exit ( 1 );

}

data_threshold = *(e->thresh_value());

   //
   // done
   //

return;

}


////////////////////////////////////////////////////////////////////////


