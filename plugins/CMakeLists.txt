cmake_minimum_required(VERSION 3.10)

#################################
#  Officially included plugins  #
#################################

# Exhaustive GS (implements ground state model of SimAnneal and searches 
# for the ground state charge configuration exhaustively)
add_subdirectory(exhaustive-gs)

# HoppingDynamics
add_subdirectory(afm-sim)

# PoisSolver (currently no Windows support)
if (NOT WIN32 AND NOT SKIP_POISSOLVER)
    add_subdirectory(poissolver)
endif()

# SimAnneal
add_subdirectory(simanneal)

# QuickSim
add_subdirectory(quicksim-siqad-plugin)
