// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __MODE_FRONTEND_H__
#define  __MODE_FRONTEND_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include "mode_exec.h"
#include "string_array.h"
#include "multivar_data.h"

class ModeFrontEnd {

   private:

     //void init_from_scratch();

   public:

      ModeFrontEnd();
     ~ModeFrontEnd();
     int run(const StringArray & Argv);
     int run(const StringArray & Argv, const MultiVarData &mvd);

  //ModeExecutive * mode_exec;
  string default_out_dir;
  //int compress_level;
  // int field_index;

  void do_quilt    ();
  void do_straight ();

  MultiVarData *get_multivar_data();

  void process_command_line(const StringArray &);
  static void set_config_merge_file (const StringArray &);
  static void set_outdir            (const StringArray &);
  static void set_logfile           (const StringArray &);
  static void set_verbosity         (const StringArray &);
  static void set_compress          (const StringArray &);
  static void set_field_index       (const StringArray &);   //  undocumented



};


#endif   /*  __MODE_FRONT_END_H__  */


/////////////////////////////////////////////////////////////////////////
