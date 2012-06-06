source("/d3/projects/MET/MET_test/test/R_test/test_const.R");
source("/d3/projects/MET/MET_test/test/R_test/test_util.R");

verb = 1;		# default verbosity level
hist = "";		# default histogram path and file name

usage = function(){
	cat("usage: Rscript comp_stat.R [-v {lev}] [-h {file}] {stat_file_1} {stat_file_2}\n",
		"  where -v    {lev} indicates verbosity level (0-3), default 1\n",
		"        -hist {file} specifies path and filename to write histogram, default off\n",
		sep="");
}

# parse and verify command line arguments
listArgs = commandArgs(TRUE);
while( 2 < length(listArgs) ){
	if( "-v" == listArgs[1] ){
		verb = listArgs[2];
	} else if( "-hist" == listArgs[1] ){
		hist = listArgs[2];
	} else {
		cat("ERROR: unrecognized option:", listArgs[1], "\n\n"); usage(); q(status=1);
	}
	listArgs = listArgs[3:length(listArgs)];
}
if( 2 != length(listArgs) ){ 
	cat("ERROR: invalid number of arguments\n\n"); usage(); q(status=1);
}
strStat1 = listArgs[1];
strStat2 = listArgs[2];

# compare the stat files and print a report
if( 1 <= verb ){ cat("stat1: ", strStat1, "\nstat2: ", strStat2, "\n", sep=""); }
listTest = compareStat(strStat1, strStat2, verb);
printCompReport(listTest, verb, hist);
