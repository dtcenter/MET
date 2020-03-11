// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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

static const int i_fho = 0;
static const int i_ctc = 1;
static const int i_cts = 2;

static const int n_txt = 3;

// Text file type
static const STATLineType txt_file_type[n_txt] = {
   stat_fho,        //  0
   stat_ctc,        //  1
   stat_cts         //  2
};

////////////////////////////////////////////////////////////////////////

//
// Struct to store genesis event defintion criteria
//

struct GenesisEventInfo {
   ConcatString         Technique;
   vector<CycloneLevel> Category;
   SingleThresh         VMaxThresh;
   SingleThresh         MSLPThresh;

   bool                 is_keeper(const TrackPoint &);
   void                 clear();
};

extern GenesisEventInfo parse_conf_genesis_event_info(Dictionary *dict);

////////////////////////////////////////////////////////////////////////

struct GenCTCInfo {
   ConcatString model;
   CTSInfo cts_info;
   unixtime fbeg, fend, obeg, oend;

   GenCTCInfo();

   void clear();
   GenCTCInfo & operator+=(const GenCTCInfo &);

   void add_fcst_valid(const unixtime, const unixtime);
   void add_obs_valid (const unixtime, const unixtime);
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
      unixtime  ValidBeg, ValidEnd;
      NumArray  InitHour;
      NumArray  Lead;

      // Polyline masking region
      ConcatString VxMaskName;
      MaskPoly     VxPolyMask;
      Grid         VxGridMask;
      MaskPlane    VxAreaMask;

      // Distance to land threshold
      SingleThresh DLandThresh;

      // Temporal and spatial matching criteria
      int GenesisSecBeg, GenesisSecEnd;
      double GenesisRadius;

      //////////////////////////////////////////////////////////////////

      void clear();

      void process_config(Dictionary &);

      bool is_keeper(const GenesisInfo &);
};

////////////////////////////////////////////////////////////////////////

class TCGenConfInfo {

   private:

      void init_from_scratch();

   public:

      TCGenConfInfo();
     ~TCGenConfInfo();

      //////////////////////////////////////////////////////////////////

      // TCPairs configuration object
      MetConfig Conf;

      // Vector of vx task filtering options [n_vx]
      std::vector<TCGenVxOpt> VxOpt;

      // Forecast initialization frequency in hours
      int InitFreqSec;

      // Begin and end forecast hours for genesis
      int LeadSecBeg, LeadSecEnd;

      // Minimum track duration
      int MinDur;

      // Genesis event criteria
      GenesisEventInfo FcstEventInfo;
      GenesisEventInfo BestEventInfo;
      GenesisEventInfo OperEventInfo;

      // Gridded data file containing distances to land
      ConcatString DLandFile;
      Grid         DLandGrid;
      DataPlane    DLandData;

      // Config file version
      ConcatString Version;

      // Output file options
      double CIAlpha;
      map<STATLineType,STATOutputType> OutputMap;

      //////////////////////////////////////////////////////////////////

      void clear();

      void read_config(const char *, const char *);

      void process_config();

      double compute_dland(double lat, double lon);

      int n_vx() const;
};

////////////////////////////////////////////////////////////////////////

inline int TCGenConfInfo::n_vx() const { return(VxOpt.size()); }

////////////////////////////////////////////////////////////////////////

#endif   /*  __TC_GEN_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
