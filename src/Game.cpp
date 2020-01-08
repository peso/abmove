/** @file Game.cpp
 Haliotis, a library for Abalone playing programs.
 Board2D class
 
 This module defines the Board2D class which holds an abalone board.
 
  Copyright (C) 2003 Peer Sommerlund

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

#include "Game.hpp"
#include <cctype> // isspace, isalnum
#include <iostream>
using std::istream;
using std::ostream;
using std::endl;
using std::make_pair;
using std::vector;

#include "config.h"
#undef HAVE_CPPUNIT

#define DEB1
#include "Trace.hpp"
#define TRACE1(x) // TRACE(x)

#ifdef HAVE_CPPUNIT
#include <cppunit/TestSuite.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/ui/text/TestRunner.h>
#include <sstream>
using std::stringstream;
#endif

#include "Persistence.hpp"

//// implementation ////////////////////////////////////////////

namespace Haliotis {

































#ifndef TODO_REMOVE_CODE
/** Print a Nacre Game on a stream, using the Nacre notation.
  @note ONLY FOR TRACING, since not all information of game is stored.
*/
ostream& operator << (ostream& out, const Game& game) {
  TRACE1("operator << (ostream, Game)");
  Game myGame(game);
  TRACE1(__LINE__);
  int lastBoardNumber = game.CurrentBoardNumber();
  TRACE1(__LINE__);
  myGame.UndoAllMoves();
  TRACE1(__LINE__);
  for (int i=0; i<lastBoardNumber; i++) {
    TRACE1("i="<<i);
    if (i != 0) out << ' ';
    if (i%2 == 0) out << (i/2)+1 << ". ";
    TRACE1("NextMove");
    out << myGame.NextMove();
    TRACE1("RedoMove");
    myGame.RedoMove();
  }
  return out;
}
#endif


////////////////////////////////////////////////////////////////
//
// GameTreeNode

struct GameTreeNode;

/** Navigate through nodes in a game tree.
  @see GameTreeNode */
class GameTreeNavigator {
  int parent_no;
  GameTreeNode* node;
public:
  GameTreeNavigator(GameTreeNode* _node);
  ~GameTreeNavigator();
  void next();
  bool more();
  void operator ++ (int) { next(); }
  void operator ++ () { next(); }
  operator Board2D::Move();
  int MovesFromStart() const;
};

/** Representation of a position in a game tree.

  @bug Note that this is a tree, not
  a graph, so two equal positions may occur in the same game tree (given that
  they were reached by different paths).
*/
struct GameTreeNode {
  /// Comment to current position or move that lead to current position.
  ::std::string comment;
  GameTreeNode* prev;
  /// Main line continues here. Next move is next->move
  GameTreeNode* next;
  /// Alternative moves
  GameTreeNode* alt;
  /**
    Move that lead to the current position. This GameTreeNode represents the
    position AFTER this move.
    @note Moves must be normalised before adding to GameTreeNode
  */
  Board2D::Move move;
public:
  GameTreeNode();
  // Create copy of other tree
  GameTreeNode(const GameTreeNode* orig, GameTreeNode* new_prev=0);
  ~GameTreeNode();
  GameTreeNode* FindNode(Move move);
  GameTreeNode* GetNode(Move move);
  GameTreeNode* FindNextNode(Move move);
  GameTreeNode* GetNextNode(Move move);
  static void RemoveNode(GameTreeNode*& nodePtr);
  Board2D::Move UndoMove() const;
  #if 0
  GameTreeNavigator MovePath();
  // The following two should probably not be here but belong in
  // separate functions in Persistence.cpp
  void Read(istream& in);
  void Write(ostream& out) const;
  #endif
};

GameTreeNode::GameTreeNode()
  : prev(0), next(0), alt(0)
{
}

GameTreeNode::GameTreeNode(const GameTreeNode* orig, GameTreeNode* new_prev)
  : prev(0), next(0), alt(0)
{
  move = orig->move;
  comment = orig->comment;
  prev = new_prev;
  if (orig->next) next = new GameTreeNode(orig->next, this);
  if (orig->alt) alt = new GameTreeNode(orig->alt, new_prev);
}

GameTreeNode::~GameTreeNode()
{
  prev = 0;
  delete alt; alt = 0;
  delete next; next = 0;
}

/**
  Usage: curPos->next->FindNode(move) --
  since curPos is the position AFTER the current move was done
*/
GameTreeNode* GameTreeNode::FindNode(Move move)
{
  for (GameTreeNode* i = this; i != 0; i = i->alt) {
    if (i->move == move) return i;
  }
  return 0;
}

#if 0
/** Find the move that led to this node */
Board2D::Move GameTreeNode::UndoMove() const
{
  if (prev == 0) return Board2D::Move::Move(); // invalid move
  return prev->FindMove(this);
}
#endif

/**
  Find move amongst alternatives, after previous position. If move is new, it
  is added to the tree.
  Usage: curPos->next->GetNode(move) --
  since curPos is the position AFTER the current move was done
*/
GameTreeNode* GameTreeNode::GetNode(Move move)
{
  TRACE1("+GetNode");
  for (GameTreeNode* i = this; i != 0; i = i->alt) {
    TRACE1("GetNode - at "<< i<<" move="<<i->move);
    if (i->move == move) {
      TRACE1("-GetNode - Found move");
      return i;
    }
    if (i->alt == 0) {
      TRACE1("-GetNode - New alternative move");
      GameTreeNode* result = new GameTreeNode();
      TRACE_ASSERT(result != 0);
      result->move = move;
      result->prev = prev;
      i->alt = result;
      return result;
    }
  }
  // You have called this function on a null pointer
  return 0;
}

/**
  Go to position that follows the given move. 
  If move is new, it is not added to the tree.
  Usage: curPos->FindNextNode(move)
*/
GameTreeNode* GameTreeNode::FindNextNode(Move move)
{
  TRACE1("+FindNextNode");
  if (next == 0) {
    // No next move
    TRACE1("-FindNextNode - no next move");
    return 0;
  }
  return next->FindNode(move);
}

/**
  Go to position that follows the given move. If move is new, it is added
  to the tree.
  Usage: curPos->GetNextNode(move)
*/
GameTreeNode* GameTreeNode::GetNextNode(Move move)
{
  TRACE1("+GetNextNode");
  if (next == 0) {
    // No next move - append move
    TRACE1("-GetNextNode - New next move");
    GameTreeNode* result = new GameTreeNode();
    TRACE_ASSERT(result != 0);
    result->move = move;
    result->prev = this;
    next = result;
    return result;
  }
  return next->GetNode(move);
}

void GameTreeNode::RemoveNode(GameTreeNode*& nodePtr)
{
  if (nodePtr == 0) return;
  GameTreeNode* p = nodePtr;
  nodePtr = nodePtr->alt;
  delete p;
}

#define HIDE_GameTreeNode_Read
#ifndef HIDE_GameTreeNode_Read
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

bool is_move(const string& str) {
  if (str.length() != 4) return false;
  return true;
}

void GameTreeNode_Read(GameTreeNode*& node, GameParser& p)
{
  // next token: next_tok, cur_tok
  if (p.eof()) return;
  if (p.cur_tok() == ")") return;
  TRACE_ASSERT(is_move(p.cur_tok()));
  node->move = parser_move(p.cur_tok());
  p.next_tok();
  if (p.cur_tok() == "{") {
    // Read comment
    p.next_tok('}');
    node->comment = p.cur_tok();
    p.next_tok();
    TRACE_ASSERT(p.cur_tok()=="}");
    p.next_tok();
  }
  if (p.cur_tok() == "(") {
    // Read variation
    p.next_tok();
    GameTreeNode_Read(node->alt,p);
    TRACE_ASSERT(p.cur_tok()==")");
    p.next_tok();
  }
  // Parse remainder
  GameTreeNode_Read(node->next,p);
}
#endif

void GameTreeNode_Read(GameTreeNode*& node, istream& in)
{
  #ifdef HIDE_GameTreeNode_Read
  bool GameTreeNode_Read_implemented = false;
  TRACE_ASSERT(GameTreeNode_Read_implemented);
  #else
  GameParser parser(in);
  GameTreeNode_Read(node,parser);
  #endif
}

// TODO: Add move numbers
void GameTreeNode_Write(const GameTreeNode* node, ostream& out)
{
  if (node == 0) return;
  out << node->move;
  if (not node->comment.empty()) out << " {" << node->comment << "}";
  // Write all variations before continuing main line
  for (const GameTreeNode* n = node->alt; n != 0; n = n->alt) {
    Move move = n->move;
    out << " (" << move;
    if (not n->comment.empty()) out << " {" << n->comment << "}";
    if (n->next) { out << " "; GameTreeNode_Write(n->next, out); }
    out << ")";
  }
  // Continue main line
  if (node->next) {
    out << " ";
    GameTreeNode_Write(node->next, out);
  }
}

ostream& operator << (ostream& out, GameTreeNode* node) {
  out << "[";
  GameTreeNode_Write(node,out);
  out << "]";
  return out;
}

#ifdef HAVE_CPPUNIT

class GameTreeNodeTest : public CppUnit::TestCase {
public:
  CPPUNIT_TEST_SUITE( GameTreeNodeTest );
    CPPUNIT_TEST( testWrite_empty );
    CPPUNIT_TEST( testWrite_noBranch );
    CPPUNIT_TEST( testWrite_branches );
  CPPUNIT_TEST_SUITE_END();

  void testWrite_empty() {
    TRACE(__FILE__ " +GameTreeNodeTest::" << __func__);
    GameTreeNode* root = 0;
    Board2D board;
    Board2D::Move m;

    { // test empty game
    TRACE("test empty game");
    stringstream out;
    GameTreeNode_Write(root,out);
    string expectedResult = "";
    TRACE( out.str() << " ?== " << expectedResult);
    CPPUNIT_ASSERT_EQUAL( expectedResult , out.str() );
    }

    TRACE(__FILE__ " -GameTreeNodeTest::" << __func__);
  }
  void testWrite_noBranch() {
    TRACE(__FILE__ " +GameTreeNodeTest::" << __func__);
    GameTreeNode* root = 0;
    Board2D board;
    Board2D::Move m;

    { // test game without branches
    TRACE("test game without branches");
    board.SetUp(BelgianDaisy);
    GameTreeNode** n = &root;
    for (int i=0; i<3; i++) {
      board.FirstMove(m);
      board.DoMove(m);
      *n = new GameTreeNode();
      (*n)->move = m;
      n = &((*n)->next);
    }
    stringstream out;
    GameTreeNode_Write(root,out);
    string expectedResult = "1.a3a3 b3b4 2.a2a3";
    TRACE( out.str() << " ?== " << expectedResult);
    CPPUNIT_ASSERT_EQUAL( expectedResult , out.str() );
    }

    TRACE(__FILE__ " -GameTreeNodeTest::" << __func__);
  }
  void testWrite_branches() {
    TRACE(__FILE__ " +GameTreeNodeTest::" << __func__);
    GameTreeNode* root = 0;
    Board2D board;
    Board2D::Move m;

    { // test game with multiple branches
    TRACE("test game with multiple branches");
    board.SetUp(BelgianDaisy);
    GameTreeNode** n = &root;
    for (int i=0; i<3; i++) {
      board.FirstMove(m);
      // Add primary move
      *n = new GameTreeNode();
      (*n)->move = m;

      // Add alternate move
      Move m2 = m;
      board.NextMove(m2);
      (*n)->alt = new GameTreeNode();
      (*n)->alt->move = m2;

      // Next board along primary move
      board.DoMove(m);
      n = &((*n)->next);
    }
    stringstream out;
    GameTreeNode_Write(root,out);
    string expectedResult = "1.a3a3 b3b4 2.a2a3";
    TRACE( out.str() << " ?== " << expectedResult);
    CPPUNIT_ASSERT_EQUAL( expectedResult , out.str() );
    }

    TRACE(__FILE__ " -GameTreeNodeTest::" << __func__);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION( GameTreeNodeTest );

#endif


/*---- Game ----------------------------------------------------*/

/* A Game is a sequence of moves. It is possible to have a view
on the game, this view has a state which is the current move.
This is visualised as a Board. It is possible to move back
and forth in the Game. It is also possible to extend the game
by adding a new move at the end. This can only be done when the board
visualises the board with an attaches list of moves. This means
  that the game has a view, which is the current board position
  and a list of moves

  A game can be saved to a stream and loaded again.
*/

Game::Game() /* Create */
: board(currentBoard)
, moveTree(0)
, currentPosition(0)
{
  currentBoard.SetUpStartPos();
  startPos=board;
}

Game::Game(const Game& orig) /* Copy */
: board(currentBoard)
, moveTree(0)
, currentPosition(0)
{
  *this = orig;
}

Game::Game(Board aBoard) // RestartFrom
: board(currentBoard)
, moveTree(0)
, currentPosition(0)
{
  startPos=aBoard;
  currentBoard=startPos;
}

Game::~Game()
{
  delete moveTree;
  moveTree = 0;
  currentPosition = 0;
}

const Game& Game::operator = (const Game& orig) {
  // Set up initial board
  RestartFrom(orig.startPos);

  // Copy nodes in game tree
  if (orig.moveTree) {
    moveTree = new GameTreeNode(orig.moveTree);

    // Find position in game
    vector<Board2D::Move> movePath = orig.CurrentMoves();
    for (vector<Board2D::Move>::iterator i
      = movePath.begin(); i != movePath.end(); i ++)
    {
      TRACE_ASSERT_MSG(DoMove(*i) == 0, "move="<< *i << endl<<board);
    }
  }

  // Copy attributes
  attributes = orig.attributes;

  return *this;
}

/** Clear all data, and set up a new start position. */
void Game::RestartFrom(Board aBoard) {
  startPos = aBoard;
  currentBoard = startPos;
  delete moveTree;
  moveTree = 0;
  currentPosition = 0;
  attributes.clear();
}

/** Compute number of moves used to reach the current position from startPos.*/
int Game::CurrentBoardNumber() const
{
  int movesFromStart = 0;
  for (GameTreeNode* p = currentPosition; p != 0; p = p->prev) {
    movesFromStart ++;
  }
  return movesFromStart;
}

/** Generate the list of moves used to reach the current position
  @return List of moves from startPos to curPos
*/
::std::vector<Board2D::Move> Game::CurrentMoves() const
{
  int moves = CurrentBoardNumber();
  ::std::vector<Board2D::Move> moveList(moves);
  for (GameTreeNode* p = currentPosition; p != 0; p = p->prev) {
    moveList[--moves] = p->move;
  }
  return moveList;
}

bool Game::BoardAlreadySeen(const Board& aBoard) const
{
  Board curBoard(startPos);

  // Compare the board with all boards prior to the current board
  ::std::vector<Board2D::Move> moveList(CurrentMoves());
  for (::std::vector<Board2D::Move>::iterator i =
    moveList.begin(); i != moveList.end(); i ++)
  {
    if (curBoard == aBoard) return true;
    TRACE_ASSERT(curBoard.DoMove(*i)==0);
  }

  TRACE_ASSERT_MSG(curBoard == board, "Game::BoardAlreadySeen\n"
    << "moveList.size()=" << moveList.size()
    << "\ncurrent board\n" << curBoard
    << "\ntarget board\n" << board);
  return curBoard == aBoard;
}

/**
  Do a move on the game board. This function also maintains the move tree,
  and automatically expands the tree if a new variant is started in the
  middle of a game. The tree can hold several variants.
  @param move  move to do at the current board
  @pre move  must be a valid normalised move
*/
int Game::DoMove(Move move)
{
  TRACE1("+Game::DoMove");

  //
  // Validate move and update board
  //

  int err = currentBoard.DoMove(move);
  if (err) {
    TRACE1("-Game::DoMove - invalid move rejected (code "<<err<<")");
    return err;
  }

  //
  // Update move tree
  //

  if (currentPosition != 0) {
    // Inside the move tree
    TRACE1("Game::DoMove - moveTree = " << moveTree);
    currentPosition = currentPosition->GetNextNode(move);
  }
  else if (moveTree != 0) {
    // At game start
    currentPosition = moveTree->GetNode(move);
  }
  else {
    // Empty moveTree
    moveTree = new GameTreeNode();
    moveTree->move = move;
    currentPosition = moveTree;
  }
  // We are not a game start, so we must be inside the moveTree
  TRACE_ASSERT(currentPosition != 0);

  TRACE1("-Game::DoMove - moveTree = " << moveTree);
  return 0;
}

/**
  Redo a move on the game board. Only moves that are already present in
  the move tree are valid.
  @param move  move to do at the current board
  @pre move  must be a valid normalised move in the game tree
  @return 0 on success, 
          1 if the move was not found in the move tree at the current node.
          2 if the move was invalid
*/
int Game::RedoMove(Move move)
{
  TRACE1("+Game::RedoMove");

  //
  // Update move tree
  //

  if (currentPosition != 0) {
    // Inside the move tree
    TRACE1("Game::RedoMove - moveTree = " << moveTree);
    currentPosition = currentPosition->GetNextNode(move);
  }
  else if (moveTree != 0) {
    // At game start
    currentPosition = moveTree->GetNode(move);
  }
  else {
    // Empty moveTree
    return 1;
  }
  // We are not a game start, so we must be inside the moveTree
  TRACE_ASSERT(currentPosition != 0);

  //
  // Validate move and update board
  //

  int err = currentBoard.DoMove(move);
  if (err) {
    TRACE1("-Game::DoMove - invalid move rejected (code "<<err<<")");
    return -2;
  }

  TRACE1("-Game::DoMove - moveTree = " << moveTree);
  return 0;
}

Move Game::PrevMove() const
{
  if (currentPosition == 0) return Move();
  return currentPosition->move;
}

Move Game::NextMove() const
{
  if (not MoreMovesToRedo()) return Move();
  return currentPosition->next->move;
}

/**
  Return all registered moves at the current board.
  @note To do one of these moves, use DoMove.
*/
vector<Board2D::Move> Game::AlternateMoves() const
{
  ::std::vector<Board2D::Move> moveList;
  GameTreeNode* first;
  if (currentPosition == 0) first = moveTree;
  else first = currentPosition->next;
  for (GameTreeNode* p = first; p != 0; p = p->alt) {
    moveList.push_back(p->move);
  }
  return moveList;
}

/**
  Will redo the main line of moves.
  @see Game::AlternateMoves for instruction on how to redo variants.
*/
void Game::RedoMove()
{
  GameTreeNode* next;
  if (currentPosition == 0) next = moveTree;
  else next = currentPosition->next;
  if (next == 0) return;
  currentPosition = next;
  TRACE_ASSERT(currentBoard.DoMove(currentPosition->move)==0);
  return;
}

void Game::UndoMove()
{
  if (currentPosition == 0) return;
  // board.UndoMove(currentPosition->move); -- not possible, see below
  currentPosition = currentPosition->prev;

  /* The code below is because the current move representation
    does not support undo. Solution: Redo all moves from start */
  currentBoard=startPos;
  ::std::vector<Board2D::Move> moveList(CurrentMoves());
  for (::std::vector<Board2D::Move>::iterator i =
    moveList.begin(); i != moveList.end(); i ++)
  {
    TRACE1(__func__<<endl<<board<<endl<<"DoMove "<<*i);
    TRACE_ASSERT(currentBoard.DoMove(*i) == 0);
  }
}

void Game::UndoAllMoves()
{
  currentBoard = startPos;
  currentPosition = 0;
}

bool Game::MoreMovesToUndo() const
{
  return currentPosition != 0;
}

bool Game::MoreMovesToRedo() const
{
  if (currentPosition == 0) return moveTree != 0;
  return currentPosition->next != 0;
}

/**
  Compute length of main variation
*/
int Game::Length() const {
  int len=0;
  for (GameTreeNode* p = moveTree; p != 0; p = p->next) {
    len ++;
  }
  return len;
}

bool Game::WhiteMovesFirst() const
{
  return startPos.whiteToMove;
}

/**
  Return the comment at the current board.
*/
string Game::GetComment() const
{
  TRACE_ASSERT(currentPosition!=0);
  return currentPosition->comment;
}

void Game::SetComment(const string& comment)
{
  TRACE_ASSERT(currentPosition!=0);
  currentPosition->comment = comment;
}

#if 0
// TODO: Move this code to Persistence.cpp
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
  ::std::getline(in,line);
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
#endif

/**
  Read game in Abalone Game Format from stream
*/
void Game::Read(istream& in)
{
  attributes.clear();
  ReadAttributes(attributes,in);

  startPos.Read(in);
  GameTreeNode_Read(moveTree,in);
  // Move current move to beginning, and copy startPos to current board
  UndoAllMoves();
}

/**
  Write game in Abalone Game Format to stream
*/
void Game::Write(ostream& out) const
{
  WriteAttributes(attributes,out);
  startPos.Write(out);
  GameTreeNode_Write(moveTree,out);
  out << endl;
}

} // namespace Haliotis
