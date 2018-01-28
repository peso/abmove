/* Class Board2D - represents an abalone board (a game state)
 *
 * Peer Sommerlund, 2003-Maj-07
*/

#ifndef _BOARD2D_HPP_
#define _BOARD2D_HPP_

#include "abmove.h"
#include <iso646.h>

//#include "config.h"

#include <string>
#include <iostream>
#include <vector>

using std::string;
using std::istream;
using std::ostream;

// TODO: Refactor this library away
#include "Settings.hpp"

// This macro can be defined if you want something to happen
// on range error in Board2D::Pos::Step() -- currently it is null
#define RANGE_CHECK_ERROR(ax)


typedef short int int8;

namespace Haliotis {

template <class T>
static inline T Abs(T x) { return x<0 ? -x : x; }

template <class T>
static inline T Max(T a, T b) { return a>b ? a : b; }

template <class T>
static inline T Min(T a, T b) { return a<b ? a : b; }

//////////////////////////////////////////////////////////////////////

/** A Hexagonal direction. There are six different directions */
typedef int8 Direction;
// Range: -1..5
//  Illegal, E, S, SW, W, N, NE
const Direction dirNone=-1;
const Direction dirRight=0;
const Direction dirRightDown=1;
const Direction dirLeftDown=2;
const Direction dirLeft=3;
const Direction dirLeftUp=4;
const Direction dirRightUp=5;
  //  4 5
  // 3 * 0
  //  2 1

/// Examine if two directions are parallel
inline bool Parallel(int dir1, int dir2) {
  return (dir2-dir1) % 3 == 0;
}

/// Return the opposite direction
inline int Opposite(int dir) {
  return (dir + 3) % 6;
}

/// Return the clockwise direction
inline int Clockwise(int dir) {
  return (dir + 1) % 6;
}


//////////////////////////////////////////////////////////////////////

/* Values used in Board.field */
const int fEmpty=0;
const int fPieceWhite=1;
const int fPieceBlack=2;

typedef int const BoardGrid[9][9];
HALIOTIS_EXPORT extern BoardGrid BelgianDaisy;
HALIOTIS_EXPORT extern BoardGrid DutchDaisy;
HALIOTIS_EXPORT extern BoardGrid GermanDaisy;
HALIOTIS_EXPORT extern BoardGrid SwissDaisy;
HALIOTIS_EXPORT extern BoardGrid TheWall;

/** A board. This represents one position in a game. It contains
 information on who is to move. */
class HALIOTIS_EXPORT Board2D {
  public:
    const static int PLAYERS = 2;
    struct Pos;
    class Move;
    void InitFieldKey();
    /// @deprecated
    bool whiteToMove;
    // range: 0..2
    int8 field[9][9];
    int8 At(Board2D::Pos BP) const;
    void SetBoardPos(Board2D::Pos BP, int8 FieldValue);
    bool MyPiece(int8 F) const;
    void SetUpStartPos();
    void SetUp(BoardGrid grid);
    void Read(istream& in);
    void Write(ostream& out) const;
    void FirstMove(Move& M) const;
    bool FirstMove(Move& M, Board2D& B) const;
    void NextMove(Move& M) const;
    bool NextMove(Move& M, Board2D& B) const;
  private:
    void SuggestNextMove(Move& M) const;
  public:
    bool ValidMove(Move M) const;
    void ExtendTail(Move& M) const;
    int DoMove(Move M);
    Board2D AfterMove(Move M) const;
    int Score(int8 player) const;
    void SetScore(int8 player, int count);
    int GetTurn() const { return MyPiece(1) ? 1 : 2; }
    void SetTurn(int newTurn) { whiteToMove = (newTurn==fPieceWhite); }
    /// @deprecated Remove references to player colour
    int OutOfBoard(bool White) const;
    /// @deprecated Remove references to player colour
    void SetOutOfBoard(bool White, int count);
    /// @deprecated Remove references to player colour
    int WhiteOff() const;
    /// @deprecated Remove references to player colour
    int BlackOff() const;
    long HashCode() const;
    int Compare(const Board2D& aBoard) const;
    bool operator == (const Board2D& aBoard) const;
    bool operator < (const Board2D& aBoard) const;
  private:

    long currentHashCode;
    int MoveSeveral(Board2D::Pos FromFirst, Board2D::Pos FromLast, Board2D::Pos ToFirst);
    int Push(Board2D::Pos A, Board2D::Pos AA);
    /** Increment or decrement the number of pieces outside the board
    @param PieceType is fPieceWhite or fPieceBlack
    @param Delta  1 for increase or -1 for decrease
    */
    void DeltaOut(int PieceType, int Delta);
    void IncPushOutOfBoard(int PieceType); // PieceType is 1 or -1

  friend ostream& operator << (ostream& out, const Board2D& board);

};

inline void SetUp(Board2D& board, BoardGrid grid) {
  board.SetUp(grid);
}

/** Print a Nacre Board2D::Pos on a stream, using the Nacre notation. */
HALIOTIS_EXPORT ostream& operator << (ostream& out, const Board2D::Pos& bp);

/** Print a Nacre Move on a stream, using the Nacre notation. */
HALIOTIS_EXPORT ostream& operator << (ostream& out, const Board2D::Move& m);

/** Print a Nacre Board on a stream, using the Nacre notation. */
HALIOTIS_EXPORT ostream& operator << (ostream& out, const Board2D& board);

/////////////////////////////////////////////////////////////////////

/** A position on the board. Since the board is 9x9 it is possible
to have invalid coordinates. This class can validate itself and
generate the next successive move. */
struct HALIOTIS_EXPORT Board2D::Pos {
  int8 x; // Right      = direction 0
  int8 y; // Down-right = direction 1

  /// Default position is invalid
  Pos() { x=-1; y=-1; }

  /// Copy constructor
  Pos(const Pos& b) { x=b.x; y=b.y; }

  /// Generate a board position from a coordinate set.
  Pos(int8 ax, int8 ay) { x=ax, y=ay; }

  /** True, if the position can contain a piece. Since we have a hexagonal
  board and represent it in a square (an array) not all of the positions
  are legal.
  */
  bool Valid() const {
    return (0<=x) and (x<=8) and (0<=y) and (y<=8)
          and (4+0<=x+y) and (x+y<=4+8);
  }

  /** Go to next valid position. If this function is called after default
  construction, it will go to the first valid position. When no more valid
  positions are left, it will stay in an invalid position. */
  void Next() {
    if (y>8) return;
    x ++;
    if (Valid()) return;
    y ++;
    x = ( y<4 ? 0+4-y : 0 );
  }

  /** Move the Board2D::Pos one step in any of the six directions. No check
  is performed for if the new position is valid. */
  void Step(Direction dir) {
    switch (dir) {
    case 0: x=x+1; break;
    case 1: y=y+1; break;
    case 2: x=x-1; y=y+1; break;
    case 3: x=x-1; break;
    case 4: y=y-1; break;
    case 5: x=x+1; y=y-1; break;
    default: RANGE_CHECK_ERROR("Direction out of range");
    }
  }


  /** Return direction to another Board2D::Pos, but only if on one of the six
    axes relative to this Board2D::Pos. */
  Direction DirectionTo(const Pos& p) const {
    Direction result;
    int8 dx, dy, delta;
    dx=p.x-x, dy=p.y-y;

    if      (dy==0)   result=0, delta=dx;
    else if (dx==0)   result=1, delta=dy;
    else if (dx==-dy) result=2, delta=dy;
    else              return -1;

    if (delta<0) result+=3;
    return result;
  }
};

inline bool operator == (const Board2D::Pos& a, const Board2D::Pos& b) {
  return a.x == b.x and a.y == b.y;
}

const Board2D::Pos NoPos = Board2D::Pos(-1,-1);

HALIOTIS_EXPORT int LineLength(Board2D::Pos a, Board2D::Pos b);
HALIOTIS_EXPORT int Dist(Board2D::Pos a, Board2D::Pos b);

/////////////////////////////////////////////////////////////////////////

/** Move is a transition from one Board to another. Think of it as
  a Command pattern to a Board
*/
class HALIOTIS_EXPORT Board2D::Move {
  public:
    Board2D::Pos head;
    Direction tailDir;
    int8 tailCount;
    Direction moveDir;
    /// Move nothing
    Move();
    /// Move a single piece
    Move(Board2D::Pos pFromFirst, Board2D::Pos pToFirst);
    /// Move several pieces
    Move(Board2D::Pos pFromFirst, Board2D::Pos pFromLast, Board2D::Pos pToFirst);

    void Clear();
    Board2D::Pos FromFirst() const;
    Board2D::Pos FromLast() const;
    Board2D::Pos FromMiddle() const;
    Board2D::Pos ToFirst() const;
    Board2D::Pos ToLast() const;
    Board2D::Pos ToMiddle() const;

    bool operator==(Move M) const {
      return  (head.x==   M.head.x)
          and (head.y==   M.head.y)
          and (tailDir==  M.tailDir)
          and (tailCount==M.tailCount)
          and (moveDir==  M.moveDir);
    }

    int compare(Move M) const {
      return (head.x    < M.head.x) ? -1
           : (head.x    > M.head.x) ? 1
           : (head.y    < M.head.y) ? -1
           : (head.y    > M.head.y) ? 1
           : (tailDir   < M.tailDir) ? -1
           : (tailDir   > M.tailDir) ? 1
           : (tailCount < M.tailCount) ? -1
           : (tailCount > M.tailCount) ? 1
           : (moveDir   < M.moveDir) ? -1
           : (moveDir   > M.moveDir) ? 1
           : 0;
    }
    bool operator < (Move M) const {
      return compare(M) < 0;
    }

    bool Valid() const; // is move inside board?

    void Read(istream& in);
    void Write(ostream& out) const;
};

//////////////////////////////////////////////////////////////////////

/** Interface used to signaling a move */
class HALIOTIS_EXPORT MoveListener {
public:
  virtual void DoMove(const Board2D::Move& m) =0;
};

/////////////////////////////////////////////////////////////////////////

/** ReverseMove generates reverse moves. The board before each generated
  move can be fetched. This is usefull if you want to traverse a database
  of positions from end to start, to propagate game results.

  @see OpeningBook
*/
class HALIOTIS_EXPORT ReverseMove {
public:
  /** Maintains a reference to the board as long as the iterator lives.
  Also initiates the move with the first valid reverse-move */
  ReverseMove(const Board2D& boardAfter);
  /** Is the present move valid? */
  bool Valid();
  /** Next valid reverse-move on the board.
  @return true if there was another move */
  bool Next();
  /** Return the board before the current reverse-move. The returned value
  is a pointer to an internal structure, so it will be invalidated by
  the next call to Next() */
  const Board2D& BoardBefore();
  /** Return the move, but in forward direction. If you use BoardBefore()
  as basis, this move will get you to the board you started with */
  operator Board2D::Move() const;
private:
  const Board2D& board;
  Board2D::Move move;
  int opponentCount; // Number of pieces pushed
  Board2D boardBefore;

  // Try do do the present move and return an error code if things does not work
  int Do();
  // Generate next move (which may be invalid)
  void SuggestNext();
};

};

// TODO: Refactor code to use correct names and delete these typedefs
using namespace Haliotis;
typedef Haliotis::Board2D Board;
typedef Haliotis::Board2D::Move Move;
typedef Haliotis::Board2D::Move BoardMove;
typedef Haliotis::Board2D::Pos BoardPos;
#include "Game.hpp"

#endif

