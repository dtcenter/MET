source("R_test/test_const.R");
source("R_test/test_util.R");

boolShowCmd		= FALSE;
boolSaDumpRow	= TRUE;

strFileSaDump	= paste(strDirTest, "/sa_out/sa_pjc_dump.stat", sep="");
strFileSaOut	= paste(strDirTest, "/sa_out/sa_pjc_agg.out", sep="");
strFileSaStat	= paste(strDirTest, "/sa_out/sa_pjc_agg.stat", sep="");
strSaJob		= "aggregate_stat -out_line_type PJC";

strDirLookin	= paste(strDirData, "/hmt-ens-d01/201012*/grid_stat", sep="");

# list of stat_analysis criteria
listSaCrit = list(
	fcst_lead		= "120000",
	fcst_init_hour	= "120000",
	line_type		= "PCT",
	fcst_var		= "APCP_06_A6_ENS_FREQ_ge12.700",
	vx_mask			= "LAND_d01",
	fcst_valid_beg	= "20101218_000000",
	fcst_valid_end	= "20101222_000000"
);

# run the stat_analysis job and read the output
runStatAnalysis(listSaCrit, strSaJob, strFileSaDump, strFileSaOut, strDirLookin, 
				show=boolShowCmd);
dfSa = readStatAnalysisOutput(strFileSaOut, strFileSaStat);
if( TRUE == FALSE ){ cat("\n\n"); }

# build a dataframe of PJC values 
dfSaPjc = data.frame(
	thresh_i	= c( t( dfSa[1,seq(3, ncol(dfSa)-1, 7)] ) ),
	oy_tp		= c( t( dfSa[1,seq(4, ncol(dfSa)-1, 7)] ) ),
	on_tp		= c( t( dfSa[1,seq(5, ncol(dfSa)-1, 7)] ) ),
	calibration	= c( t( dfSa[1,seq(6, ncol(dfSa)-1, 7)] ) ),
	refinement	= c( t( dfSa[1,seq(7, ncol(dfSa)-1, 7)] ) ),
	likelihood	= c( t( dfSa[1,seq(8, ncol(dfSa)-1, 7)] ) ),
	baser		= c( t( dfSa[1,seq(9, ncol(dfSa)-1, 7)] ) )
);
# adjust the stat_analysis percentile thresholds to the midpoints
dfSaPjc$thresh_ii = dfSaPjc$thresh_i;
for(i in 2:nrow(dfSaPjc)){
	dfSaPjc[i-1,]$thresh_i = 0.5*(dfSaPjc[i,]$thresh_ii + dfSaPjc[i-1,]$thresh_ii);
}
dfSaPjc[i,]$thresh_i = 0.5*(1 + dfSaPjc[i,]$thresh_ii);
dfSaPjc = dfSaPjc[ !is.na(dfSaPjc$calibration), ];

# remove the METViewer output plot script
strFileMvPlot = paste(strDirMvScripts, "/plot_pjc.R", sep="");
rmFile(strFileMvPlot);

# run the METViewer job to generate a reliability diagram, and read the data
strCmdMvRely = paste(strMvExec, "xml/plot_pjc.xml");
system(strCmdMvRely, intern=(FALSE == boolShowCmd));
source(strFileMvPlot);
dfMvPjc = dfPct;
setwd(strDirTest);

# compare the stat_analysis results to the metviewer results
dfDiff = data.frame(
	thresh_i	= cbind( round(dfSaPjc$thresh_i,    5) - round(dfMvPjc$thresh_i,    5) ),
	oy_tp		= cbind( round(dfSaPjc$oy_tp,       5) - round(dfMvPjc$oy_tp,       5) ),
	on_tp		= cbind( round(dfSaPjc$on_tp,       5) - round(dfMvPjc$on_tp,       5) ),
	calibration	= cbind( round(dfSaPjc$calibration, 5) - round(dfMvPjc$calibration, 5) ),
	refinement	= cbind( round(dfSaPjc$refinement,  5) - round(dfMvPjc$refinement,  5) ),
	likelihood	= cbind( round(dfSaPjc$likelihood,  5) - round(dfMvPjc$likelihood,  5) ),
	baser		= cbind( round(dfSaPjc$baser,       5) - round(dfMvPjc$baser,       5) )
);
print(dfDiff);
