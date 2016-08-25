


########################################################################


WAVELET_STAT_EXEC = ${CORE_DIR}/wavelet_stat/wavelet_stat


########################################################################


   ##
   ##  wavelet_stat
   ##
   ##     prerequisites: pcp_combine
   ##


wavelet_stat: ${WAVELET_STAT_EXEC} pcp_combine
	touch wavelet_stat


########################################################################


