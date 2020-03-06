/* Class Game - An abalone game tree
 *
 * Peer Sommerlund, 2003-Maj-07
*/

#ifndef _GAME_HPP_
#define _GAME_HPP_

#include "abmove.h"

#include <string>
#include <iostream>
#include <vector>
#include "Board2D.hpp"

namespace Haliotis {

//////////////////////////////////////////////////////////////////////

struct GameTreeNode;

class HALIOTIS_EXPORT Game {
public:
  const Board2D& board; //< Current board, as seen after move done by currentPosition
  Settings attributes;
private:
  Board2D startPos;
  std::string startComment;
  Board2D currentBoard; //< Current board, as seen after move done by currentPosition
  GameTreeNode* moveTree;
  GameTreeNode* currentPosition;
public:
  Game();
  Game(const Game& g);
  Game(Board2D aBoard);
  ~Game();
  const Game& operator = (const Game&);
  void RestartFrom(Board2D aBoard);
  void Read(istream& in);
  void Write(ostream& out) const;
  int DoMove(Board2D::Move M);
  int RedoMove(Board2D::Move M);
  void UndoMove();
  void UndoAllMoves();
  /// Redo main line move
  void RedoMove();
  /// From current position, have we reached the end of the primary variation?
  bool MoreMovesToUndo() const;
  /// Are we at the root
  bool MoreMovesToRedo() const;
  Board2D::Move PrevMove() const;
  Board2D::Move NextMove() const;
  std::vector<Board2D::Move> AlternateMoves() const;
  const Board2D& CurrentBoard() const { return board; }
  int CurrentBoardNumber() const;
  std::vector<Board2D::Move> CurrentMoves() const;
  bool BoardAlreadySeen(const Board2D& aBoard) const;
  /// @deprecated Does not make sense for a tree representation
  int Length() const;
  /// @deprecated
  bool WhiteMovesFirst() const;
  std::string GetComment() const;
  void SetComment(const std::string& comment);
  const Board2D& StartPos() const { return startPos; }
};

/////////////////////////////////////////////////////////////////////////

/** Print a Nacre Game on a stream, using the Nacre notation. */
HALIOTIS_EXPORT std::ostream& operator << (std::ostream& out, const Game& game);

};


#endif

