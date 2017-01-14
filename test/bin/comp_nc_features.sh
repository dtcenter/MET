#!/bin/bash

# check for environment variables, use defaults if necessary
MET_TEST_BASE=${MET_TEST_BASE:-/d3/projects/MET/MET_development/svn-met-dev.cgd.ucar.edu/trunk/test}
MET_TEST_RSCRIPT=${MET_TEST_RSCRIPT:-/usr/local/bin/Rscript}

# get environment settings
. ${MET_TEST_BASE}/bin/set_env.sh

NC_FILE_PREFIX=ascii2nc_trmm_compression

function get_file_size {
  fi_file=$1
  fo_size=-1
  
  if [ "" != "$fi_file" ]; then
    [ -e $fi_file ] && fo_size=`stat -c%s $fi_file`
  fi
  echo "$fo_size"
}

nc_output_dir=$1
if [ "" != "$nc_output_dir" -a -e "$nc_output_dir" ]; then
  nc_comression_files=`ls $nc_output_dir/${NC_FILE_PREFIX}*.nc`
  for nc_comression_file1 in $nc_comression_files
  do
    file_size1=`get_file_size $nc_comression_file1`
    for nc_comression_file2 in $nc_comression_files
    do
      [ $nc_comression_file1 == $nc_comression_file2 ] && continue
      file_size2=`get_file_size $nc_comression_file2`
      #echo "   DEBUG: $nc_comression_file1 - $file_size1 : $file_size2 - $nc_comression_file2"
      if [ $file_size1 -eq  $file_size2 ]; then
        echo "ERROR: same file size with different compression ($nc_comression_file1 and $nc_comression_file2)"
      fi
    done
  done
fi

echo "  Done `basename $0` at `date`"
