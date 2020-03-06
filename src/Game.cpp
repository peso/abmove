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
  if (currentPosition == 0) return moveTree->move;
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
  if (currentPosition==0) return startComment;
  else return currentPosition->comment;
}

void Game::SetComment(const string& comment)
{
  if (currentPosition==0) startComment = comment;
  else currentPosition->comment = comment;
}

/**
  Read game in Abalone Game Format from stream
*/
void Game::Read(istream& in)
{
  AbaloneGameFormat_Read(in, *this);
}

/**
  Write game in Abalone Game Format to stream
*/
void Game::Write(ostream& out) const
{
  AbaloneGameFormat_Write(out, *this);
}

} // namespace Haliotis
