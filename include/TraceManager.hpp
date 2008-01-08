#ifndef TRACEMANAGER_HPP
#define TRACEMANAGER_HPP

#include <string>
#include "Settings.hpp"
#include "TraceFlag.hpp"

struct TraceFlagRegister;

class HALIOTIS_EXPORT TraceManager: public Configurable {
public:
  static TraceManager& GetInstance();

  void Register(TraceFlag& traceFlag);

  bool SetTraceFlag(
    const std::string flag_name,
    const std::string file_name,
    bool enable);

  /**
    Read the configuration from a file. If the file is present,
    update it with the known trace flags.
  */
  void SyncConfigFile(std::string configFile);


  /** Insert class specific configuration into settings.
  Will overwrite those already present silently. */
  virtual void GetConfiguration(Settings& settings) const;

  /** Configure class according to settings.
  Extra settings are silently ignored. */
  virtual void SetConfiguration(const Settings& settings);
private:
  TraceManager();
  TraceFlagRegister* m_traceFlagRegister;
};

#endif
