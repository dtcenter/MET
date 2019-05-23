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

#include "vx_config.h"
#include "vx_util.h"

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
      StringArray AModel;    // Forecast ATCF ID
      StringArray BModel;    // Reference ATCF ID

      // Track filtering criteria
      StringArray StormId;   // List of storm ids
      StringArray Basin;     // List of basin names
      StringArray Cyclone;   // List of cyclone numbers
      StringArray StormName; // List of storm names

      // Timing information
      unixtime ValidBeg, ValidEnd;

      // JHG, should we also include init_hour?  Wait to hear from Kathryn.

      // Polyline masking region
      ConcatString VxMaskName;
      MaskPoly     VxPolyMask;
      Grid         VxGridMask;
      MaskPlane    VxAreaMask;

      // Distance to land threshold
      SingleThresh DLandThresh;

      //////////////////////////////////////////////////////////////////

      void clear();

      void process_config(Dictionary &);
};

////////////////////////////////////////////////////////////////////////

class TCGenConfInfo {

   private:

      void init_from_scratch();
      void load_dland();

   public:

      TCGenConfInfo();
     ~TCGenConfInfo();

      //////////////////////////////////////////////////////////////////

      // TCPairs configuration object
      MetConfig Conf;

      // Vector of vx task filtering options [n_vx]
      std::vector<TCGenVxOpt> VxOpt;

      // BEST track ATCF ID's
      StringArray BestTechnique;

      // Gridded data file containing distances to land
      Grid      DLandGrid;
      DataPlane DLandData;

      // Config file version
      ConcatString Version;

      //////////////////////////////////////////////////////////////////

      void clear();

      void read_config(const char *, const char *);

      void process_config();

      double compute_dland(double lat, double lon);

      int get_n_vx() const;
};

////////////////////////////////////////////////////////////////////////

inline int TCGenConfInfo::get_n_vx() const { return(VxOpt.size()); }

////////////////////////////////////////////////////////////////////////

#endif   /*  __TC_GEN_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
