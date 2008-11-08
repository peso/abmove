/** @file TraceManager.cpp

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

#include "TraceManager.hpp"

#include <iso646.h>
#include <string>
#include <map>
#include <fstream>
using namespace std;
#include "Settings.hpp"

// Set to 1 to get trace in file tm.log
#if 0
static std::ofstream tmTraceFile("tm.log");
#define TR(x) { tmTraceFile << x << std::endl; }
#else
#define TR(x)
#endif


void RegisterTraceFlag(TraceFlag& trace_flag) {
  TraceManager::GetInstance().Register(trace_flag);
}

/**
  Set the specified trace flag in the specified file.
  
  @param flag_name  Name of the trace flag, if empty apply to all flags in file
  @param file_name  Name of the file, if empty apply to all files with flag
  @param enable  True if you want trace, false to prevent trace
  
  @note You must specify at least one of flag_name and file_name
*/
bool SetTraceFlag(const char* flag_name, const char* file_name, bool enable) {
  TR("SetTraceFlag flag="<<flag_name<<", file="<<file_name<<" to "<<enable);
  return TraceManager::GetInstance()
    .SetTraceFlag(flag_name,file_name,enable);
}

////////////////////////////////////////////////////////////////

struct TraceFlagRegister {
  void Register(TraceFlag& traceFlag) {
    TR("Register flag="<<traceFlag.GetFlagName()<<" file="<<traceFlag.GetFileName());
    flag2instance.insert(make_pair(traceFlag.GetFlagName(),&traceFlag));
    file2instance.insert(make_pair(traceFlag.GetFileName(),&traceFlag));
  }
  bool SetFlagsInFile(const string flag_name, const string file_name, bool enable) {
    int matches = 0;

    TR("SetFlagsInFile before loop of size="<<file2instance.size());
    for (Str2Flag::iterator i = file2instance.find(flag_name);
         i != file2instance.end() and i->first == file_name;
         i++)
    {
      TraceFlag* trace_flag = i->second;
      TR("SetFlagsInFile test flag="<<trace_flag->GetFlagName()<<" file="<<trace_flag->GetFileName());
      if (!flag_name.empty() and trace_flag->GetFlagName() != flag_name) continue;
      TR("SetFlagsInFile match flag "<<trace_flag->GetFlagName());
      trace_flag->SetTraceEnabled(enable);
      matches++;
    }
    return matches > 0;
  }
  bool SetFlagsWithName(const string flag_name, const string file_name, bool enable) {
    int matches = 0;

    TR("SetFlagsWithName before loop of size="<<flag2instance.size());
    for (Str2Flag::iterator i = flag2instance.find(flag_name);
         i != flag2instance.end() and i->first == flag_name;
         i++)
    {
      TraceFlag* trace_flag = i->second;
      TR("SetFlagsWithName test flag="<<trace_flag->GetFlagName()<<" file="<<trace_flag->GetFileName());
      if (!file_name.empty() and trace_flag->GetFileName() != file_name) continue;
      TR("SetFlagsWithName match file "<<trace_flag->GetFileName());
      trace_flag->SetTraceEnabled(enable);
      matches++;
    }
    return matches > 0;
  }
  bool SetTraceFlag(const string flag_name, const string file_name, bool enable) {
    if (flag2instance.find(flag_name) != flag2instance.end())
      return SetFlagsWithName(flag_name,file_name,enable);
    else
      return SetFlagsInFile(flag_name,file_name,enable);
  }


  typedef multimap<string, TraceFlag*> Str2Flag;
  Str2Flag flag2instance;
  Str2Flag file2instance;
};

TraceManager::TraceManager()
: m_traceFlagRegister(new TraceFlagRegister())
{
}

TraceManager& TraceManager::GetInstance() {
  static TraceManager instance;
  return instance;
}
void TraceManager::Register(TraceFlag& traceFlag) {
  m_traceFlagRegister->Register(traceFlag);
}

bool TraceManager::SetTraceFlag(
  const string flag_name, const string file_name, bool enable)
{
  return m_traceFlagRegister->SetTraceFlag(flag_name,file_name,enable);
}

/** Insert class specific configuration into settings.
Will overwrite those already present silently. */
void TraceManager::GetConfiguration(Settings& settings) const {
  for (TraceFlagRegister::Str2Flag::const_iterator i
         = m_traceFlagRegister->flag2instance.begin();
       i != m_traceFlagRegister->flag2instance.end();
       i++)
  {
    const TraceFlag& tf = *(i->second);
    string key = string(tf.GetFlagName()) + "," + string(tf.GetFileName());
    bool value = tf.IsTraceEnabled();
    Set(settings,key,value);
  }
}

/** Configure class according to settings.
Extra settings are silently ignored. */
void TraceManager::SetConfiguration(const Settings& settings) {
  for (TraceFlagRegister::Str2Flag::iterator i
         = m_traceFlagRegister->flag2instance.begin();
       i != m_traceFlagRegister->flag2instance.end();
       i++)
  {
    TraceFlag& tf = *(i->second);
    string key = string(tf.GetFlagName()) + "," + string(tf.GetFileName());
    bool value = tf.IsTraceEnabled();
    Get(settings,key,value);
    tf.SetTraceEnabled(value);
  }
}

void TraceManager::SyncConfigFile(std::string configFile) {
  {
    ifstream in(configFile.c_str());
    if (not in) return;
    in >> *this;
  }
  ofstream out(configFile.c_str());
  out << *this;
}
