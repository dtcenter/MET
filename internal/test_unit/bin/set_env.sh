export NETCDF=${NETCDF:-/usr/local/netcdf}
export LDFLAGS=-Wl,-rpath,/usr/local/hdf5-1.8.5patch1/lib:/usr/local/netcdf/lib
export LD_LIBRARY_PATH=${NETCDF}/lib:/usr/local/hdf5-1.8.5patch1/lib:${LD_LIBRARY_PATH}
export NETCDF_INCLUDE=${NETCDF}/include
export NETCDF_LIB=${NETCDF}/lib
