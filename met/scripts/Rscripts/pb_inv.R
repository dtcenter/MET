library(ncdf);

usage = function(){
	cat("usage: pb_inv [-sid {sid}] [-msg {msg}] [-gc {gc}] {pb2nc_file}\n",
		"  where\n",
		"      -sid {sid} prints only observations with station id sid, optional\n",
		"      -msg {msg} prints only observations with message type msg, optional\n",
		"      -gc {gc} prints only observations with grib code gc, optional\n",
		"      pb2nc_file is the name of a PB2NC output NetCDF file\n",
		sep="");
}

# parse and verify command line arguments
strSid = ""; strMsg = ""; strGc = "";
listArgs = commandArgs(TRUE);
if( 1 > length(listArgs) ){	usage(); q(); }
if( 1 < length(listArgs) ){
	for( i in seq(1, length(listArgs) - 1, by=2) ){
		if       ( "-sid" == listArgs[i] ){
			strSid = listArgs[i+1];
		} else if( "-msg" == listArgs[i] ){
			strMsg = listArgs[i+1];
		} else if( "-gc"  == listArgs[i] ){
			strGc = listArgs[i+1];
		} else {
			cat("ERROR: unrecognized input argument '", listArgs[i], "'\n", sep="");
			usage(); q(status=1);
		}
	}
}

# open the input NetCDF file
strPbNc = listArgs[ length(listArgs) ];
ncPb = open.ncdf(c(strPbNc), write=F);

# build the header data frame
dfHdr = data.frame(
	typ    = get.var.ncdf(ncPb, ncPb$var[["hdr_typ"]]),
	sid    = get.var.ncdf(ncPb, ncPb$var[["hdr_sid"]]),
	vld    = get.var.ncdf(ncPb, ncPb$var[["hdr_vld"]]),
	lat    = get.var.ncdf(ncPb, ncPb$var[["hdr_arr"]])[1,],
	lon    = get.var.ncdf(ncPb, ncPb$var[["hdr_arr"]])[2,],
	elv    = get.var.ncdf(ncPb, ncPb$var[["hdr_arr"]])[3,]
);
dfHdr$hdr_id = seq(0,nrow(dfHdr)-1);

# build the observation data frame
dfObs = data.frame(
	hdr_id = get.var.ncdf(ncPb, ncPb$var[["obs_arr"]])[1,],
	gc     = get.var.ncdf(ncPb, ncPb$var[["obs_arr"]])[2,],
	lvl    = get.var.ncdf(ncPb, ncPb$var[["obs_arr"]])[3,],
	hgt    = get.var.ncdf(ncPb, ncPb$var[["obs_arr"]])[4,],
	ob     = get.var.ncdf(ncPb, ncPb$var[["obs_arr"]])[5,]
);

# apply the filtering criteria and merge the headers and obs
if( strSid != "" ){ dfHdr = dfHdr[dfHdr$sid == strSid,]; }
if( strMsg != "" ){ dfHdr = dfHdr[dfHdr$typ == strMsg,]; }
if( strGc  != "" ){ dfObs = dfObs[dfObs$gc  == strGc, ]; } 
dfSid = merge(dfHdr, dfObs);

# print the formatted observations
options(scipen=20);
apply(dfSid, 1, function(d){ cat(d,"\n") })

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

#close.ncdf(ncPb);
