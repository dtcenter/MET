
library(ncdf4)

verb = F;

usage = function(){
	cat("usage: Rscript mgnc.R [-v] {nc_file}\n",
			"  where -v indicates verbose mode, default off\n\n");
}

# parse and verify command line arguments
listArgs = commandArgs(TRUE);
if( 2 == length(listArgs) ){
	if( "-v" != listArgs[1] ){ usage(); q(status=1); }
	verb = T;
	listArgs = listArgs[2];
} else if( 1 != length(listArgs) ){ usage(); q(status=1); }

# check for the existence of the file
strLs = system(paste("ls", listArgs[1], "2>/dev/null"), intern=TRUE);
if( 1 > length(strLs) ){
	if( verb ){ cat("File not found:", listArgs[1], "\n"); }
	q(status=1);
}

if( verb ){ cat("Opening", listArgs[1], "\n"); }
ncfile = nc_open(listArgs[1]);

# check each variable in the file
intNumVar = 0;
for(strVarName in names(ncfile$var)){
	
	# get the variable values and return bad status if it's all NA
	var = ncvar_get(ncfile, strVarName);
	if( verb ){ cat("Checking", strVarName, "... "); }
	size = length(var);
	if( size <= sum( is.na(var) ) ){
		if( verb ){ cat("all NAs\n"); }
		q(status=1);
	} else if( verb ){ cat("OK\n"); }

	intNumVar = intNumVar + 1;
}

# if no variables were found, throw an error
if( 1 > intNumVar ){
	if( verb ){ cat("No variables found\n"); }
	q(status=1);
}

#nc_close(ncfile);

# otherwise, exit with success
q(status=0);


