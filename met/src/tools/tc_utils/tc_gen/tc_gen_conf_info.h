// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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

#include "mask_poly.h"

#include "vx_tc_util.h"
#include "vx_config.h"
#include "vx_util.h"

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

class TCGenVxOpt {

   private:

      void init_from_scratch();

   public:

      TCGenVxOpt();
     ~TCGenVxOpt();

      //////////////////////////////////////////////////////////////////

      // Description string
      ConcatString Desc;

      // Model comparisons
      StringArray  Model;    // Forecast ATCF ID's

      // Track filtering criteria
      StringArray StormId;   // List of storm ids
      StringArray Basin;     // List of basin names
      StringArray Cyclone;   // List of cyclone numbers
      StringArray StormName; // List of storm names

      // Timing information
      unixtime  InitBeg, InitEnd;
      TimeArray InitInc;
      TimeArray InitExc;
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

      // Model initialization frequency in hours
      int InitFreq;

      // Begin and end forecast hours for genesis
      int LeadSecBeg, LeadSecEnd;

      // Minimum track duration
      int MinDur;

      // Genesis event criteria
      GenesisEventInfo ModelEventInfo;
      GenesisEventInfo  BestEventInfo;
      GenesisEventInfo  OperEventInfo;

      // Gridded data file containing distances to land
      ConcatString DLandFile;
      Grid         DLandGrid;
      DataPlane    DLandData;

      // Config file version
      ConcatString Version;

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
