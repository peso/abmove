/**
  @file AEPWrap.cpp
*/

#include "AEPWrap.hpp"
namespace AEP = AbaloneEngineProtocol;

#include <sstream>
#include "CheckInput.h"
#include "Persistence.hpp"
#define DEB1
#include "Trace.hpp"



////////////////////////////////////////////////////////////////
//
//  Move notation
//

string to_aep(const Board2D::Move& m) {
  std::stringstream str;
  // TODO: Find a more explicit way of getting the correct notation
  str << m;
  return str.str();
}

////////////////////////////////////////////////////////////////
//
//  Commands in AEP
//

  enum Command {
    cmd_NONE,
    cmd_AEP,
    cmd_DEBUG,
    cmd_ISREADY,
    cmd_SETOPTION,
    cmd_POSITION,
    cmd_GO,
    cmd_STOP,
    cmd_PONDERHIT,
    cmd_QUIT,
    cmd_size
  };
  static char* cmdName[cmd_size] = {
    "",
    "aep",
    "debug",
    "isready",
    "setoption",
    "position",
    "go",
    "stop",
    "ponderhit",
    "quit"
  };
  // Return command ID, 0 if not found
  static Command GetCommand(string str) {
    int i = cmd_size;
    while (--i > 0) {
      if (str == cmdName[i]) break;
    }
    return (Command)i;
  }


////////////////////////////////////////////////////////////////
//
//  Abalone Engine Protocol interface
//

//strtok() {};
/** Generic wrapper of an engine that allows it to communicate through the
  Abalone Engine Protocol (AEP). The engine must implement the EngineInterface
  and at least once pr second call the InputHandler provided.

  The wrapper provides functions that can check stdin for commands and act
  on commands from the client.
*/
class EngineWrapper: public AEP::InputHandler {
private:
  AEP::Engine* m_engine;
  bool m_searching;
  bool m_quit;
  /// Buffer used when reading from stdin
  /// During parsing and handling of a command it is illegal to change
  string m_commandLine;
  Command m_command;

public:

  bool MustQuit() const { return m_quit; }
  Command GetCommand() const { return m_command; }

  EngineWrapper(AEP::Engine* engine)
  : m_engine(engine), m_searching(false), m_quit(false)
  {
    m_engine->SetInputHandler(this);
  }

  virtual ~EngineWrapper()
  {}

  unsigned m_first;
  // TODO: read up on strtok() for std::string or boost-someting
  string get_token() {
    // skip leading blanks
    unsigned i = m_first;
    while (i < m_commandLine.size() and m_commandLine[i] == ' ') i++;
    // find end of token (non-blank or end of line)
    unsigned j = i + 1;
    while (j < m_commandLine.size() and m_commandLine[j] != ' ') j++;
    // find next token
    m_first = j + 1;
    while (m_first < m_commandLine.size() and m_commandLine[m_first] == ' ') m_first++;
    // return token
    return m_commandLine.substr(i,j-i);
  }

  /** True, if get_token can find more tokens */
  bool more_tokens() const {
    return m_first < m_commandLine.size();
  }

  string get_tail() const {
    return m_commandLine.substr(m_first,string::npos);
  }

  /** Blocking read of stdin. If a command is read, it is also handled. */
  void WaitInput() {
    int c = std::cin.get();
    if (c==-1) return;
    const unsigned MAX_COMMAND_LENGTH = 4000;
    if (m_commandLine.size() > MAX_COMMAND_LENGTH) {
      c = '\n';
      APP_ERROR("Command line exceeded "<<MAX_COMMAND_LENGTH<<" chars");
    }
    if (c!='\n') m_commandLine += static_cast<char>(c);
    else HandleCommand();
  }
  /** Nonblocking read of stdin. If a command is read, it is also handled.
  @note Should be called from the Engine while
  searching to make it possible to interrupt search. */
  void CheckInput() {
    while (::CheckInput()) WaitInput();
  }
  /** True, if the input functions WaitInput() or CheckInput() some not yet
  processed data */
  bool InputReady() {
    return m_commandLine.size() > 0;
  }
  /** Send the string to the GUI (through stdout), and add a newline '\n' to
  indicate end-of-reply. */
  void reply(string str) {
    std::cout << str << std::endl << std::flush;
  }
  /** Process the command in input buffer and clear the buffer */
  void HandleCommand() {
    const string original_commandLine = m_commandLine;
    TRACE("+HandleCommand : "<<m_commandLine);
    m_first = 0;
    m_command = ::GetCommand(get_token());
    switch (m_command) {
      case cmd_AEP:       cmd_aep(); break;
      case cmd_DEBUG:     cmd_debug(); break;
      case cmd_ISREADY:   cmd_isready(); break;
      case cmd_SETOPTION: cmd_setoption(); break;
      case cmd_POSITION:  cmd_position(); break;
      case cmd_GO:        cmd_go(); break;
      case cmd_STOP:      cmd_stop(); break;
      case cmd_PONDERHIT: cmd_ponderhit(); break;
      case cmd_QUIT:      cmd_quit(); break;
      default: {
        TRACE("Unknown command : "<<m_commandLine);
      }
    }
    m_commandLine = "";
    TRACE("-HandleCommand : "<<original_commandLine);
  }
  void cmd_aep() {
    const char* s;
    s = m_engine->GetName();
    if (s) reply("id name "+string(s));
    s = m_engine->GetAuthor();
    if (s) reply("id author "+string(s));
    // TODO: Return options of engine
    reply("readyok");
  }
  void cmd_debug() {
    string direction = get_token();
    if (direction == "on") m_engine->SetDebug(true);
    else if (direction == "off") m_engine->SetDebug(false);
    else {
      TRACE("Error: Valid values for debug command are 'on' or 'off' : "<<m_commandLine);
    }
  }
  /**
    This command is used to synchronize the engine with the GUI. When the GUI
    has sent a command or multiple commands that can take some time to
    complete, this command can be used to wait for the engine to be ready
    again or to ping the engine to find out if it is still alive.
    E.g. this should be sent after setting the path to the tablebases as this
    can take some time. This command is also required once before the engine
    is asked to do any search to wait for the engine to finish initializing.
    This command must always be answered with "readyok"
  */
  void cmd_isready() {
    reply("readyok");
  }
  /**
    This command is used to configure the engine
  */
  void cmd_setoption() {
    APP_WARNING("command 'setoption' not implemented.");
  }
  /**
    Define starting position and moves up to current position. Use this command
    to set up board including moves leading to it. After this command the GUI
    would typically send the "go" command.

    @note Any syntax or semantic error is a fatal error and will cause the
    program to exit. This was chosen because wrong data means bugs in the GUI.
    To exit the engine clearly indicates that something is wrong.

    @example A sample command for the first few moves after Belgian Daisy
    position abp 00000 000000 0000000 00000000 000000000 00000000 00000000 000000 00000 moves a2a3 b7b6
  */
  void cmd_position() {
    if (m_searching) {
      APP_ERROR("Cannot set up new position while searching");
    }
    Game game;
    string start_pos;
    string t = get_token();
    if (t != "abp") {
      APP_FATAL("command 'position' starts with '"<<t<<"' instead of 'abp'");
      exit(1);
    }
    // Parse start position
    while (more_tokens()) {
      t = get_token();
      if (t == "moves") break;
      start_pos += t; // Concatenate startpos
    }
    // Set up start position
    {
      Board startPos;
      std::istringstream s(start_pos);
      startPos.Read(s);
      // Adjust number of pieces out - since Read assumes more data
      for (int player = 1; player <= 2; player ++) {
        int marbles = 0;
        Board2D::Pos pos;
        pos.Next();
        do {
          if (startPos.At(pos) == player) marbles ++;
          pos.Next();
        } while (pos.Valid());
        int missing = 14 - marbles;
        int opponent = 3 - player;
        startPos.SetScore(opponent,missing);
      }
      game.RestartFrom(startPos);
    }
    // Parse moves
    int moveNr = 0;
    while (more_tokens()) {
      moveNr++;
      t = get_token();
      //TRACE(__func__ <<" parsing move #"<<moveNr<<" '"<<t<<"'");
      std::istringstream s(t);
      Move m;
      if (!readMove(s, game)) {
        APP_FATAL("command 'position', move #"<<moveNr
          <<" '"<<m<<"' is invalid."<<std::endl
          <<"Board=\n"
          <<game.board);
        exit(1);
      }
    }
    // Transfer game to engine
    m_engine->SetGame(game);
  }
  /**
    Begin searching the current position. This starts the engine and it may
    reply any time it wants and several times. It is recommended to reply at
    least every time a ply has been fully searched and every time the main line
    changes. The engine will continue searching untill

    @example A sample command for a 10 ply search, max time 9000 miliseconds
    go depth 10 movetime 9000
  */
  void cmd_go() {
    // Begin searching

    if (m_searching) {
      APP_ERROR("Cannot start new search while searching");
      return;
    }
    m_engine->ResetSearchParameters();
    while (more_tokens()) {
      // Parse subcommand
      string cmd = get_token();
      if (cmd == "searchmoves") {
        m_engine->SetSearchParameter(cmd,get_tail());
      } else if (cmd == "ponder") {
        m_engine->SetSearchParameter(cmd,"1");
      } else if (cmd == "time") {
        m_engine->SetSearchParameter(cmd+"1",get_token());
        m_engine->SetSearchParameter(cmd+"2",get_token());
      } else if (cmd == "inc") {
        m_engine->SetSearchParameter(cmd+"1",get_token());
        m_engine->SetSearchParameter(cmd+"2",get_token());
      } else if (cmd == "depth") {
        m_engine->SetSearchParameter(cmd,get_token());
      } else if (cmd == "mate") {
        m_engine->SetSearchParameter(cmd,get_token());
      } else if (cmd == "movetime") {
        m_engine->SetSearchParameter(cmd,get_token());
      } else if (cmd == "infinite") {
        m_engine->SetSearchParameter("movetime","inf");
      } else if (cmd == "nodes") {
        m_engine->SetSearchParameter(cmd,get_token());
      } else {
        APP_WARNING("Unknown 'go' subcommand: '"<<cmd<<"'");
      }
    }
    // Make engine begin search of current position
    m_searching = true;
    Board2D::Move m;
    m_engine->GetMove(m);
    // TODO: Make engine emit final statistics
    reply("bestmove "+to_aep(m));
    m_searching = false;
  }
  void cmd_stop() {
    // Note: This command may be called from inside the search
    // thus m_engine->GetMove(m) should allow this callback
    // Correct engine implementation of StopSearch, should
    // exit search as fast as possible when returning from CheckInput()
    m_engine->StopSearch();
  }
  void cmd_ponderhit() {
    APP_WARNING("command 'ponderhit' not implemented.");
  }
  void cmd_quit() {
    m_quit = true;
    cmd_stop();
  }
};

/** Drive engine through the abalone engine protocol (AEP)
  @return Error Code. 0 if exit without errors */
int AEP::Play(Engine& player) {
  // Disable buffering of stdout and stdin
  // Initialise engine
  TRACE("Initialise engine");
  EngineWrapper aep(&player);
  TRACE("Read input");
  while (not aep.MustQuit()) {
    aep.WaitInput();
  }
  return 0;
}
