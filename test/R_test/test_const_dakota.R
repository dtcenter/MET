
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
#  Testing Scripts Information
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
strDirTest  = "/d3/projects/MET/MET_test/test";
strDirHdr   = paste(strDirTest, "/hdr", sep="");
strDirTmp   = paste(strDirTest, "/tmp", sep="");

intSigFig   = 6;
intSigFigBc = 1;
intAbsDifBc = 5;
boolRmTmp   = TRUE;

listNcDiffAttrExcl = c("FileOrigins");


# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
#  Testing System Utilities
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
strNcDiffExec = "/usr/local/bin/ncdiff";
strModeConv   = paste(strDirTest, "/bin/mode_conv.pl", sep="");
strTcstConv   = paste(strDirTest, "/bin/tcst_conv.pl", sep="");


# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
#  MET constants
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
#strDirMetBase	= "/d1/pgoldenb/opt/MET_builds/METv3.0.1_gnu4";
strDirMetBase	= "/d3/projects/MET/MET_releases/METv3.0.1";
strSaExec		= paste(strDirMetBase, "/bin/stat_analysis", sep="");


# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
#  METViewer constants
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
strDirMv		= "/d3/projects/METViewer/src/apps/verif/metviewer";
strMvExec		= paste(strDirMv, "/bin/mv_batch.sh", sep="");
strDirMvScripts	= paste(strDirTest, "/R_work/scripts", sep="");
strDirMvData	= paste(strDirTest, "/R_work/data", sep="");


# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
#  Test Data Information
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
strDirData		= "/d1/pgoldenb/var/hmt/2011/dwr_domains";
listMetOutDir	= list(
	met_2_0		= "/d1/pgoldenb/opt/MET_builds/METv2.0_gnu4/out",
	met_3_0		= "/d1/pgoldenb/opt/MET_builds/METv3.0_gnu4/out",
	met_3_0_1	= "/d1/pgoldenb/opt/MET_builds/METv3.0.1_gnu4/out" 
);
