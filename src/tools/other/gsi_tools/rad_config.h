

////////////////////////////////////////////////////////////////////////


#ifndef  __GSI_RAD_CONFIG_H__
#define  __GSI_RAD_CONFIG_H__


////////////////////////////////////////////////////////////////////////


#include "config_file.h"
#include "threshold.h"


////////////////////////////////////////////////////////////////////////


class GsiRadConfig {

   protected:

      void init_from_scratch();

      // void assign(const GsiRadConfig &);

      void get_config_data();

      MetConfig Config;


      SingleThresh data_threshold;

   public:

      GsiRadConfig();
     ~GsiRadConfig();
      GsiRadConfig(const GsiRadConfig &);
      GsiRadConfig & operator=(const GsiRadConfig &);

      void clear();

         //
         //  do stuff
         //

      bool read(const char * filename);

      bool data_ok(double value) const;   //  meets data threshold?

};


////////////////////////////////////////////////////////////////////////


inline bool GsiRadConfig::data_ok(double __value__) const { return ( data_threshold.check(__value__) ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __GSI_RAD_CONFIG_H__  */


////////////////////////////////////////////////////////////////////////


