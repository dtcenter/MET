library(ncdf4);

# get the MET_TEST_BASE environment variable and include the support scripts
met_test_base = system("echo $MET_TEST_BASE", intern=T);
if( "" == met_test_base ){
	cat("ERROR: environment variable MET_TEST_BASE not set\n\n"); q(status=1);
}
source(paste(met_test_base, "/R_test/test_const.R", sep=""));
source(paste(met_test_base, "/R_test/test_util.R", sep=""));

verb = 1;
strict = F;
hist = 0;		# default histogram plot production
file_size_delta = 0.01; # 1% file size difference
compare_nc_var = 0;

usage = function(){
	cat("usage: Rscript comp_dir.R [-v {lev}] [-hist {0|1}] [-strict] [-nc_var] {dir_1} {dir_2}\n",
			"  where -v {lev}    indicates verbosity level (0-3), default 1\n",
			"        -hist {0|1} 1 to produce histogram error plots for each file, default 0\n",
            "        -nc_var     compare NetCDF variables, default: no\n",
            "        -comp_nc_var     compare NetCDF variables, default: no\n",
            "        -compare_nc_var  compare NetCDF variables, default: no\n",
			"        -strict     applies strict equality when comparing numerical values, default false\n\n",
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
	} else if( "-compare_nc_var" == listArgs[1] || "-comp_nc_var" == listArgs[1] || "-nc_var" == listArgs[1] ){
		compare_nc_var = 1;
		listArgs = listArgs[2:length(listArgs)];
	} else {
		cat("ERROR: unrecognized option:", listArgs[1], "\n\n"); usage(); q(status=1);
	}
}
if( 2 != length(listArgs) ){ 
	cat("ERROR: invalid number of arguments\n\n"); usage(); q(status=1);
}
strDir1 = gsub("/$", "", listArgs[1]);
strDir2 = gsub("/$", "", listArgs[2]);

# build a list of files in each stat folder
listTest1 = system(paste("find", strDir1, "| egrep '\\.stat$|\\.txt$|\\.tcst|\\.nc$|\\.out$|\\.ps$|\\.png$' | sort"), intern=T);
listTest1Files = gsub(paste(strDir1, "/", sep=""), "", listTest1);
listTest2 = system(paste("find", strDir2, "| egrep '\\.stat$|\\.txt$|\\.tcst|\\.nc$|\\.out$|\\.ps$|\\.png$' | sort"), intern=T);
listTest2Files = gsub(paste(strDir2, "/", sep=""), "", listTest2);

if( 1 <= verb ){ cat("dir1:", strDir1, "contains", length(listTest1Files), "files\n");
                 cat("dir2:", strDir2, "contains", length(listTest2Files), "files\n\n"); }

if( 5 <= verb ){
    boolRmTmp = FALSE;
}
                 
# report files missing from stat folder 1
listMiss = listTest2Files[ !(listTest2Files %in% listTest1Files) ];
if( 0 < length(listMiss) ){
	if( 1 <= verb ){ 
		cat("ERROR: folder", strDir1, "missing", length(listMiss), "files\n");
		for(strMiss in listMiss){ cat("   ", strMiss, "\n"); }
	} else {
		quit(status=1);
	}
}

# report files missing from stat folder 2
listMiss = listTest1Files[ !(listTest1Files %in% listTest2Files) ];
if( 0 < length(listMiss) ){
	if( 1 <= verb ){ 
		cat("ERROR: folder", strDir2, "missing", length(listMiss), "files\n");
		for(strMiss in listMiss){ cat("   ", strMiss, "\n"); }
	} else {
		quit(status=1);
	}
}

# compare the files that are common to both stat folders
for(strFile in listTest1Files[ listTest1Files %in% listTest2Files ]){

  if( 1 <= verb ){
      cat("\n# # # # # # # # # # # # # # # # # # # # # # # # # # # # # #\n\n",
          "COMPARING ", strFile, "\n", sep="");
  }

	# build the two stat file names
	strFile1 = paste(strDir1, "/", strFile, sep="");
	strFile2 = paste(strDir2, "/", strFile, sep="");

	# if the files are NetCDF, compare accordingly
	if( TRUE == grepl("\\.nc$", strFile1, perl=T) ){
		if( 1 <= verb ){ cat("file1: ", strFile1, "\nfile2: ", strFile2, "\n", sep=""); }
		compareNc(strFile1, strFile2, verb, strict, file_size_delta, compare_nc_var);
	}

	# if the files are PostScript, PNG, or end in .out, compare accordingly
	else if( TRUE == grepl("\\.out$", strFile1, perl=T) ||
				TRUE == grepl("\\.ps$",  strFile1, perl=T) ||
				TRUE == grepl("\\.png$", strFile1, perl=T) ){
		if( 1 <= verb ){ cat("file1: ", strFile1, "\nfile2: ", strFile2, "\n", sep=""); }
		compareDiff(strFile1, strFile2, verb);
	}

	# compare the stat files and print a report
	else {
		strHistFile = "";
		if( 1 == hist ){
			strHistFile = gsub("\\.stat$", ".png", strFile);
			strHistFile = gsub("\\.tcst$", ".png", strHistFile);
			strHistFile = gsub("\\.txt$",  ".png", strHistFile);
		}
		if( 1 <= verb ){ cat("file1: ", strFile1, "\nfile2: ", strFile2, "\n", sep=""); }
		status = try(listTest <- compareStat(strFile1, strFile2, verb, strict));
		if(class(status) == "try-error") {
			cat("ERROR: compareStat() failed\n\n");
		} else {
			printCompReport(listTest, verb, strHistFile);
		}
	}

}

if( 1 <= verb ){
  cat("\n# # # # # # # # # # # # # # # # # # # # # # # # # # # # # #\n\n",
      "comp_dir complete\n");
}


