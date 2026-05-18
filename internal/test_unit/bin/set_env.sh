export NETCDF=${NETCDF:-/nrit/ral/netcdf}
export LDFLAGS=-Wl,-rpath,${NETCDF}/lib
export LD_LIBRARY_PATH=${NETCDF}/lib:${LD_LIBRARY_PATH}
export NETCDF_INCLUDE=${NETCDF}/include
export NETCDF_LIB=${NETCDF}/lib
