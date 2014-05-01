
# get the MET_TEST_BASE environment variable and include the support scripts
met_test_base = system("echo $MET_TEST_BASE", intern=T);
if( "" == met_test_base ){
	cat("ERROR: environment variable MET_TEST_BASE not set\n\n"); q(status=1);
}
source(paste(met_test_base, "/R_test/test_const.R", sep=""));
source(paste(met_test_base, "/R_test/test_util.R", sep=""));

verb = 1;		# default verbosity level
hist = "";		# default histogram path and file name
strict = F;     # default strict off

usage = function(){
	cat("usage: Rscript comp_stat.R [-v {lev}] [-hist {file}] [-strict] {stat_file_1} {stat_file_2}\n",
		"  where -v {lev}     indicates verbosity level (0-3), default 1\n",
		"        -hist {file} specifies path and filename to write histogram, default off\n",
		"        -strict      applies strict equality when comparing numerical values\n",
		sep="");
}

# parse and verify command line arguments
listArgs = commandArgs(TRUE);
while( 2 < length(listArgs) ){
	if( "-v" == listArgs[1] ){
		verb = listArgs[2];
		listArgs = listArgs[3:length(listArgs)];
	} else if( "-hist" == listArgs[1] ){
		hist = listArgs[2];
		listArgs = listArgs[3:length(listArgs)];
	} else if( "-strict" == listArgs[1] ){
		strict = 1;
		listArgs = listArgs[2:length(listArgs)];
	} else {
		cat("ERROR: unrecognized option:", listArgs[1], "\n\n"); usage(); q(status=1);
	}
}
if( 2 != length(listArgs) ){ 
	cat("ERROR: invalid number of arguments\n\n"); usage(); q(status=1);
}
strStat1 = listArgs[1];
strStat2 = listArgs[2];

# compare the stat files and print a report
if( 1 <= verb ){ cat("stat1: ", strStat1, "\nstat2: ", strStat2, "\n", sep=""); }
status = try(listTest <- compareStat(strStat1, strStat2, verb, strict));
if(class(status) == "try-error") {
	cat("ERROR: compareStat() failed\n\n");
} else {
	printCompReport(listTest, verb, hist);
}
