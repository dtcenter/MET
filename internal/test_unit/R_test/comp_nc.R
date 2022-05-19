library(ncdf4)

# get the MET_TEST_BASE environment variable and include the support scripts
met_test_base = system("echo $MET_TEST_BASE", intern=T);
if( "" == met_test_base ){
	cat("ERROR: environment variable MET_TEST_BASE not set\n\n"); q(status=1);
}
source(paste(met_test_base, "/R_test/test_const.R", sep=""));
source(paste(met_test_base, "/R_test/test_util.R", sep=""));

verb = 1;
strict = F;
file_size_delta = 0.01;

usage = function(){
	cat("usage: Rscript comp_nc.R [-v {lev}] [-strict] {nc_file_1} {nc_file_2}\n",
			"  where -v {lev}  indicates verbosity level (0-2), default 1\n",
			"        -delta {file_size_delta} Allowed file size difference. If negative, skip size checking. default 0\n",
			"        -strict   applies strict equality when comparing numerical values, default false\n\n",
			sep="");
}

# parse and verify command line arguments
listArgs = commandArgs(TRUE);
while( 2 < length(listArgs) ){
	if( "-v" == listArgs[1] ){
		verb = listArgs[2];
		listArgs = listArgs[3:length(listArgs)];
	} else if( "-delta" == listArgs[1] ){
		file_size_delta = listArgs[2];
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
compareNc(strNcFile1, strNcFile2, verb, strict, file_size_delta);
