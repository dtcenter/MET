// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __MTD_TXT_OUTPUT_H__
#define  __MTD_TXT_OUTPUT_H__


////////////////////////////////////////////////////////////////////////


#include "mtd_file_int.h"
#include "mtd_file_float.h"
#include "mtd_config_info.h"
#include "3d_att_single_array.h"
#include "3d_att_single_array.h"
#include "3d_att_pair_array.h"
#include "3d_single_columns.h"
#include "3d_pair_columns.h"
#include "3d_txt_header.h"
#include "2d_columns.h"
#include "2d_att_array.h"


////////////////////////////////////////////////////////////////////////


extern void do_3d_single_txt_output(const SingleAtt3DArray & fcst_att, 
                                    const SingleAtt3DArray &  obs_att, 
                                    const MtdConfigInfo &, 
                                    const char * output_filename);


extern void do_3d_pair_txt_output(const PairAtt3DArray &, 
                                  const MtdConfigInfo &, 
                                  const bool is_cluster, 
                                  const char * output_filename);


extern void do_2d_txt_output(const MtdFloatFile & fcst_raw, 
                             const MtdFloatFile &  obs_raw, 
                             const SingleAtt2DArray & fcst_single_att, 
                             const SingleAtt2DArray &  obs_single_att, 
                             const SingleAtt2DArray & fcst_cluster_att, 
                             const SingleAtt2DArray &  obs_cluster_att, 
                             const MtdConfigInfo &, 
                             const char * output_filename);

   //
   //  for single fields
   //

extern void do_2d_txt_output(const MtdFloatFile & fcst_raw, 
                             const MtdFloatFile &  obs_raw, 
                             const SingleAtt2DArray &, 
                             const MtdConfigInfo &, 
                             const char * output_filename);


extern void do_3d_single_txt_output(const SingleAtt3DArray &, 
                                    const MtdConfigInfo &, 
                                    const char * output_filename);


////////////////////////////////////////////////////////////////////////


#endif   /*  __MTD_TXT_OUTPUT_H__  */


////////////////////////////////////////////////////////////////////////


