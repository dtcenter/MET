// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef __VX_RENDER_H__
#define __VX_RENDER_H__


////////////////////////////////////////////////////////////////////////


#include "vx_ps.h"

#include "vx_pxm.h"

#include "renderinfo.h"
#include "ascii85_filter.h"
#include "bit_filter.h"
#include "hex_filter.h"
#include "psout_filter.h"
#include "rle_filter.h"
#include "flate_filter.h"


////////////////////////////////////////////////////////////////////////


extern void render(PSfile &, const Pbm &, const RenderInfo &);

extern void render(PSfile &, const Pgm &, const RenderInfo &);

extern void render(PSfile &, const Ppm &, const RenderInfo &);

extern void render(PSfile &, const Pcm &, const RenderInfo &);


////////////////////////////////////////////////////////////////////////


#endif   //  __VX_RENDER_H__


////////////////////////////////////////////////////////////////////////


