

////////////////////////////////////////////////////////////////////////


#ifndef  __MET_CONFIG_FILE_H__
#define  __MET_CONFIG_FILE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "string_array.h"

#include "dictionary.h"


////////////////////////////////////////////////////////////////////////


static const bool default_config_error_out = true;


////////////////////////////////////////////////////////////////////////


class MetConfig : public Dictionary {

   private:

      void init_from_scratch();

      void assign(const MetConfig &);

      StringArray Filename;

      bool Debug;

      bool LastLookupStatus;

   public:

      MetConfig();
     ~MetConfig();
      MetConfig(const MetConfig &);
      MetConfig & operator=(const MetConfig &);

      void clear();

      void dump(ostream &, int = 0) const;

         //
         //  set stuff
         //

      void set_debug(bool = true);

         //
         //  get stuff
         //

      StringArray filename() const;

      bool debug() const;

      bool last_lookup_status () const;

         //
         //  do stuff
         //

      bool read(const char * filename);

      const DictionaryEntry * lookup(const char * name);

         //
         //  convenience functions
         //

      bool         lookup_bool   (const char * name, bool error_out = default_config_error_out);
      int          lookup_int    (const char * name, bool error_out = default_config_error_out);
      double       lookup_double (const char * name, bool error_out = default_config_error_out);
      ConcatString lookup_string (const char * name, bool error_out = default_config_error_out);


};


////////////////////////////////////////////////////////////////////////


inline StringArray MetConfig::filename() const { return ( Filename ); }

inline bool MetConfig::debug() const { return ( Debug ); }

inline bool MetConfig::last_lookup_status() const { return ( LastLookupStatus ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_CONFIG_FILE_H__  */


////////////////////////////////////////////////////////////////////////


