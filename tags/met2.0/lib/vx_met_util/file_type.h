// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __VX_FILE_TYPE_H__
#define  __VX_FILE_TYPE_H__

////////////////////////////////////////////////////////////////////////

//
// Enumeration for supported input file types
//
enum FileType {

   NoFileType = 0,
   GbFileType = 1,
   NcFileType = 2,
   BfFileType = 3
};

//
// List of file name extensions corresponding to these file types
//
static const char *gb_file_ext [] = { "grib", "grb", "gb" };
static const int   n_gb_file_ext  = sizeof(gb_file_ext)/sizeof(*gb_file_ext);

static const char *nc_file_ext [] = { "netcdf", "ncf", "nc" };
static const int   n_nc_file_ext  = sizeof(nc_file_ext)/sizeof(*nc_file_ext);

static const char *bf_file_ext [] = { "bufr", "bfr", "prepbufr", "pb" };
static const int   n_bf_file_ext  = sizeof(bf_file_ext)/sizeof(*bf_file_ext);

//
// List of magic cookies for file types
//
static const char *magic_cookie_str [] = { "GRIB", "CDF", "BUFR" };
static const int   n_magic_cookie_str  = sizeof(magic_cookie_str)
                                        /sizeof(*magic_cookie_str);

//
// Number of bytes to read looking for the magic cookie
//
static const int   buf_size = 16;

////////////////////////////////////////////////////////////////////////

//
// Routine to determine the file type for the file name specified
//
extern FileType get_file_type(const char *);

////////////////////////////////////////////////////////////////////////

#endif   //  __VX_FILE_TYPE_H__

////////////////////////////////////////////////////////////////////////
