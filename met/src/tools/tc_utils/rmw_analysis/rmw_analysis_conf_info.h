// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __RMW_ANALYSIS_CONF_INFO_H__
#define  __RMW_ANALYSIS_CONF_INFO_H__

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <iostream>

#include "vx_config.h"
#include "vx_data2d_factory.h"
#include "vx_data2d.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

class RMWAnalysisConfInfo {

    private:

        void init_from_scratch();

        // Number of forecast fields
        int n_data;

    public:

        // RMWAnalysis configuration object
        MetConfig Conf;

        // Track filtering criteria
        StringArray Model;     // List of model names
        StringArray StormId;   // List of storm ids
        StringArray Basin;     // List of basin names
        StringArray Cyclone;   // List of cyclone numbers
        StringArray StormName; // List of storm names

        // Timing information
        unixtime  InitBeg, InitEnd;
        unixtime  ValidBeg, ValidEnd;

        // Polyline masking regions
        ConcatString InitMaskName;
        MaskPoly     InitPolyMask;
        Grid         InitGridMask;
        MaskPlane    InitAreaMask;

        ConcatString ValidMaskName;
        MaskPoly     ValidPolyMask;
        Grid         ValidGridMask;
        MaskPlane    ValidAreaMask;

        // Variable information
        VarInfo** data_info;

        // Config file version
        ConcatString Version;

        RMWAnalysisConfInfo();
        ~RMWAnalysisConfInfo();

        void clear();

        void read_config(const char *, const char *);
        void process_config();

        int get_n_data() const;
};

////////////////////////////////////////////////////////////////////////

inline int RMWAnalysisConfInfo::get_n_data() const {
    return n_data;
}

////////////////////////////////////////////////////////////////////////

#endif   /*  __RMW_ANALYSIS_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
