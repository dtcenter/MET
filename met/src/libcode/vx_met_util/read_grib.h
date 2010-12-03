// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __VX_READ_GRIB_RECORD_H__
#define  __VX_READ_GRIB_RECORD_H__

////////////////////////////////////////////////////////////////////////

#include "constants.h"
#include "grib_classes.h"
#include "grid.h"
#include "vx_wrfdata.h"
#include "vx_math.h"

////////////////////////////////////////////////////////////////////////

extern bool   has_grib_code(GribFile &, int);

extern bool   get_grib_record(GribFile &, GribRecord &, const GCInfo &,
                              WrfData &, Grid &, int &);
extern bool   get_grib_record(GribFile &, GribRecord &, const GCInfo &,
                              const unixtime, const int,
                              WrfData &, Grid &, int &);

extern int    find_grib_record(GribFile &, const GCInfo &);
extern int    find_grib_record(GribFile &, const GCInfo &,
                               const unixtime, const int);

extern int    find_grib_record_levels(GribFile &, const GCInfo &,
                                      NumArray &, NumArray &);
extern int    find_grib_record_levels(GribFile &, const GCInfo &,
                                      const unixtime, const int,
                                      NumArray &, NumArray &);

extern void   read_grib_record(const char *, GribRecord &, int,
                               const GCInfo &, WrfData &, Grid &, int);
extern void   read_grib_record(GribFile &, GribRecord &, int,
                               const GCInfo &, WrfData &, Grid &, int);
extern void   read_single_grib_record(GribFile &, GribRecord &, int,
                                      WrfData &, Grid &, int);

extern void   derive_wdir_record(GribFile &, GribRecord &, WrfData &,
                                 Grid &, const GCInfo &,
                                 const unixtime, const int, int);

extern void   derive_wind_record(GribFile &, GribRecord &, WrfData &,
                                 Grid &, const GCInfo &,
                                 const unixtime, const int, int);

extern void   read_pds(GribRecord &, int &, unixtime &, unixtime &,
                       int &);
extern void   read_gds(GribRecord &, Grid &, int &, int &, int &);
extern void   read_pds_prob(GribRecord &, int &, double &, double &);

extern double decode_lat_lon(const unsigned char *, int);

extern bool   all_bits_set(const unsigned char *, int);

extern void   get_level_info(const GribRecord &, int &, int &);

extern bool   is_grid_relative(const GribRecord &);

extern int    get_bit_from_octet(unsigned char, int);

extern void   rotate_uv_grid_to_earth(const Grid &,
                                      const WrfData &, const WrfData &,
                                      WrfData &, WrfData &);

extern void   rotate_wdir_grid_to_earth(const Grid &,
                                        const WrfData &, WrfData &);

////////////////////////////////////////////////////////////////////////

#endif   //  __VX_READ_GRIB_RECORD_H__

////////////////////////////////////////////////////////////////////////
