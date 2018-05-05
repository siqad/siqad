#####################################################################
# Customized qmake compilation settings
# Last modified: 2017.05.08
# Author: Jake Retallick
#####################################################################

#CONFIG -= debug	# uncomment to exclude debugging symbols

# support of c++ range-based loops
CONFIG += c++11
CONFIG += debug
#CONFIG += release

QT += core gui widgets svg printsupport uitools

greaterThan(QT_MAJOR_VERSION, 5): QT += widgets

TEMPLATE = app
TARGET = db-sim
INCLUDEPATH += .

###########################
# APPLICATION INFORMATION #
###########################

VERSION = 0.0.1
DEFINES += APP_VERSION=\\\"0.0.1\\\"
DEFINES += APPLICATION_NAME=\\\"SiQAD\\\"
DEFINES += ORGANIZATION_NAME=\\\"WalusLab\\\"

QMAKE_TARGET_COMPANY = "WalusLab"
QMAKE_TARGET_PRODUCT = "SiQAD"
QMAKE_TARGET_DESCRIPTION = "A CAD tool that enables the creation and simulation of quantum dot networks"
QMAKE_TARGET_COPYRIGHT = "GPLv3"

RESOURCES = resources/application.qrc
win32:RC_ICONS = resources/ico/app.ico
macx:ICON = resources/ico/app.icns


#################################
# INPUT GUI HEADERS AND SOURCES #
#################################

SOURCES += src/main.cc

# settings

HEADERS += \
	src/settings/settings.h \
	src/settings/settings_dialog.h

SOURCES += \
	src/settings/settings.cc \
	src/settings/settings_dialog.cc

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
	src/gui/widgets/primitives/lattice.h \
	src/gui/widgets/primitives/sim_engine.h \
	src/gui/widgets/primitives/sim_job.h \
	src/gui/widgets/primitives/electrode.h \
  src/gui/widgets/primitives/afmarea.h \
  src/gui/widgets/primitives/afmpath.h \
  src/gui/widgets/primitives/afmnode.h \
  src/gui/widgets/primitives/afmseg.h \
  src/gui/widgets/primitives/pot_plot.h \
	src/gui/widgets/primitives/resize_frame.h


SOURCES += \
	src/gui/widgets/primitives/emitter.cc \
	src/gui/widgets/primitives/item.cc \
	src/gui/widgets/primitives/aggregate.cc \
	src/gui/widgets/primitives/dbdot.cc \
	src/gui/widgets/primitives/latdot.cc \
	src/gui/widgets/primitives/ghost.cc \
	src/gui/widgets/primitives/layer.cc \
	src/gui/widgets/primitives/lattice.cc \
	src/gui/widgets/primitives/sim_engine.cc \
	src/gui/widgets/primitives/sim_job.cc \
	src/gui/widgets/primitives/electrode.cc \
	src/gui/widgets/primitives/afmarea.cc \
	src/gui/widgets/primitives/afmpath.cc \
	src/gui/widgets/primitives/afmnode.cc \
	src/gui/widgets/primitives/afmseg.cc \
  src/gui/widgets/primitives/pot_plot.cc \
	src/gui/widgets/primitives/resize_frame.cc


# widgets

HEADERS += \
	src/gui/application.h \
	src/gui/property_map.h \
	src/gui/widgets/property_editor.h \
	src/gui/widgets/property_form.h \
	src/gui/widgets/design_panel.h \
	src/gui/widgets/dialog_panel.h \
	src/gui/widgets/input_field.h \
	src/gui/widgets/info_panel.h \
	src/qcustomplot.h \
  src/gui/widgets/layer_editor.h \
  src/gui/widgets/sim_manager.h \
  src/gui/widgets/sim_visualize_panel.h \
  src/gui/widgets/afm_panel.h


SOURCES += \
	src/gui/application.cc \
	src/gui/property_map.cc \
	src/gui/widgets/property_editor.cc \
	src/gui/widgets/property_form.cc \
	src/gui/widgets/design_panel.cc \
	src/gui/widgets/dialog_panel.cc \
	src/gui/widgets/input_field.cc \
	src/gui/widgets/info_panel.cc \
	src/qcustomplot.cpp \
  src/gui/widgets/layer_editor.cc \
  src/gui/widgets/sim_manager.cc \
  src/gui/widgets/sim_visualize_panel.cc \
  src/gui/widgets/afm_panel.cc


#####################
# BUILD DIRECTORIES #
#####################

release:	DESTDIR = build/release
debug:		DESTDIR = build/debug

OBJECTS_DIR	= $$DESTDIR/.obj
MOC_DIR		= $$DESTDIR/.moc
RCC_DIR		= $$DESTDIR/.qrc
UI_DIR		= $$DESTDIR/.ui

##############
# COPY FILES #
##############

# official physics engines

# directory holding the binary, $$OUT_PWD makes it relative to the
# directory containing the Makefile
EXEC_DIR = $$OUT_PWD/$$DESTDIR
# directory holding the physics engines, relative to db-sim.pro
PHYS_DIR = src/phys

macx:   EXEC_DIR = $${DESTDIR}/$${TARGET}.app/Contents/MacOS


sim_common.path = $$EXEC_DIR/src/phys
sim_common.files = \
    $$PHYS_DIR/installing-new-engines.txt
INSTALLS += sim_common

# SimAnneal

sim_simanneal.path = $$EXEC_DIR/src/phys/simanneal
sim_simanneal.files = \
    $$PHYS_DIR/simanneal/sim_anneal.physeng
linux: sim_simanneal.files += $$PHYS_DIR/simanneal/simanneal
macx:  sim_simanneal.files += $$PHYS_DIR/simanneal/simanneal
win32: sim_simanneal.files += $$PHYS_DIR/simanneal/simanneal.exe
INSTALLS += sim_simanneal

# AFMMarcus

sim_afmmarcus.path = $$EXEC_DIR/src/phys/afmmarcus
sim_afmmarcus.files = \
    $$PHYS_DIR/afmmarcus/afm_line_scan.physeng \
    $$PHYS_DIR/afmmarcus/hopping_animator.physeng \
    $$PHYS_DIR/afmmarcus/marcus_and_pois.physeng \
win32:  sim_afmmarcus.files += \
    $$PHYS_DIR/afmmarcus/src/afmmarcus.exe
!win32: sim_afmmarcus.files += \
    $$PHYS_DIR/afmmarcus/src/afmmarcus \
    $$PHYS_DIR/afmmarcus/src/python/afm.py \
    $$PHYS_DIR/afmmarcus/src/python/animator.py \
    $$PHYS_DIR/afmmarcus/src/python/channel.py \
    $$PHYS_DIR/afmmarcus/src/python/clocking.py \
    $$PHYS_DIR/afmmarcus/src/python/db-sim-connector.py \
    $$PHYS_DIR/afmmarcus/src/python/hopper.py \
    $$PHYS_DIR/afmmarcus/src/python/lineview.py \
    $$PHYS_DIR/afmmarcus/src/python/marcus.py \
    $$PHYS_DIR/afmmarcus/src/python/model.py \
    $$PHYS_DIR/afmmarcus/src/python/tip_model.py
sim_afmmarcus_stylesheets.path = $$EXEC_DIR/src/phys/afmmarcus/stylesheets
sim_afmmarcus_stylesheets.files = \
    $$PHYS_DIR/afmmarcus/src/python/stylesheets/animator.qss
sim_afmmarcus_data.path = $$EXEC_DIR/src/phys/afmmarcus/data
sim_afmmarcus_data.files = \
		$$PHYS_DIR/afmmarcus/src/python/data/TIBB_vs_H.dat \
		$$PHYS_DIR/afmmarcus/src/python/data/TIBB_vs_R_d200pm.dat \
		$$PHYS_DIR/afmmarcus/src/python/data/tip_DB_system.py
INSTALLS += sim_afmmarcus
INSTALLS += sim_afmmarcus_stylesheets
INSTALLS += sim_afmmarcus_data

# PoisSolver

sim_poissolver.path = $$EXEC_DIR/src/phys/poissolver
sim_poissolver.files = $$PHYS_DIR/poissolver/poissolver.physeng
linux: sim_poissolver.files += $$PHYS_DIR/poissolver/PoisFFT/bin/objs/poissolver
INSTALLS += sim_poissolver
