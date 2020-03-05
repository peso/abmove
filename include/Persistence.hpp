/** @file Functionality to serialise class Game
 *
 ** @author Peer Sommerlund, 2006-Apr-26
*/

#ifndef Persistence_hpp
#define Persistence_hpp

#include <iostream>

#include "Game.hpp"

#include "Settings.hpp"
#ifndef __SETTINGS_HPP__
class StreamInterface {
public:
  virtual void Read(std::istream& in) = 0;
  virtual void Write(std::ostream& out) const = 0;
};

inline ostream& operator << (std::ostream& out, const StreamInterface& data) {
  data.Write(out);
  return out;
}

inline istream& operator >> (std::istream& in, StreamInterface& data) {
  data.Read(in);
  return in;
}
#endif

void AbaloneGameFormat_Read(std::istream& in, Haliotis::Game& game);
void AbaloneGameFormat_Write(std::ostream& out, const Haliotis::Game& game);

/**
  Implement a filter that allows you to read an single abalone game from a file.

  @note The .AG (Abalone Game) format is specified in
  http://groups.yahoo.com/groups/abalone_prog

  @example
    Game game;
    in  >> ag_format(game);
    out << aba_format(game);
*/
#define USE_AG_FORMAT_CLASS
#ifdef USE_AG_FORMAT_CLASS
class ag_format: public StreamInterface {
  Haliotis::Game* mutableGame;
  const Haliotis::Game* constGame;
public:
  ag_format(Haliotis::Game& game);
  ag_format(const Haliotis::Game& game);
  ag_format(Haliotis::Game* game);
  ag_format(const Haliotis::Game* game);
  virtual void Read(std::istream& in);
  virtual void Write(std::ostream& out) const;
};
#else
StreamInterface& ag_format(Haliotis::Game& var);
const StreamInterface& ag_format(const Haliotis::Game& var);
#endif

// TODO Sort these pasted functions
std::string apf(const Haliotis::Game& game);
std::string MoveList(const Haliotis::Game& game);
bool parse_fftl(const std::string&  str,
  const Haliotis::Board2D&        board,
  Haliotis::Board2D::Move&         move);
bool readMove(std::istream& in, Haliotis::Game& game);
void ReadAttributes(Settings& att, istream& in);
void WriteAttributes(const Settings& att, ostream& out);

#endif
