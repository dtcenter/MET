#!/bin/ksh

if [ $# != 1 ]; then
  echo "$0: Must specify the nightly build directory (e.g. NBYYYMMDD) to be processed."
  exit 1;
fi

# Get the log files for the two revisions
REF_LOG=`ls -1 ${1}/test_unit_*.log | egrep -i  ref`
NEW_LOG=`ls -1 ${1}/test_unit_*.log | egrep -iv ref`
REF_STR=`basename ${REF_LOG} | sed -r 's/test_unit_//g' | sed -r 's/.log//g'`
NEW_STR=`basename ${NEW_LOG} | sed -r 's/test_unit_//g' | sed -r 's/.log//g'`

# Get the list of tests
for TEST in `egrep "^TEST:" ${REF_LOG} | awk '{print $2}' | awk '!x[$0]++'`; do
   REF_RT=`grep "TEST: $TEST " ${REF_LOG} | tail -1 | cut -d'-' -f3 | tr -d ' ' | sed 's/sec//g'`
   NEW_RT=`grep "TEST: $TEST " ${NEW_LOG} | head -1 | cut -d'-' -f3 | tr -d ' ' | sed 's/sec//g'`
   echo "${REF_STR} = $(printf "%-8.3f" ${REF_RT})	${NEW_STR} = $(printf "%-8.3f" ${NEW_RT})	DIFF =	$(printf "%-8.3f" $(( ${NEW_RT} - ${REF_RT} )))	TEST = $TEST"
done

