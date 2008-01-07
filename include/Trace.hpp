#ifndef __TRACE_HPP__
#define __TRACE_HPP__

/** @file Trace.hpp

If you want to use trace, you should declare a static variable with the name
of TRACE_FILE

@example
  /// Name of file used for trace messages
  const char* TRACE_FILE = "your_app_name.log";

*/

#include "abmove.h"

#include <fstream>
#include <string>
#include <sstream>

// Provoke a segmentation fault - make the debugger stop here
#define SEGV *(int *)0 = 0;

HALIOTIS_EXPORT std::string TimeStamp();

//
// TRACE
//

// Warning, this implementation has no mutex protection
class HALIOTIS_EXPORT TraceCollector {
private:
  TraceCollector();

public:
  static void SetTraceFile(const char* filename);
  static TraceCollector& GetInstance();

  std::ofstream traceStream;
  void TraceTimeStamp();
};

#if defined(DEB1) && !defined(_CONFIG_H_)
#include "config.h"
#endif

#if defined(DEB1) && !defined(_CONFIG_H_)
#error You must include "config.h" before you include "Trace.hpp"
// The reason is that USE_LOG must also be defined to enable trace
// This define was included to be able to build release versions without
// trace by changing a single file.
#endif

#if defined(DEB1) && defined(USE_LOG) && !defined(NDEBUG)

/// Select a trace file to use
#define DEFINE_TRACE_FILE(filename) \
  TraceCollector::GetInstance().SetTraceFile(filename);

/// Send something to the trace stream
#define TRACE__(x) { \
  TraceCollector::GetInstance().traceStream << x; \
}
/// Send to trace stream and flush it
#define TRACE_(x) TRACE__(x << ::std::flush)
/// Send to trace stream, but prefix with timestamp
#define TRACE_NO_ENDL(x) { \
  TraceCollector::GetInstance().TraceTimeStamp(); \
  TRACE_THREAD__ \
  TRACE__(x) \
}
/// Send to trace stream, but prefix with timestamp and postfix with endl
#define TRACE(x) TRACE_NO_ENDL(x << ::std::endl)
// Trace thread ID if in an wxWidget environment
#ifdef _WX_THREAD_H_
#define TRACE_THREAD__ TRACE__(wxThread::GetCurrentId() << ' ')
#else
#define TRACE_THREAD__
#endif
// This macro is for temporary search of a failure
#define TRACE_CHECKPOINT TRACE(__FILE__ << ":" << __LINE__ << " "<< __func__)

#else

#define TRACE__(x)
#define TRACE_(x)
#define TRACE(x)
#define TRACE_CHECKPOINT ThisIsAnError(You_must_define_DEB1_and_include,"config.h")

#endif

//
// TRACE_ASSERT
//

/** Implement a specialisation of this class and give it to the TraceCollector
to be able to show an assertion failure to the user before the program exits
*/
struct AssertFailureListener {
  virtual void AssertFailure(std::string msg) const = 0;
};

/** Global variable storing assert listener */
extern HALIOTIS_EXPORT AssertFailureListener* assertListener;

/** Set the global assert listener */
inline void SetAssertFailureListener(AssertFailureListener* listener)
{ assertListener = listener; }

#ifndef NDEBUG
#define TRACE_ASSERT(x) { \
  bool ok = x; \
  if (not ok) { \
    ::std::ostringstream msg; \
    msg << "Assertion failed: " << #x \
	      << ", file " << __FILE__ \
	      << ", line " << __LINE__; \
    {::std::ofstream assertFile("assert.log"); \
    assertFile << msg.str() << ::std::endl;} \
    TRACE(msg.str()); \
    if (assertListener) assertListener->AssertFailure(msg.str()); \
    SEGV; \
  } \
}
#define TRACE_ASSERT_MSG(ass,assmsg) { \
  bool ok = ass; \
  if (not ok) { \
    ::std::ostringstream msg; \
    msg << "Assertion failed: " << #ass \
	      << ", file " << __FILE__ \
	      << ", line " << __LINE__ \
        << ::std::endl \
        << "Assertion message: " << assmsg; \
    {::std::ofstream assertFile("assert.log"); \
    assertFile << msg.str() << ::std::endl;} \
    TRACE(msg.str()); \
    if (assertListener) assertListener->AssertFailure(msg.str()); \
    SEGV; \
  } \
}
#else
#define TRACE_ASSERT(x)
#define TRACE_ASSERT_MSG(assertion,msg)
#endif

//
// APP_LOG
//

/** Severity of log message */
enum LogLevel {
  LogLevel_DEBUG,
  LogLevel_INFO,
  LogLevel_WARN,
  LogLevel_ERROR,
  LogLevel_FATAL
};

/** Implement a specialisation of this class and use SetAppLogListener
to be able to react to log messages
*/
struct AppLogListener {
  /** Make an entry in the log */
  virtual void Log(LogLevel level, std::string msg) const = 0;
};

/** Global variable storing log listener */
extern HALIOTIS_EXPORT AppLogListener* appLogListener;

/** Set the global log listener */
inline void SetAppLogListener(AppLogListener* listener)
{ appLogListener = listener; }

/** Global variable storing names of log levels */
extern HALIOTIS_EXPORT const char * level_name[];

#define APP_LOG(level,x) { \
  ::std::ostringstream msg; \
  msg << x; \
  TRACE(level_name[level] \
  	<< ", file " << __FILE__ \
	<< ", line " << __LINE__ \
        << ": " << msg.str()); \
  if (appLogListener) appLogListener->Log(level,msg.str()); \
}

#define APP_DEBUG(x)   APP_LOG(LogLevel_DEBUG,x)
#define APP_INFO(x)    APP_LOG(LogLevel_INFO,x)
#define APP_WARNING(x) APP_LOG(LogLevel_WARN,x)
#define APP_ERROR(x)   APP_LOG(LogLevel_ERROR,x)
#define APP_FATAL(x)   APP_LOG(LogLevel_FATAL,x)

//
// Runtime configurable trace
//

#include "TraceFlag.hpp"

#ifdef NDEBUG
#define TRACE_FLAG(trace_flag)
#define TRACE_ENABLED(trace_flag) false
#else
#define TRACE_FLAG(trace_flag) static TraceFlag trace_flag(#trace_flag,__FILE__);
#define TRACE_ENABLED(trace_flag) trace_flag.IsTraceEnabled()
#endif

//
// Helper class
//

/**
  Provide a way to find out if system has looped for too long time.

  Example:
  <pre>
  #ifdef DEBUG_bug012345
  static ImAliveMessage flag(10.0); // msg every 10 seconds
  if (flag.sendMessageNow()) {
    TRACE(flag.loops() << " iterations at XXYYZZ");
    // write your code here
  }
  #endif
  </pre>
*/
class ImAliveMessage {
  int m_totalLoops;
  double m_secPrMessage;
  time_t m_lastMessage;
public:
  ImAliveMessage(double secPrMessage) :
    m_totalLoops(0),
    m_secPrMessage(secPrMessage)
  {
    time(&m_lastMessage);
  }
  bool sendMessageNow() {
    m_totalLoops ++;
    time_t messageNow;
    time(&messageNow);
    bool sendNow = difftime(messageNow,m_lastMessage) >= m_secPrMessage;
    if (sendNow) {
      m_lastMessage = messageNow;
    }
    return sendNow;
  }
  int loops() { return m_totalLoops; }
};


#endif

