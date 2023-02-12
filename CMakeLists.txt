cmake_minimum_required (VERSION 3.1)
set(CMAKE_CXX_STANDARD 11 CACHE STRING "The C++ standard to use")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# The name of our project is "abmove".  CMakeLists files in this project can
# refer to the root source directory of the project as ${abmove_SOURCE_DIR} and
# to the root binary directory of the project as ${abmove_BINARY_DIR}.
project (abmove)
set(abmove_MAJOR_VERSION 0)
set(abmove_MINOR_VERSION 2)
set(abmove_VERSION "${abmove_MAJOR_VERSION}.${abmove_MINOR_VERSION}")

# Define installation directories - used by CMakeLists.txt in subdirectory
# They are cached so the user can overriding the default values
set(INSTALL_LIB_DIR lib
  CACHE PATH "Installation directory for libraries")
set(INSTALL_BIN_DIR bin
  CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDE_DIR include
  CACHE PATH "Installation directory for header files")
if(WIN32 AND NOT CYGWIN)
  set(DEF_INSTALL_CMAKE_DIR CMake)
else()
  set(DEF_INSTALL_CMAKE_DIR lib/CMake/abmove)
endif()
set(INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR}
  CACHE PATH "Installation directory for CMake files")

# Make relative paths absolute (needed later on)
foreach(p LIB BIN INCLUDE CMAKE)
  set(var INSTALL_${p}_DIR)
  if(NOT IS_ABSOLUTE "${${var}}")
    set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
  endif()
endforeach()
 
# Recurse into the "src" subdirectory - output will be placed in "build".
# This does not actually cause another cmake executable to run.  The same 
# process will walk through the project's entire directory structure.
add_subdirectory (src build)

# Add all targets to the build-tree export set
export(TARGETS abmove
  FILE "${PROJECT_BINARY_DIR}/abmoveTargets.cmake")
 
# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
export(PACKAGE abmove)
 
# Create the abmoveConfig.cmake and abmoveConfigVersion files
# for the build tree and the install tree
# make two versions of abmoveConfig.cmake since it specifies directories
file(RELATIVE_PATH
  REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}" "${INSTALL_INCLUDE_DIR}")
# ... for the build tree
set(CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include" "${PROJECT_BINARY_DIR}/build")
configure_file(abmoveConfig.cmake.in
  "${PROJECT_BINARY_DIR}/abmoveConfig.cmake" @ONLY)
# ... for the install tree
# Note that abmove_CMAKE_DIR is not expanded here, but inserted as text
# This works because abmoveConfig.cmake.in defines abmove_CMAKE_DIR before
# @CONF_INCLUDE_DIRS@ is used
set(CONF_INCLUDE_DIRS "\${abmove_CMAKE_DIR}/${REL_INCLUDE_DIR}")
configure_file(abmoveConfig.cmake.in
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/abmoveConfig.cmake" @ONLY)
# version file is independent of location, so we have only one
configure_file(abmoveConfigVersion.cmake.in
  "${PROJECT_BINARY_DIR}/abmoveConfigVersion.cmake" @ONLY)
 
# Install the abmoveConfig.cmake and abmoveConfigVersion.cmake
install(FILES
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/abmoveConfig.cmake"
  "${PROJECT_BINARY_DIR}/abmoveConfigVersion.cmake"
  DESTINATION "${INSTALL_CMAKE_DIR}")
 
# Install the export set for use with the install-tree
install(EXPORT abmoveTargets DESTINATION "${INSTALL_CMAKE_DIR}")

 # Parameters for building a package
set(CPACK_PACKAGE_NAME abmove)
set(CPACK_PACKAGE_VERSION ${abmove_VERSION})
set(CPACK_PACKAGE_CONTACT "Peer Sommerlund <peso@users.sourceforge.net>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "abmove is a library for playing the board game Abalone")
include(CPack)
