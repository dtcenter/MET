
#
# Global Variables
#

# fileExists() tests whether or not the specified file exists and returns a boolean
#   indicating the results of the test.
#
# INPUTS:
#    file: path and file name of file to test
fileExists = function(file){
	strLs = system(paste("ls", file, "2>/dev/null"), intern=TRUE);
	return ( 0 < length(strLs) );
}

# rmFile() determines whether or not a file with the specified name exists, and if
#   it does, deletes it.  If warn is set to TRUE, a warning is printed if the file
#   exists (default FALSE).
#
# INPUTS:
#    file: path and file name of the file to remove
#    warn: (optional) TRUE to print a warning for existing files, default FALSE
rmFile = function(file, warn=FALSE){
	if( fileExists(file) ){
		if( warn ){ cat("  **  WARNING: deleting existing file", file, "\n"); }
		system(paste("rm -f", file));
	}
}

# runStatAnalysis() runs the specified stat_analysis job, applying the specified critieria.
#   The criteria should be formatted into a list object with the name of each item
#   corresponding to the critieria name in stat_analysis.  The value for each criteria will
#   be put in double quotes.  The job must contain any additional parameters required,
#   except for dump_row.  The specified dump_row and output file will be used, along with
#   the optionally specified lookin folder pattern.  The show parameter (default TRUE)
#   controls the display of the command and output.  This function assumes that the string
#   strSaExec is set to the stat_analysis executable to call.
#
# INPUTS:
#      crit: list object containing filtering criteria for the stat_analysis job, where
#            list item names are criteria names
#       job: stat_analysis job, including required parameters
#  fileDump: string containing file to specify for dump_row option
#   fileOut: string containing file to specify for out option
# dirLookin: string containing folder pattern to specify for lookin option
#      show: specifies whether to show stat_analysis command and output, default TRUE
runStatAnalysis = function(crit, job, fileDump, fileOut, dirLookin, show=TRUE){

	# remove the stat_analysis output, if it exists
	rmFile(fileDump);
	rmFile(fileOut);

	# build and run the stat_analysis command
	strCmdSa = paste(strSaExec, " \\", "\n", "  -lookin ", dirLookin, " -v 2 \\", "\n", sep="");
	for( strCrit in names(crit) ){
		strCmdSa = paste(strCmdSa, "  -", strCrit, " \"", crit[[strCrit]], "\" \\", "\n", sep="");
	}
	strCmdSa = paste(strCmdSa, "  -dump_row ", fileDump, " \\", "\n",
							   "  -out ", fileOut, " \\", "\n",
							   "  -job ", job, sep="");
	if( TRUE == show ){ cat(strCmdSa, "\n"); }
	strSaOut = system(strCmdSa, intern=(TRUE != show));
}

# readStatAnalysisOutput() formats the specified stat_analysis output file and puts the
#   result into the specified stat file.  Then, the stat file contents are read into a
#   dataframe and returned.
#
# INPUTS:
#   fileOut: string(s) containing path and filename of existing stat_analysis output file
#  fileStat: string containing path and filename of stat file to write
readStatAnalysisOutput = function(fileOut, fileStat){

	# append the contents of the stat_analysis output file to the specified stat file
	rmFile(fileStat);
	for(strFile in fileOut){
		strCmdFmtStat = paste("cat", strFile,
				"| perl -e\'while(<>){",
					"next if (", ifelse(fileExists(fileStat), 2, 1), " > $i++) or /^\\s+$/;",
					"s/^ *[^ ]+ +//;",
					"print \"$_\";",
				"}\'",
				">>", fileStat);
		system(strCmdFmtStat);
	}

	# read the formatted stat file into a dataframe and return it
	return (read.table(fileStat, header=TRUE));
}

# isStatEmpty() assumes that the input string contains the path and file name of a MET
#   output stat file that will be checked for emptyness, which is defined as containing
#   no lines that start with what appears to be a version number.  The function returns
#   a boolean indicating if the input file is empty or not.
#
# INPUTS:
#     stat: path and file name of a MET stat file to test for emptyness
isStatEmpty = function(stat){
	strCmdLines = paste("cat ", stat, " 2>/dev/null | egrep '^V[0-9]' | wc -l", sep="");
	intLines = as.numeric( system(strCmdLines, intern=T) );
	return ( 1 > intLines );
}

# compListStr() assumes that inputs l1 and l2 are lists of strings to be compared.  Items
#   present in l2 but not in l1 cause a warning that includes the specifed warning string
#   to be printed or program exit, depending on the specified verbosity.
#
# INPUTS:
#       l1: string list to compare
#       l2: string list to compare
#     warn: warning string to print when extra items are present
#     verb: verbosity level
compListStr = function(l1, l2, warn, verb){
	listMiss = l2[ !(l2 %in% l1) ];
	if( 0 < length(listMiss) ){
		if( 1 <= verb ){
			cat("ERROR:", warn, length(listMiss), "\n");
			if( 2 <= verb ){ for(strMiss in listMiss){ cat("   ", strMiss, "\n");	} }
		} else {
			quit(status=1);
		}
	}
}

# compMapStr() assumes that inputs m1 and m2 are lists of strings whose values are to be
#   compared for keys that they have in common.  Values that do not match between m1 and
#   m2 for a particular key cause a warning that includes the specifed warning string to
#   be printed or program exit, depending on the specified verbosity.
#
# INPUTS:
#       m1: string list whose values are strings to compare
#       m2: string list whose values are strings to compare
#     warn: warning string to print when extra items are present
#     verb: verbosity level
compMapStr = function(m1, m2, warn, verb){
	listM1Names = names(m1);
	listM2Names = names(m2);
	for(strAttr in listM1Names[ listM1Names %in% listM2Names ]){
		if( strAttr == "FileOrigins" ){ next; }
		if( strAttr == "RunCommand" ){ next; }
		if( strAttr == "MET_version" ){ next; }
		if( m1[[strAttr]] != m2[[strAttr]] ){
			if( 1 <= verb ){
				cat("ERROR:", warn, strAttr, "\n");
			} else {
				quit(status=1);
			}
		}
	}
}


# getStatHeaders() parses the headers file for the specified version of MET, which is
#   assumed to have the format V_v[_vv[..]], for example 2_0 or 3_0_1.  The MET output
#   stat file headers for the specified line type are parsed into an array which is
#   returned.
#
# INPUTS:
#      ver: MET version whose header schema file to read
#      lty: line type whose header schema to read from the headers file, e.g. CNT
getStatHeaders = function(ver, lty){
	strHeaderFile = paste(strDirHdr, "/met_", ver, ".hdr", sep="");
	strHeaderCmd = paste("cat ", strHeaderFile, " | ",
						 "egrep '^", toupper(lty), " +:' | ",
						 "sed -r 's/.*: (.*)/\\1/' | sed -r 's/_VAR_//g'", sep="");
	return ( unlist(strsplit(system( strHeaderCmd, intern=T ), '\\s+', perl=T)) );
}

# isLtyVarLength() determines the last fixed column for MET stat lines of the specified
#   type for the specified version of MET.  If the line type has a variable number of
#   columns, the return value is the last fixed column index (starting at 1).  Otherwise,
#   the return value is 0 for line types of fixed column width.
#
# INPUTS:
#      lty: MET stat line type to consider
#      ver: MET version to consider
isLtyVarLength = function(lty, ver){
	strHeaderFile = paste(strDirHdr, "/met_", ver, ".hdr", sep="");
	strMultiCmd = paste("egrep '_VAR_' ", strHeaderFile, " | awk '{print $1}'", sep="");
	intMultiIdx = 0;
	if( 0 < length( grep(paste("^", lty, "$", sep=""), system(strMultiCmd, intern=T)) ) ){
		strIdxCmd = paste("egrep '^", lty, "' ", strHeaderFile, " | ",
						  "sed -r 's/.+: (.+)/\\1/' | sed -r 's/_VAR_//g' | wc -w", sep="");
		intMultiIdx = as.numeric( system(strIdxCmd, intern=T) );
	}
	return (intMultiIdx);
}

# readStatData() assumes that the specified stat file has been generated by the specified
#   version of MET, ver (format V_v[_vv[..]], e.g. 2_0 or 3_0_1), and contains one or more
#   lines of the specified type, lty.  A data frame is constructed by writing the headers
#   for the specified version of MET and then the specified lines from the stat file into
#   a temp file.  The strDirTmp constant is assumed to point at the tmp location and
#   listMetOutDir is assumed to contain.  A data frame is loaded with the data and
#   returned.
#
# INPUTS:
#     stat: path and filename of a MET stat file to read data from
#      ver: MET version whose stat file is to read
#      lty: line type of data to read, format {line_type}[#{length}], see getStatLty()
#    rmTmp: (optional) remove temporary file, default TRUE
readStatData = function(stat, ver, lty, rmTmp=TRUE){
	if( isStatEmpty(stat) ){ return ( data.frame() ); }

	# parse the line type to determine if the line type is variable length
	intLtyLen = 0;
	strLtyPar = sub("\\w+#(\\d+)", "\\1", lty, perl=T);
	if( lty != strLtyPar ){
		intLtyLen = as.numeric(strLtyPar);
		lty = sub("(\\w+)#\\d+", "\\1", lty, perl=T);
	}

	# build the list of headers and the parsing string for the line type
	intLtyIdx = isLtyVarLength(lty, ver);
	listHdr = getStatHeaders(ver, lty);
	strParse = paste("~/ LINE_TYPE / & / ", lty, " /", sep="");
	if( 0 < intLtyIdx ){
		if( intLtyLen < intLtyIdx ){
			cat("ERROR: line type", lty, "shorter than minimum length", intLtyIdx, "\n");
			return (NA);
		}
		listHdr = listHdr[1:(length(listHdr)-1)];
		for(intIdx in seq(intLtyIdx, intLtyLen)){ listHdr = append(listHdr, paste("COL", intIdx, sep="_")); }
		strParse = paste(strParse, " and (", intLtyLen, " == @l)", sep="");
	}

	# write the headers to a temp file
	strTmp = paste(strDirTmp, "/", "tmp_", lty, "_", as.numeric(Sys.time()), ".stat", sep="");
	cat( paste(listHdr, collapse=" "), "\n", file=strTmp );

	# write the rows corresponding to the input line type to the temp file
	strCmdTmp = paste("cat ", stat, " | ",
					  "perl -e'while(<>){@l=split(); next unless ", strParse, "; print $_;}' ",
					  " >> ", strTmp, sep="");
	system( strCmdTmp );
	# read the temp file into a data frame and return it
	dfStat = read.table(strTmp, header=T);
	if( TRUE == rmTmp ){ rmFile(strTmp) }
	return (dfStat);
}

# getStatMetVer() assumes that the input string contains the path and filename of a MET
#   output stat file that will be parsed for a MET version number.  It is further assumed
#   that the MET version number is the first token on the second line of the file.  If
#   successful, the version number is returned in the format V_v[_vv[..]], e.g. 2_0 or
#   3_0_1.
# 
# Since met-8.1, the MET team has been creating bugfix release versions of the form x.y.z
# where x, y, and z specify the major, minor, and patch levels.  The header columns are
# defined for each major and minor version number but not the patch level.  This function
# has been modified to return the major and minor version number but not the patch level. 
#
# INPUTS:
#      stat: path and filename of a MET stat file to read data from
getStatMetVer = function(stat){
	if( isStatEmpty(stat) ){ return (NA); }
	strCmdStatVer = paste("cat ", stat, " | head -2 | tail -1 | awk '{print $1}'", sep="");
        strVrs = unlist ( strsplit( sub("^V", "", system(strCmdStatVer, intern=TRUE)), "[.]" ) );
	return ( paste ( strVrs[1], strVrs[2], sep='_' ) );
}

# getStatLty() assumes that the input string contains the path and file name of a MET
#   output stat file that will be parsed to build a list of unique sorted line types
#   present in the file, which are returned.  In the case of line types of variable
#   length, one item is returned for each group of a particular line type that have the
#   same length using the format {line_type}#{length}, e.g. PSTD#35 for a group of PSTD
#   lines with length 35.
#
# INPUTS:
#      stat: path and filename of a MET stat file to read line types from
getStatLty = function(stat){
	strVer = getStatMetVer(stat);
	strHdr = getStatHeaders(strVer, "CNT");

	# location of the LINE_TYPE column
	intLtyCol = which(unlist(strsplit(strHdr, ' ')) == "LINE_TYPE");

	# parse out the distinct line types present in the input file
	strCmdLty = paste("cat ", stat, " 2>/dev/null | egrep '^V[0-9]' | awk '{print $", intLtyCol, "}' | sort -u", sep="");
	listLty = system(strCmdLty, intern=T);

	# modify the list to include the length of variable length lines
	listLtyRet = c();
	for(strLty in listLty){

		# for lines of fixed length, add them and continue
		if( 0 == isLtyVarLength(strLty, strVer) ){
			listLtyRet = append(listLtyRet, strLty);
			next;
		}

		# for lines of variable length, add an entry for each distinct length
		strCmdLtyLin = paste(
				"LT=$(cat ", stat, " | egrep ' ", strLty, " ' | wc -l); ",
				"for L in $(seq 1 $LT); do ",
				"  echo $(cat ", stat, " | egrep ' ", strLty, " ' | head -$L | tail -1 | wc -w); ",
				"done | sort -u", sep="");
		listLtyLin = system(strCmdLtyLin, intern=T);
		for(strLtyLin in listLtyLin){
			listLtyRet = append(listLtyRet, paste(strLty, "#", strLtyLin, sep=""));
		}
	}
	return (listLtyRet);
}


# compareStatLty() assumes that the specified strings stat1 and stat2 contain the paths
#   and file names of MET output stat files.  The function examines only the rows of the
#   specified line in each file, comparing them to one another.  A list data structure
#   containing four name/value pairs is returned indicating pass/fail for the
#   corresponding tests:
#     sup - indicates if the line type is supported
#     hdr - indicates if the headers are identical
#    nrow - indicates if the number of rows are identical
#     num - indicates if the non-bootstrap numbers match to the level stored in intSigFig
#  num_bc - indicates if the bootstrap numbers match to the level stored in intSigFigBc
#             and intAbsDifBc (allowed absolute difference after rounding)
#
# INPUTS:
#    stat1: path and file name of first stat file to compare
#    stat2: path and file name of second stat file to compare
#      lty: line type whose data to compare, format {line_type}[#{length}], see getStatLty()
#     verb: (optional) verbosity level, 0 for no output
#   strict: (optional) require strict numerical equality, default no
compareStatLty = function(stat1, stat2, lty, verb=0, strict=0){

	# check for "empty" stat files
	if( isStatEmpty(stat1) ){ cat("ERROR: stat file", stat1, "contains no data\n"); return (NA); }
	if( isStatEmpty(stat2) ){ cat("ERROR: stat file", stat2, "contains no data\n"); return (NA); }

	# determine the version of the two stat files
	strV1 = getStatMetVer(stat1);
	strV2 = getStatMetVer(stat2);

	# get the headers and determine if there are differences therein
	strLtyPar = sub("(\\w+)(#\\d+)?", "\\1", lty, perl=T)
	listV1Hdr = getStatHeaders(strV1, strLtyPar);
	listV2Hdr = getStatHeaders(strV2, strLtyPar);
	listV1HdrExt = listV1Hdr[ !(listV1Hdr %in% listV2Hdr) ];
	listV2HdrExt = listV2Hdr[ !(listV2Hdr %in% listV1Hdr) ];
	if( 0 < length(listV1HdrExt) ){
		if( 1 <= verb ){
			cat("ERROR: version", strV1, "contains extra headers:",
				paste(listV1HdrExt, collapse=" "), "\n");
		}
		return (list("hdr" = FALSE));
	}
	if( 0 < length(listV2HdrExt) ){
		if( 1 <= verb ){
			cat("ERROR: version", strV2, "contains extra headers:",
				paste(listV2HdrExt, collapse=" "), "\n");
		}
		return (list("hdr" = FALSE));
	}
	boolTestHdr = ( (1 > length(listV1HdrExt)) & (1 > length(listV2HdrExt)) );
	listHdr = listV1Hdr[listV1Hdr %in% listV2Hdr];

	# build the complete path and file names of the stat files and read them into data frames
	dfV1 = readStatData(stat1, strV1, lty);
	dfV2 = readStatData(stat2, strV2, lty);

	# replace NA with bad data value in the data columns
	for(i in 22:ncol(dfV1)){
		ind = is.na(dfV1[,i]);
		if(sum(ind) > 0) { dfV1[ind,i] = -9999.0; }
	}
	for(i in 22:ncol(dfV2)){
		ind = is.na(dfV2[,i]);
		if(sum(ind) > 0) { dfV2[ind,i] = -9999.0; }
	}

	# check for a mis-match on number of rows, and report if any are found
	listNrow = c(nrow(dfV1), nrow(dfV2));
	boolTestNrow = ( listNrow[1] == listNrow[2] );
	if( FALSE == boolTestNrow ){
		if( 1 <= verb ){
			cat("ERROR: differing number of rows", listNrow[1], "vs.", listNrow[2],
				"for row type", lty, "between versions",	strV1, "vs.", strV2, "\n");
		}
		return (list("nrow" = FALSE));
	}

	# compare the information in the header columns
	for(intCol in 2:21){
		listMatch = apply(data.frame(dfV1[,intCol], dfV2[,intCol]), 1,
				function(a){ a[1] == a[2] });
		intNumDiff = sum( !listMatch[ !is.na(listMatch) ] );
		if( 0 < intNumDiff ){
			if( 1 <= verb ){
				cat("ERROR: header information mismatch in column ",
						listHeaderCols[intCol], "\n", sep="");
			}
			return (list("hdr" = FALSE));
		}
	}

	# performance storage
	listTotHist = c();
	intTotComp = 0;
	intTotDiff = 0;

	# check the numerical columns for differences
	boolTestNum = TRUE;
	boolTestNumBc = TRUE;
	listHdrVx = names(dfV1);
	for(strCol in listHdrVx[ 22:length(listHdrVx) ]){

		# compare the columns in different ways, depending on bootstrapping
		boolBc = ( 0 < length( grep("_BC[UL]$", strCol, perl=TRUE) ) );
		boolNum = is.numeric( dfV1[[strCol]] ) | is.numeric( dfV2[[strCol]] );
		if( TRUE == boolBc & FALSE == strict){
			listDiff = signif(dfV1[[strCol]], intSigFig) - signif(dfV2[[strCol]], intSigFig);
			if( 0 < intAbsDifBc ){ listDiff[ abs(listDiff) < intAbsDifBc ] = 0; }
		} else if( TRUE == boolNum ) {
			if( TRUE == strict ){
				listDiff = dfV1[[strCol]] - dfV2[[strCol]];
			} else {
				listDiff = signif(dfV1[[strCol]], intSigFig) - signif(dfV2[[strCol]], intSigFig);
			}
			listTotHist = append(listTotHist, listDiff[is.na(listDiff) != TRUE & abs(listDiff) > 0]);
			intTotComp = intTotComp + length(listDiff);
			intTotDiff = intTotDiff + length(listDiff[is.na(listDiff) != TRUE & abs(listDiff) > 0]);
		} else {
			listDiff = as.numeric(as.character(dfV1[[strCol]]) != as.character(dfV2[[strCol]]));
		}
		listDiff[ is.na(listDiff) ] = 0;

		# report any differences found
		intNumDiff = sum(listDiff != 0);
		if( 0 < intNumDiff ){
			if( 1 <= verb ){
				cat("ERROR: found",
					format(intNumDiff, width="5", justify="right"), "differences in row type",
					format(lty, width="12", justify="left"),
					"column", format(strCol, width=12, justify="left"),
					" - max abs:", format(max(abs(listDiff)), scientific=F), "\n");
			}
			if( 2 <= verb ){
				sum = summary(listDiff);
				for(stat in names(sum)){
					cat(format( paste(stat, ":", sep=""), justify="right", width=12 ), sum[[stat]], "\n");
				}
			}
			if( 3 <= verb ){
				cat(paste(dfV1[listDiff != 0,][[strCol]], "vs.", dfV2[listDiff != 0,][[strCol]], "\n"), sep="");
			}

			if( TRUE  == boolBc ){ boolTestNumBc	= FALSE; }
			if( FALSE == boolBc ){ boolTestNum		= FALSE; }

		}
	}

	listRet = list(
		"num"		= boolTestNum,
		"num_bc"	= boolTestNumBc,
		"tot_comp" 	= intTotComp,
		"tot_diff"	= intTotDiff,
		"tot_hist"	= listTotHist
	);

	# return a data structure containing the results of the tests
	return( listRet );
}

# compareStat() assumes that the specified strings stat1 and stat2 contain the paths and
#   file names of MET output stat files.  The stat files are compared and a list data
#   structure is returned containing all line types compared with failed tests for each
#   one: hdr, nrow, num, num_bc and lty.  See compareStatLty() documentation.  An lty
#   test failure indicates that there is a mis-match in the line types present in the
#   two files.
#
# INPUTS:
#    stat1: path and file name of first stat file to compare
#    stat2: path and file name of second stat file to compare
#     verb: (optional) verbosity level, 0 for no output
#   strict: (optional) require strict numerical equality, default no
compareStat = function(stat1, stat2, verb=0, strict=0){
	listTest = list();

	# verify that the files exist
	if( ! fileExists(stat1) ){ cat("ERROR: stat file does not exist:", stat1, "\n"); return (NA); }
	if( ! fileExists(stat2) ){ cat("ERROR: stat file does not exist:", stat2, "\n"); return (NA); }

	# run diff command, use -w to ignore white space to check for a match
	strCmd = paste(strDiffExec, stat1, stat2);
	strCmdOut = system(paste(strCmd, "2>&1"), intern=T);
	if( 0 == length(strCmdOut) ){
		listTest$tot_hist = "ALL";
		listTest$tot_comp = "ALL";
		listTest$tot_diff = 0;
		return( listTest );
	}

	# if the files are MODE files, convert them to temporary stat files
	if( TRUE == grepl("^.*[^a-z]mode[^a-z].*\\.txt$", stat1, perl=T) ){
		strTmp1 = paste(strDirTmp, "/", "tmp_mode1_", as.numeric(Sys.time()), ".stat", sep="");
		system( paste(strModeConv, stat1, ">", strTmp1) );
		stat1 = strTmp1;
		strTmp2 = paste(strDirTmp, "/", "tmp_mode2_", as.numeric(Sys.time()), ".stat", sep="");
		system( paste(strModeConv, stat2, ">", strTmp2) );
		stat2 = strTmp2;
		rmStat = TRUE;
	} else {
		rmStat = FALSE;
	}

	# if the files are .tcst files, convert them to temporary stat files
	if( TRUE == grepl("^.*\\.tcst$", stat1, perl=T) ){
		strTmp1 = paste(strDirTmp, "/", "tmp_tcst1_", as.numeric(Sys.time()), ".stat", sep="");
		system( paste(strTcstConv, stat1, ">", strTmp1) );
		stat1 = strTmp1;
		strTmp2 = paste(strDirTmp, "/", "tmp_tcst2_", as.numeric(Sys.time()), ".stat", sep="");
		system( paste(strTcstConv, stat2, ">", strTmp2) );
		stat2 = strTmp2;
		rmStat = TRUE;
	} else {
		rmStat = FALSE;
	}

	# compare the line types present in the two files
	listV1Lty = getStatLty(stat1);
	listV2Lty = getStatLty(stat2);
	for(strLty in listV1Lty[ !(listV1Lty %in% listV2Lty) ]){
		if( 1 <= verb ){ cat("ERROR: line type", strLty, "not found in stat2\n"); }
		listTest[[strLty]] = c("lty");
	}
	for(strLty in listV2Lty[ !(listV2Lty %in% listV1Lty) ]){
		if( 1 <= verb ){ cat("ERROR: line type", strLty, "not found in stat1\n"); }
		listTest[[strLty]] = c("lty");
	}

	# performance information
	listTotHist = c();
	intTotComp = 0;
	intTotDiff = 0;

	# for common line types, perform a line type comparison
	listLty = listV1Lty[ listV1Lty %in% listV2Lty ];
	for(strLty in listLty){

		# perform the comparison for the current line type
		listResults = compareStatLty(stat1, stat2, strLty, verb, strict);

		# store performance information, if present
		if( is.null(listResults$tot_comp) == FALSE ){
			listTotHist = append(listTotHist, listResults$tot_hist);
			listResults$tot_hist = NULL;
			intTotComp = intTotComp + listResults$tot_comp;
			listResults$tot_comp = NULL;
			intTotDiff = intTotDiff + listResults$tot_diff;
			listResults$tot_diff = NULL;
		}

		# store the failed tests for the current line type
		listFail = listResults[listResults == FALSE];
		if( 0 < length( listFail ) ){
			listTest[[strLty]] = paste(names(listFail), collapse=",");
		} else {
			listTest[[strLty]] = NA;
		}
	}

	listTest$tot_hist	= listTotHist;
	listTest$tot_comp	= intTotComp;
	listTest$tot_diff	= intTotDiff;

	# remove the temporary mode files, if required
	if( TRUE == rmStat ){
		rmFile(stat1);
		rmFile(stat2);
	}

	return( listTest );
}

# printCompReport() assumes that the specified list is the data structure returned
#   by the function compareStat().  Depending on the verbosity level, a report of
#   line types that passed and failed is printed.  If verbosity is 0, then quit is
#   called when the first failure is encountered.
#
# INPUTS:
#  listTest: data structure returned by compareStat() which will be reported on
#      verb: (optional) verbosity level, 0 for no output
#      hist: (optional) path and filename to write error histogram to
printCompReport = function(listTest, verb=0, hist=""){

	# print the performance information
	if( 1 <= verb ){
		cat("\nSUMMARY for non-bootstrap numerical values\n",
			"# comparisons: ", listTest$tot_comp, "\n",
			"# differences: ", listTest$tot_diff, "\n", sep="");
		if( 0 < listTest$tot_diff ){
			sum = summary(listTest$tot_hist);
			for(stat in names(sum)){
				cat(format( paste(stat, ":", sep=""), justify="right", width=12 ), sum[[stat]], "\n");
			}
			if( hist != "" ){
				bitmap(hist);
				hist(listTest$tot_hist, xlab="Error Size", ylab="Frequency", main=hist);
				dev.off();
				cat("WROTE:", hist, "\n");
			}
		}
		cat("\n");
	}
	listTest$tot_comp = NULL;
	listTest$tot_diff = NULL;
	listTest$tot_hist = NULL;

	# print the results of the individual stat file tests
	for(strLty in names( listTest )){
		if( is.na( listTest[[strLty]] ) ){
			if( 1 <= verb ){ cat("passed", strLty, "\n"); }
		} else {
			if( 1 <= verb ){
				cat("ERROR: failed tests for ", strLty, ": ", paste(listTest[[strLty]], collapse=", "), "\n", sep="");
			} else {
				quit(status=2);
			}
		}
	}
}

# compareNc() assumes that the specified strings nc1 and nc2 contain the paths and
#   file names of MET output NetCDF files.  The netcdf files are compared and, if the
#   specified verbosity level is greater than zero, warning messages are printed
#   describing the differences.  If the verbosity level is zero, the function exits
#   R with non-zero status when the first difference is found, otherwise it returns.
#
# INPUTS:
#      nc1: path and file name of first NetCDF file to compare
#      nc2: path and file name of second NetCDF file to compare
#     verb: (optional) verbosity level, 0 for no output
#   strict: (optional) require strict numerical equality, default no
#    delta: (optional) the allowed file size percentage difference, ignore the file size checking if negative
# comp_var: (optional) Compare the variables, too, default: no
compareNc = function(nc1, nc2, verb, strict=0, delta=-1, comp_var=0){

	strNcDump1 = paste(strDirTmp, "/", "ncdump_hdr1_", as.numeric(Sys.time()), ".txt", sep="");
	strNcDump2 = paste(strDirTmp, "/", "ncdump_hdr2_", as.numeric(Sys.time()), ".txt", sep="");
	strNcDiff  = paste(strDirTmp, "/", "ncdiff_", as.numeric(Sys.time()), ".nc", sep="");

	# build and run the ncdump command for the first file
	if (0 < comp_var) {
		strCmd = paste(strNcDumpExec, " \\\n  ", nc1, " \\\n  > ", strNcDump1, sep="");
	} else {
		strCmd = paste(strNcDumpExec, " -h \\\n  ", nc1, " \\\n  > ", strNcDump1, sep="");
	}
	if( 2 <= verb ){ cat("NCDUMP:", strCmd, "\n"); }
	strCmdOut = system(paste(strCmd, "2>&1"), intern=T);

	# build and run the ncdump command for the second file
	if (0 < comp_var) {
		strCmd = paste(strNcDumpExec, " \\\n  ", nc2, " \\\n  > ", strNcDump2, sep="");
	} else {
		strCmd = paste(strNcDumpExec, " -h \\\n  ", nc2, " \\\n  > ", strNcDump2, sep="");
	}
	if( 2 <= verb ){ cat("NCDUMP:", strCmd, "\n"); }
	strCmdOut = system(paste(strCmd, "2>&1"), intern=T);

	# build and run the diff command for the ncdump output
	strCmd = paste(strDiffExec, " \\\n  ", strNcDump1, " \\\n  ", strNcDump2,
		" \\\n -I 'FileOrigins' -I'MET_version' -I'RunCommand'", sep="");
	if( 2 <= verb ){ cat("DIFF:", strCmd, "\n"); }
	str = system(paste(strCmd, "2>&1"), intern=T);
	strCmdOut = system(paste(strCmd, "2>&1"), intern=T);

	# if there are differences in the header, warn and quit
	if( 0 < length(strCmdOut) ){
		if( 1 <= verb ){
			cat("ERROR: NetCDF headers differ:\n", paste(strCmdOut, collapse='\n'), "\n", sep='');
			return();
		} else {
			quit(status=1);
		}
	}

	skip_nc_size_checking = system("echo $MET_TEST_NO_NC_SIZE", intern=T);
	if (skip_nc_size_checking != "" ) {
		cat("INFO: NetCDF file size checking was disabled by environment variable MET_TEST_NO_NC_SIZE\n");
	} else if (delta < 0 ) {
		cat("INFO: NetCDF file size checking was disabled\n");
	} else {
		ncFileSize1 = file.size(nc1);
		ncFileSize2 = file.size(nc2);
		ncFileSizeDiff = abs(ncFileSize1 - ncFileSize2);
		if ( (ncFileSizeDiff / ncFileSize1) > delta) {
			if( 1 <= verb ){
				cat("ERROR: NetCDF file size difference exceeds", delta*100.0, "%\n");
				cat("       file size difference:", ncFileSizeDiff, "\n");
				cat("      ", nc1, ":", ncFileSize1, "\n");
				cat("      ", nc2, ":", ncFileSize2, "\n");
			}
		}
	}

	# build and run the ncdiff command
	strCmd = paste(strNcDiffExec, " -x -v time_bounds \\\n  ", nc1, " \\\n  ", nc2, " \\\n  ", strNcDiff, sep="");
	if( 2 <= verb ){ cat("NCDIFF:", strCmd, "\n"); }
	strCmdOut = system(paste(strCmd, "2>&1"), intern=T);

	# if the ncdiff file failed for some reason, warn and quit
	if( 0 < length(strCmdOut) ){
		if( 1 <= verb ){
			cat("ERROR: ncdiff error ", strCmdOut, "\n");

			# if there is a variable mismatch, print a side-by-side table of variable names in the NetCDF files
			if( TRUE == grepl("variable .* is in list one and not in list two", strCmdOut) ){
				strNcV = paste(strDirTmp, "/", "ncv_", as.numeric(Sys.time()), ".txt", sep="");
				strCmd = paste(strNcDumpExec, nc1, "| grep -P '^\\t\\w' >",  strNcV, "; echo >> ", strNcV, ";",
							   strNcDumpExec, nc2, "| grep -P '^\\t\\w' >>", strNcV, "; cat", strNcV, "| perl -e'",
							   "while(<>){if(/^\\s*$/){$s++;next;}chomp();if(!$s){push @l1,$_}else{push @l2,$_}}",
							   "while(0<(@l1+@l2)){printf \"%-90s %-90s\\n\",(@l1?shift @l1:\"\"),(@l2?shift @l2:\"\");}'");
				for(strDiff in system(strCmd, intern=T)){ cat(strDiff, "\n"); }
				cat("\n");
				rmFile(strNcV);
			}
			return();
		} else {
			quit(status=1);
		}
	}
	if( ! fileExists(strNcDiff) ){
		if( 1 <= verb ){
			cat("ERROR: ncdiff output file does not exist\n");
			return();
		} else {
			quit(status=1);
		}
	}

	# open the NetCDF files for reading
	ncFile1 = nc_open(c(nc1), write=F);
	ncFile2 = nc_open(c(nc2), write=F);
	ncFileD = nc_open(c(strNcDiff), write=F);

	# read the global attributes from each file
	listAtt1 = ncatt_get(ncFile1, varid=0);
	listAtt1Nam = names(listAtt1);
	listAtt2 = ncatt_get(ncFile2, varid=0);
	listAtt2Nam = names(listAtt2);

	# compare the global attributes
	compListStr(listAtt1Nam, listAtt2Nam, paste("file", nc1, "missing global attributes:"), verb);
	compListStr(listAtt2Nam, listAtt1Nam, paste("file", nc2, "missing global attributes:"), verb);
	compMapStr (listAtt1, listAtt2, "value of global attribute differs for", verb);

	# establish the numerical difference threshold
	dblDiffThresh = 10^(-1*intSigFig);
	if( TRUE == strict ){ dblDiffThresh = 0; }

	# for each variable present in the file, check for differences
	for(strVar in names(ncFileD$var)){

		# check the variable attributes for differences
		listAtt1 = ncatt_get(ncFile1, varid=strVar);
		listAtt1Nam = names(listAtt1);
		listAtt2 = ncatt_get(ncFile2, varid=strVar);
		listAtt2Nam = names(listAtt2);

		# compare the field attributes
		compListStr(listAtt1Nam, listAtt2Nam, paste("file", nc1, "missing field", strVar, "attributes:"), verb);
		compListStr(listAtt2Nam, listAtt1Nam, paste("file", nc2, "missing field", strVar, "attributes:"), verb);
		compMapStr (listAtt1, listAtt2, paste("value of field", strVar, "attribute differs for"), verb);

		# check the numerical data for differences
		dataNcVar = ncvar_get(ncFileD, ncFileD$var[[ strVar ]]);
		boolDiff = FALSE;
		strVarType = "";
		if( is.numeric(dataNcVar[1]) ){
			strVarType = "numerical";
			intNumDiff = sum(dblDiffThresh < abs(dataNcVar), na.rm=T );
      		valDiff = max(c(0, abs(dataNcVar)), na.rm=T);
			boolDiff = (dblDiffThresh < valDiff);
			if( TRUE == boolDiff & 1 <= verb ){
				cat("ERROR: found",
					format(intNumDiff, width="5", justify="right"), "differences in var",
					format(strVar, width="37", justify="left"),
					" - max abs:", format(valDiff, scientific=F), "\n");
			}
		} else {
			strVarType = "string";
			dataNcVar1 = ncvar_get(ncFile1, ncFile1$var[[ strVar ]]);
			dataNcVar2 = ncvar_get(ncFile2, ncFile2$var[[ strVar ]]);
			listDiff = (as.character(dataNcVar1) != as.character(dataNcVar2));
			boolDiff = (0 < sum(listDiff));
			if( TRUE == boolDiff & 1 <= verb ){ cat("ERROR:", sum(listDiff), "string difference(s) found in var", strVar, "\n"); }
		}

		# quit or print a message depending on the presence of a difference and verbosity
		if( boolDiff ){
			if( 0 == verb ){ quit(status=1); }
		}
		else if( 1 <= verb ){ cat("passed", strVarType, "var", strVar, "\n"); }

	}

	# close the diff file
	nc_close(ncFile1);
	nc_close(ncFile2);
	nc_close(ncFileD);

	# remove the temporary diff files, if required
	if( TRUE == boolRmTmp ){
	   rmFile(strNcDump1);
	   rmFile(strNcDump2);
	   rmFile(strNcDiff);
	}
}

# compareDiff() assumes that the specified strings file1 and file2 contain the paths and
#   file names of files to be diffed.  They are compared using the diff command
#   ignoring whitespace and header lines.
#
# INPUTS:
#    file1: path and file name of first file to compare
#    file2: path and file name of second file to compare
#     verb: (optional) verbosity level, 0 for no output
compareDiff = function(file1, file2, verb=0){
	listTest = list();

	# verify that the files exist
	if( ! fileExists(file1) ){ cat("ERROR: file does not exist:", file1, "\n"); return (NA); }
	if( ! fileExists(file2) ){ cat("ERROR: file does not exist:", file2, "\n"); return (NA); }

	# build and run the diff command, use -w to ignore white space
	strCmd = paste(strDiffExec, " -w -I 'JOB_LIST:' -I 'FILTER:' \\\n  ", file1, " \\\n  ", file2, "\n", sep="");
	if( 2 <= verb ){ cat("DIFF:", strCmd, "\n"); }
	strCmdOut = system(paste(strCmd, "2>&1"), intern=T);

	# if the diff failed, warn and quit
	if( 0 < length(strCmdOut) ){
		if( 1 <= verb ){
			cat("ERROR: diff error:", strCmdOut, sep="\n");
			return();
		} else {
			quit(status=1);
		}
	}
	else if( 1 <= verb ){ cat("passed diff\n"); }
}

listHeaderCols = c("VERSION", "DESC", "MODEL",
                   "FCST_LEAD", "FCST_VALID_BEG", "FCST_VALID_END",
                   "OBS_LEAD", "OBS_VALID_BEG", "OBS_VALID_END",
                   "FCST_VAR", "FCST_UNITS", "FCST_LEV",
                   "OBS_VAR", "OBS_UNITS", "OBS_LEV",
                   "OBTYPE", "VX_MASK", "INTERP_MTHD", "INTERP_PNTS",
                   "FCST_THRESH", "OBS_THRESH", "COV_THRESH", "ALPHA",
                   "LINE_TYPE");
