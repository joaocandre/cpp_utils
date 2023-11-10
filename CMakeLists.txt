# Example CMakeLists.txt for a plain cmake project compilable/installable in a catkin workspace
# cf. https://tuebel.github.io/blog/ros/cmake/2020/06/27/ros-plain-cmake.html#installing-the-targets

#########################
## CMake tools/helpers ##
#########################

cmake_policy(SET CMP0007 NEW)      # avoids list/range issue win FindPython module in newer CMake versions
enable_language(CXX)               # to disable warning when including GNUInstallDirs module

include(GNUInstallDirs)  # provides variables for default installation directories e.g. ${CMAKE_INSTALL_LIBDIR} (check below)
include(CMakePackageConfigHelpers) # provides functions for the automatic generation of the CMake configuration files

## Add custom module to facilitate dependency fetching if needed!
## @todo  auto-fetch from git repository?
## Get custom helper modules from remote repository
include(CMakeFetchHelpers OPTIONAL RESULT_VARIABLE CMakeFetchHelpers_FOUND)
if(CMakeFetchHelpers_FOUND)
    message(STATUS "Found CMakeFetchHelpers!")
else()
    include(FetchContent)
    FetchContent_Declare(cmake-helper-modules
        GIT_REPOSITORY  https://github.com/joaocandre/cmake-helper-modules
        GIT_TAG main
    )
    FetchContent_MakeAvailable(cmake-helper-modules)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${cmake-helper-modules_SOURCE_DIR}/cmake")
    include(CMakeFetchHelpers)
endif()


###################
## Global Config ##
###################

cmake_minimum_required(VERSION 3.12)
project(cpp_utils VERSION 1.0 LANGUAGES CXX)
set(${PROJECT_NAME}_VERSION 1.0)

# set the path where CMake package configuration files (*-config.cmake) will be installed
# when finding a package, CMake searches by default lib/<package name>/cmake (among others)
set(ConfigPackageLocation ${CMAKE_INSTALL_LIBDIR}/${CMAKE_PROJECT_NAME}/cmake)

# Doxygen descriptors (requires matching Doxyfile.in)
# set(CMAKE_PROJECT_DESCRIPTION "C++ library implementing generic motion representation dynamics.")  # for CMake<3.9
set(CMAKE_PROJECT_BRIEF "C++ header-only library providing generic extensions to STL containers.")

## Compile using C++17 standard
# set(CMAKE_CXX_STANDARD 17)
add_compile_options(-std=c++17)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
# GCC or Clang compiler flags (verbose warnings)
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # add_compile_options(-Wall -Wextra -Wpedantic)
  add_compile_options(-O3)  # optimization default for all build types!
endif()

## option (interactive) to enable/disable doc generation
option(${PROJECT_NAME}_GEN_DOC "Generate documentation (doxygen)" OFF)

## option (interactive) to enable/disable third-party wrappers
option(${PROJECT_NAME}_ENABLE_PLOT_UTILS "High-evel utility wrappers around matplotlib-cpp" ON)
option(${PROJECT_NAME}_ENABLE_XML_UTILS  "High-evel utility wrappers around TinyXML2" ON)

## set default build type to Release
## only valid when null string (first call)
## in order to change betwenn build types, add flag explicitely i.e. -DCMAKE_BUILD_TYPE=
## or remove CMakeCache from project root

if (${CMAKE_BUILD_TYPE})
    message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
endif()

##################
## Dependencies ##
##################

## Boost
find_package(Boost REQUIRED COMPONENTS system)

if("${Boost_FIND_VERSION}" VERSION_LESS "1.66.1")
    add_definitions(-DBOOST_LEGACY_VERSION)
    # add_compile_definitions(BOOST_LEGACY_VERSION)
endif()

## Doxygen
find_package(Doxygen)

if (${PROJECT_NAME}_ENABLE_PLOT_UTILS)
    set (matplotlib_cpp_BUILD_EXAMPLES OFF CACHE INTERNAL "Turn off examples")
    # search_for_package(matplotlib_cpp git@github.com:lava/matplotlib-cpp master)
    search_for_package(matplotlib_cpp https://github.com/joaocandre/matplotlib-cpp master)  # @todo revert to upstream repo if PR is accepted
endif()

if (${PROJECT_NAME}_ENABLE_XML_UTILS)
    set (tinyxml2_BUILD_TESTING OFF CACHE INTERNAL "Turn off tests")
    set (tinyxml2_SHARED_LIBS ON CACHE INTERNAL "Enable shared library compilation")
    search_for_package(tinyxml2 https://github.com/leethomason/TinyXML2 9.0.0)
endif()

###########
## Build ##
###########

## declare header-only library
## cf. https://www.mariobadr.com/creating-a-header-only-library-with-cmake.html
##     https://stackoverflow.com/a/49143782/2340002

# declare library as interface
add_library(${PROJECT_NAME} INTERFACE)

# target header locations
# @note CMake documentation states: “Include directories usage requirements commonly differ between the build-tree and the install-tree.”
target_include_directories(${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                                                     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/storage>
                                                     $<INSTALL_INTERFACE:include>)

# file(GLOB BUILD_INTERFACE_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/include/*/*.hpp)
# file(GLOB INSTALL_INTERFACE_SRCS include/*/*.hpp)
# target_sources(${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:${BUILD_INTERFACE_SRCS}>
#                                          $<INSTALL_INTERFACE:${INSTALL_INTERFACE_SRCS}>)

# @note INTERFACE target, linking dependencies only adds include paths
if (${PROJECT_NAME}_ENABLE_PLOT_UTILS)
    target_link_libraries(${PROJECT_NAME} INTERFACE ${matplotlib_cpp_TARGET})
endif()

if (${PROJECT_NAME}_ENABLE_XML_UTILS)
    target_link_libraries(${PROJECT_NAME} INTERFACE ${tinyxml2_TARGET})
endif()

########### API documentation

if(${DOXYGEN_FOUND} AND ${PROJECT_NAME}_GEN_DOC)
    # cf. https://vicrucann.github.io/tutorials/quick-cmake-doxygen/

    # set input and output files
    # generates Doxyfile with source and target paths (cf. 'Doxyfile.in')
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile)

    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message(STATUS "Building documentation (doxygen) for target ${PROJECT_NAME}")

    # note the option ALL which allows to build the docs together with the application
    add_custom_target(doc_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )
elseif (NOT ${DOXYGEN_OUT})
    message(STATUS "Doxygen needs to be installed to generate the doxygen documentation")
endif()

#############
## Install ##
#############

# headers
# @todo only add extra/xml.hpp and extra/plotting.hpp if ENABLE_XML_UTILS and ENABLE_PLOTTING_UTILS are enabled
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.hpp")

# export targets
install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}   # GNUInstallDirs helper variables
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}   # GNUInstallDirs helper variables
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})   # GNUInstallDirs helper variables
        # PUBLIC_HEADER DESTINATION include COMPONENT Development)
        # used to generate full to export cpp_utilsTargets (currently only the lib target)

# install dependency onto targets file and to export list *if building missing dependencies*
# propagates requirements to including/dependent projects
# additionaly, removes the need for other dependent projects to link to matplotlib or tinyxml2 directly

# if (${matplotlib_cpp_TARGET}_IMPORTED)
#     message(WARNING "matplotlib is imported")
# endif()
# message(WARNING "${tinyxml2_IMPORTED}")
# if (${tinyxml2_TARGET}_IMPORTED)
#     message(WARNING "TinyXML2 is imported")
# endif()

if (${PROJECT_NAME}_ENABLE_PLOT_UTILS AND NOT ${matplotlib_cpp_IMPORTED})
    install(TARGETS ${matplotlib_cpp_TARGET} EXPORT ${PROJECT_NAME}Targets)
endif()

if (${PROJECT_NAME}_ENABLE_XML_UTILS AND NOT ${tinyxml2_IMPORTED})
    install(TARGETS ${tinyxml2_TARGET} EXPORT ${PROJECT_NAME}Targets)
endif()

############
## Export ##
############

# section extracted from:
# https://tuebel.github.io/blog/ros/cmake/2020/06/27/ros-plain-cmake.html#installing-the-targets

# generate .cmake target configuration file
# for build tree / in workspace
export(EXPORT ${PROJECT_NAME}Targets
       # TARGETS ${PROJECT_NAME}  # alternatively
       FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake
       NAMESPACE ${PROJECT_NAME}::)

# @note: the export command makes the targets available in the build tree only.
#        to make the targets available for projects which are not part of the build tree, we also have to install them
install(EXPORT ${PROJECT_NAME}Targets
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${ConfigPackageLocation})  # @note 'ConfigPackageLocation' was set @line 21

###################
## CMake Package ##
###################

# to write *ConfigVersion.cmake
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    VERSION ${PLAIN_CMAKE_VERSION}
    COMPATIBILITY SameMajorVersion)

# to write *Config.cmake
set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR})
configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION ${ConfigPackageLocation}
    PATH_VARS INCLUDE_INSTALL_DIR)

# move both to target directory
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    DESTINATION ${ConfigPackageLocation})
