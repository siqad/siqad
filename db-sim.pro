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
TARGET = siqad
INCLUDEPATH += .

###########################
# APPLICATION INFORMATION #
###########################

VERSION = 0.1.0
DEFINES += APP_VERSION=\\\"0.1.0\\\"
DEFINES += APPLICATION_NAME=\\\"SiQAD\\\"
DEFINES += ORGANIZATION_NAME=\\\"WalusLab\\\"

QMAKE_TARGET_COMPANY = "WalusLab"
QMAKE_TARGET_PRODUCT = "SiQAD"
QMAKE_TARGET_DESCRIPTION = "Silicon Quantum Atomic Designer"
QMAKE_TARGET_COPYRIGHT = "GPLv3"

RESOURCES = resources/application.qrc
win32:RC_ICONS = resources/ico/app.ico
macx:ICON = resources/ico/app.icns


#################################
# INPUT GUI HEADERS AND SOURCES #
#################################

SOURCES += src/main.cc

# global

HEADERS += \
    src/global.h

SOURCES += \
    src/global.cc

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
   	src/gui/widgets/primitives/ghost.h \
   	src/gui/widgets/primitives/items.h \
   	src/gui/widgets/primitives/layer.h \
   	src/gui/widgets/primitives/lattice.h \
   	src/gui/widgets/primitives/sim_engine.h \
   	src/gui/widgets/primitives/sim_job.h \
   	src/gui/widgets/primitives/electrode.h \
   	src/gui/widgets/primitives/electrode_poly.h \
    src/gui/widgets/primitives/afmarea.h \
    src/gui/widgets/primitives/afmpath.h \
    src/gui/widgets/primitives/afmnode.h \
    src/gui/widgets/primitives/afmseg.h \
    src/gui/widgets/primitives/pot_plot.h \
    src/gui/widgets/primitives/resizablerect.h \
    src/gui/widgets/primitives/resizerotaterect.h \
   	src/gui/widgets/primitives/resizable_poly.h \
   	src/gui/widgets/primitives/polygon_handle.h \
   	src/gui/widgets/primitives/polygon_segment.h \
   	src/gui/widgets/primitives/hull/hull.h \
   	src/gui/widgets/primitives/hull/convex.h \
   	src/gui/widgets/primitives/hull/hulls.h \
    src/gui/widgets/primitives/labels/labelgroup.h \
    src/gui/widgets/primitives/labels/textlabel.h \
    src/gui/widgets/primitives/visualaids/screenshot_clip_area.h \
    src/gui/widgets/primitives/visualaids/scale_bar.h



SOURCES += \
    src/gui/widgets/primitives/emitter.cc \
   	src/gui/widgets/primitives/item.cc \
   	src/gui/widgets/primitives/aggregate.cc \
   	src/gui/widgets/primitives/dbdot.cc \
   	src/gui/widgets/primitives/ghost.cc \
   	src/gui/widgets/primitives/layer.cc \
   	src/gui/widgets/primitives/lattice.cc \
   	src/gui/widgets/primitives/sim_engine.cc \
   	src/gui/widgets/primitives/sim_job.cc \
   	src/gui/widgets/primitives/electrode.cc \
   	src/gui/widgets/primitives/electrode_poly.cc \
   	src/gui/widgets/primitives/afmarea.cc \
   	src/gui/widgets/primitives/afmpath.cc \
   	src/gui/widgets/primitives/afmnode.cc \
   	src/gui/widgets/primitives/afmseg.cc \
    src/gui/widgets/primitives/pot_plot.cc \
    src/gui/widgets/primitives/resizablerect.cc \
    src/gui/widgets/primitives/resizerotaterect.cc \
   	src/gui/widgets/primitives/resizable_poly.cc \
   	src/gui/widgets/primitives/polygon_handle.cc \
   	src/gui/widgets/primitives/polygon_segment.cc \
   	src/gui/widgets/primitives/hull/hull.cc \
   	src/gui/widgets/primitives/hull/convex.cc \
    src/gui/widgets/primitives/labels/labelgroup.cc \
    src/gui/widgets/primitives/labels/textlabel.cc \
    src/gui/widgets/primitives/visualaids/screenshot_clip_area.cc \
    src/gui/widgets/primitives/visualaids/scale_bar.cc


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
    src/gui/widgets/layer_manager.h \
    src/gui/widgets/sim_manager.h \
    src/gui/widgets/sim_visualize_panel.h \
    src/gui/widgets/afm_panel.h \
  	src/gui/commander.h \
  	src/gui/widgets/completer.h \
  	src/gui/widgets/item_manager.h \
  	src/gui/widgets/electrode_poly_helper.h \
    src/gui/widgets/screenshot_manager.h



SOURCES += \
    src/gui/application.cc \
   	src/gui/property_map.cc \
   	src/gui/widgets/property_editor.cc \
   	src/gui/widgets/property_form.cc \
   	src/gui/widgets/design_panel.cc \
   	src/gui/widgets/dialog_panel.cc \
   	src/gui/widgets/input_field.cc \
   	src/gui/widgets/info_panel.cc \
    src/gui/widgets/layer_manager.cc \
    src/gui/widgets/sim_manager.cc \
    src/gui/widgets/sim_visualize_panel.cc \
   	src/gui/widgets/afm_panel.cc \
   	src/gui/commander.cc \
   	src/gui/widgets/completer.cc \
   	src/gui/widgets/item_manager.cc \
   	src/gui/widgets/electrode_poly_helper.cc \
    src/gui/widgets/screenshot_manager.cc



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
    $$PHYS_DIR/installing-new-engines.txt \
    $$PHYS_DIR/is_python3.py
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
sim_afmmarcus.files += \
    $$PHYS_DIR/afmmarcus/src/swig_siqadconn/_siqadconn.cpython-36m-x86_64-linux-gnu.so
win32: sim_afmmarcus.files += \
    $$PHYS_DIR/afmmarcus/src/_siqadconn.pyd
sim_afmmarcus.files += \
    $$PHYS_DIR/afmmarcus/src/afm.py \
    $$PHYS_DIR/afmmarcus/src/animator.py \
    $$PHYS_DIR/afmmarcus/src/channel.py \
    $$PHYS_DIR/afmmarcus/src/clocking.py \
    $$PHYS_DIR/afmmarcus/src/db-sim-connector.py \
    $$PHYS_DIR/afmmarcus/src/hopper.py \
    $$PHYS_DIR/afmmarcus/src/lineview.py \
    $$PHYS_DIR/afmmarcus/src/marcus.py \
    $$PHYS_DIR/afmmarcus/src/model.py \
    $$PHYS_DIR/afmmarcus/src/tip_model.py \
    $$PHYS_DIR/afmmarcus/src/qt_import.py \
    $$PHYS_DIR/afmmarcus/src/siqadconn.py
sim_afmmarcus_stylesheets.path = $$EXEC_DIR/src/phys/afmmarcus/stylesheets
sim_afmmarcus_stylesheets.files = \
    $$PHYS_DIR/afmmarcus/src/stylesheets/animator.qss
sim_afmmarcus_data.path = $$EXEC_DIR/src/phys/afmmarcus/data
sim_afmmarcus_data.files = \
		$$PHYS_DIR/afmmarcus/src/data/TIBB_vs_H.dat \
		$$PHYS_DIR/afmmarcus/src/data/TIBB_vs_R_d200pm.dat \
		$$PHYS_DIR/afmmarcus/src/data/tip_DB_system.py
INSTALLS += sim_afmmarcus
INSTALLS += sim_afmmarcus_stylesheets
INSTALLS += sim_afmmarcus_data

# PoisSolver

sim_poissolver.path = $$EXEC_DIR/src/phys/poissolver
sim_poissolver.files = $$PHYS_DIR/poissolver/poissolver.physeng
linux: sim_poissolver.files += \
		$$PHYS_DIR/poissolver/FEM/src/poissolver \
		$$PHYS_DIR/poissolver/FEM/src/python/poisson3D.py \
		$$PHYS_DIR/poissolver/FEM/src/python/poisson_class.py \
		$$PHYS_DIR/poissolver/FEM/src/python/subdomains.py \
		$$PHYS_DIR/poissolver/FEM/src/python/helpers/mesh_writer_3D.py \
		$$PHYS_DIR/poissolver/FEM/src/python/helpers/helpers.py \
		$$PHYS_DIR/poissolver/FEM/src/python/dolfin_convert.py \
		$$PHYS_DIR/poissolver/FEM/src/python/siqadconn.py \
		$$PHYS_DIR/poissolver/FEM/src/python/_siqadconn.cpython-36m-x86_64-linux-gnu.so
INSTALLS += sim_poissolver
