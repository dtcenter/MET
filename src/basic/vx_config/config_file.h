// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
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

      void set_buffer_from_file(const char *);

      void set_buffer_from_string(const char *);

      bool parse_buffer();

      StringArray Filename;

      std::stringstream ConfigStream;

      bool Debug;

   public:

      MetConfig();
     ~MetConfig();
      MetConfig(const MetConfig &);
      MetConfig & operator=(const MetConfig &);
      explicit MetConfig(const char *filename);

      void clear() override;

      void dump(std::ostream &, int = 0) const;

      void debug_dump(int = 0) const;

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

      bool time_offset_warning(int);

      StringArray filename() const;

      bool debug() const;

         //
         //  do stuff
         //

      bool read(const char * filename);

      bool read_string(const char * config_string);

      const DictionaryEntry * lookup(const char * name);

      const DictionaryEntry * lookup(const char * name, const ConfigObjectType expected_type);

};


////////////////////////////////////////////////////////////////////////


inline StringArray MetConfig::filename() const { return Filename; }

inline bool MetConfig::debug() const { return Debug; }


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_CONFIG_FILE_H__  */


////////////////////////////////////////////////////////////////////////


