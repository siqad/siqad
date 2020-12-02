#!/bin/bash

# This is intended to be used for qmake compilation for Windows targets on Appveyor. This is to be called by the SiQAD CMake step.
# Argument list:
# 0th arg: this script
# 1st arg: qmake command
# 2nd arg: make command
# 3rd arg: qt pro file location
# 4th arg: copy target location for siqad.exe
# 5th arg: zipper library path

set -e

if [ ! -z ${MXE_PATH+x} ]; then
    export PATH="$PATH:$MXE_PATH"
fi

CMPL_QMAKE_COMMAND=$1
CMPL_MAKE_COMMAND=$2
SIQAD_Q_PRO_FILE=$3
SIQAD_EXE_TARGET=$4
ZIPPER_LIB_PATH=$5

echo "QMAKE: ${CMPL_QMAKE_COMMAND}"
echo "MAKE: ${CMPL_MAKE_COMMAND}"
echo "QPRO: ${SIQAD_Q_PRO_FILE}"
echo "EXE TARGET: ${SiQAD_EXE_TARGET}"
echo "ZIPPER: ${ZIPPER_LIB_PATH}"

#if [ -z ${CMPL_QMAKE_COMMAND+x} ] || [ -z ${CMPL_MAKE_COMMAND+x} ] || [ -z ${SIQAD_Q_PRO_FILE+x} ]; then
#    echo "One or more of the following environmental variables aren't set: CMPL_QMAKE_COMMAND, CMPL_MAKE_COMMAND, SIQAD_Q_PRO_FILE"
#    exit 1
#fi

# determine number of physical cores to use for compilation
cpu_cores=$(grep "^core id" /proc/cpuinfo | sort -u | wc -l)

# compile
export ZIPPER_LIB_PATH="${ZIPPER_LIB_PATH}"
dir -R "${ZIPPER_LIB_PATH}"
${CMPL_QMAKE_COMMAND} "${SIQAD_Q_PRO_FILE}"
${CMPL_MAKE_COMMAND}
mv release/siqad.exe "${SIQAD_EXE_TARGET}"
