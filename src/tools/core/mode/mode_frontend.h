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

 public:

   typedef enum {SINGLE_VAR, MULTIVAR_PASS1, MULTIVAR_PASS1_MERGE, MULTIVAR_PASS2, MULTIVAR_SUPER} Processing_t;

   ModeFrontEnd();
   ~ModeFrontEnd();


   string default_out_dir;

   // default single var mode interface, and multivar pass1 options
   int run(const StringArray & Argv, Processing_t ptype=SINGLE_VAR, int field_index=-1);

   // pass2 multivar interface
   int run_multivar_pass2(const StringArray & Argv, const MultiVarData &mvd, bool has_union,
                          ShapeData &f_merge, ShapeData &o_merge, int field_index);

   // multivar superobject interface, no intensities
   int run_super(const StringArray & Argv, ShapeData &f_super, ShapeData &o_super,
                 ShapeData &f_merge, ShapeData &o_merge,
                 GrdFileType ftype, GrdFileType otype, const Grid &grid, bool has_union);

   void do_quilt    (Processing_t ptype);

   // single var mode, or multivar pass1
   void do_straight (Processing_t ptype);

   // multivar pass2 
   void do_straight (Processing_t ptype, const MultiVarData &mvd,
                     ShapeData &f_merge, ShapeData &o_merge);

   // multivar super, no intensities
   void do_straight (Processing_t ptype, 
                     ShapeData &f_merge, ShapeData &o_merge);


   MultiVarData *get_multivar_data();
   void addMultivarMergePass1(MultiVarData *mvdi);

   void process_command_line(const StringArray &, bool is_multivar);
   // void process_command_line_multivar_pass2(const StringArray & argv,
   //                                          const MultiVarData &mvd);

   static string stype(Processing_t t);

   static void set_config_merge_file (const StringArray &);
   static void set_outdir            (const StringArray &);
   static void set_logfile           (const StringArray &);
   static void set_verbosity         (const StringArray &);
   static void set_compress          (const StringArray &);
   // static void set_field_index       (const StringArray &);   //  undocumented

};


#endif   /*  __MODE_FRONT_END_H__  */


/////////////////////////////////////////////////////////////////////////
