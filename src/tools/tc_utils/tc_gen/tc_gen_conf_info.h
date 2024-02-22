// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __TC_GEN_CONF_INFO_H__
#define  __TC_GEN_CONF_INFO_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <vector>

#include "vx_statistics.h"
#include "vx_tc_util.h"
#include "vx_config.h"
#include "vx_grid.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

// Indices for the output flag types in the configuration file

static const int i_fho    = 0;
static const int i_ctc    = 1;
static const int i_cts    = 2;
static const int i_pct    = 3;
static const int i_pstd   = 4;
static const int i_pjc    = 5;
static const int i_prc    = 6;
static const int i_genmpr = 7;

static const int n_txt = 8;

// Text file type
static const STATLineType txt_file_type[n_txt] = {
   stat_fho,   //  0
   stat_ctc,   //  1
   stat_cts,   //  2
   stat_pct,   //  3
   stat_pstd,  //  4
   stat_pjc,   //  5
   stat_prc,   //  6
   stat_genmpr //  7
};

// Output data type names

static const std::string genesis_name      ("GENESIS");
static const std::string genesis_dev_name  ("GENESIS_DEV");
static const std::string genesis_ops_name  ("GENESIS_OPS");
static const std::string prob_genesis_name ("PROB_GENESIS");
static const std::string genesis_shape_name("GENESIS_SHAPE");

// Names for output data plane types

static const std::string fgen_str       = "fcst_genesis";
static const std::string ftrk_str       = "fcst_track";
static const std::string fdev_fyoy_str  = "fcst_dev_fy_oy";
static const std::string fdev_fyon_str  = "fcst_dev_fy_on";
static const std::string fops_fyoy_str  = "fcst_ops_fy_oy";
static const std::string fops_fyon_str  = "fcst_ops_fy_on";
static const std::string bgen_str       = "best_genesis";
static const std::string btrk_str       = "best_track";
static const std::string bdev_fyoy_str  = "best_dev_fy_oy";
static const std::string bdev_fnoy_str  = "best_dev_fy_on";
static const std::string bops_fyoy_str  = "best_ops_fy_oy";
static const std::string bops_fnoy_str  = "best_ops_fy_on";

static const int n_ncout = 12;

static const std::string ncout_str[n_ncout] = {
   fgen_str,      ftrk_str,      bgen_str,      btrk_str,
   fdev_fyoy_str, fdev_fyon_str, bdev_fyoy_str, bdev_fnoy_str,
   fops_fyoy_str, fops_fyon_str, bops_fyoy_str, bops_fnoy_str
};

////////////////////////////////////////////////////////////////////////

struct TCGenNcOutInfo {

   bool do_latlon;
   bool do_fcst_genesis;
   bool do_fcst_tracks;
   bool do_fcst_fy_oy;
   bool do_fcst_fy_on;
   bool do_best_genesis;
   bool do_best_tracks;
   bool do_best_fy_oy;
   bool do_best_fn_oy;

      //////////////////////////////////////////////////////////////////

   TCGenNcOutInfo();
   TCGenNcOutInfo & operator+=(const TCGenNcOutInfo &);

   void clear();   //  sets everything to true

   bool all_false() const;

   void set_all_false();
   void set_all_true();
};

////////////////////////////////////////////////////////////////////////

class TCGenVxOpt {

   private:

      void init_from_scratch();

   public:

      TCGenVxOpt();
     ~TCGenVxOpt();

      //////////////////////////////////////////////////////////////////

      ConcatString Desc;     // Description string
      StringArray  Model;    // Forecast ATCF ID's

      // Analysis track filtering criteria
      StringArray StormId;   // List of storm ids
      StringArray StormName; // List of storm names

      // Timing information
      unixtime  InitBeg, InitEnd;
      TimeArray InitInc, InitExc;
      unixtime  ValidBeg, ValidEnd;
      NumArray  InitHour;
      NumArray  Lead;

      // Spatial masking information
      ConcatString VxMaskConf;
      ConcatString VxMaskName;
      MaskPoly     VxPolyMask;
      Grid         VxGridMask;
      StringArray  VxBasinMask;
      MaskPlane    VxAreaMask;

      // Distance to land threshold
      SingleThresh DLandThresh;

      // Matching logic
      bool GenesisMatchPointTrack;

      // Temporal and spatial matching criteria
      double GenesisMatchRadius;
      int    GenesisMatchBeg, GenesisMatchEnd;

      // Temporal and spatial scoring options
      double DevHitRadius;
      int    DevHitBeg, DevHitEnd;
      int    OpsHitBeg, OpsHitEnd;
      bool   DiscardFlag, DevFlag, OpsFlag;

      // Output file options
      ThreshArray ProbGenThresh;
      double CIAlpha;
      std::map<STATLineType,STATOutputType> OutputMap;
      TCGenNcOutInfo NcInfo;
      SingleThresh ValidGenesisDHrThresh;
      bool BestUniqueFlag;

      //////////////////////////////////////////////////////////////////

      void clear();

      void process_config(Dictionary &);
      void process_basin_mask(const Grid &, const DataPlane &,
                              const StringArray &);
      void parse_nc_info(Dictionary &);

      bool is_keeper(const GenesisInfo &)  const;
      bool is_keeper(const ProbGenInfo &)  const;
      bool is_keeper(const GenShapeInfo &) const;

      STATOutputType output_map(STATLineType) const;
};

////////////////////////////////////////////////////////////////////////

class TCGenConfInfo {

   private:

      void init_from_scratch();

   public:

      TCGenConfInfo();
     ~TCGenConfInfo();

      //////////////////////////////////////////////////////////////////

      // TCGen configuration object
      MetConfig Conf;

      // Vector of vx task filtering options [n_vx]
      std::vector<TCGenVxOpt> VxOpt;

      // Forecast initialization frequency in hours
      int InitFreqHr;

      // Forecast lead time frequency in hours
      int ValidFreqHr;

      // Begin and end forecast hours for genesis
      int FcstSecBeg, FcstSecEnd;

      // Minimum track duration
      int MinDur;

      // Genesis event criteria
      GenesisEventInfo FcstEventInfo;
      GenesisEventInfo BestEventInfo;
      ConcatString     OperTechnique;

      // Gridded data file containing distances to land
      ConcatString DLandFile;
      Grid         DLandGrid;
      DataPlane    DLandData;

      // Gridded data file containing TC basins
      ConcatString BasinFile;
      Grid         BasinGrid;
      DataPlane    BasinData;
      StringArray  BasinAbbr;

      // Grid for NetCDF output file
      Grid NcOutGrid;

      // Summary of output file options across all filters
      std::map<STATLineType,STATOutputType> OutputMap;
      TCGenNcOutInfo NcInfo;
   
      // Config file version
      ConcatString Version;

      //////////////////////////////////////////////////////////////////

      void clear();

      void read_config(const char *, const char *);

      void process_config();
      void process_flags(const std::map<STATLineType,STATOutputType> &,
                         const TCGenNcOutInfo &);

      double compute_dland(double lat, double lon);

      ConcatString compute_basin(double lat, double lon);

      int n_vx() const;
      int compression_level();
   
      STATOutputType output_map(STATLineType) const;

      // Maximum across all verification tasks
      int get_max_n_prob_thresh() const;
};

////////////////////////////////////////////////////////////////////////

inline int TCGenConfInfo::n_vx()         const { return(VxOpt.size());          }
inline int TCGenConfInfo::compression_level()  { return(Conf.nc_compression()); }

////////////////////////////////////////////////////////////////////////

class GenCTCInfo {

   private:

      void init_from_scratch();

   public:

      GenCTCInfo();
     ~GenCTCInfo();

      //////////////////////////////////////////////////////////////////

   ConcatString Model;
   unixtime FcstBeg, FcstEnd;
   unixtime BestBeg, BestEnd;
   CTSInfo CTSDev, CTSOps;

   const TCGenVxOpt* VxOpt;
   const Grid *NcOutGrid;

   SingleThresh ValidGenesisDHrThresh;
   bool BestUniqueFlag;

   // Number of hits per BEST track genesis event
   std::map<const GenesisInfo *,int> BestDevHitMap;
   std::map<const GenesisInfo *,int> BestOpsHitMap;

   // Output DataPlane variables
   std::map<const std::string,DataPlane> DpMap;

      //////////////////////////////////////////////////////////////////

   void clear();

   void set_vx_opt(const TCGenVxOpt *, const Grid *);

   void inc_dev(GenesisPairCategory,
                const GenesisInfo *, const GenesisInfo *);
   void inc_ops(GenesisPairCategory,
                const GenesisInfo *, const GenesisInfo *);
   void inc_best_unique();

   void add_fcst_gen(const GenesisInfo &);
   void add_best_gen(const GenesisInfo &);

   void inc_pnt(double, double, const std::string &);
   void inc_trk(const GenesisInfo &, const std::string &);
};

////////////////////////////////////////////////////////////////////////

class ProbGenPCTInfo {

   private:

      void init_from_scratch();

      PCTInfo DefaultPCT;

   public:

      ProbGenPCTInfo();
     ~ProbGenPCTInfo();

      //////////////////////////////////////////////////////////////////

   ConcatString Model;
   ConcatString VarName;
   ConcatString VxMask;

   unixtime InitBeg, InitEnd;
   unixtime BestBeg, BestEnd;
   const TCGenVxOpt* VxOpt;
   IntArray LeadTimes;

   // Map of lead times to PCT tables
   std::map<int,PCTInfo> PCTMap;

   // Map of lead times to vectors of pair info
   std::map<int,std::vector<const ProbGenInfo *> >  ProbGenMap;
   std::map<int,std::vector<const GenShapeInfo *> > GenShapeMap;
   std::map<int,std::vector<int> >                  ProbIdxMap;
   std::map<int,std::vector<const GenesisInfo *> >  BestGenMap;
   std::map<int,std::vector<bool> >                 BestEvtMap;

      //////////////////////////////////////////////////////////////////

   void clear();

   void set_vx_opt(const TCGenVxOpt *);

   void add_probgen(const ProbGenInfo &, int,
                    const GenesisInfo *, bool);

   void add_genshape(const GenShapeInfo &, int,
                     const GenesisInfo *, bool);

};

////////////////////////////////////////////////////////////////////////

#endif   /*  __TC_GEN_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
