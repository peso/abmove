
################################################################
# Library abmove


# Examine what features are available on the build platform
INCLUDE (CheckFunctionExists)
CHECK_FUNCTION_EXISTS(select HAVE_SELECT)

# Build configuration file for the build platform
CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/../include/config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/config.h)


include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
  ${CMAKE_CURRENT_BINARY_DIR}
)

# static header files
set(static_headers
    ../include/abmove.h
    ../include/AEPWrap.hpp
    ../include/Board2D.hpp
    ../include/CheckInput.h
    ../include/Game.hpp
    ../include/Persistence.hpp
    ../include/Settings.hpp
    ../include/TraceFlag.hpp
    ../include/Trace.hpp
    ../include/TraceManager.hpp
)
# module files
set(module_files
    AEPWrap.cpp
    Board2D.cpp
    CheckInput.c
    Persistence.cpp
    Settings.cpp
    Trace.cpp
    TraceManager.cpp
)


add_library (abmove ${static_headers} ${module_files})

set_target_properties(abmove
    PROPERTIES PUBLIC_HEADER "${static_headers};${CMAKE_CURRENT_BINARY_DIR}/config.h"
)

################################################################
# Installation

# libabmove.a
install(TARGETS abmove
    EXPORT abmoveTargets
    ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
    PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}"
)
