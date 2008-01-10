/**
  @file AEPWrap.hpp
*/
#ifndef AEPWrap_HPP
#define AEPWrap_HPP

#include <memory>
#include <string>
#include "Board2D.hpp"
using namespace Haliotis;

namespace AbaloneEngineProtocol {

#ifdef USE_OPTION
class Option {
protected:
  const char* m_name;
public:
  Option(const char* name) : m_name(name) {}
  virtual ~Option() {};
  /// Get a string representation of the Option that is ready for AEP
  /// transmission.
  virtual string ToString() = 0;
  /// Copy value from one option to another. Must have same type
  virtual Option& operator = (Option& src);
};

/** Parse a string (following command setoption) and create the corresponding
option element */
std::auto_ptr<Option> ParseString(string str);

class OptionCheck: public Option {
private:
  bool m_value;
public:
  OptionCheck(char* name, bool enabled)
  : Option(name), m_value(enabled) {}
};

class OptionSpin: public Option {
private:
  int m_value;
  int m_min;
  int m_max;
public:
  OptionSpin(char* name, int min, int max, int def)
  : Option(name), m_value(def), m_min(min), m_max(max) {}
};

class OptionCombo: public Option {
};

class OptionButton: public Option {
};

class OptionString: public Option {
private:
  std::string m_value;
public:
  OptionCheck(char* name, std::string def)
  : Option(name), m_value(def) {}
};

/*
  OptionCheck a("Ponder",true);
  OptionSpin a("range",3,10,3);
  OptionCombo a("range",{"a","bcd","e"});
  OptionButton b("name");
  OptionString s("name"."default");
*/
#endif

////////////////////////////////////////////////////////////////
//
// Specification of EngineInterface
//

/** Interface for processing input during search. */
class InputHandler {
public:
  /** Check input and return immediately if none is present. If the input
  accumulated to a command, do this command. */
  virtual void CheckInput() = 0;
};

/** Interface that must be implemented by engine. The AEPWrapper will use
  it to issue commands. */
class Engine {
private:
  InputHandler* m_inputHandler;
public:
  Engine() : m_inputHandler(0) {}
  /// Set function to call during search
  inline void SetInputHandler(InputHandler* callback) {
    m_inputHandler = callback;
  }
  /// Check if input should be processed. May call back into the engine.
  inline void CheckInput() {
    if (m_inputHandler) m_inputHandler->CheckInput();
  }
  /// Set up board position and moves leading to it
  virtual void SetGame(const Game& game) =0;
  /// Get name of engine
  virtual const char* GetName() const =0;
  /// Get author of engine
  virtual const char* GetAuthor() const =0;
  /// Control debug output
  virtual void SetDebug(bool enable) =0;
  /// Get move from engine (ask it do perform a search)
  virtual void GetMove(Board2D::Move& m) =0;
  /// Tell engine to set a flag so it will stop searching.
  virtual void StopSearch() = 0;

  virtual void ResetSearchParameters() =0;
  virtual void SetSearchParameter(string name, string value) =0;
};



/** Drive engine through the abalone engine protocol (AEP) */
int Play(Engine& player);

} // namespace AbaloneEngineProtocol

#endif
