#!/bin/sh
#
#  $Id: test.sh,v 4.2.1.1 2007/10/07 07:10:47 stevej098 Exp $
#
#---------------------------------------------------------------------

# set -x

SCRIPTS="closed feedback fesc mc-test multi_class shadowcpu simple_series passport"

#---------------------------------------------------------------------

STR=`uname -a | sed 's/^.* //'`

# echo $STR

if [ ${STR} = "Cygwin" ] ; then
   TYPE="Intel"
else
   TYPE="UNIX"
fi

# echo $TYPE

EXTS='exe py pl'

#---------------------------------------------------------------------

pwd

for SCRIPT in $SCRIPTS; do
   egrep -v 'Using:|Ver:|of :' ${SCRIPT}.out >  ${SCRIPT}.tst

   for EXT in $EXTS ; do
      if [ \( $EXT = "exe" \) -a \( $TYPE != 'Intel' \) ] ; then
         ./${SCRIPT}        | egrep -v 'Using:|Ver:|of :' >  ${SCRIPT}_${EXT}.tst
      else
         ./${SCRIPT}.${EXT} | egrep -v 'Using:|Ver:|of :' >  ${SCRIPT}_${EXT}.tst
      fi

      diff ${SCRIPT}_${EXT}.tst ${SCRIPT}.tst
      diff ${SCRIPT}_${EXT}.tst ${SCRIPT}.tst > diff.log

      NO=`wc -l diff.log | awk '{print $1}'`

      if [ $NO -eq 0 ] ; then
         echo "$SCRIPT $EXT - OK"
      else
         echo "$SCRIPT $EXT - Failed: check ${SCRIPT}_${EXT}.tst"
      fi
   done

   /bin/rm -f ${SCRIPT}.tst
done

#---------------------------------------------------------------------

