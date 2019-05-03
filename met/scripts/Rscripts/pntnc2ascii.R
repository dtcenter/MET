library(ncdf4);

usage = function(){
	cat("\nUsage: pntnc2ascii\n",
		"        [-sid name]\n",
		"        [-msg_type name]\n",
		"        [-obs_var name]\n",
		"        nc_file\n\n",
		"        where   \"-sid name\" prints only observations for that station id (optional).\n",
		"                \"-msg_typ name\" prints only observations for that message type (optional).\n",
		"                \"-obs_var name\" prints only observations for that observation variable or GRIB code (optional).\n",
		"                \"nc_file\" is the netCDF point observation file (required).\n\n",
		sep="");
}

# parse and verify command line arguments
strSid = ""; strMsg = ""; strObs = "";
listArgs = commandArgs(TRUE);
if( 1 > length(listArgs) ){ usage(); q(); }
if( 1 < length(listArgs) ){
	for( i in seq(1, length(listArgs) - 1, by=2) ){
		if       ( "-sid" == listArgs[i] ){
			strSid = listArgs[i+1];
		} else if( "-msg_typ" == listArgs[i] || "-msg_type" == listArgs[i] ){
			strMsg = listArgs[i+1];
		} else if( "-obs_var"  == listArgs[i] ){
			strObs = listArgs[i+1];
		} else {
			cat("ERROR: unrecognized input argument '", listArgs[i], "'\n", sep="");
			usage(); q(status=1);
		}
	}
	#cat(" INFO: strSid: [", strSid, "]", " strSid: [", strMsg, "]", " strObs: [", strObs, "]\n", sep="");
}


# open the input NetCDF file
strPntNc = listArgs[ length(listArgs) ];
ncPnt = nc_open(c(strPntNc), write=F);

# build the header data frame
use_index = F;
if( 0 < length(ncPnt$var[["hdr_lat"]]) ) {
    use_index = T;
    dfHdr = data.frame(
        typ    = ncvar_get(ncPnt, ncPnt$var[["hdr_typ"]]),
        sid    = ncvar_get(ncPnt, ncPnt$var[["hdr_sid"]]),
        vld    = ncvar_get(ncPnt, ncPnt$var[["hdr_vld"]]),
        lat    = ncvar_get(ncPnt, ncPnt$var[["hdr_lat"]]),
        lon    = ncvar_get(ncPnt, ncPnt$var[["hdr_lon"]]),
        elv    = ncvar_get(ncPnt, ncPnt$var[["hdr_elv"]])
    );
    if( strSid != "" ) {
        sid_list = ncvar_get(ncPnt, ncPnt$var[["hdr_sid_table"]]);
        if( 0 < length(sid_list)){
            strSid = which(sid_list == strSid);
            if (strSid > 0) strSid = strSid - 1;
        }
    };
    if( strMsg != "") {
        msg_list = ncvar_get(ncPnt, ncPnt$var[["hdr_typ_table"]]);
        if (0 < length(msg_list)){
            strMsg = which(msg_list == strMsg);
            if (strMsg > 0) strMsg = strMsg - 1;
        }
    };
} else {
    dfHdr = data.frame(
        typ    = ncvar_get(ncPnt, ncPnt$var[["hdr_typ"]]),
        sid    = ncvar_get(ncPnt, ncPnt$var[["hdr_sid"]]),
        vld    = ncvar_get(ncPnt, ncPnt$var[["hdr_vld"]]),
        lat    = ncvar_get(ncPnt, ncPnt$var[["hdr_arr"]])[1,],
        lon    = ncvar_get(ncPnt, ncPnt$var[["hdr_arr"]])[2,],
        elv    = ncvar_get(ncPnt, ncPnt$var[["hdr_arr"]])[3,]
    );
}
dfHdr$hdr_id = seq(0,nrow(dfHdr)-1);

# replace empty SID strings with NA
if( ! use_index ) {
    dfHdr$sid[dfHdr$sid == ""] = NA;
}

# check for the obs_qty variable
if( 0 < length(ncPnt$var[["obs_qty"]]) ) {
	ObsQty = ncvar_get(ncPnt, ncPnt$var[["obs_qty"]]);
    if( 0 < length(ncPnt$var[["obs_qty_table"]]) ) {
        ObsQtyList = ncvar_get(ncPnt, ncPnt$var[["obs_qty_table"]]);
        ObsQty = ObsQtyList[ObsQty+1];
    }
} else {
	ObsQty = rep(-9999, ncPnt$dim[["nobs"]]$len);
}

# build the observation data frame
if( 0 < length(ncPnt$var[["obs_val"]]) ) {
    if( 0 < length(ncPnt$var[["obs_vid"]]) ) {
        vid_or_gc = ncvar_get(ncPnt, ncPnt$var[["obs_vid"]])
    } else {
        vid_or_gc = ncvar_get(ncPnt, ncPnt$var[["obs_gc"]])
    }
    dfObs = data.frame(
        hdr_id = ncvar_get(ncPnt, ncPnt$var[["obs_hid"]]),
        var    = vid_or_gc,
        lvl    = ncvar_get(ncPnt, ncPnt$var[["obs_lvl"]]),
        hgt    = ncvar_get(ncPnt, ncPnt$var[["obs_hgt"]]),
        qty    = ObsQty,
        ob     = signif(ncvar_get(ncPnt, ncPnt$var[["obs_val"]]), 5)
    );
} else {
    dfObs = data.frame(
        hdr_id = ncvar_get(ncPnt, ncPnt$var[["obs_arr"]])[1,],
        var    = ncvar_get(ncPnt, ncPnt$var[["obs_arr"]])[2,],
        lvl    = ncvar_get(ncPnt, ncPnt$var[["obs_arr"]])[3,],
        hgt    = ncvar_get(ncPnt, ncPnt$var[["obs_arr"]])[4,],
        qty    = ObsQty,
        ob     = signif(ncvar_get(ncPnt, ncPnt$var[["obs_arr"]])[5,], 5)
    );
}

# check for the obs_var variable and update dfObs$var
if( 0 < length(ncPnt$var[["obs_var"]]) ) {
	ObsVar = ncvar_get(ncPnt, ncPnt$var[["obs_var"]]);
	dfObs$var = ObsVar[dfObs$var+1];
}

# apply the filtering criteria and merge the headers and obs
if( strSid != "" ){ dfHdr = dfHdr[dfHdr$sid == strSid,]; }
if( strMsg != "" ){ dfHdr = dfHdr[dfHdr$typ == strMsg,]; }
if( strObs != "" ){ dfObs = dfObs[dfObs$var == strObs,]; }

if( use_index ) {
    if( 0 < length(ncPnt$var[["hdr_typ_table"]]) ) {
        HdrTypList = ncvar_get(ncPnt, ncPnt$var[["hdr_typ_table"]]);
        dfHdr$typ = HdrTypList[dfHdr$typ+1];
    }
    if( 0 < length(ncPnt$var[["hdr_sid_table"]]) ) {
        HdrSidList = ncvar_get(ncPnt, ncPnt$var[["hdr_sid_table"]]);
        dfHdr$sid = HdrSidList[dfHdr$sid+1];
    }
    if( 0 < length(ncPnt$var[["hdr_vld_table"]]) ) {
        HdrVldList = ncvar_get(ncPnt, ncPnt$var[["hdr_vld_table"]]);
        dfHdr$vld = HdrVldList[dfHdr$vld+1];
    }
}

dfSid = merge(dfHdr, dfObs);

# format level column to HHMMSS for accumulation intervals
ind_accum = ( dfSid$var == 61 | dfSid$var == "APCP"  |
              dfSid$var == 62 | dfSid$var == "NCPCP" |
              dfSid$var == 63 | dfSid$var == "ACPCP" ) & ( dfSid$lvl != -9999 );
dfSid$lvl[ind_accum] = format(.POSIXct(dfSid$lvl[ind_accum], tz="GMT"), "%H%M%S");

# print the formatted observations
options(scipen=20, digits=6);
apply(dfSid[,2:12], 1, function(d){ cat(d,"\n") })

# better formatting, but very slow...
#for(i in 1:nrow(dfSid)){
#	cat( sprintf("%-8s%-8s%-15s%11.5f%11.5f%6d%5d%10.2f%14.5f%12.5f\n",
#			dfSid[i,]$typ,
#			dfSid[i,]$sid,
#			dfSid[i,]$vld,
#			dfSid[i,]$lat,
#			dfSid[i,]$lon,
#			dfSid[i,]$elv,
#			dfSid[i,]$gc,
#			dfSid[i,]$lvl,
#			dfSid[i,]$hgt,
#			dfSid[i,]$ob
#		 ), sep="");
#}

# close the input NetCDF file
nc_close(ncPnt);
