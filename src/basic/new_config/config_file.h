

////////////////////////////////////////////////////////////////////////


#ifndef  __MET_CONFIG_FILE_H__
#define  __MET_CONFIG_FILE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "string_array.h"

#include "dictionary.h"


////////////////////////////////////////////////////////////////////////


class MetConfig {

   private:

      void init_from_scratch();

      void assign(const MetConfig &);

      StringArray Filename;

      Dictionary Dict;

      bool Debug;

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

         //
         //  do stuff
         //

      bool read(const char * filename);

      const DictionaryEntry * lookup(const char * name) const;

};


////////////////////////////////////////////////////////////////////////


inline StringArray MetConfig::filename() const { return ( Filename ); }

inline bool MetConfig::debug() const { return ( Debug ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_CONFIG_FILE_H__  */


////////////////////////////////////////////////////////////////////////


