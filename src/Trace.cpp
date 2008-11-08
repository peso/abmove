/** @file Trace.cpp

  Copyright (C) 2008 Peer Sommerlund

  This library is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as 
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
  General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
*/

#include "config.h"
#define DEB1
#include "Trace.hpp"

// This implementation uses STL streams
// Warning: It has no mutex

#include <ctime>

// Global variables in this module

#if defined(HALIOTIS_BUILD_SHLIB)
static const char* trace_file = "trace.log";
#else
extern const char* TRACE_FILE;
static const char* trace_file = TRACE_FILE;
#endif

static TraceCollector* theTraceCollector = 0;

// Implementation of TraceCollector

void TraceCollector::SetTraceFile(const char* filename)
{
  trace_file = filename;
  if (theTraceCollector != 0) {
    GetInstance().traceStream.close();
    GetInstance().traceStream.open(filename);
  }
}

TraceCollector& TraceCollector::GetInstance()
{
  if (theTraceCollector == 0) {
    theTraceCollector = new TraceCollector();
  }
  return *theTraceCollector;
}

TraceCollector::TraceCollector()
: traceStream(trace_file)
{
  time_t now = time(0);
  traceStream << '\n'
    << "----------------------------------------------------------------\n"
    << "Trace started at " << ctime(&now) << std::flush;
}

std::string TimeStamp()
{
  time_t now = time(0);

  // Convert to Coordinated Universal Time (UTC)
  tm* now_tm = gmtime(&now);

  // Print on trace stream
  if (now_tm == 0) {
    return "--gmtime()==0-- ";
  }
  char buf[80];
  strftime(buf,sizeof(buf),"%Y%m%d %H:%M:%S ",now_tm);
  return buf;
}

void TraceCollector::TraceTimeStamp()
{
  traceStream << TimeStamp();
}

/// The global assert failure instance is stored here
AssertFailureListener* assertListener = 0;

/// The global log-listener instance is stored here
AppLogListener* appLogListener = 0;

/// The global level_name instance is stored here
const char * level_name[5] = {
  "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};
