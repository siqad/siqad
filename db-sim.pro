#####################################################################
# Customized qmake compilation settings
# Last modified: 2017.05.08
# Author: Jake Retallick
#####################################################################

# CONFIG -= debug	# uncomment to exclude debugging symbols

# support of c++ range-based loops
CONFIG += c++11
CONFIG += debug

QT += core gui widgets

greaterThan(QT_MAJOR_VERSION, 5): QT += widgets

TEMPLATE = app
TARGET = db-sim
INCLUDEPATH += .

RESOURCES = resources/application.qrc

#################################
# INPUT GUI HEADERS AND SOURCES #
#################################

SOURCES += src/main.cc

# settings

HEADERS += src/settings/settings.h
SOURCES += src/settings/settings.cc

# primitives for widgets

HEADERS += \
	src/gui/widgets/primitives/emitter.h \
	src/gui/widgets/primitives/item.h \
	src/gui/widgets/primitives/aggregate.h \
	src/gui/widgets/primitives/dbdot.h \
	src/gui/widgets/primitives/latdot.h \
	src/gui/widgets/primitives/ghost.h \
	src/gui/widgets/primitives/items.h \
	src/gui/widgets/primitives/layer.h \
	src/gui/widgets/primitives/lattice.h

SOURCES += \
	src/gui/widgets/primitives/emitter.cc \
	src/gui/widgets/primitives/item.cc \
	src/gui/widgets/primitives/aggregate.cc \
	src/gui/widgets/primitives/dbdot.cc \
	src/gui/widgets/primitives/latdot.cc \
	src/gui/widgets/primitives/ghost.cc \
	src/gui/widgets/primitives/layer.cc \
	src/gui/widgets/primitives/lattice.cc

# widgets

HEADERS += \
	src/gui/application.h \
	src/gui/widgets/design_panel.h \
	src/gui/widgets/dialog_panel.h \
	src/gui/widgets/input_field.h \
	src/gui/widgets/info_panel.h

SOURCES += \
	src/gui/application.cc \
	src/gui/widgets/design_panel.cc \
	src/gui/widgets/dialog_panel.cc \
	src/gui/widgets/input_field.cc \
	src/gui/widgets/info_panel.cc



#####################
# BUILD DIRECTORIES #
#####################

release:	DESTDIR = build/release
debug:		DESTDIR = build/debug

OBJEECTS_DIR	= $$DESTDIR/.obj
MOC_DIR		= $$DESTDIR/.moc
RCC_DIR		= $$DESTDIR/.qrc
UI_DIR		= $$DESTDIR/.ui
