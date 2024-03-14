// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __TC_RMW_CONF_INFO_H__
#define  __TC_RMW_CONF_INFO_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "vx_config.h"
#include "vx_data2d_factory.h"
#include "vx_data2d.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

class TCRMWConfInfo {

    private:

        void init_from_scratch();

        // Number of data fields
        int n_data;

    public:

        // TCRMW configuration object
        MetConfig Conf;

        // Track line filtering criteria
        ConcatString Model;
        ConcatString StormId;
        ConcatString Basin;
        ConcatString Cyclone;
        unixtime     InitInc;
        unixtime     ValidBeg, ValidEnd;
        TimeArray    ValidInc, ValidExc;
        NumArray     ValidHour;
        NumArray     LeadTime;

        // Range/Azimuth information
        int    n_range;
        int    n_azimuth;
        double delta_range_km;
        double rmw_scale;

	// Wind conversion information
	bool compute_tangential_and_radial_winds;
	ConcatString u_wind_field_name;
	ConcatString v_wind_field_name;
	ConcatString tangential_velocity_field_name;
	ConcatString radial_velocity_field_name;
	ConcatString tangential_velocity_long_field_name;
	ConcatString radial_velocity_long_field_name;

        // Variable information
        VarInfo** data_info;

        TCRMWConfInfo();
        ~TCRMWConfInfo();

        void clear();

        void read_config(const char *, const char *);
        void process_config(GrdFileType);

        int get_n_data() const;
};

////////////////////////////////////////////////////////////////////////

inline int TCRMWConfInfo::get_n_data() const {
    return n_data;
}

////////////////////////////////////////////////////////////////////////

#endif   /*  __TC_RMW_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
