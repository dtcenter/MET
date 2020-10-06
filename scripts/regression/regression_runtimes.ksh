#!/bin/ksh

if [ $# != 1 ]; then
  echo "$0: Must specify the nightly build log file (e.g. NBYYYMMDD.out) to be processed."
  exit 1;
fi

# Get the revision numbers
REV2=`grep "^building..." $1 | head -1 | cut -d' ' -f2 | cut -d'.' -f1`
REV1=`grep "^building..." $1 | tail -1 | cut -d' ' -f2 | cut -d'.' -f1`

# Get the list of tests
for TEST in `egrep "^TEST:" $1 | cut -d':' -f2 | cut -d'-' -f1 | awk '!x[$0]++'`; do
   RT2=`grep "TEST: $TEST " $1 | head -1 | cut -d'-' -f3 | tr -d ' ' | sed 's/sec//g'`
   RT1=`grep "TEST: $TEST " $1 | tail -1 | cut -d'-' -f3 | tr -d ' ' | sed 's/sec//g'`
   echo "$REV1 = $(printf "%-8.3f" $RT1)	$REV2 = $(printf "%-8.3f" $RT2)	DIFF =	$(printf "%-8.3f" $(( $RT2 - $RT1 )))	TEST = $TEST"
done



