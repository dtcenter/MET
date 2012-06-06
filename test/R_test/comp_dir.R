library(ncdf);

source("/d3/projects/MET/MET_test/test/R_test/test_const.R");
source("/d3/projects/MET/MET_test/test/R_test/test_util.R");

verb = 1;		# default verbosity level
hist = 0;		# default histogram plot production

usage = function(){
	cat("usage: Rscript comp_dir.R [-v {lev}] {dir_1} {dir_2}\n",
			"  where -v    {lev} indicates verbosity level (0-3), default 1\n",
			"        -hist {0|1} 1 to produce histogram error plots for each file, default 0\n",
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
strDir1 = gsub("/$", "", listArgs[1]);
strDir2 = gsub("/$", "", listArgs[2]);

if( 1 <= verb ){ cat("dir1:", strDir1, "\ndir2:", strDir2, "\n\n"); }

# build a list of files in each stat folder
listTest1 = system(paste("find", strDir1, "| egrep '\\.stat$|\\.txt$|\\.nc$' | egrep -v 'mode_analysis' | sort"), intern=T);
listTest1Files = gsub(paste(strDir1, "/", sep=""), "", listTest1);
listTest2 = system(paste("find", strDir2, "| egrep '\\.stat$|\\.txt$|\\.nc$' | egrep -v 'mode_analysis' | sort"), intern=T);
listTest2Files = gsub(paste(strDir2, "/", sep=""), "", listTest2);

# report files missing from stat folder 1
listMiss = listTest2Files[ !(listTest2Files %in% listTest1Files) ];
if( 0 < length(listMiss) ){
	if( 1 <= verb ){ 
		cat("WARNING: folder", strDir1, "missing", length(listMiss), "files\n");	
		if( 2 <= verb ){ for(strMiss in listMiss){ cat("   ", strMiss, "\n");	} }
	} else {
		quit(status=1);
	}
}

# report files missing from stat folder 2
listMiss = listTest1Files[ !(listTest1Files %in% listTest2Files) ];
if( 0 < length(listMiss) ){
	if( 1 <= verb ){ 
		cat("WARNING: folder", strDir2, "missing", length(listMiss), "files\n");	
		if( 2 <= verb ){ for(strMiss in listMiss){ cat("   ", strMiss, "\n");	} }
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
		if( 1 <= verb ){ cat("stat1: ", strFile1, "\nstat2: ", strFile2, "\n", sep=""); }
		compareNc(strFile1, strFile2, verb);
	}
	
	# compare the stat files and print a report
	else {
		strHistFile = "";
		if( 1 == hist ){
			strHistFile = gsub("\\.stat$", ".png", strFile);
			strHistFile = gsub("\\.txt$",  ".png", strHistFile);
		}
		if( 1 <= verb ){ cat("stat1: ", strFile1, "\nstat2: ", strFile2, "\n", sep=""); }
		listTest = compareStat(strFile1, strFile2, verb);
		printCompReport(listTest, verb, strHistFile);
	}
	
}



