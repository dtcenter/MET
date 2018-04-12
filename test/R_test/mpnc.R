
library(ncdf4)

verb = F;

usage = function(){
	cat("usage: Rscript mpnc.R [-v] {nc_file}\n",
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
intNumHdrVar = 0;
intNumArrVar = 0;
intNum1dVar = 0;
for(strVarName in names(ncfile$var)){
	
	# get the variable values
	var = ncvar_get(ncfile, strVarName);
	if( verb ){ cat("Checking", strVarName, "... "); }
	
	# if the variable contains header information, make sure it's not empty
	if( strVarName == "hdr_typ" | 
	    strVarName == "hdr_sid" | 
		strVarName == "hdr_vld" | 
		strVarName == "obs_qty" ){ 
	    
		if( 1 > nrow(var) ){
			if( verb ){ cat(strVarName, "empty\n"); }
			q(status=1);
		} else if( verb ){ cat("OK\n"); }		
		intNumHdrVar = intNumHdrVar + 1;
	}
	
	# if the variable contains array information, ensure it's not empty
	# and not all zero/NA
	else if( strVarName == "hdr_arr" | strVarName == "obs_arr" ){
		size = nrow(var) * ncol(var);
		if( size <= sum( 0 == var[!is.na(var)] ) ){
			if( verb ){ cat("all zeroes\n"); }
			q(status=1);
		} else if( size <= sum( is.na(var) ) ){
			if( verb ){ cat("all NAs\n"); }
			q(status=1);
		} else if( verb ){ cat("OK\n"); }
		intNumArrVar = intNumArrVar + 1;
	}	
	else if( strVarName == "hdr_lat" | strVarName == "hdr_lon" |
             strVarName == "obs_hid" | strVarName == "obs_val"){
		size = nrow(var);
		if( size <= sum( 0 == var[!is.na(var)] ) ){
			if( verb ){ cat("all zeroes\n"); }
			q(status=1);
		} else if( size <= sum( is.na(var) ) ){
			if( verb ){ cat("all NAs\n"); }
			q(status=1);
		} else if( verb ){ cat("OK\n"); }
		intNum1dVar = intNum1dVar + 1;
	}
	else if( strVarName == "hdr_elv" |
             strVarName == "obs_vid" | strVarName == "obs_gc" |
             strVarName == "obs_lvl" | strVarName == "obs_hgt"){
		size = nrow(var);
		if( size <= sum( 0 == var[!is.na(var)] ) ){
			if( verb ){ cat("all zeroes\n"); }
		} else if( size <= sum( is.na(var) ) ){
			if( verb ){ cat("all NAs\n"); }
		} else if( verb ){ cat("OK\n"); }
		intNum1dVar = intNum1dVar + 1;
	}
    else if( verb ){ cat("ignored\n"); }

}

# if one or more variables are missing, throw an error
if( 3 != intNumHdrVar & 4 != intNumHdrVar ){
	if( verb ){ cat("Unexpected number of header variables (", intNumHdrVar, ")\n"); }
	q(status=1);
}
if( 2 != intNumArrVar & 8 != intNum1dVar){
	if( verb ){ cat("Unexpected number of array variables (", intNumArrVar, ") or 1D variables (", intNum1dVar, ")\n"); }
	q(status=1);
}

#close_nc(ncfile);

# otherwise, exit with success
q(status=0);


