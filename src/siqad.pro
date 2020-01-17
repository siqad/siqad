###############################################################################
# qmake compilation settings, intended for cross-compilation for Windows.
# Use CMake for other OSs.
# Last modified: 2020-01-13
# Author: Jake Retallick
###############################################################################

CONFIG += qt c++11
CONFIG += release

QT += core gui widgets svg printsupport uitools charts

#greaterThan(QT_MAJOR_VERSION, 5): QT += widgets

TEMPLATE = app
TARGET = siqad
INCLUDEPATH += .

###########################
# APPLICATION INFORMATION #
###########################

# TODO read all these from environment
VERSION = 0.1.0
DEFINES += APP_VERSION=\\\"0.1.0\\\"
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

#SOURCES += src/main.cc
#
## global
#
#HEADERS += \
#    src/global.h
#
#SOURCES += \
#    src/global.cc
#
## settings
#
#HEADERS += \
#	src/settings/settings.h \
#	src/settings/settings_dialog.h
#
#SOURCES += \
#	src/settings/settings.cc \
#	src/settings/settings_dialog.cc
#
## primitives for widgets
#
#HEADERS += \
#    src/gui/widgets/primitives/emitter.h \
#   	src/gui/widgets/primitives/item.h \
#   	src/gui/widgets/primitives/aggregate.h \
#   	src/gui/widgets/primitives/dbdot.h \
#   	src/gui/widgets/primitives/ghost.h \
#   	src/gui/widgets/primitives/items.h \
#   	src/gui/widgets/primitives/layer.h \
#   	src/gui/widgets/primitives/lattice.h \
#   	src/gui/widgets/primitives/electrode.h \
#    src/gui/widgets/primitives/afmarea.h \
#    src/gui/widgets/primitives/afmpath.h \
#    src/gui/widgets/primitives/afmnode.h \
#    src/gui/widgets/primitives/afmseg.h \
#    src/gui/widgets/primitives/pot_plot.h \
#    src/gui/widgets/primitives/resizablerect.h \
#    src/gui/widgets/primitives/resizerotaterect.h \
#   	src/gui/widgets/primitives/hull/hull.h \
#   	src/gui/widgets/primitives/hull/convex.h \
#   	src/gui/widgets/primitives/hull/hulls.h \
#    src/gui/widgets/primitives/labels/labelgroup.h \
#    src/gui/widgets/primitives/labels/textlabel.h \
#    src/gui/widgets/primitives/visual_aids/screenshot_clip_area.h \
#    src/gui/widgets/primitives/visual_aids/scale_bar.h
#
#
#
#SOURCES += \
#    src/gui/widgets/primitives/emitter.cc \
#   	src/gui/widgets/primitives/item.cc \
#   	src/gui/widgets/primitives/aggregate.cc \
#   	src/gui/widgets/primitives/dbdot.cc \
#   	src/gui/widgets/primitives/ghost.cc \
#   	src/gui/widgets/primitives/layer.cc \
#   	src/gui/widgets/primitives/lattice.cc \
#   	src/gui/widgets/primitives/electrode.cc \
#   	src/gui/widgets/primitives/afmarea.cc \
#   	src/gui/widgets/primitives/afmpath.cc \
#   	src/gui/widgets/primitives/afmnode.cc \
#   	src/gui/widgets/primitives/afmseg.cc \
#    src/gui/widgets/primitives/pot_plot.cc \
#    src/gui/widgets/primitives/resizablerect.cc \
#    src/gui/widgets/primitives/resizerotaterect.cc \
#   	src/gui/widgets/primitives/hull/hull.cc \
#   	src/gui/widgets/primitives/hull/convex.cc \
#    src/gui/widgets/primitives/labels/labelgroup.cc \
#    src/gui/widgets/primitives/labels/textlabel.cc \
#    src/gui/widgets/primitives/visual_aids/screenshot_clip_area.cc \
#    src/gui/widgets/primitives/visual_aids/scale_bar.cc
#
## components for widgets
#
#HEADERS += \
#   	src/gui/widgets/components/plugin_engine.h \
#   	src/gui/widgets/components/sim_engine.h \
#   	src/gui/widgets/components/sim_job.h \
#    src/gui/widgets/components/job_results/job_result_types.h \
#    src/gui/widgets/components/job_results/job_result.h \
#    src/gui/widgets/components/job_results/db_locations.h \
#    src/gui/widgets/components/job_results/electron_config_set.h \
#    src/gui/widgets/components/job_results/potential_landscape.h \
#    src/gui/widgets/components/job_results/sqcommands.h
#
#SOURCES += \
#   	src/gui/widgets/components/plugin_engine.cc \
#   	src/gui/widgets/components/sim_engine.cc \
#   	src/gui/widgets/components/sim_job.cc \
#    src/gui/widgets/components/job_results/job_result.cc \
#    src/gui/widgets/components/job_results/db_locations.cc \
#    src/gui/widgets/components/job_results/electron_config_set.cc \
#    src/gui/widgets/components/job_results/potential_landscape.cc \
#    src/gui/widgets/components/job_results/sqcommands.cc
#
## widgets
#
#HEADERS += \
#    src/gui/application.h \
#  	src/gui/commander.h \
#   	src/gui/property_map.h \
#   	src/gui/widgets/property_editor.h \
#   	src/gui/widgets/property_form.h \
#   	src/gui/widgets/design_panel.h \
#   	src/gui/widgets/dialog_panel.h \
#   	src/gui/widgets/input_field.h \
#   	src/gui/widgets/info_panel.h \
#    src/gui/widgets/afm_panel.h \
#    src/gui/widgets/color_dialog.h \
#    src/gui/widgets/rotate_dialog.h \
#  	src/gui/widgets/completer.h \
#  	src/gui/widgets/managers/item_manager.h \
#    src/gui/widgets/managers/layer_manager.h \
#    src/gui/widgets/managers/plugin_manager.h \
#    src/gui/widgets/managers/job_manager.h \
#    src/gui/widgets/managers/sim_manager.h \
#    src/gui/widgets/managers/screenshot_manager.h \
#    src/gui/widgets/visualizers/sim_visualizer.h \
#    src/gui/widgets/visualizers/electron_config_set_visualizer.h \
#    src/gui/widgets/visualizers/potential_landscape_visualizer.h
#
#
#
#SOURCES += \
#    src/gui/application.cc \
#   	src/gui/commander.cc \
#   	src/gui/property_map.cc \
#   	src/gui/widgets/property_editor.cc \
#   	src/gui/widgets/property_form.cc \
#   	src/gui/widgets/design_panel.cc \
#   	src/gui/widgets/dialog_panel.cc \
#   	src/gui/widgets/input_field.cc \
#   	src/gui/widgets/info_panel.cc \
#    src/gui/widgets/afm_panel.cc \
#    src/gui/widgets/color_dialog.cc \
#    src/gui/widgets/rotate_dialog.cc \
#   	src/gui/widgets/completer.cc \
#   	src/gui/widgets/managers/item_manager.cc \
#    src/gui/widgets/managers/layer_manager.cc \
#    src/gui/widgets/managers/plugin_manager.cc \
#    src/gui/widgets/managers/job_manager.cc \
#    src/gui/widgets/managers/sim_manager.cc \
#    src/gui/widgets/managers/screenshot_manager.cc \
#    src/gui/widgets/visualizers/sim_visualizer.cc \
#    src/gui/widgets/visualizers/electron_config_set_visualizer.cc \
#    src/gui/widgets/visualizers/potential_landscape_visualizer.cc



#####################
# BUILD DIRECTORIES #
#####################

#DESTDIR = build
#isEmpty($${SIQAD_INSTALL_ROOT}) {
#    
#} else {
#    DESTDIR = $${SIQAD_INSTALL_ROOT}
#}

#OBJECTS_DIR	= $$DESTDIR/.obj
#MOC_DIR		= $$DESTDIR/.moc
#RCC_DIR		= $$DESTDIR/.qrc
#UI_DIR		= $$DESTDIR/.ui
OBJECTS_DIR	= .obj
MOC_DIR		= .moc
RCC_DIR		= .qrc
UI_DIR		= .ui

###############
## COPY FILES #
###############
#
## official physics engines
#
## directory holding the binary, $$OUT_PWD makes it relative to the
## directory containing the Makefile
#EXEC_DIR = $$OUT_PWD/$$DESTDIR
## directory holding the physics engines, relative to db-sim.pro
#PHYS_DIR = src/phys
## directory holding plugins
#PLUGIN_DIR = src/plugins
#
#macx:   EXEC_DIR = $${DESTDIR}/$${TARGET}.app/Contents/MacOS
#
#
#install_common.path = $$EXEC_DIR
#install_common.files = LICENSE
#INSTALLS += install_common
#
#sim_common.path = $$EXEC_DIR/src/phys
#sim_common.files = \
#    $$PHYS_DIR/installing-new-engines.txt \
#    $$PHYS_DIR/is_python3.py
#INSTALLS += sim_common
#
## SimAnneal
#
#sim_simanneal.path = $$EXEC_DIR/src/phys/simanneal
#sim_simanneal.files = \
#    $$PHYS_DIR/simanneal/sim_anneal.physeng \
#    $$PHYS_DIR/simanneal/LICENSE
#linux: sim_simanneal.files += $$PHYS_DIR/simanneal/simanneal
#macx:  sim_simanneal.files += $$PHYS_DIR/simanneal/simanneal
#win32: sim_simanneal.files += $$PHYS_DIR/simanneal/simanneal.exe
#INSTALLS += sim_simanneal
#
## QPUAnneal
#
#sim_qpuanneal.path = $$EXEC_DIR/src/phys/qpuanneal
#sim_qpuanneal.files = \
#    $$PHYS_DIR/qpuanneal/qpu_anneal.physeng \
#    $$PHYS_DIR/qpuanneal/qpu_anneal_qbsolv.physeng \
#    $$PHYS_DIR/qpuanneal/src/groundstate-dwave.py \
#    $$PHYS_DIR/qpuanneal/src/siqadconn.py \
#    $$PHYS_DIR/qpuanneal/src/_siqadconn*.so \
#    $$PHYS_DIR/qpuanneal/src/_siqadconn*.pyd \
#    $$PHYS_DIR/qpuanneal/LICENSE
#INSTALLS += sim_qpuanneal
#
## ExhaustiveGS
#
#sim_exhaustive_gs.path = $$EXEC_DIR/src/phys/exhaustive_gs
#sim_exhaustive_gs.files = \
#    $$PHYS_DIR/exhaustive_gs/exhaustive_gs.physeng \
#    $$PHYS_DIR/exhaustive_gs/src/exhaustive_db_ground_state_search.py \
#    $$PHYS_DIR/exhaustive_gs/src/exhaustive_3_states_search.py \
#    $$PHYS_DIR/exhaustive_gs/src/siqadconn.py \
#    $$PHYS_DIR/exhaustive_gs/src/_siqadconn.*.so \
#    $$PHYS_DIR/exhaustive_gs/src/_siqadconn.*.pyd
#    $$PHYS_DIR/exhaustive_gs/LICENSE
#INSTALLS += sim_exhaustive_gs
#
## AFMMarcus
#
#sim_afmmarcus.path = $$EXEC_DIR/src/phys/afmmarcus
#sim_afmmarcus.files = \
#    $$PHYS_DIR/afmmarcus/afm_line_scan.physeng \
#    $$PHYS_DIR/afmmarcus/hopping_animator.physeng \
#    $$PHYS_DIR/afmmarcus/marcus_and_pois.physeng \
#    $$PHYS_DIR/afmmarcus/src/_siqadconn*.so \
#    $$PHYS_DIR/afmmarcus/src/_siqadconn*.pyd \
#    $$PHYS_DIR/afmmarcus/src/siqadconn.py \
#    $$PHYS_DIR/afmmarcus/src/afm.py \
#    $$PHYS_DIR/afmmarcus/src/animator.py \
#    $$PHYS_DIR/afmmarcus/src/channel.py \
#    $$PHYS_DIR/afmmarcus/src/clocking.py \
#    $$PHYS_DIR/afmmarcus/src/energy_tracker.py \
#    $$PHYS_DIR/afmmarcus/src/db-sim-connector.py \
#    $$PHYS_DIR/afmmarcus/src/hopper.py \
#    $$PHYS_DIR/afmmarcus/src/lineview.py \
#    $$PHYS_DIR/afmmarcus/src/marcus.py \
#    $$PHYS_DIR/afmmarcus/src/model.py \
#    $$PHYS_DIR/afmmarcus/src/tip_model.py \
#    $$PHYS_DIR/afmmarcus/src/qt_import.py \
#    $$PHYS_DIR/afmmarcus/LICENSE
#sim_afmmarcus_stylesheets.path = $$EXEC_DIR/src/phys/afmmarcus/stylesheets
#sim_afmmarcus_stylesheets.files = \
#    $$PHYS_DIR/afmmarcus/src/stylesheets/animator.qss
#sim_afmmarcus_data.path = $$EXEC_DIR/src/phys/afmmarcus/data
#sim_afmmarcus_data.files = \
#    $$PHYS_DIR/afmmarcus/src/data/TIBB_vs_H.dat \
#   	$$PHYS_DIR/afmmarcus/src/data/TIBB_vs_R_d200pm.dat \
#   	$$PHYS_DIR/afmmarcus/src/data/tip_DB_system.py
#INSTALLS += sim_afmmarcus
#INSTALLS += sim_afmmarcus_stylesheets
#INSTALLS += sim_afmmarcus_data
#
## PoisSolver
#
#sim_poissolver.path = $$EXEC_DIR/src/phys/poissolver
#sim_poissolver.files = \
#    $$PHYS_DIR/poissolver/poissolver.physeng \
#    $$PHYS_DIR/poissolver/FEM/src/poissolver \
#    $$PHYS_DIR/poissolver/FEM/src/python/poisson3D.py \
#    $$PHYS_DIR/poissolver/FEM/src/python/poisson_class.py \
#    $$PHYS_DIR/poissolver/FEM/src/python/plotter.py \
#    $$PHYS_DIR/poissolver/FEM/src/python/capacitance.py \
#    $$PHYS_DIR/poissolver/FEM/src/python/resistance.py \
#    $$PHYS_DIR/poissolver/FEM/src/python/res_graph.py \
#    $$PHYS_DIR/poissolver/FEM/src/python/mesher.py \
#    $$PHYS_DIR/poissolver/FEM/src/python/dopant.py \
#    $$PHYS_DIR/poissolver/FEM/src/python/ac.py \
#    $$PHYS_DIR/poissolver/FEM/src/python/resistance.py \
#    $$PHYS_DIR/poissolver/FEM/src/python/res_graph.py \
#    $$PHYS_DIR/poissolver/FEM/src/python/charge_density.py \
#    $$PHYS_DIR/poissolver/FEM/src/python/genexp.py \
#    $$PHYS_DIR/poissolver/FEM/src/python/exporter.py \
#    $$PHYS_DIR/poissolver/FEM/src/python/subdomains.py \
#    $$PHYS_DIR/poissolver/FEM/src/python/mesh_writer_3D.py \
#    $$PHYS_DIR/poissolver/FEM/src/python/helpers.py \
#    $$PHYS_DIR/poissolver/FEM/src/python/dolfin_convert.py \
#    $$PHYS_DIR/poissolver/FEM/src/python/siqadconn.py \
#    $$PHYS_DIR/poissolver/FEM/src/python/_siqadconn.*.so \
#    $$PHYS_DIR/poissolver/FEM/src/docker/Dockerfile \
#    $$PHYS_DIR/poissolver/LICENSE
#sim_poissolver_swig.path = $$EXEC_DIR/src/phys/poissolver/swig_siqadconn
#sim_poissolver_swig.files = \
#    $$PHYS_DIR/poissolver/FEM/src/python/swig_siqadconn/setup.py \
#    $$PHYS_DIR/poissolver/FEM/src/python/swig_siqadconn/siqadconn.cc \
#    $$PHYS_DIR/poissolver/FEM/src/python/swig_siqadconn/siqadconn.h \
#    $$PHYS_DIR/poissolver/FEM/src/python/swig_siqadconn/siqadconn.i \
#    $$PHYS_DIR/poissolver/FEM/src/python/swig_siqadconn/swig_generate_and_compile
#INSTALLS += sim_poissolver
#INSTALLS += sim_poissolver_swig
#
## DB pattern recognition
#
#db_pair_recognition.path = $$EXEC_DIR/plugins/db_pattern_recognition
#db_pair_recognition.files = \
#    $$PLUGIN_DIR/db_pattern_recognition/dbp_recognition.sqplug \
#    $$PLUGIN_DIR/db_pattern_recognition/dbp_recognition.py \
#    $$PLUGIN_DIR/db_pattern_recognition/_siqadconn*.so \
#    $$PLUGIN_DIR/db_pattern_recognition/_siqadconn*.pyd \
#    $$PLUGIN_DIR/db_pattern_recognition/siqadconn.py
#INSTALLS += db_pair_recognition
