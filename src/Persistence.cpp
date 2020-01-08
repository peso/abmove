/** @file Persistence.cpp

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
#include "Persistence.hpp"

//// Headers ///////////////////////////////////////////////////

#include "config.h"
#undef HAVE_CPPUNIT

#define DEB1
#include "Trace.hpp"
#define TRACE1(x) // TRACE(x)

#ifdef HAVE_CPPUNIT
#include <cppunit/TestSuite.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/ui/text/TestRunner.h>
#endif

#include <iostream>
using namespace std;

#include "Board2D.hpp"

//// interface /////////////////////////////////////////////////

using namespace Haliotis;

#if 0
/**
  The Notation class converts moves and positions to and from
  string representation via streams.

  This class is usefull if you want your program to support several
  notations, and want it to be changeable at runtime.
*/
class Notation {
public:
  const StreamInterface& operator() (const Move& move) {
  }
  StreamInterface& operator() (Move& move) {
  }
};

/** A board position in Waterloo1993 format

  Each row (starting at the bottom with white) will be
  lettered successively starting at 'a'

  Each space within a row, starting the the leftmost space,
  will be numbered successively starting at '1'

  Thus the valid coordinates are a1-a5, b1-b6, c1-c7, d1-d8,
  e1-e9, f1-f8, g1-g7, h1-h6, and i1-i5.

            i  # # # # #
           h  # # # # # #
          g  . . # # # . .
         f  . . . . . . . .
        e  . . . . . . . . .
         d  . . . . . . . .
          c  . . O O O . .
           b  O O O O O O
            a  O O O O O

  where # is black and O is white. Note that black is player 1
  and white is player 2.

  -- Nacre::BoardPos conventions ---

  note that Nacre::BoardPos has player 1 at the bottom
  where Waterloo1993::Pos has player 1 at the top

  The basis is a square where the corners are cut off.
  each row is numbered 0 to 8 and each diagonal 0 to 8.

  Nacre::BoardPos classic start position:


                 0 1 2 3 4
                / / / / / 5
            8  2 2 2 2 2 / 6
           7  2 2 2 2 2 2 / 7
          6  . . 2 2 2 . . / 8
         5  . . . . . . . . /
        4  . . . . . . . . .
         3  . . . . . . . .
          2  . . 1 1 1 . .
           1  1 1 1 1 1 1
            0  1 1 1 1 1

*/
class Waterloo1993Pos: public StreamInterface {
  BoardPos& bp;
public:
  Waterloo1993Pos(BoardPos& _bp): bp(_bp) {};
  virtual void Read(istream& in)
  {
    char row, space;
    in >> row >> space;
    bp.y = 'i'-row;
    bp.x = space-'1' + (bp.y<4 ? 4-bp.y : 0);
  }
  virtual void Write(ostream& out) const
  {
    out << (char)('i'-bp.y)
        << bp.x + 1 - (bp.y<4 ? 4-bp.y : 0);
  }
};

class Waterloo1993: public StreamInterface {
public:
  Waterloo1993(Game* _game);
  virtual void Read(istream& in);
  virtual void Write(ostream& out) const;
};

/** A board position in Abapro2003 format

  Each row (starting at the bottom with white) will be
  lettered successively starting at 'a'

  Each NW-SE diagonal (the SW-most is first)
  will be numbered successively starting at '1'

            i  2 2 2 2 2
           h  2 2 2 2 2 2
          g  . . 2 2 2 . .
         f  . . . . . . . .
        e  . . . . . . . . .
         d  . . . . . . . . \
          c  . . 1 1 1 . . \ 9
           b  1 1 1 1 1 1 \ 8
            a  1 1 1 1 1 \ 7
                \ \ \ \ \ 6
                 1 2 3 4 5

  where 1 is black and 2 is white. Note that black is player 1
  and white is player 2.

  -- Nacre::BoardPos conventions ---

  The basis is a square where the corners are cut off.
  each row is numbered 0 to 8 and each diagonal 0 to 8.

  Nacre::BoardPos classic start position:


                 0 1 2 3 4
                / / / / / 5
            8  2 2 2 2 2 / 6
           7  2 2 2 2 2 2 / 7
          6  . . 2 2 2 . . / 8
         5  . . . . . . . . /
        4  . . . . . . . . .
         3  . . . . . . . .
          2  . . 1 1 1 . .
           1  1 1 1 1 1 1
            0  1 1 1 1 1

*/
class Abapro2003Pos: public StreamInterface {
  BoardPos& bp;
public:
  Abapro2003Pos(BoardPos& _bp): bp(_bp) {};
  virtual void Read(istream& in)
  {
    char row, diag;
    in >> row >> diag;
    bp.y = 'i'-row;
    bp.x = diag-'1' + 4 - bp.y;
  }
  virtual void Write(ostream& out) const
  {
    out << (char)('i'-bp.y)
        << bp.x + 1 - 4 + bp.y;
  }
};

class Abapro2003: public StreamInterface {
public:
  Abapro2003(Game* _game);
  virtual void Read(istream& in);
  virtual void Write(ostream& out) const;
};

/** A board position in Nacre2005 format

  Each row (starting at the bottom with black) will be
  lettered successively starting at 'a'

  Diagonals are used to address the horisontal part.

  Thus the valid coordinates are a1-a5, b1-b6, c1-c7, d1-d8,
  e1-e9, f2-f9, g3-g9, h4-h9, and i5-i9.

            i  2 2 2 2 2
           h  2 2 2 2 2 2
          g  . . 2 2 2 . .
         f  . . . . . . . .
        e  . . . . . . . . .
         d  . . . . . . . . \
          c  . . 1 1 1 . . \ 9
           b  1 1 1 1 1 1 \ 8
            a  1 1 1 1 1 \ 7
                \ \ \ \ \ 6
                 1 2 3 4 5

  where 1 is black and 2 is white. Note that black is player 1
  and white is player 2.

  -- Nacre::BoardPos conventions ---

  note that Nacre::BoardPos has player 1 at the bottom
  where Abapro2003::Pos has player 1 at the top

  The basis is a square where the corners are cut off.
  each row is numbered 0 to 8 and each diagonal 0 to 8.

  Nacre::BoardPos classic start position:


                 0 1 2 3 4
                / / / / / 5
            0  2 2 2 2 2 / 6
           1  2 2 2 2 2 2 / 7
          2  . . 2 2 2 . . / 8
         3  . . . . . . . . /
        4  . . . . . . . . .
         5  . . . . . . . .
          6  . . 1 1 1 . .
           7  1 1 1 1 1 1
            8  1 1 1 1 1

*/
class Nacre2005Pos: public StreamInterface {
  BoardPos& bp;
public:
  Nacre2005Pos(BoardPos& _bp): bp(_bp) {};
  virtual void Read(istream& in)
  {
    char row, diagonal;
    in >> row >> diagonal;
    bp.y = 'i'-tolower(row);
    bp.x = diagonal-'1' + bp.y - 4;
  }
  virtual void Write(ostream& out) const
  {
    out << (char)('i'-bp.y)
        << bp.x + 1 - bp.y + 4;
  }
};

class Nacre2005: public StreamInterface {
public:
  Nacre2005(Game* _game);
  virtual void Read(istream& in);
  virtual void Write(ostream& out) const;
};

//// Implementation ////////////////////////////////////////////

#ifdef USE_WATERLOO1993
/*
  Waterloo1993 game format EBNF

  game = tags moves
  tags = { tag }
  tag = '[' name ' "' value '"]'
  moves = moveNr move move;
  moveNr = "%d."
  move = pos [ pos ] '-' pos
       | endOfGame
  pos = 'a'..'i' '1'..'9'
  endOfGame = EndOfFile or   double-newline

*/

void LoadProperties(istream& in)
{
  // peek char
  // while char is '['
  //   read line
  //   (format example '[Black "George Smith"]')
  //   property[name]=value;
}

void LoadMove(istream& in)
{
  // See if move is really an end-of-game marker
  if (isdigit(in.peek()) {
    LoadEndOfGame();
    return;
  }
  else if (isEmptyLine(in)) {
    return;
  }
  // LoadTailMarblePos
  if (not Waterloo1993Pos(tail).Read(in)) abort();
  // optional: Read head marble pos
  if (in.peek == '-') {
    head = tail;
  }
  else {
    if (not Waterloo1993Pos(head).Read(in)) abort();
  }
  // skip '-'
  if (not Skip('-',in)) abort();
  // Read new tail marble pos
  if (not Waterloo1993Pos(newTail).Read(in)) abort;
  m = Move(tail,head,newTail);
}

Waterloo1993::Read(istream& in)
{
  LoadProperties(in);
  loop
    SkipMoveNumber
    LoadMove
    LoadMove
}

Waterloo1993::Write(ostream& out) const
{
  // Write properties
  WriteTags(out);

  int moveNr = 0;
  const int players = 2;
  Game g(game);
  for (Game g(game); g.MoreMovesToUndo(); g.Redo()) {
    // If first player to move, then write moveNumber
    if (moveNr % players == 0) {
      out << moveNr/players + 1 << '.' << ' ';
    }
    moveNr ++;
    // Write move
    out << g.
  }
  while (g.MoveMovesToUndo())
}

#endif

void Nacre2005::Read(istream& in) {
  // Read properties
  // Read start position
  // Read moves
}

void Nacre2005::Write(ostream& out) const {
}

#endif

////////////////////////////////////////////////////////////////

void AbaloneGameFormat_Read(istream& in, Game& game);
void AbaloneGameFormat_Write(ostream& out, const Game& game);

#ifndef USE_AG_FORMAT_CLASS
class AbaloneGameFormat_StreamInterface: public StreamInterface {
private:
  Game& theGame;
public:
  AbaloneGameFormat_StreamInterface(Game& game)
  : theGame(game)
  {}
  virtual void Read(istream& in) {
    AbaloneGameFormat_Read(in, theGame);
  }
  virtual void Write(ostream& out) const {
    AbaloneGameFormat_Write(out, theGame);
  }
};

class const_AbaloneGameFormat_StreamInterface: public StreamInterface {
private:
  const Game& theGame;
public:
  const_AbaloneGameFormat_StreamInterface(const Game& game)
  : theGame(game)
  {}
  virtual void Read(istream& in) {
    const bool modify_game_allowed = false;
    TRACE_ASSERT_MSG(modify_game_allowed,
      "You are not allowed to modify a const Game instance"
    );
  }
  virtual void Write(ostream& out) const {
    AbaloneGameFormat_Write(out, theGame);
  }
};

StreamInterface& ag_format(Game& var) {
  return AbaloneGameFormat_StreamInterface(var);
}

const StreamInterface& ag_format(const Game& var) {
  return const_AbaloneGameFormat_StreamInterface(var);
}
#else

ag_format::ag_format(Game& game)
: mutableGame(&game)
, constGame(0)
{ }
ag_format::ag_format(const Game& game)
: mutableGame(0)
, constGame(&game)
{ }
ag_format::ag_format(Game* game)
: mutableGame(game)
, constGame(0)
{ }
ag_format::ag_format(const Game* game)
: mutableGame(0)
, constGame(game)
{ }
void ag_format::Read(std::istream& in) {
  TRACE_ASSERT_MSG(mutableGame != 0,"Cannot modify a const Game");
  AbaloneGameFormat_Read(in, *mutableGame);
}
void ag_format::Write(std::ostream& out) const {
  const Game* game = mutableGame;
  if (game == 0) game = constGame;
  TRACE_ASSERT_MSG(game != 0, "Cannot write a NULL Game");
  AbaloneGameFormat_Write(out, *game);
}
#endif



void SkipWhiteSpace(istream& in)
{
  char c;
  c = in.peek();
  while ((c==' ' or c=='\n' or c=='\r' or c=='\t') and in) {
    in.get(c);
    c = in.peek();
  }
}

/**
  @note In PGN lingo, attribute are named "tag pairs"
  @return 0 = success,
          1 = no more Attributes.
          2 = file empty.
          3 = parse error
*/
int ReadAttribute(istream& in, string& key, string& value) {
  SkipWhiteSpace(in);
  if (in.fail()) return 2; // file empty
  if (in.peek() != '[') return 1; // no more attributes
  string line;
  std::getline(in,line);
  // The following parser is very simple.
  // For a proper implementation of PGN, see the specifications
  // Everything must be on the same line.

  // Find Key (= "tag name")
  unsigned int i = 0;
  if (line[i++] != '[') return 3; // parse error
  unsigned int j = line.find_first_of(" \"",i);
  if (j==string::npos) return 3; // parse error
  key = line.substr(i,j-i);
  TRACE1("key ["<<i<<":"<<j<<"]="<<key);

  // Skip white space
  while (line[j]==' ') j++;
  if (line[j]!='"') return 3; // parse error

  // Find Value (= "tag value")
  j++; // j is now first char of value
  unsigned int k = j;
  do {
    k = line.find_first_of("\\\"",k); // find '"' or '\'
    if (k==string::npos) return 3; // parse error
    if (line[k]=='"') break; // we found the last "
    // the present char is '\', so delete it and skip the next char
    line.erase(k,1);
    k++;
  } while (k < line.length());
  // Since we have deleted escape characters, we can simply copy now
  value = line.substr(j,k-j);
  TRACE1("value ["<<j<<":"<<k<<"]="<<value);
  return 0; // success
}

/**
  Read from a stream into attributes. The format
  is as specified in PGN. Remember to clear() attributes
  before you call this function.
*/
void ReadAttributes(Settings& att, istream& in)
{
  string key, value;
  int errorCode;
  while ( (errorCode = ReadAttribute(in,key,value))
           == 0)
  {
    // Write key-value pair into attributes
    Set(att,key,value);
  }
  TRACE1("ReadAttributes - done with exit code "<<errorCode);
}

void WriteEscaped(const string& str, ostream& out)
{
  out << '"';
  for (unsigned int i=0; i<str.length(); i++) {
    // we must escape " and '\'
    if ((str[i]=='\\') or (str[i]=='\"')) out << '\\';
    out << str[i];
  }
  out << '"';
}

void WriteAttribute(const string& key, const string& value, ostream& out)
{
  out << '[' << key << ' ';
  WriteEscaped(value,out);
  out << ']' << endl;
}

void WriteAttributes(const Settings& att, ostream& out)
{
  const char* seven_tag_roster[] = {
    "Event","Site","Date","Round","Black","White","Result" };

  // Print seven_tag_roster entries first
  Settings::const_iterator i;
  for (int j=0; j<7; j++) {
    i = att.find(seven_tag_roster[j]);
    if (i != att.end()) WriteAttribute(i->first, i->second, out);
  }

  // Print remaining attributes
  for (i = att.begin(); i != att.end(); i++) {
    bool found = false;
    for (int j=0; j<7; j++) {
      if (seven_tag_roster[j] == i->first) found = true;
    }
    if (not found) WriteAttribute(i->first, i->second, out);
  }

  // Print blank line (if any attributes were printed)
  if (att.size() > 0) out << endl;
}

bool expectMoveNumber(istream& in, int move_nr) {
  // TODO: Add validation of input
  if (not in) return false;
  char c;
  SkipWhiteSpace(in);
  in >> c;
  int move_nr_read = 0;
  while (isdigit(c)) {
    move_nr_read = move_nr_read * 10 + (c-'0');
    in >> c;
  }
  in >> c; // skip trailing '.'
  return move_nr_read == move_nr;
}

// Convert Abalone standard coordinate to BoardPos=Board2D::Pos
// @note row must be a lower-case letter
bool parse_ab_pos(char row, char col, Board2D::Pos& bp) {
  if (not ('a'<=row and row<='i' and '1'<=col and col<='9')) return false;
  bp.y = 'i'-row;
  bp.x = col - '1';
  return true;
}

/** convert a FFTL move to Board2D::Move.
  @param board  Board before move is done
  @param from_first  First marble to move
  @param to_last  Destination of last player marble.
    Note that some opponent marbles may also be moved, in which case this
    is the first opponetn marble to push.
  @param move  Resulting Board2D::Move
  @return false, if it was not possible to do a valid move. */
bool convert_ab_move(
  const Board2D& board,
  const Board2D::Pos from_first,
  const Board2D::Pos to_last,
  Board2D::Move& move)
{
  move.head = from_first;
  int8 dx, dy;
  dx = to_last.x - from_first.x;
  dy = to_last.y - from_first.y;
  if (not (-4 <= dx and dx <= 4 and -4 <= dy and dy <= 4)) {
    TRACE("convert_ab_move dx,dy="<<dx<<","<<dy<<" -> invalid move");
    return false;
  }

  const int x = -1; // invalid move
  const int y = -2; // small broadside move
  // For small broadside moves, initialise tail with left-most direction
  const int tailDir[9][9] = {
  {x,x,x,x,x,x,x,x,x        },
  { x,x,x,x,4,4,5,5,x       },
  {  x,x,x,4,4,4,5,5,x      },
  {   x,x,3,3,4,5,5,0,x     },
  {    x,3,3,3,x,0,0,0,x    },
  {     x,3,2,2,1,0,0,x,x   },
  {      x,2,2,1,1,1,x,x,x  },
  {       x,2,2,1,1,x,x,x,x },
  {        x,x,x,x,x,x,x,x,x}
  };
  const int tailCount[9][9] = {
  {x,x,x,x,x,x,x,x,x        },
  { x,x,x,x,3,3,3,3,x       },
  {  x,x,x,3,2,2,2,3,x      },
  {   x,x,3,2,1,1,2,3,x     },
  {    x,3,2,1,x,1,2,3,x    },
  {     x,3,2,1,1,2,3,x,x   },
  {      x,3,2,2,2,3,x,x,x  },
  {       x,3,3,3,3,x,x,x,x },
  {        x,x,x,x,x,x,x,x,x}
  };
  const int moveDir[9][9] = {
  {x,x,x,x,x,x,x,x,x        },
  { x,x,x,x,4,5,4,5,x       },
  {  x,x,x,3,4,y,5,0,x      },
  {   x,x,4,y,4,5,y,5,x     },
  {    x,3,3,3,x,0,0,0,x    },
  {     x,2,y,2,1,y,1,x,x   },
  {      x,3,2,y,1,0,x,x,x  },
  {       x,2,1,2,1,x,x,x,x },
  {        x,x,x,x,x,x,x,x,x}
  };


  move.tailDir   = tailDir  [dy+4][dx+4];
  move.tailCount = tailCount[dy+4][dx+4];
  move.moveDir   = moveDir  [dy+4][dx+4];
  if (move.moveDir == -2) {
    // short broadside move - must check board
    Board2D::Pos p = move.head;
    p.Step(move.tailDir);
    if (board.At(p) != fEmpty) {
      // Tail is to the left, movedir to the right
      move.moveDir = Clockwise(move.tailDir);
    }
    else {
      // movedir is to the left, tail is to the right
      move.moveDir = move.tailDir;
      move.tailDir = Clockwise(move.tailDir);
    }
  }
  if (not (move.tailDir != -1 and move.moveDir != -1)) {
    TRACE("convert_ab_move "
    <<"dx,dy="<<dx<<","<<dy<<" -> "
    <<"tailDir="<<move.tailDir<<",moveDir="<<move.moveDir
    <<" -> invalid move");
    return false;
  }
  // This should never happen. tailCount is -1 only when tailDir is -1
  TRACE_ASSERT_MSG(move.tailCount != -1, "convert_ab_move "
    <<"dx,dy="<<dx<<","<<dy<<" -> "
    <<"tailCount="<<move.tailCount
    <<" -> invalid move");

  return true;
}

/** Do one FFTL move on a game. */
bool doFFTL(Game& game,  char fa, char f1, char ta, char t1) {
  if (fa<'a' or 'i'<fa) return false; // Parse error
  if (f1<'1' or '9'<f1) return false; // Parse error
  if (ta<'a' or 'i'<ta) return false; // Parse error
  if (t1<'1' or '9'<t1) return false; // Parse error
  Board2D::Pos from_first;
  parse_ab_pos(fa,f1,from_first);
  if (not from_first.Valid()) return false; // Invalid move
  Board2D::Pos to_last;
  parse_ab_pos(ta,t1,to_last);
  if (not to_last.Valid()) return false; // Invalid move
  Board2D::Move move;
  if (not convert_ab_move(game.board,from_first,to_last,move)) return false;
  if (game.DoMove(move) != 0) return false; // Invalid move
  return true;
}

class GameParser {
  istream& m_in;
  string m_cur_tok;
public:
  GameParser(istream& in): m_in(in) {}
  bool eof() const { return !m_in.good(); }
  string cur_tok() const { return m_cur_tok; };
  void next_tok(string delim = " \t\n\v\f\r") {
    char c;
    // skip whitespace
    do {
      c = (char)m_in.get();
      if (!m_in.good()) return;
    } while (delim.find(c));
    // read more if alpha-num
    m_cur_tok = c;
    while (!isalnum(m_in.peek())) {
      m_cur_tok += (char)m_in.get();
      if (!m_in.good()) return;
    }
    m_in.putback(c);
  }
};

/** Parse one FFTL move and add to game. */
bool parseMove(string s, Game& game) {
  return doFFTL(game,s[0],s[1],s[2],s[3]);
}

void AbaloneGameFormat_ReadGameTree(Game& game, GameParser& p)
{
  // next token: next_tok, cur_tok
  if (p.eof()) return;
  if (p.cur_tok() == ")") return;
  TRACE_ASSERT(parseMove(p.cur_tok(),game));
  p.next_tok();
  if (p.cur_tok() == "{") {
    // Read comment
    p.next_tok("}");
    game.SetComment(p.cur_tok());
    p.next_tok();
    TRACE_ASSERT(p.cur_tok()=="}");
    p.next_tok();
  }
  if (p.cur_tok() == "(") {
    // Read variation
    Board2D::Move move = game.PrevMove();
    game.UndoMove();
    p.next_tok();
    // NOTE: Semantics may be wrong: Multiple same moves will collapse
    AbaloneGameFormat_ReadGameTree(game,p);
    TRACE_ASSERT(p.cur_tok()==")");
    p.next_tok();
    TRACE_ASSERT(game.DoMove(move)==0);
  }
  // Parse remaining principal variation
  AbaloneGameFormat_ReadGameTree(game,p);
  game.UndoMove();
}

/** Read one FFTL move from stream and add to game. */
bool readMove(istream& in, Game& game) {
  if (not in) return false;
  SkipWhiteSpace(in);
  char fa,f1,ta,t1;
  in >> fa >> f1 >> ta >> t1;
  if (not in) return false; // EOF
  return doFFTL(game,fa,f1,ta,t1);
}

void AbaloneGameFormat_Read(istream& in, Game& game) {
  Settings attributes;
  ReadAttributes(attributes,in);

  Board startPos;
  startPos.Read(in);
  game.RestartFrom(startPos); // will also clear attributes

  game.attributes = attributes;

  int move_nr = 0;
  while (in) {
    Board2D::Move move;
    if (!expectMoveNumber(in,++move_nr)) break;
    if (!readMove(in,game)) break;
    if (!readMove(in,game)) break;
  }
  // Rewind game to start
  game.UndoAllMoves();
}

void AbaloneGameFormat_Write(ostream& out, const Game& const_game) {
  WriteAttributes(const_game.attributes,out);
  const_game.board.Write(out);
  Game game(const_game);
  // TODO: This is only for sequental games
  game.UndoAllMoves();
  while (game.MoreMovesToRedo()) {
    game.NextMove().Write(out);
    game.RedoMove();
  }
  out << endl; // extra newline to mark end of game
}


#ifdef HAVE_CPPUNIT


/**
  Test the Abalone Portable Notation implementation
*/
class APN_Test : public CppUnit::TestCase {
public:
  CPPUNIT_TEST_SUITE( APN_Test );
    CPPUNIT_TEST( testSimpleFile );
  CPPUNIT_TEST_SUITE_END();

  void testSimpleFile() {
    TRACE("+testSimpleFile" << __func__);

    const char* simpleFile=
      "[GameType \"Abalone-v1\"]\n";

    #if 0
    Game game;
    PortableGameNotation png;
    stringstream buf(simpleFile);
    pgn.read(buf,game);
    buf >> pgn(game);
    CPPUNIT_ASSERT( game.Attribute("GameType") == "Abalone-v1");
    #else
    TRACE("*** NOT IMPLEMENTED");
    #endif
    TRACE("-testSimpleFile");
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION( APN_Test );

#endif


////////////////////////////////////////////////////////////////
//
//  Board and move notation
//

/** Build a string of pos using the AbalonePositionFormat.
  @todo Move this function to abmove
*/
string apf(const Game& game) {
  // Exploit that Board2D::Pos::Next() will run through positions in the same
  // order as is needed for apf. For safety I have added a TRACE_ASSERT that
  // will discover buffer overflows as result of a changed algorithm, but not
  // all algorithm changes.
  string result = "xxxxx xxxxxx xxxxxxx xxxxxxxx xxxxxxxxx xxxxxxxx xxxxxxx xxxxxx xxxxx x";
  Board2D::Pos pos;
  pos.Next();
  int last_y = pos.y;
  unsigned i = 0;
  while (pos.Valid()) {
    if (pos.y != last_y) result[i++] = ' ';
    switch (game.board.At(pos)) {
      case 0: result[i++] = '.'; break;
      case 1: result[i++] = '1'; break;
      case 2: result[i++] = '2'; break;
      default:
        bool board_At_in_range = false;
        TRACE_ASSERT_MSG(board_At_in_range,
          result << endl << i << endl << game.board.At(pos)
        );
    }
    last_y = pos.y;
    pos.Next();
    TRACE_ASSERT(i < result.size());
  }
  result[i++] = ' ';
  result[i++] = '0' + game.board.GetTurn();
  TRACE_ASSERT(i == result.size());
  return result;
  //return "22.11 222111 .22.11. ........ ......... ........ .11.22. 111222 11.22 1";
}

/** Print a move using the from-first-to-last notation
 @note  The notation used is selected in Board.cpp
*/
static string fftl(const Move& move) {
  stringstream result;
  result << move;
  return result.str();
}

/** Parse a move using the from-first-to-last notation
 @note  The notation used is selected in Board.cpp
 @bug Will fail on assertion if move is not valid
*/
bool parse_fftl(const string& str, const Board2D& board, Move& move) {
  if (str.length() != 4) return false;
  Board2D::Pos ff,tl;
  if (not parse_ab_pos(str[0],str[1],ff)) return false;
  if (not parse_ab_pos(str[2],str[3],tl)) return false;
  if (not convert_ab_move(board,ff,tl, move)) return false;
  return true;
}

/** Build a string of move sequence */
string MoveList(const Game& game) {
  std::vector<Board2D::Move> moves = game.CurrentMoves();
  string result;
  for (unsigned i = 0; i < moves.size(); i ++) {
    if (i > 0) result += " ";
    result += fftl(moves[i]);
  }
  return result;
}
