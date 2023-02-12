#ifndef __SETTINGS_HPP__
#define __SETTINGS_HPP__

#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <sstream>

// Make header file work even if "Trace.hpp" is not included
#ifdef TRACE
#define SETTINGS_TRACE(x) TRACE(x)
#else
#define SETTINGS_TRACE(x)
#endif


//
// String conversion
//

inline void Assign(std::string& var, const std::string& val) { var = val; }
inline void Assign(bool& var,   const std::string& val) { var = atoi(val.c_str())!=0; }
inline void Assign(int& var,    const std::string& val) { var = atoi(val.c_str()); }
inline void Assign(long& var,   const std::string& val) { var = atol(val.c_str()); }
inline void Assign(float& var,  const std::string& val) { var = (float)atof(val.c_str()); }
inline void Assign(double& var, const std::string& val) { var = atof(val.c_str()); }

/**
  Convert a type to a string by using operator << .
  This very nice helper function is good for creating Settings.
*/
template <class T>
inline std::string ToString(T i) {
  std::ostringstream out;
  out << i;
  return out.str();
}

//
// Stream Interface
//

#include <iostream>

class StreamInterface {
public:
  virtual void Read(std::istream& in) = 0;
  virtual void Write(std::ostream& out) const = 0;
};

inline std::ostream& operator << (std::ostream& out, const StreamInterface& data) {
  data.Write(out);
  return out;
}

inline std::istream& operator >> (std::istream& in, StreamInterface& data) {
  data.Read(in);
  return in;
}

//
// Settings
//

/** Settings is a way to transfer configuration information. It simply stores
key-value pairs. Be carefull not to use the same key twice, unless you want
the value to be shared. */
typedef std::map<std::string,std::string> Settings;

/** Abstract base class for configurable classes */
class Configurable {
public:
  /** Insert class specific configuration into settings.
  Will overwrite those already present silently. */
  virtual void GetConfiguration(Settings& settings) const =0;

  /** Configure class according to settings.
  Extra settings are silently ignored. */
  virtual void SetConfiguration(const Settings& settings) =0;
};

template <class T>
bool Get(const Settings& conf, const std::string& label, T& var)
{
  Settings::const_iterator i = conf.find(label);
  if (i==conf.end()) return false;
  Assign(var,i->second);
  return true;
}

template <class T>
void Set(Settings& conf, const std::string& label, const T& value)
{
  conf[label] = ToString(value);
}

/// Make it easyer to mix Configurable objects and simple variables
inline void Get(const Settings& conf, Configurable& obj) {
  obj.SetConfiguration(conf);
}

/// Make it easyer to mix Configurable objects and simple variables
inline void Set(Settings& conf, const Configurable& obj) {
  obj.GetConfiguration(conf);
}

/// Make it easy to map two different set of settings
inline bool Map(Settings& conf, const std::string& src_label,
                         const std::string& tgt_label)
{
  Settings::const_iterator i = conf.find(src_label);
  if (i==conf.end()) return false;
  conf[tgt_label] = conf[src_label];
  return true;
}

using std::string;

/** Read settings from a stream.
@return  false if the instream was not purely settings. This is a little
problematic, since it means that either your input stream must be fully
dedicated to the settings, or you must load everything into a string buffer */
inline bool operator >> (std::istream& in, Settings& conf) {
  std::string line;
  SETTINGS_TRACE("read lines");
  while (getline(in,line)) {
    size_t sep = line.find('=');
    if (sep==string::npos) {
      SETTINGS_TRACE("Did not find '=' in line \""<<line<<"\"");
      return false;
    }
    string key = string(line,0,sep);
    string value = string(line,sep+1);
    // NOTE: getline does not return '\n' but '\r' (on windows) slips through.
    sep = value.find('\r');
    if (sep != string::npos) {
      // erase carriage return and all following chars
      SETTINGS_TRACE("erase from pos "<<sep<<" to "<<value.size()-1);
      value.erase(sep);
    }
    SETTINGS_TRACE("line \""<<line<<"\" is split into "
        <<"key \""<<key<<"\" and value \""<<value<<"\"");
    conf[key] = value;
    // Remove trailing data
  }
  SETTINGS_TRACE("end of lines");
  return true;
}

/** Write settings to a stream. */
inline std::ostream& operator << (std::ostream& out, const Settings& conf) {
  for (Settings::const_iterator i = conf.begin();
    i != conf.end();
    i++)
  {
    out << i->first << '=' << i->second << std::endl;
  }
  return out;
}

/** Write a configuration to a stream */
inline std::ostream& operator << (std::ostream& out, const Configurable& conf) {
  Settings settings;
  conf.GetConfiguration(settings);
  return out << settings;
}

/** Read a configuration from a stream.
  @note Warning: Unused data in the stream is quietly discarded. */
inline std::istream& operator >> (std::istream& in, Configurable& conf) {
  Settings settings;
  in >> settings;
  conf.SetConfiguration(settings);
  return in;
}

/**
  InitFile holds a number of sections, each section is a named list of
  settings (key-value pairs separated by '='). If the InitFile is kept
  between reading and writing it, the order of entities will be preserved as
  will the comments.
*/
class InitFile: public StreamInterface {
private:
  /** One section in the init file. It starts with a section header and
  holds all lines to the next section header. A special section is the one
  named "" which holds all lines up to the first real section header. */
  struct Section: public Configurable {
    std::string name;
    std::vector<std::string> lines;

    virtual ~Section() {};

    /** Insert class specific configuration into settings.
    Will overwrite those already present silently. */
    virtual void GetConfiguration(Settings& settings) const;

    /** Configure class according to settings.
    Extra settings are silently ignored. */
    virtual void SetConfiguration(const Settings& settings);
  };
  std::vector<Section> sections;
public:
  virtual ~InitFile() {};
  void Read(std::istream& in);
  void Write(std::ostream& out) const;
  std::vector<std::string> GetSectionNames() const;
  bool GetSettings(std::string name, Settings& settings) const;
  void SetSettings(std::string name, const Settings& settings);
};

#endif
