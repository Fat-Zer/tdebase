#!/bin/sh

SUM=$( @MD5SUM@ @CMAKE_CURRENT_SOURCE_DIR@/fish.pl | cut -d ' ' @MD5SUM_CUT@ )

#echo "#define CHECKSUM "\"$SUM\"" > fishcode.h
#echo 'static const char *fishCode(' >> fishcode.h
#sed -e 's/\\/\\\\/g;s/"/\\"/g;s/^[ 	]*/"/;/^"# /d;s/[ 	]*$$/\\n"/;/^"\\n"$$/d;s/{CHECKSUM}/'$$SUM'/;' @CMAKE_CURRENT_SOURCE_DIR@/fish.pl >> fishcode.h
#echo ');' >> fishcode.h
