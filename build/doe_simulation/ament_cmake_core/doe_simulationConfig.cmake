# generated from ament/cmake/core/templates/nameConfig.cmake.in

# prevent multiple inclusion
if(_doe_simulation_CONFIG_INCLUDED)
  # ensure to keep the found flag the same
  if(NOT DEFINED doe_simulation_FOUND)
    # explicitly set it to FALSE, otherwise CMake will set it to TRUE
    set(doe_simulation_FOUND FALSE)
  elseif(NOT doe_simulation_FOUND)
    # use separate condition to avoid uninitialized variable warning
    set(doe_simulation_FOUND FALSE)
  endif()
  return()
endif()
set(_doe_simulation_CONFIG_INCLUDED TRUE)

# output package information
if(NOT doe_simulation_FIND_QUIETLY)
  message(STATUS "Found doe_simulation: 0.0.0 (${doe_simulation_DIR})")
endif()

# warn when using a deprecated package
if(NOT "" STREQUAL "")
  set(_msg "Package 'doe_simulation' is deprecated")
  # append custom deprecation text if available
  if(NOT "" STREQUAL "TRUE")
    set(_msg "${_msg} ()")
  endif()
  # optionally quiet the deprecation message
  if(NOT doe_simulation_DEPRECATED_QUIET)
    message(DEPRECATION "${_msg}")
  endif()
endif()

# flag package as ament-based to distinguish it after being find_package()-ed
set(doe_simulation_FOUND_AMENT_PACKAGE TRUE)

# include all config extra files
set(_extras "")
foreach(_extra ${_extras})
  include("${doe_simulation_DIR}/${_extra}")
endforeach()
