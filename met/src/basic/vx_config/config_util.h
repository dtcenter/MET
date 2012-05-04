////////////////////////////////////////////////////////////////////////
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __CONFIG_UTIL_H__
#define  __CONFIG_UTIL_H__

////////////////////////////////////////////////////////////////////////

#include "string"
#include "map"

#include "config_constants.h"
#include "config_file.h"
#include "data_file_type.h"

////////////////////////////////////////////////////////////////////////

// Global MetConfig object containing config value constants
static MetConfig conf_const(replace_path(config_const_filename));

////////////////////////////////////////////////////////////////////////

extern ConcatString   parse_conf_version(Dictionary *dict);
extern ConcatString   parse_conf_model(Dictionary *dict);
extern GrdFileType    parse_conf_file_type(Dictionary *dict);
extern map<STATLineType,STATOutputType>
                      parse_conf_output_flag(Dictionary *dict);
extern int            parse_conf_n_vx(Dictionary *dict);
extern Dictionary     parse_conf_i_vx_dict(Dictionary *dict, int index);
extern StringArray    parse_conf_message_type(Dictionary *dict);
extern NumArray       parse_conf_ci_alpha(Dictionary *dict);
extern BootInfo       parse_conf_boot(Dictionary *dict);
extern InterpInfo     parse_conf_interp(Dictionary *dict);
extern NbrhdInfo      parse_conf_nbrhd(Dictionary *dict);
extern DuplicateType  parse_conf_duplicate_flag(Dictionary *dict);
extern ConcatString   parse_conf_tmp_dir(Dictionary *dict);
extern GridDecompType parse_conf_grid_decomp_flag(Dictionary *dict);
extern WaveletType    parse_conf_wavelet_type(Dictionary *dict);
extern PlotInfo       parse_conf_plot_info(Dictionary *dict);

extern void check_prob_thresh(const ThreshArray &);

extern const char * statlinetype_to_string(const STATLineType);
extern void         statlinetype_to_string(const STATLineType, char *);
extern STATLineType string_to_statlinetype(const char *);

extern FieldType    int_to_fieldtype(int);

extern ConcatString fieldtype_to_string(FieldType);
extern ConcatString griddecomptype_to_string(GridDecompType);
extern ConcatString wavelettype_to_string(WaveletType);

////////////////////////////////////////////////////////////////////////

#endif   /*  __CONFIG_UTIL_H__  */

////////////////////////////////////////////////////////////////////////
