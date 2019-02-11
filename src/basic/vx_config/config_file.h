// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __MET_CONFIG_FILE_H__
#define  __MET_CONFIG_FILE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "dictionary.h"
#include "config_funcs.h"


////////////////////////////////////////////////////////////////////////


class MetConfig : public Dictionary {

   private:

      void init_from_scratch();

      void assign(const MetConfig &);

      StringArray Filename;

      bool Debug;

   public:

      MetConfig();
     ~MetConfig();
      MetConfig(const MetConfig &);
      MetConfig & operator=(const MetConfig &);
      MetConfig(const char *filename);

      void clear();

      void dump(ostream &, int = 0) const;

         //
         //  set stuff
         //

      void set_debug(bool = true);

      void set_exit_on_warning();

         //
         //  get stuff
         //

      int nc_compression();

      int output_precision();
      
      ConcatString get_tmp_dir();

      StringArray filename() const;

      bool debug() const;

         //
         //  do stuff
         //

      bool read(const char * filename);

      bool read_string(const char *);

      const DictionaryEntry * lookup(const char * name);

      const DictionaryEntry * lookup(const char * name, const ConfigObjectType expected_type);

};


////////////////////////////////////////////////////////////////////////


inline StringArray MetConfig::filename() const { return ( Filename ); }

inline bool MetConfig::debug() const { return ( Debug ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_CONFIG_FILE_H__  */


////////////////////////////////////////////////////////////////////////


