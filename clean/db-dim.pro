#####################################################################
# Customized qmake compilation settings
# Last modified: 2017.05.08
# Author: Jake Retallick
#####################################################################

# CONFIG -= debug	# uncomment to exclude debugging symbols

QT += core gui

greaterThan(QT_MAJOR_VERSION, 5): QT += widgets

TEMPLATE = app
TARGET = db-sim
INCLUDEPATH += .

RESOURCES = resources/application.qrc

# INPUT GUI HEADERS AND SOURCES
HEADERS += \
	src/gui/application.h

SOURCES += \
	src/main.cc \
	src/gui/application.cc


#####################
# BUILD DIRECTORIES #
#####################

release:	DESTDIR = build/release
debug:		DESTDIR = build/debug

OBJEECTS_DIR	= $$DESTDIR/.obj
MOC_DIR		= $$DESTDIR/.moc
RCC_DIR		= $$DESTDIR/.qrc
UI_DIR		= $$DESTDIR/.ui
