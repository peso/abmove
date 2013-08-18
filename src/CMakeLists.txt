
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

add_library (abmove AEPWrap.cpp Board2D.cpp CheckInput.c Persistence.cpp Settings.cpp Trace.cpp TraceManager.cpp)

