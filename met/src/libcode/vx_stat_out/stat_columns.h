// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __STAT_COLUMNS_H__
#define  __STAT_COLUMNS_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>

#include "stat_column_defs.h"
#include "stat_hdr_columns.h"
#include "vx_statistics.h"

////////////////////////////////////////////////////////////////////////

extern void parse_row_col(const char *, int &, int &);

////////////////////////////////////////////////////////////////////////

extern void open_txt_file (ofstream *&,  const char *);
extern void close_txt_file(ofstream *&,  const char *);

////////////////////////////////////////////////////////////////////////

// Update the mask name with climo CDF bin information
extern ConcatString append_climo_bin(const ConcatString &, int, int);

// Write out the header row for fixed length line types
extern void write_header_row(const char **, int, int, AsciiTable &, int, int);

// Write out the header row for variable length line types
extern void write_mctc_header_row  (int, int, AsciiTable &, int, int);
extern void write_pct_header_row   (int, int, AsciiTable &, int, int);
extern void write_pstd_header_row  (int, int, AsciiTable &, int, int);
extern void write_pjc_header_row   (int, int, AsciiTable &, int, int);
extern void write_prc_header_row   (int, int, AsciiTable &, int, int);
extern void write_eclv_header_row  (int, int, AsciiTable &, int, int);
extern void write_rhist_header_row (int, int, AsciiTable &, int, int);
extern void write_phist_header_row (int, int, AsciiTable &, int, int);
extern void write_orank_header_row (int, int, AsciiTable &, int, int);
extern void write_relp_header_row  (int, int, AsciiTable &, int, int);

extern void write_fho_row   (StatHdrColumns &, const CTSInfo &, STATOutputType,
                             AsciiTable &, int &, AsciiTable &, int &);
extern void write_ctc_row   (StatHdrColumns &, const CTSInfo &, STATOutputType,
                             AsciiTable &, int &, AsciiTable &, int &);
extern void write_cts_row   (StatHdrColumns &, const CTSInfo &, STATOutputType,
                             AsciiTable &, int &, AsciiTable &, int &);
extern void write_mctc_row  (StatHdrColumns &, const MCTSInfo &, STATOutputType,
                             AsciiTable &, int &, AsciiTable &, int &);
extern void write_mcts_row  (StatHdrColumns &, const MCTSInfo &, STATOutputType,
                             AsciiTable &, int &, AsciiTable &, int &);
extern void write_cnt_row   (StatHdrColumns &, const CNTInfo &, STATOutputType,
                             int, int, AsciiTable &, int &, AsciiTable &, int &);
extern void write_sl1l2_row (StatHdrColumns &, const SL1L2Info &, STATOutputType,
                             int, int, AsciiTable &, int &, AsciiTable &, int &);
extern void write_sal1l2_row(StatHdrColumns &, const SL1L2Info &, STATOutputType,
                             int, int, AsciiTable &, int &, AsciiTable &, int &);

extern void write_vl1l2_row (StatHdrColumns &, const VL1L2Info &, STATOutputType,
                             AsciiTable &, int &, AsciiTable &, int &);

extern void write_val1l2_row(StatHdrColumns &, const VL1L2Info &, STATOutputType,
                             AsciiTable &, int &, AsciiTable &, int &);

extern void write_vcnt_row  (StatHdrColumns &, const VL1L2Info &, STATOutputType,
                             AsciiTable &, int &, AsciiTable &, int &);

extern void write_pct_row   (StatHdrColumns &, const PCTInfo &, STATOutputType,
                             int, int, AsciiTable &, int &, AsciiTable &, int &,
                             bool update_thresh = true);
extern void write_pstd_row  (StatHdrColumns &, const PCTInfo &, STATOutputType,
                             int, int, AsciiTable &, int &, AsciiTable &, int &,
                             bool update_thresh = true);
extern void write_pjc_row   (StatHdrColumns &, const PCTInfo &, STATOutputType,
                             int, int, AsciiTable &, int &, AsciiTable &, int &,
                             bool update_thresh = true);
extern void write_prc_row   (StatHdrColumns &, const PCTInfo &, STATOutputType,
                             int, int, AsciiTable &, int &, AsciiTable &, int &,
                             bool update_thresh = true);
extern void write_eclv_row  (StatHdrColumns &, const PCTInfo &, const NumArray &, STATOutputType,
                             int, int, AsciiTable &, int &, AsciiTable &, int &);
extern void write_eclv_row  (StatHdrColumns &, const CTSInfo &, const NumArray &, STATOutputType,
                             AsciiTable &, int &, AsciiTable &, int &);
extern void write_nbrctc_row(StatHdrColumns &, const NBRCTSInfo &, STATOutputType,
                             AsciiTable &, int &, AsciiTable &, int &);
extern void write_nbrcts_row(StatHdrColumns &, const NBRCTSInfo &, STATOutputType,
                             AsciiTable &, int &, AsciiTable &, int &);
extern void write_nbrcnt_row(StatHdrColumns &, const NBRCNTInfo &, STATOutputType,
                             AsciiTable &, int &, AsciiTable &, int &);
extern void write_grad_row  (StatHdrColumns &, const GRADInfo &, STATOutputType,
                             AsciiTable &, int &, AsciiTable &, int &);
extern void write_dmap_row  (StatHdrColumns &, const DMAPInfo &, STATOutputType,
                             AsciiTable &, int &, AsciiTable &, int &);
extern void write_mpr_row   (StatHdrColumns &, const PairDataPoint *, STATOutputType,
                             AsciiTable &, int &, AsciiTable &, int &,
                             bool update_thresh = true);
extern void write_isc_row   (StatHdrColumns &, const ISCInfo &, STATOutputType,
                             AsciiTable &, int &, AsciiTable &, int &);
extern void write_ecnt_row  (StatHdrColumns &, const ECNTInfo &, STATOutputType,
                             int, int, AsciiTable &, int &, AsciiTable &, int &);
extern void write_rps_row   (StatHdrColumns &, const RPSInfo &, STATOutputType,
                             AsciiTable &, int &, AsciiTable &, int &);
extern void write_rhist_row (StatHdrColumns &, const PairDataEnsemble *, STATOutputType,
                             AsciiTable &, int &, AsciiTable &, int &);
extern void write_phist_row (StatHdrColumns &, const PairDataEnsemble *, STATOutputType,
                             AsciiTable &, int &, AsciiTable &, int &);
extern void write_orank_row (StatHdrColumns &, const PairDataEnsemble *, STATOutputType,
                             AsciiTable &, int &, AsciiTable &, int &);
extern void write_ssvar_row (StatHdrColumns &, const PairDataEnsemble *, double, STATOutputType,
                             AsciiTable &, int &, AsciiTable &, int &);
extern void write_relp_row  (StatHdrColumns &, const PairDataEnsemble *, STATOutputType,
                             AsciiTable &, int &, AsciiTable &, int &);

////////////////////////////////////////////////////////////////////////

// Write out the column headers
extern void write_header_cols(const StatHdrColumns &,  AsciiTable &, int);

// Write out the columns of data specific to each line type
extern void write_fho_cols   (const CTSInfo &,
                              AsciiTable &, int, int);
extern void write_ctc_cols   (const CTSInfo &,
                              AsciiTable &, int, int);
extern void write_cts_cols   (const CTSInfo &, int,
                              AsciiTable &, int, int);
extern void write_mctc_cols  (const MCTSInfo &,
                              AsciiTable &, int, int);
extern void write_mcts_cols  (const MCTSInfo &, int,
                              AsciiTable &, int, int);
extern void write_cnt_cols   (const CNTInfo &, int,
                              AsciiTable &, int, int);
extern void write_sl1l2_cols (const SL1L2Info &,
                              AsciiTable &, int, int);
extern void write_sal1l2_cols(const SL1L2Info &,
                              AsciiTable &, int, int);
extern void write_vl1l2_cols (const VL1L2Info &,
                              AsciiTable &, int, int);
extern void write_val1l2_cols(const VL1L2Info &,
                              AsciiTable &, int, int);
extern void write_vcnt_cols  (const VL1L2Info &,
                              AsciiTable &, int, int);
extern void write_pct_cols   (const PCTInfo &,
                              AsciiTable &, int, int);
extern void write_pstd_cols  (const PCTInfo &, int,
                              AsciiTable &, int, int);
extern void write_pjc_cols   (const PCTInfo &,
                              AsciiTable &, int, int);
extern void write_prc_cols   (const PCTInfo &,
                              AsciiTable &, int, int);
extern void write_eclv_cols  (const TTContingencyTable &, const NumArray &,
                              AsciiTable &, int, int);
extern void write_nbrctc_cols(const NBRCTSInfo &,
                              AsciiTable &, int, int);
extern void write_nbrcts_cols(const NBRCTSInfo &, int,
                              AsciiTable &, int, int);
extern void write_nbrcnt_cols(const NBRCNTInfo &, int,
                              AsciiTable &, int, int);
extern void write_nbrcnt_cols(const NBRCNTInfo &, int,
                              AsciiTable &, int, int);
extern void write_grad_cols  (const GRADInfo &,
                              AsciiTable &, int, int);
extern void write_dmap_cols  (const DMAPInfo &,
                              AsciiTable &, int, int);
extern void write_mpr_cols   (const PairDataPoint *, int,
                              AsciiTable &, int, int);
extern void write_isc_cols   (const ISCInfo &, int,
                              AsciiTable &, int, int);
extern void write_ecnt_cols  (const ECNTInfo &,
                              AsciiTable &, int, int);
extern void write_rps_cols   (const RPSInfo &,
                              AsciiTable &, int, int);
extern void write_rhist_cols (const PairDataEnsemble *,
                              AsciiTable &, int, int);
extern void write_phist_cols (const PairDataEnsemble *,
                              AsciiTable &, int, int);
extern void write_orank_cols (const PairDataEnsemble *, int,
                              AsciiTable &, int, int);
extern void write_ssvar_cols (const PairDataEnsemble *, int, double,
                              AsciiTable &, int, int);
extern void write_relp_cols  (const PairDataEnsemble *,
                              AsciiTable &, int, int);

// Setup column justification for STAT AsciiTable objects
extern void justify_stat_cols(AsciiTable &);

////////////////////////////////////////////////////////////////////////

#endif   /*  __STAT_COLUMNS_H__  */

////////////////////////////////////////////////////////////////////////
