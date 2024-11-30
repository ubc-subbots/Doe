# generated from ament/cmake/core/templates/nameConfig.cmake.in

# prevent multiple inclusion
if(_doe_perception_CONFIG_INCLUDED)
  # ensure to keep the found flag the same
  if(NOT DEFINED doe_perception_FOUND)
    # explicitly set it to FALSE, otherwise CMake will set it to TRUE
    set(doe_perception_FOUND FALSE)
  elseif(NOT doe_perception_FOUND)
    # use separate condition to avoid uninitialized variable warning
    set(doe_perception_FOUND FALSE)
  endif()
  return()
endif()
set(_doe_perception_CONFIG_INCLUDED TRUE)

# output package information
if(NOT doe_perception_FIND_QUIETLY)
  message(STATUS "Found doe_perception: 0.0.0 (${doe_perception_DIR})")
endif()

# warn when using a deprecated package
if(NOT "" STREQUAL "")
  set(_msg "Package 'doe_perception' is deprecated")
  # append custom deprecation text if available
  if(NOT "" STREQUAL "TRUE")
    set(_msg "${_msg} ()")
  endif()
  # optionally quiet the deprecation message
  if(NOT doe_perception_DEPRECATED_QUIET)
    message(DEPRECATION "${_msg}")
  endif()
endif()

# flag package as ament-based to distinguish it after being find_package()-ed
set(doe_perception_FOUND_AMENT_PACKAGE TRUE)

# include all config extra files
set(_extras "")
foreach(_extra ${_extras})
  include("${doe_perception_DIR}/${_extra}")
endforeach()
