// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
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

#include "config_file.h"
#include "mask_poly.h"

#include "vx_util.h"

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

      // Consensus model definition
      int NCon;                // Number of consensus models
      StringArray  ConModel;   // Consensus model names
      StringArray *ConMembers; // Members for each consensus model
      NumArray     ConMinReq;  // Minimum required consensus members

      // Timing information
      unixtime  InitBeg, InitEnd;
      NumArray  InitHH;
      unixtime  ValidBeg, ValidEnd;

      // Polyline masking regions
      MaskPoly  InitMask;
      MaskPoly  ValidMask;

      // Gridded data file containing distances to land
      ConcatString DLandFile;

      // ASCII watch/warnings file
      ConcatString WatchWarnFile;
      
      // Merge 6-hour TrackPoints into 12-hour interpolated tracks
      bool Interp12;
      
      // Check for duplicate TrackLines
      bool CheckDup;

      // Only retain TrackPoints in both the ADECK and BDECK tracks
      bool MatchPoints;      

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
