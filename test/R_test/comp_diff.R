
# get the MET_TEST_BASE environment variable and include the support scripts
met_test_base = system("echo $MET_TEST_BASE", intern=T);
if( "" == met_test_base ){
	cat("ERROR: environment variable MET_TEST_BASE not set\n\n"); q(status=1);
}
source(paste(met_test_base, "/R_test/test_const.R", sep=""));
source(paste(met_test_base, "/R_test/test_util.R", sep=""));

verb = 1;

usage = function(){
	cat("usage: Rscript comp_diff.R [-v {lev}] {file_1} {file_2}\n",
			"  where -v {lev}  indicates verbosity level (0-2), default 1\n\n",
			sep="");
}

# parse and verify command line arguments
listArgs = commandArgs(TRUE);
while( 2 < length(listArgs) ){
	if( "-v" == listArgs[1] ){
		verb = listArgs[2];
		listArgs = listArgs[3:length(listArgs)];
	} else {
		cat("ERROR: unrecognized option:", listArgs[1], "\n\n"); usage(); q(status=1);
	}
}
if( 2 != length(listArgs) ){ 
	cat("ERROR: invalid number of arguments\n\n"); usage(); q(status=1);
}
strFile1 = listArgs[1];
strFile2 = listArgs[2];

# verify the existence of the files
if( FALSE == fileExists(strFile1) ){
	if( 1 <= verb ){ cat("ERROR: input file does not exist:", strFile1); }
	q(status=1);
}
if( FALSE == fileExists(strFile2) ){
	if( 1 <= verb ){ cat("ERROR: input file does not exist:", strFile2); }
	q(status=1);
}

# compare the input files
compareDiff(strFile1, strFile2, verb);
