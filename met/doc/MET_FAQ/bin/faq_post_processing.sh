#!/bin/bash

DEBUG=0
DEBUG_P="     DEBUG]"

SHOW_HELP=0
KEEP_TIMESTAMP=1
DO_UPLOAD=0
HTML_DIR=.

TMP_OUT_DIR=/dev/shm/$LOGNAME
OUTPUT_DIR=/dev/shm/$LOGNAME/faq_output
if [ ! -d /dev/shm/ ]; then
  OUTPUT_DIR=/tmp/$LOGNAME/faq_output
  TMP_OUT_DIR=/tmp
fi
[ ! -d $OUTPUT_DIR ] && mkdir -p $OUTPUT_DIR


if [ $# -gt 0 ]; then
  for arg in $@
  do
    [ "" == "$arg" ] && continue
    if [ $arg == "DEBUG" -o $arg == "debug" ]; then
      DEBUG=1
    elif [ $arg == "UPLOAD" -o $arg == "upload" ]; then
      DO_UPLOAD=1
    elif [ $arg == "timestamp" ]; then
      KEEP_TIMESTAMP=0
    elif [ $arg == "HELP" -o $arg == "help" -o $arg == "-help"  -o $arg == "--help" ]; then
      SHOW_HELP=1
    elif [ -d $arg ]; then
      HTML_DIR=$arg
    fi
  done
fi

if [ $SHOW_HELP -eq 1 ]; then
  echo "   Usage: $0 <source_xhtml_dir> <upload> <timestamp>"
  echo "                    upload: Upload the HTML and image files to mandan (Optional)"
  echo "                 timestamp: Do not keep the timestamp from the raw XHTML (Optional)"
  echo "        <source_xhtml_dir>: The directory where the XHTML and image files are"
  exit 0
fi

[ $DEBUG -gt 0 ] && echo "$DEBUG_P  HTML_DIR: $HTML_DIR"
CUR_DIR=`pwd`
cd $HTML_DIR
html_files=`find -name "*xhtml"`
for html_file in $html_files
do
  [ `basename $html_file` == "MET_FAQ_LINKS_MASTER.xhtml" ] && continue
  echo "   procesing $HTML_DIR/$html_file ..."
  source_sub_dir=`dirname $html_file | sed -e 's|^\./||'`
  tmp_dir=$OUTPUT_DIR/$source_sub_dir
  
  [ ! -d $tmp_dir ] && mkdir -p $tmp_dir
  #tmp_html_file=${html_file}.tmp
  tmp_html_file=$OUTPUT_DIR/${html_file}
  image_count=`ls $source_sub_dir/*png $source_sub_dir/*jpg 2>/dev/null | wc -w`
  [ $DEBUG -gt 0 -a $image_count -gt 0 ] && echo "$DEBUG_P  images at $source_sub_dir: $image_count"
  if [ $image_count -gt 0 ]; then
    cat $html_file | \
    sed -e 's|<a \(id=.*\) />|<a \1></a>|g' \
        -e 's|<h2 class="section_">|<h2>|g' \
        -e "s|src='\(.*\.png'\)|src='${source_sub_dir}/\1|g"       \
        -e "s|alt='image: \(.*\.png'\)|alt='image: ${source_sub_dir}/\1|g" \
        -e "s|/[0-9]*_\(.*\)_MET_FAQ_${source_sub_dir}_\(.*\).png'|/\2.png'|g"  \
        -e 's|<h2 class="section">|<h2>|g'  > $tmp_html_file
  else
    cat $html_file | \
    sed -e 's|<a \(id=.*\) />|<a \1></a>|g' \
        -e 's|<h2 class="section_">|<h2>|g' \
        -e 's|<h2 class="section">|<h2>|g'  > $tmp_html_file
  fi
  [ $KEEP_TIMESTAMP -eq 1 ] && touch -r $html_file $tmp_html_file
  #if [ -e $tmp_html_file ]; then
  #  [ $tmp_html_file -nt $html_file ] && mv $tmp_html_file $html_file
  #fi
done
echo ""

cd $CUR_DIR
cd $HTML_DIR

tmp_sed_file=$TMP_OUT_DIR/tmp_`basename $0`.tmp
html_file=MET_FAQ_LINKS_MASTER.xhtml
if [ -e $html_file ]; then
  [ -e $tmp_sed_file ] && rm $tmp_sed_file
  echo 's|<a \(id=.*\) />|<a \1></a>|g'      >> $tmp_sed_file
  echo 's|<h2 class="section_">|<h2>|g'      >> $tmp_sed_file
  echo 's|<li class="labeling_item">|<li>|g' >> $tmp_sed_file
  sub_dirs=`ls * -d`
  for sub_dir in $sub_dirs
  do
    [ -f $sub_dir ] && continue
    
    echo "s|faqs/${sub_dir}/${sub_dir}_\(.*\).xhtml|faqs/faq.php?name=${sub_dir}\&category=\1|g" >> $tmp_sed_file
  done
  tmp_dir=$OUTPUT_DIR/$source_sub_dir
  [ ! -d $tmp_dir ] && mkdir -p $tmp_dir
  tmp_html_file=$OUTPUT_DIR/${html_file}
  
  cat $html_file | sed -f $tmp_sed_file > $tmp_html_file
fi
#[ -e $tmp_sed_file ] && rm $tmp_sed_file
echo $tmp_sed_file
cat $tmp_sed_file

cd $CUR_DIR
cd $HTML_DIR
images_files=""
for other_ext in png jpg php html cfg
do
  other_files=`find -name "*.$other_ext"`
  for other_file in $other_files
  do
    output_dir=$OUTPUT_DIR/`dirname $other_file`
    [ ! -d $output_dir ] && mkdir -p $output_dir
    echo "   copying $HTML_DIR/$other_file ..."
    is_raw_image_name=`basename $other_file | grep MET_FAQ | wc -w`
    [ $is_raw_image_name -gt 0 ] && continue
    cp -p $other_file $output_dir
  done
done

if [ $DO_UPLOAD -eq 1 ]; then
  echo "   Uploading files to mandan ..."
  rsync -auvC --include="*/" --include="*html" --include="*.png" --include="*.php" --include="*.jpg" --include="*.cfg" --exclude="*" $OUTPUT_DIR/ mandan:/d2/www/dtcenter/met/users/support/faqs/
fi
