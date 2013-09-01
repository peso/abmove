cmake_minimum_required (VERSION 2.6)

# The name of our project is "abmove".  CMakeLists files in this project can
# refer to the root source directory of the project as ${abmove_SOURCE_DIR} and
# to the root binary directory of the project as ${abmove_BINARY_DIR}.
project (abmove)

# Recurse into the "src" subdirectory - output will be placed in "build".
# This does not actually cause another cmake executable to run.  The same 
# process will walk through the project's entire directory structure.
add_subdirectory (src build)

# Parameters for building a package
set(CPACK_PACKAGE_NAME abmove)
set(CPACK_PACKAGE_VERSION 0.2a)
set(CPACK_PACKAGE_CONTACT "Peer Sommerlund <peso@users.sourceforge.net>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "abmove is a library for playing the board game Abalone")
include(CPack)
