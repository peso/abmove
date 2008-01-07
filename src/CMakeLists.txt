
################################################################
# Library abmove

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

add_library (abmove Board2D.cpp Persistence.cpp Settings.cpp Trace.cpp TraceManager.cpp)

