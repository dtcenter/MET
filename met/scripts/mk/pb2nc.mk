


########################################################################


PB2NC_EXEC = ${OTHER_DIR}/pb2nc/pb2nc


########################################################################


   ##
   ##  pb2nc
   ##
   ##     prerequisites:
   ##


pb2nc: ${PB2NC_EXEC}
	@ echo
	@ echo "*** Running PB2NC on a PREPBUFR file ***"
	${PB2NC_EXEC} \
 	../data/sample_obs/prepbufr/ndas.t00z.prepbufr.tm12.20070401.nr \
 	${TEST_OUT_DIR}/pb2nc/sample_pb.nc \
 	config/PB2NCConfig_G212 \
 	-v 2
	@
	@ touch pb2nc


########################################################################


