#ifndef TRACEFLAG_HPP
#define TRACEFLAG_HPP

// @file TraceFlag.hpp implemented in TraceManager.cpp

#include "abmove.h"

class TraceFlag;

HALIOTIS_EXPORT void RegisterTraceFlag(TraceFlag& trace_flag);
HALIOTIS_EXPORT bool SetTraceFlag(const char* flag_name, const char* file_name, bool enable);

class TraceFlag {
public:
  TraceFlag(const char* name, const char* source_file)
  : m_flagname(name)
  , m_filename(source_file) {
    RegisterTraceFlag(*this);
  }
  bool IsTraceEnabled() const { return m_enabled; }
  void SetTraceEnabled(bool enabled) { m_enabled = enabled; }
  const char * GetFlagName() const { return m_flagname; }
  const char * GetFileName() const { return m_filename; }
private:
  const char * m_flagname;
  const char * m_filename;
  volatile bool m_enabled;
};

#endif
