#!/bin/bash
DIR=$AUX_DIR/MET_TC_development/RMW
$EVENTS_DIR=
TRACK=aal142016.dat
# ./tc_rmw $DIR/gfs.t00z.pgrb2.0p25.f000 \
./tc_rmw $AUX_DIR/Events/2016-09-29-GulfHurricane/MERRA2_400.inst3_3d_asm_Np.20160927.nc4 \
    -config TCRMWConfig_test \
    -adeck $DIR/$TRACK -v 4
