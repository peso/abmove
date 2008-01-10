
################################################################
# Library abmove

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

add_library (abmove AEPWrap.cpp Board2D.cpp CheckInput.c Persistence.cpp Settings.cpp Trace.cpp TraceManager.cpp)

