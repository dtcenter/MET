# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
#  Environment Variables
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #
met_base = system("echo $MET_BASE", intern=T);
if( "" == met_base ){
   cat("ERROR: environment variable MET_BASE not set\n\n"); q(status=1);
}
met_test_base = system("echo $MET_TEST_BASE", intern=T);
if( "" == met_test_base ){
   cat("ERROR: environment variable MET_TEST_BASE not set\n\n"); q(status=1);
}


# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
#  Testing Scripts Information
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
strDirTest  = met_test_base;
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
strNcDiffExec = system("which ncdiff", intern=T);
if(length(strNcDiffExec) == 0) {
  strNcDiffExec = "/usr/local/nco/bin/ncdiff";
}
strNcDumpExec = system("which ncdump", intern=T);
strDiffExec   = system("which diff",   intern=T);
strEgrepExec  = system("which egrep",  intern=T);
strModeConv   = paste(strDirTest, "/perl/mode_conv.pl", sep="");
strTcstConv   = paste(strDirTest, "/perl/tcst_conv.pl", sep="");


# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
#  MET constants
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
strDirMetBase	= met_base;
strSaExec		= paste(strDirMetBase, "/bin/stat_analysis", sep="");


# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
#  METViewer constants
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
strDirMv		= "/home/pgoldenb/apps/verif/metviewer";
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
