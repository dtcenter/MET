#!/bin/bash

DEBUG=0
DEBUG_P="     DEBUG]"
INFO_P="      INFO]"
WARN_P="  == WARN =="
SHOW_HELP=0
DO_UPLOAD=1
FAQ_DIR=`pwd`

TMP_OUT_DIR=/dev/shm/$LOGNAME
OUTPUT_DIR=/dev/shm/$LOGNAME/faq_output
if [ ! -d /dev/shm/ ]; then
  OUTPUT_DIR=/tmp/$LOGNAME/faq_output
  TMP_OUT_DIR=/tmp
fi
[ -e $OUTPUT_DIR ] && FAQ_DIR=$OUTPUT_DIR


if [ $# -gt 0 ]; then
  for arg in $@
  do
    [ "" == "$arg" ] && continue
    my_arg=`echo $arg | tr "[:upper:]" "[:lower:]" | sed -e "s|_|-|g"`
    [ $DEBUG -gt 0 ] && echo "$DEBUG_P arg: $arg  FAQ_DIR: $FAQ_DIR"
    if [ $my_arg == "debug" -o $my_arg == "-debug" ]; then
      DEBUG=1
    elif [ $my_arg == "help" -o $my_arg == "-help"  -o $my_arg == "--help" ]; then
      SHOW_HELP=1
    elif [ -d $arg ]; then
      FAQ_DIR=$arg
    else
      echo "$INFO_P Ignored the argument [$arg]"
    fi
  done
fi

if [ $SHOW_HELP -eq 1 ]; then
  echo "   Usage: $0 <FAQ_DIR>"
  echo "       <FAQ_DIR>: The directory where the XHTML and image files are. Default: $OUTPUT_DIR"
  exit 0
fi

rsync_cmd="rsync -auvC --include=\"*/\" --include=\"*html\" --include=\"*.png\" --include=\"*.php\" --include=\"*.jpg\" --include=\"*.cfg\" --exclude=\"*\" $OUTPUT_DIR/ mandan:/d2/www/dtcenter/met/users/support/faqs/"
if [ $DO_UPLOAD -eq 1 ]; then
  echo "   Uploading files to mandan ..."
  eval $rsync_cmd
  exit_code=$?
  if [ $exit_code -eq 0 ]; then
    echo ""
    echo "  Files were uploaded"
    echo ""
  fi
else
  echo " Upload the files to web server (mandan) with following command"
  echo "   $rsync_cmd"
fi
