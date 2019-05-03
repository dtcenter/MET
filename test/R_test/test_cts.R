source("R_test/test_const.R");
source("R_test/test_util.R");

boolShowCmd		= TRUE;
boolSaDumpRow	= TRUE;

strFileSaDump	= paste(strDirTest, "/sa_out/sa_cts_dump_{fcst_lead}.stat", sep="");
strFileSaOut	= paste(strDirTest, "/sa_out/sa_cts_agg_{fcst_lead}.out", sep="");
strFileSaStat	= paste(strDirTest, "/sa_out/sa_cts_agg.stat", sep="");
strSaJob		= "aggregate_stat -out_line_type CTS";

listFcstLead	= c("060000", "120000", "180000", "240000");

strDirLookin	= paste(strDirData, "/arw-tom-gep3-d01/201012*/grid_stat", sep="");

# list of stat_analysis criteria
listSaCrit = list(
	fcst_init_hour	= "120000",
	line_type		= "CTC",
	fcst_var		= "APCP_06",
	vx_mask			= "LAND_d01",
	fcst_valid_beg	= "20101218_000000",
	fcst_valid_end	= "20101222_000000"
);

#######

# construct the name of the final stat file and delete it, if present
strFileSaStatAll = sub("_\\{fcst_lead\\}", "", strFileSaStat);
rmFile(strFileSaStatAll);

# run stat_analysis once for each lead time and append results to the output stat file
listSaOut = c();
for(strFcstLead in listFcstLead){
	
	# build the filenames for the current fcst_lead
	strFileSaDumpLead	= sub("\\{fcst_lead\\}", strFcstLead, strFileSaDump);
	strFileSaOutLead	= sub("\\{fcst_lead\\}", strFcstLead, strFileSaOut);
	listSaOut			= append(listSaOut, strFileSaOutLead);
		
	# run the stat_analysis job for the current lead time
	listSaCrit$fcst_lead = strFcstLead;
	runStatAnalysis(listSaCrit, strSaJob, strFileSaDumpLead, strFileSaOutLead, 
					show=boolShowCmd);
	if( TRUE == boolShowCmd ){ cat("\n"); }
	
}

# read the stat_analysis output files
dfSa = readStatAnalysisOutput(listSaOut, strFileSaStat);

# remove the METViewer output plot script
strFileMvPlot = paste(strDirMvScripts, "/plot_cts.R",    sep="");   #### change
strFileMvData = paste(strDirMvData,    "/plot_cts.data", sep="");   #### change
rmFile(strFileMvPlot);

# run the METViewer job to generate a reliability diagram, and read the data
strCmdMvRely = paste(strMvExec, "xml/plot_pstd.xml");   #### change
system(strCmdMvRely, intern=(FALSE == boolShowCmd));
dfMv = read.delim(strFileMvData);

#######
 
 
# reformat the metviewer output to facilitate a comparison
dfDiff = data.frame(
		baser		= dfSa$BASER		- round(dfMv[grep("_BASER$",		dfMv$stat_name),]$stat_value, 5),
		reliability	= dfSa$RELIABILITY	- round(dfMv[grep("_RELIABILITY$",	dfMv$stat_name),]$stat_value, 5),
		resolution	= dfSa$RESOLUTION	- round(dfMv[grep("_RESOLUTION$",	dfMv$stat_name),]$stat_value, 5),
		uncertainty	= dfSa$UNCERTAINTY	- round(dfMv[grep("_UNCERTAINTY$",	dfMv$stat_name),]$stat_value, 5),
		roc_auc		= dfSa$ROC_AUC		- round(dfMv[grep("_ROC_AUC$",		dfMv$stat_name),]$stat_value, 5),
		brier		= dfSa$BRIER		- round(dfMv[grep("_BRIER$",		dfMv$stat_name),]$stat_value, 5),
		brier_ncl	= dfSa$BRIER_NCL	- round(dfMv[grep("_BRIER$",		dfMv$stat_name),]$stat_bcl,   5),
		brier_ncu	= dfSa$BRIER_NCU	- round(dfMv[grep("_BRIER$",		dfMv$stat_name),]$stat_bcu,   5)
);
print(dfDiff);
