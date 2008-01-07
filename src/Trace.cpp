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
