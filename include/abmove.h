/**
  @file ABMOVE_H
  Defines used to control linking.

  Define HALIOTIS_BUILD_SHLIB to build abmove dll
  Define HALIOTIS_USE_SHLIB to link abmove dll
  Define nothing to build and link libabmove.a
*/
#ifndef ABMOVE_H
#define ABMOVE_H

#if defined(HALIOTIS_BUILD_SHLIB)
  // build shared library
  #define HALIOTIS_EXPORT __declspec(dllexport)
  #define HALIOTIS_IMPORT __declspec(dllimport)
#elif defined(HALIOTIS_USE_SHLIB)
  // link shared library
  #define HALIOTIS_EXPORT __declspec(dllimport)
  #define HALIOTIS_IMPORT __declspec(dllexport)
#else
  // build/link static library
  #define HALIOTIS_EXPORT
  #define HALIOTIS_IMPORT
#endif

#endif
