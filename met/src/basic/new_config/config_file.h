

////////////////////////////////////////////////////////////////////////


#ifndef  __MET_CONFIG_FILE_H__
#define  __MET_CONFIG_FILE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "dictionary.h"


////////////////////////////////////////////////////////////////////////


class MetConfig {

   private:

      void init_from_scratch();

      void assign(const MetConfig &);

      ConcatString Filename;

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

      ConcatString filename() const;

      bool debug() const;

         //
         //  do stuff
         //

      bool read(const char * filename);

      const DictionaryEntry * lookup(const char * name) const;



};


////////////////////////////////////////////////////////////////////////


inline ConcatString MetConfig::filename() const { return ( Filename ); }

inline bool MetConfig::debug() const { return ( Debug ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_CONFIG_FILE_H__  */


////////////////////////////////////////////////////////////////////////


