// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __MODE_FIELD_INFO_H__
#define  __MODE_FIELD_INFO_H__


////////////////////////////////////////////////////////////////////////


#include "vx_config.h"
#include "vx_data2d.h"
#include "vx_grid.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_math.h"


////////////////////////////////////////////////////////////////////////


typedef std::map<ConcatString, ThreshArray>  AttrFilterMap;


////////////////////////////////////////////////////////////////////////


class Mode_Field_Info {

   protected:

      void init_from_scratch();

      void assign(const Mode_Field_Info &);


      Dictionary * dict;   //  not allocated

      MetConfig * conf;    //  not allocated

      GrdFileType gft;

      char FO;   //  'F' or 'O', for fcst or obs

      bool Multivar;

   public:

      Mode_Field_Info();
     ~Mode_Field_Info();
      Mode_Field_Info(const Mode_Field_Info &);
      Mode_Field_Info & operator=(const Mode_Field_Info &);

      void clear();

      void set (const bool _multivar, int _index, Dictionary *, MetConfig *, GrdFileType, char _fo, bool do_clear = false);

      int index;

         //
         //  member data
         //

      int            conv_radius;         // Convolution radius in grid squares
      double         vld_thresh;          // Minimum ratio of valid data points in the convolution area

      VarInfo *      var_info;            // allocated
      IntArray       conv_radius_array;   // List of convolution radii in grid squares

      ThreshArray    conv_thresh_array;   // List of conv thresholds to use
      ThreshArray    merge_thresh_array;  // Lower convolution threshold used for double merging method
      SingleThresh   conv_thresh;         // Convolution threshold to define objects
      SingleThresh   merge_thresh;        // Lower convolution threshold used for double merging method

      MergeType      merge_flag;          // Define which merging methods should be employed

      PlotInfo       raw_pi;              // Raw forecast plotting info

         //
         //  member functions
         //

      void           set_merge_thresh_by_index (int);
      int            n_merge_threshs () const;
      bool           need_merge_thresh () const;   //  mergetype is both or thresh
      AttrFilterMap  filter_attr_map;              //  Discard objects that don't meet these attribute thresholds

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __MODE_FIELD_INFO_H__  */


////////////////////////////////////////////////////////////////////////


