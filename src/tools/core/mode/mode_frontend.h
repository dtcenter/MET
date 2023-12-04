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
#include "mode_data_type.h"

class ModeFrontEnd {

   private:

   public:

   ModeFrontEnd();
   ~ModeFrontEnd();


   string default_out_dir;

   Grid create_verification_grid(const StringArray & Argv);

   // run the multivar simple object, where there is only one input data, either forecast or obs
   int create_multivar_simple_objects(const StringArray & Argv, ModeDataType dtype, const Grid &verification_grid,
                                      int field_index=-1, int n_files=1);

   // run the multivar simple object merge algorithm, with one input data, either forecast or obs
   int create_multivar_merge_objects(const StringArray & Argv, ModeDataType dtype, const Grid &verification_grid,
                                     int field_index=-1, int n_files=1);

   // run the default single var mode interface (traditional  mode)
   int run_traditional(const StringArray & Argv); 

   // run the multivar intensity algorithm, where one forecast and one obs are restricted to be within superobjects
   // and the traditional mode algorithm compares them
   int multivar_intensity_comparisons(const StringArray & Argv, const MultiVarData &mvdf, const MultiVarData &mvdo,
                                      bool has_union_f, bool has_union_o, ShapeData &merge_f,
                                      ShapeData &merge_o, int field_index_f, int field_index_o);

   // multivar superobject interface, with no intensities
   int run_super(const StringArray & Argv, ShapeData &f_super, ShapeData &o_super,
                 ShapeData &f_merge, ShapeData &o_merge,
                 GrdFileType ftype, GrdFileType otype, const Grid &grid, bool has_union);

   // so far only implemented for traditional mode
   void do_quilt    ();

   // MODE algorithm for traditional, multivar simple, or multivar merge cases
   void do_straight ();

   // MODE algorithm when doing multivar intensities
   void do_straight_multivar_intensity (const MultiVarData &mvdf,
                                        const MultiVarData &mvdo, ShapeData &mergef,
                                        ShapeData &mergeo);

   // MODE algorithm when doing multivar super with no intensities
   void do_straight_multivar_super (ShapeData &f_merge, ShapeData &o_merge);


   MultiVarData *get_multivar_data(ModeDataType dtype);

   void add_multivar_merge_data(MultiVarData *mvdi, ModeDataType dtype);

   void init(ModeExecutive::Processing_t p);
   void do_straight_init(int &NCT, int &NCR) const;

   void process_command_line_for_simple_objects(const StringArray &, ModeDataType dtype);
   void process_command_line(const StringArray &, bool is_multivar);

   static void set_config_merge_file (const StringArray &);
   static void set_outdir            (const StringArray &);
   static void set_logfile           (const StringArray &);
   static void set_verbosity         (const StringArray &);
   static void set_compress          (const StringArray &);

};


#endif   /*  __MODE_FRONT_END_H__  */


/////////////////////////////////////////////////////////////////////////
