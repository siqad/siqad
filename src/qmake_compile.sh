#!/bin/bash

# Set envirnomental variable ${CMPL_QMAKE_COMMAND} to specify the qmake command for compilation.
# Set envirnomental variable ${CMPL_MAKE_COMMAND} to specify the make command for compilation, might just still be make for some toolchains.
# Set envirnomental variable ${SIQAD_Q_PRO_FILE} to specify the make command for compilation, might just still be make for some toolchains.

set -e

if [ ! -z ${MXE_PATH+x} ]; then
    export PATH="$PATH:$MXE_PATH"
fi

if [ -z ${CMPL_QMAKE_COMMAND+x} ] || [ -z ${CMPL_MAKE_COMMAND+x} ] || [ -z ${SIQAD_Q_PRO_FILE+x} ]; then
    echo "One or more of the following environmental variables aren't set: CMPL_QMAKE_COMMAND, CMPL_MAKE_COMMAND, SIQAD_Q_PRO_FILE"
    exit 1
fi

# determine number of physical cores to use for compilation
cpu_cores=$(grep "^core id" /proc/cpuinfo | sort -u | wc -l)

# compile
${CMPL_QMAKE_COMMAND} ${SIQAD_Q_PRO_FILE}
${CMPL_MAKE_COMMAND} -j ${cpu_cores}
