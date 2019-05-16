#!/bin/bash
#
# Run Fortify Source Code Analyzer on a specified revision of MET
#=======================================================================
#
# This run_fortify_sca.sh script will check out the specified version
# of MET and run the Fortify Source Code Analyzer on it.  First,
# go to the directory where you would like the SCA output written and
# then run:
#    svn co https://svn-met-dev.cgd.ucar.edu/build/scripts
#    scripts/run_fortify_sca.sh trunk [rev] | branch name [rev] | tag name [rev]
#
# Usage: run_fortify_sca.sh [-q] trunk [rev] | branch name [rev] | tag name [rev]
#    Test the latest version of MET repository trunk:
#       run_fortify_sca.sh trunk
#    Test the specified revision of MET:
#       run_fortify_sca.sh trunk [rev]
#    Test the specified branched version of MET:
#       run_fortify_sca.sh tag branch name [rev]
#    Test the specified tagged version of MET:
#       run_fortify_sca.sh tag name [rev]
#
#=======================================================================

# Constants
SVN_BASE="https://svn-met-dev.cgd.ucar.edu"
FORTIFY_BIN=/d3/projects/Fortify/Fortify_SCA_and_Apps_18.10/bin

# Check for quiet flag
QUIET=0
if [[ $1 == "-q" ]]; then QUIET=1; shift; REDIR=">/dev/null 2>&1"; fi

function usage {
   [ $QUIET -eq 1 ] && return
        echo
        echo "USAGE: run_fortify_sca.sh [-q] trunk [rev] | branch name [rev] | tag name [rev]"
        echo "   where"
        echo "                  -q specifies no text output, exit status indicates success"
        echo "         trunk [rev] specifies the repository trunk"
        echo "      tag name [rev] specifies a named tag, e.g. met-6.0"
        echo "   branch name [rev] specifies a named branch, e.g. met-6.0_bugfix"
        echo "               [rev] optionally specifies a revision number, e.g. met-6.0_bugfix 4160"
        echo
}

function log {
   [ $QUIET -eq 0 ] && echo $*
}

# Check for arguments
if [[ $# -lt 1 ]]; then usage; exit; fi

# Parse the input parameters
case $1 in
   trunk)
      MET_BASE_DIR="trunk/met"
      if [ $# -lt 2 ]; then
         REV=$(svn info $SVN_BASE | egrep '^Rev' | awk '{print $2}')
      else
         REV=$2
      fi
      NAME="trunk_rev${REV}"
      BUILD_ID="met_trunk"
      ;;
   tag)
      if [ $# -lt 2 ]; then
         log "ERROR: tag not specified";
         usage;
         exit 1;
      fi
      MET_BASE_DIR="tags/met/$2"
      if [ $# -lt 3 ]; then
         REV=$(svn info $SVN_BASE/tags/met/$2 | egrep 'Last Changed Rev' | awk '{print $4}')
      else
         REV=$3
      fi
      NAME="tag_$2_rev${REV}"
      BUILD_ID="met_tag_$2"
      ;;
   branch)
      if [ $# -lt 2 ]; then
         log "ERROR: branch not specified";
         usage;
         exit 1;
      fi
      MET_BASE_DIR="branches/met/$2"
      if [ $# -lt 3 ]; then
         REV=$(svn info $SVN_BASE/branches/met/$2 | egrep 'Last Changed Rev' | awk '{print $4}')
      else
         REV=$3
      fi
      NAME="branch_$2_rev${REV}"
      BUILD_ID="met_branch_$2"
      ;;
   *)
      log "ERROR: unrecognized input argument '$1'";
      usage;
      exit 1;
      ;;
esac

# Check out the requested version of MET
[ -e "met_$NAME" ] && eval "rm -rf met_$NAME"
SVN_MET="svn export -q -r ${REV} ${SVN_BASE}/${MET_BASE_DIR} met_$NAME"
log "checking out MET: $SVN_MET"
eval "$SVN_MET" $REDIR
if [ $? -ne 0 ]; then log "ERROR: checkout of met_$NAME failed"; exit 1; fi

# Create logfile
LOGFILE=../run_sca_fortify_$NAME.log
rm -f $LOGFILE

# Build the MET instance
cd met_$NAME
log "building met_$NAME..."
log "logging to $LOGFILE..."

# Run bootstrap
eval "./bootstrap" >> $LOGFILE
if [ $? -ne 0 ]; then log "ERROR: bootstrap of met_$NAME failed"; exit 1; fi

# Do no manually set the CXX and F77 compilers.
# Let the configure script pick them.
# Otherwise, the Fortify logic does not work.
export MET_DEVELOPMENT=true

# Run the configure script
eval "./configure --prefix=`pwd` \
            --enable-grib2 \
            --enable-modis \
            --enable-mode_graphics \
            --enable-lidar2nc \
            --enable-python" >> $LOGFILE
if [ $? -ne 0 ]; then log "ERROR: configuration of met_$NAME failed"; exit 1; fi

# Run Fortify SCA clean
eval "${FORTIFY_BIN}/sourceanalyzer -b ${BUILD_ID} -clean" >> $LOGFILE
if [ $? -ne 0 ]; then log "ERROR: Fortify SCA clean for $BUILD_ID failed"; exit 1; fi

# Run Fortify SCA make
eval "${FORTIFY_BIN}/sourceanalyzer -b ${BUILD_ID} -logfile translate_${BUILD_ID}.log -debug -verbose make" >> $LOGFILE
if [ $? -ne 0 ]; then log "ERROR: Fortify SCA make for $BUILD_ID failed"; exit 1; fi

# Run Fortify SCA scan
eval "${FORTIFY_BIN}/sourceanalyzer -b ${BUILD_ID} -scan -f ${BUILD_ID}.fpr" >> $LOGFILE
if [ $? -ne 0 ]; then log "ERROR: Fortify SCA FPR scan for $BUILD_ID failed"; exit 1; fi

# Run Fortify to make an MBS file
eval "${FORTIFY_BIN}/sourceanalyzer -b ${BUILD_ID} -export-build-session ${BUILD_ID}.mbs" >> $LOGFILE
if [ $? -ne 0 ]; then log "ERROR: Fortify SCA MBS file for $BUILD_ID failed"; exit 1; fi

# Run Fortify report generator to make a PDF file
TODAY=`date +%Y%m%d`
eval "${FORTIFY_BIN}/ReportGenerator -format pdf -f ${BUILD_ID}_${TODAY}_rev${REV}.pdf -source ${BUILD_ID}.fpr" >> $LOGFILE
if [ $? -ne 0 ]; then log "ERROR: Fortify ReportGenerator for $BUILD_ID failed"; exit 1; fi

exit
