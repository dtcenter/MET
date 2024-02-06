// ** Copyright UCAR (c) 1992 - 2024
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

class ModeFrontEnd {

   private:

   public:

   ModeFrontEnd();
   ~ModeFrontEnd();


   string default_out_dir;


   // run the default single var mode interface (traditional  mode)
   int run_traditional(const StringArray & Argv); 

   void init();
   
   // so far only implemented for traditional mode
   void do_quilt    ();

   // MODE algorithm for traditional mode
   void do_straight ();

   void do_straight_init(int &NCT, int &NCR) const;

   void process_command_line(const StringArray &);

   static void set_config_merge_file (const StringArray &);
   static void set_outdir            (const StringArray &);
   static void set_logfile           (const StringArray &);
   static void set_verbosity         (const StringArray &);
   static void set_compress          (const StringArray &);

};


#endif   /*  __MODE_FRONT_END_H__  */


/////////////////////////////////////////////////////////////////////////
