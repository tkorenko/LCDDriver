#!/bin/sh

MAKE="gmake"
PROG="build/testsRunner"
ARGS="-c"

while true
do
    ${MAKE}
    if [ "$?" -eq "0" -a -x "${PROG}" ]; then
        echo "##============================================##"
        echo "##                                            ##"
        echo "##               TEST SUITE RUN               ##"
        echo "##                                            ##"
        echo "##============================================##"
        ${PROG} ${ARGS}
    fi
    WATCHTHEM=`ls ../src/*.h ../src/*.c *.h *.cpp`
    holdon $WATCHTHEM
    sleep 1
done
