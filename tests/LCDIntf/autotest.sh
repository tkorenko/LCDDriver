#!/bin/sh

MAKE="gmake"
PROG="build/testsRunner"
ARGS="-c"

SRC_DIR="../../src"
TESTS_CMN_DIR="../common"

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

    WATCHTHEM=""
    for D in ${SRC_DIR} ${TESTS_CMN_DIR} .
    do
        WATCHTHEM="${WATCHTHEM} `ls ${D}/*.[hc] ${D}/*.cpp`"
    done
    holdon $WATCHTHEM
    sleep 1
done
