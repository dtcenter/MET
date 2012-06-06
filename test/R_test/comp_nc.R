library(ncdf)

source("/d3/projects/MET/MET_test/test/R_test/test_const.R");
source("/d3/projects/MET/MET_test/test/R_test/test_util.R");

verb = 1;

usage = function(){
	cat("usage: Rscript comp_nc.R [-v {lev}] {nc_file_1} {nc_file_2}\n",
			"  where -v {lev} indicates verbosity level (0-2), default 1\n\n");
}

# parse and verify command line arguments
listArgs = commandArgs(TRUE);
if( 4 == length(listArgs) ){
	if( "-v" != listArgs[1] ){ usage(); q(status=1); }
	verb = listArgs[2];
	listArgs = listArgs[3:4];
} else if( 2 != length(listArgs) ){ usage(); q(status=1); }
strNcFile1 = listArgs[1];
strNcFile2 = listArgs[2];

# verify the existence of the files
if( FALSE == fileExists(strNcFile1) ){
	if( 1 <= verb ){ cat("ERROR: input file does not exist:", strNcFile1); }
	q(status=1);
}
if( FALSE == fileExists(strNcFile2) ){
	if( 1 <= verb ){ cat("ERROR: input file does not exist:", strNcFile2); }
	q(status=1);
}

# compare the input files
compareNc(strNcFile1, strNcFile2, verb);
