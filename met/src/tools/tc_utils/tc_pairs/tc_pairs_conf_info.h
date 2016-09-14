// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __TC_PAIRS_CONF_INFO_H__
#define  __TC_PAIRS_CONF_INFO_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "mask_poly.h"

#include "vx_config.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

struct ConsensusInfo {
   ConcatString Name;
   StringArray  Members;
   NumArray     Required;
   int          MinReq;
};

////////////////////////////////////////////////////////////////////////

class TCPairsConfInfo {

   private:

      void init_from_scratch();

   public:

      // TCPairs configuration object
      MetConfig Conf;

      // Track filtering criteria
      StringArray Model;     // List of model names
      StringArray StormId;   // List of storm ids
      StringArray Basin;     // List of basin names
      StringArray Cyclone;   // List of cyclone numbers
      StringArray StormName; // List of storm names

      // Timing information
      unixtime  InitBeg, InitEnd;
      TimeArray InitInc;
      TimeArray InitExc;
      NumArray  InitHour;
      unixtime  ValidBeg, ValidEnd;

      // Polyline masking regions
      MaskPoly  InitMask;
      MaskPoly  ValidMask;

      // Check for duplicate ATCFTrackLines
      bool CheckDup;

      // 12-hour track interpolation logic
      Interp12Type Interp12;

      // Consensus model definition
      int NConsensus;           // Number of consensus models
      ConsensusInfo *Consensus; // Consensus model definition

      // Time-lagged track definition
      NumArray LagTime;

      // CLIPER/SHIFOR baseline model definition along with BEST
      // and operational technique names
      StringArray BestTechnique;
      StringArray BestBaseline;
      StringArray OperTechnique;
      StringArray OperBaseline;

      // Analysis track datasets
      TrackType AnlyTrack;

      // Only retain TrackPoints in both the ADECK and BDECK tracks
      bool MatchPoints;

      // Gridded data file containing distances to land
      ConcatString DLandFile;

      // ASCII watch/warnings file
      ConcatString WatchWarnFile;

      // Watch/warnings time offset
      int WatchWarnOffset;

      // Config file version
      ConcatString Version;

      TCPairsConfInfo();
     ~TCPairsConfInfo();

      void clear();

      void read_config(const char *, const char *);
      void process_config();
};

////////////////////////////////////////////////////////////////////////

#endif   /*  __TC_PAIRS_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
