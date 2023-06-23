#---------------------------------*-CMake-*----------------------------------#
# Copyright 2020-2022 UT-Battelle, LLC, and other Celeritas developers.
# See the top-level COPYRIGHT file for details.
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
#----------------------------------------------------------------------------#

cmake_minimum_required(VERSION 3.12)

project(CMSSW LANGUAGES CXX)
cmake_policy(VERSION 3.12...3.22)

find_package(Geant4 REQUIRED)

file(GLOB_RECURSE source "*.cc")
add_library(cmssw ${source})

target_include_directories(cmssw PUBLIC "${PROJECT_SOURCE_DIR}")
target_link_libraries(cmssw PUBLIC ${Geant4_LIBRARIES})

#----------------------------------------------------------------------------#
