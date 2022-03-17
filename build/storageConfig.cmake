
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was storageConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

# @note replaced "motion" w/ ${PROJECT_NAME} -> test if still compatible w/ catkin!

# Include configuration file generated with CMakeLists.txt
include(${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}Targets.cmake)

# In order to use the library in a catkin package
# storage_INCLUDE_DIRS and storage_LIBRARIES need to be defined.
# Otherwise catkin_package() would generate a warning complaining about it.
set_and_check(${PROJECT_NAME}_INCLUDE_DIRS "${PACKAGE_PREFIX_DIR}/include")
set(${PROJECT_NAME}_LIBRARIES ${PROJECT_NAME}::${PROJECT_NAME})

# Dependency forwarding
# Required to avoid explicitly finding mplibrary's dependencies on 3rd party packages
include(CMakeFindDependencyMacro)
find_dependency(Boost 1.0 REQUIRED COMPONENTS system NO_MODULE)

# confirm that all required components have been found
check_required_components(${PROJECT_NAME})
