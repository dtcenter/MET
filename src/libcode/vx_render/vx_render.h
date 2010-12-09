// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef __VX_PXM_RENDER_H__
#define __VX_PXM_RENDER_H__


////////////////////////////////////////////////////////////////////////


#include "vx_ps.h"

#include "vx_pxm.h"

#include "vx_renderinfo.h"
#include "vx_ascii85_filter.h"
#include "vx_bit_filter.h"
#include "vx_hex_filter.h"
#include "vx_psout_filter.h"
#include "vx_rle_filter.h"


////////////////////////////////////////////////////////////////////////


extern void render(PSfile &, const Pbm &, const RenderInfo &);

extern void render(PSfile &, const Pgm &, const RenderInfo &);

extern void render(PSfile &, const Ppm &, const RenderInfo &);

extern void render(PSfile &, const Pcm &, const RenderInfo &);


////////////////////////////////////////////////////////////////////////


#endif   //  __VX_PXM_RENDER_H__


////////////////////////////////////////////////////////////////////////


