###############################################################################
# qmake compilation settings, intended for cross-compilation for Windows.
# Use CMake for other OSs.
# Last modified: 2020-01-13
# Author: Jake Retallick
###############################################################################

CONFIG += qt c++11
CONFIG += release

QT += core gui widgets svg printsupport uitools charts

TEMPLATE = app
TARGET = siqad
INCLUDEPATH += .

###########################
# APPLICATION INFORMATION #
###########################

# TODO read all these from environment
VERSION = 0.2.2
DEFINES += APP_VERSION=\\\"0.2.2\\\"
DEFINES += APPLICATION_NAME=\\\"SiQAD\\\"
DEFINES += ORGANIZATION_NAME=\\\"WalusLab\\\"

QMAKE_TARGET_COMPANY = "WalusLab"
QMAKE_TARGET_PRODUCT = "SiQAD"
QMAKE_TARGET_DESCRIPTION = "Silicon Quantum Atomic Designer"
QMAKE_TARGET_COPYRIGHT = "GPLv3"

RESOURCES = resources/application.qrc
win32:RC_ICONS = resources/ico/siqad.ico
# macx:ICON = resources/ico/siqad.icns


#################################
# INPUT GUI HEADERS AND SOURCES #
#################################

for(F, $$list($$cat(./source_files))) {
    SOURCES += "$$F"
}

for(F, $$list($$cat(./header_files))) {
    HEADERS += "$$F"
}


#####################
# BUILD DIRECTORIES #
#####################

OBJECTS_DIR	= .obj
MOC_DIR		= .moc
RCC_DIR		= .qrc
UI_DIR		= .ui
