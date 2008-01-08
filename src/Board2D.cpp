/* Haliotis, a library for Abalone playing programs.
 * Board2D class
 *
 * This module defines the Board2D class which holds an abalone board.
 *
 * Peer Sommerlund, 2003-May-16
*/

#include "Board2D.hpp"
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

//// implementation ////////////////////////////////////////////

namespace Haliotis {
































/*---- BoardPos -----------------------------------------------*/

bool ValidBoardPos(BoardPos bp) {
  return bp.Valid();
}

void NextValidBoardPos(BoardPos& P)
{
  P.x=P.x+1;
  if (not ValidBoardPos(P)) {
    P.y=P.y+1;
    if (P.y<4) P.x=0+4-P.y; else P.x=0;
  };
};

int Dist(BoardPos A, BoardPos B) {
  int dx,dy;
  dx=B.x-A.x; dy=B.y-A.y;
  if (dx==0) return abs(dy);
  else if (dy==0) return abs(dx);
  else if (dx+dy==0) return abs(dx);
  else return -1;
}

int LineLength(BoardPos A, BoardPos B) {
  if (not ValidBoardPos(A) or not ValidBoardPos(B)) return -1;
  return Dist(A,B)+1;
}

/*---- Move ---------------------------------------------------*/

Move::Move() /* NoMove */
: head(NoPos)
{
  // head=NoPos;
  tailDir=0; tailCount=1;
  moveDir=0;
}

/// Move single piece
Move::Move(BoardPos pFromFirst, BoardPos pToFirst)
{
  head=pFromFirst; tailDir=0; tailCount=1;
  moveDir=head.DirectionTo(pToFirst);
}

/// Move several pieces
Move::Move(BoardPos pFromFirst, BoardPos pFromLast, BoardPos pToFirst)
{
  head=pFromFirst;
  tailDir=head.DirectionTo(pFromLast);
  tailCount=LineLength(pFromFirst,pFromLast);
  moveDir=head.DirectionTo(pToFirst);
}

void Move::Clear()
{
  head=NoPos; tailDir=0; tailCount=1;
  moveDir=0;
}

BoardPos Move::FromFirst() const
{
  return head;
}

BoardPos Move::FromLast() const
{
  int i;
  BoardPos Result;
  Result=head;
  for (i=2; i<=tailCount; i++) Result.Step(tailDir);
  return Result;
}

BoardPos Move::FromMiddle() const
{
  BoardPos Result;
  Result=head;
  for (int i=2; i<=tailCount-1; i++) Result.Step(tailDir);
  return Result;
}

BoardPos Move::ToFirst() const
{
  BoardPos Result;
  Result=FromFirst(); Result.Step(moveDir);
  return Result;
}

BoardPos Move::ToLast() const
{
  BoardPos Result;
  Result=FromLast(); Result.Step(moveDir);
  return Result;
}

BoardPos Move::ToMiddle() const
{
  BoardPos Result;
  Result=FromMiddle(); Result.Step(moveDir);
  return Result;
}

/* is move inside board? */
bool Move::Valid() const
{
  bool Result;

  Result=FromFirst().Valid() and ToFirst().Valid();
  if (tailCount>1)
    Result=Result and FromLast().Valid() and ToLast().Valid();
  return Result;
}

/** Parse a string that contains a move and store the move
  The string MUST have format "%d,%d-%d,%d %d,%d"
*/
void Move::Read(istream& in)
{
  BoardPos pFromFirst, pFromLast, pToFirst;

  char ch;

  while (in.get(ch) and ch<'0');
  if (not in) return;
  pFromFirst.x = ch - '0';
  in.get(ch); // Ignore ','
  in.get(ch);
  pFromFirst.y = ch - '0';

  in.get(ch); // Ignore '-'

  in.get(ch);
  pFromLast.x = ch - '0';
  in.get(ch); // Ignore ','
  in.get(ch);
  pFromLast.y = ch - '0';

  in.get(ch); // Ignore ' '

  in.get(ch);
  pToFirst.x = ch - '0';
  in.get(ch); // Ignore ','
  in.get(ch);
  pToFirst.y = ch - '0';
  #if 0
  char comma, minus, blank;
  in >> pFromFirst.x >> comma >> pFromFirst.y
     >> minus
     >> pFromLast.x >> comma >> pFromLast.y
     >> blank
     >> pToFirst.x >> comma >> pToFirst.y
     ;
  #endif
  if (in) *this = Move(pFromFirst,pFromLast,pToFirst);
}

/** Generate a string representing the move that can be parsed later */
void Move::Write(ostream& out) const
{
  // %d,%d-%d,%d %d,%d
  out << FromFirst().x << ',' << FromFirst().y
      << '-'
      << FromLast().x << ',' << FromLast().y
      << ' '
      << ToFirst().x << ',' << ToFirst().y
      ;
}

/*---- Hashing function ---------------------------------------*/

static void Randomize() {
  // TODO: Initialise v of LongRandom with e.g. time
}

static long LongRandom() {
  static long v;
  return v =(48271L * v + 399268537L) % 2147483647L;
}

/** Common variables used for Zorbist computation of hash key.
  Note that this structure is instantiated as a global variable.
  This means it can be treated as a singleton class, whith automatic
  initialisation before anything else runs.
*/
struct ZorbistHashingFunction {
  long fieldKeyWhite[61];
  long fieldKeyBlack[61];
  long whiteMoveKey;
  ZorbistHashingFunction();
  long HashKey(const Board& board) const;
} hashingFunction;

/** Initializes the tables used to compute the Zorbist Hash Key.
*/
ZorbistHashingFunction::ZorbistHashingFunction()
{
  Randomize();
  for (int i=0; i<=60; i++) {
    fieldKeyWhite[i] = LongRandom();
    fieldKeyBlack[i] = LongRandom();
  }
  whiteMoveKey = LongRandom();
}

/** Given a board, compute a 32 bit hashkey */
inline long ZorbistHashingFunction::HashKey(const Board& board) const {
  int x,y;
  int fieldNr;

  long Result=0;
  fieldNr=0;
  for (y=0; y<=4; y++)
  for (x=0+4-y; x<=8; x++, fieldNr++)
  switch (board.field[x][y]) {
    case fPieceWhite: Result^=fieldKeyWhite[fieldNr]; break;
    case fPieceBlack: Result^=fieldKeyBlack[fieldNr]; break;
  }
  for (y=5; y<=8; y++)
  for (x=0; x<=4+8-y; x++, fieldNr++)
  switch (board.field[x][y]) {
    case fPieceWhite: Result^=fieldKeyWhite[fieldNr]; break;
    case fPieceBlack: Result^=fieldKeyBlack[fieldNr]; break;
  }
  if (board.whiteToMove) Result^=whiteMoveKey;
  return Result;
}

/*---- Board --------------------------------------------------*/

/** Given a move, this function tries to move the pieces on the board.

  @return
    0 is success, other is not.
     if pushing
         -1:   illegal push-direction
         -1:   tried to use enemy pieces
          0:   push succeded
          1:   agressor too long
          2:   agressor not long enough
          3:   victim has backup
          4:   push out of board (suicide)
     if moving several
          0:   move succeded
          1:   opponent blocks
*/
int Board::DoMove(Move M) {
  int Result;
  TRACE1("Board::DoMove");

  if (M.tailCount==1)
    Result=Push(M.FromFirst(),M.ToFirst());
  else if (M.tailDir==M.moveDir)
    Result=Push(M.FromFirst(),M.ToFirst());
  else if (Opposite(M.tailDir) == M.moveDir)
    Result=Push(M.FromLast(),M.ToLast());
  else
    Result=MoveSeveral(M.FromFirst(),M.FromLast(),M.ToFirst());

  /* If move was successfull, switch sides */
  if (Result==0) whiteToMove=not whiteToMove;

  return Result;
}

void Board::DeltaOut(int PieceType, int Delta)
{
  if (PieceType==fPieceWhite)
    field[0][0]=field[0][0]+Delta; /*white off*/
  else if (PieceType==fPieceBlack)
    field[8][8]=field[8][8]+Delta; /*black off*/
  ;
}

void Board::SetBoardPos(BoardPos bp, int8 FieldValue)
{
  /* Count previous content as off board */
  DeltaOut(field[bp.x][bp.y],+1);
  /* Count new content as on board */
  field[bp.x][bp.y]=FieldValue;
  DeltaOut(field[bp.x][bp.y],-1);
}

/* Sets up the initial position. White to move.
*/
void Board::SetUpStartPos()
{
  int x,y;

  // Since 0,0 and 8,8 is also set to zero, we get
  // OutOfBoard set to zero indirectly.
  for (x=0; x<=8; x++)
    for (y=0; y<=8; y++)
      field[x][y]=fEmpty;
#ifdef TRANSPOSITION_TABLE
  currentHashCode=0;
#endif
  for (x=4; x<=8; x++) field[x][0]=fPieceBlack;
  for (x=3; x<=8; x++) field[x][1]=fPieceBlack;
  for (x=4; x<=6; x++) field[x][2]=fPieceBlack;
  for (x=2; x<=4; x++) field[x][6]=fPieceWhite;
  for (x=0; x<=5; x++) field[x][7]=fPieceWhite;
  for (x=0; x<=4; x++) field[x][8]=fPieceWhite;
  whiteToMove=true;
}

void Board::SetUp(BoardGrid grid)
{
  int x,y;

  // Since 0,0 and 8,8 is also set to zero, we get
  // OutOfBoard set to zero indirectly.
  for (x=0; x<=8; x++)
    for (y=0; y<=8; y++)
      field[x][y]=grid[y][x];
  currentHashCode=0;
  whiteToMove=true;
}

BoardGrid GermanDaisy = {
  {0, 0, 0, 0,   0, 0, 0, 0, 0},
   {0, 0, 0,   2, 2, 0, 0, 1, 1},
    {0, 0,   2, 2, 2, 0, 1, 1, 1},
     {0,   0, 2, 2, 0, 0, 1, 1, 0},
      {  0, 0, 0, 0, 0, 0, 0, 0, 0},
       {  0, 1, 1, 0, 0, 2, 2, 0,   0},
        {  1, 1, 1, 0, 2, 2, 2,   0, 0},
         {  1, 1, 0, 0, 2, 2,   0, 0, 0},
          {  0, 0, 0, 0, 0,   0, 0, 0, 0}
};


BoardGrid BelgianDaisy = {
  {0, 0, 0, 0,   2, 2, 0, 1, 1},
   {0, 0, 0,   2, 2, 2, 1, 1, 1},
    {0, 0,   0, 2, 2, 0, 1, 1, 0},
     {0,   0, 0, 0, 0, 0, 0, 0, 0},
      {  0, 0, 0, 0, 0, 0, 0, 0, 0},
       {  0, 0, 0, 0, 0, 0, 0, 0,   0},
        {  0, 1, 1, 0, 2, 2, 0,   0, 0},
         {  1, 1, 1, 2, 2, 2,   0, 0, 0},
          {  1, 1, 0, 2, 2,   0, 0, 0, 0}
};

BoardGrid SwissDaisy = {
  {0, 0, 0, 0,   0, 0, 0, 0, 0},
   {0, 0, 0,   2, 2, 0, 0, 1, 1},
    {0, 0,   2, 1, 2, 0, 1, 2, 1},
     {0,   0, 2, 2, 0, 0, 1, 1, 0},
      {  0, 0, 0, 0, 0, 0, 0, 0, 0},
       {  0, 1, 1, 0, 0, 2, 2, 0,   0},
        {  1, 2, 1, 0, 2, 1, 2,   0, 0},
         {  1, 1, 0, 0, 2, 2,   0, 0, 0},
          {  0, 0, 0, 0, 0,   0, 0, 0, 0}
};


BoardGrid DutchDaisy = {
  {0, 0, 0, 0,   2, 2, 0, 1, 1},
   {0, 0, 0,   2, 1, 2, 1, 2, 1},
    {0, 0,   0, 2, 2, 0, 1, 1, 0},
     {0,   0, 0, 0, 0, 0, 0, 0, 0},
      {  0, 0, 0, 0, 0, 0, 0, 0, 0},
       {  0, 0, 0, 0, 0, 0, 0, 0,   0},
        {  0, 1, 1, 0, 2, 2, 0,   0, 0},
         {  1, 2, 1, 2, 1, 2,   0, 0, 0},
          {  1, 1, 0, 2, 2,   0, 0, 0, 0}
};

#if 0
// Wrong definition, taken from the program Abalot
BoardGrid TheWall = {
  {0, 0, 0, 0,   2, 2, 2, 2, 2},
   {0, 0, 0,   2, 2, 2, 2, 2, 2},
    {0, 0,   2, 2, 2, 2, 2, 2, 2},
     {0,   0, 0, 0, 0, 0, 0, 0, 0},
      {  0, 0, 0, 0, 0, 0, 0, 0, 0},
       {  0, 0, 0, 0, 0, 0, 0, 0,   0},
        {  1, 1, 1, 1, 1, 1, 1,   0, 0},
         {  1, 1, 1, 1, 1, 1,   0, 0, 0},
          {  1, 1, 1, 1, 1,   0, 0, 0, 0}
};
#else
BoardGrid TheWall = {
  {0, 0, 0, 0,   0, 0, 2, 0, 0},
   {0, 0, 0,   0, 0, 0, 0, 0, 0},
    {0, 0,   0, 2, 2, 2, 2, 2, 0},
     {0,   2, 2, 2, 2, 2, 2, 2, 2},
      {  0, 0, 0, 0, 0, 0, 0, 0, 0},
       {  1, 1, 1, 1, 1, 1, 1, 1,   0},
        {  0, 1, 1, 1, 1, 1, 0,   0, 0},
         {  0, 0, 0, 0, 0, 0,   0, 0, 0},
          {  0, 0, 1, 0, 0,   0, 0, 0, 0}
};
#endif

/** Read a board from a stream. The board is assumed to have the following
format:
    . . . . .
   . . . . . .
  . . . . . . .
 . . . . . . . .
. . . . . . . . .
 . . . . . . . .
  . . . . . . .
   . . . . . .
    . . . . .
1-0
2-0
3-0

First a graphical representation, then a list of how many pieces of each player
have been pushed from the board. Note that you can NOT see which player did
the push-out. The following example shows a situation where player 1 have just
pushed one of player 2's pieces out of the board
    . . . . .
   . . . . . .
  . . . . . . .
 . . . . . . . .
. . . . . . . . .
 . . . . 1 . . .
  . . . . 1 . .
   . . . . 1 .
    . . . . 2
1-0
2-1
*/
void Board::Read(istream& in)
{
  for (int y=0; y<=8; y++)
    for (int x=0; x<=8; x++) {
      if (not BoardPos(x,y).Valid()) continue;
      char f;
      in >> f;
      if (not in) return;
      int player = f-'0';
      if (1 <= player and player <= 2)
        field[x][y] = player;
      else
        field[x][y] = fEmpty;
    }
  ;

  int sideToMove;
  in >> sideToMove;
  whiteToMove = (sideToMove == fPieceWhite);

  for (int p=1; p<=PLAYERS; p++) {
    string line;
    in >> line;
    if (not in) return;
    // Note: hardcoded to handle two players
    if      (line[0]=='0'+fPieceWhite) SetOutOfBoard(true, line[2]-'0');
    else if (line[0]=='0'+fPieceBlack) SetOutOfBoard(false,line[2]-'0');
    else ; // Mark read as failed
  }
}

void Board::Write(ostream& out) const
{
  for (int y=0; y<=8; y++) {
    for (int i=0; i<Max(y-4,4-y); i++) {
      out << ' ';
    }
    for (int x=0; x<=8; x++) {
      if (not BoardPos(x,y).Valid()) continue;
      out << ' ';
      if (field[x][y]==fEmpty) out << '.';
      else out << (char)('0'+field[x][y]);
    }
    out << '\n';
  }
  out << (whiteToMove ? fPieceWhite : fPieceBlack) << '\n';
  for (int p=1; p<=PLAYERS; p++) {
    int piecesLost = OutOfBoard(p==fPieceWhite);
    out << p << '-' << piecesLost << '\n';
    // Note: hardcoded to handle two players
  }
}

/*
    1 - . . . . .
   2 - . . . . . .
  3 - . . . . . . .
 4 - . . . . . . . .
5 - . . . . . . . . .
 6 - . . . . . . . . \
  7 - . . . . . . . \ I
   8 - . . . . . . \ H
    9 - . . . . . \ G
         \ \ \ \ \ F
          A B C D E

BoardPos has I1 in bottom left corner
*/
void Print_A9BL_CC(ostream& out, const BoardPos& bp) {
  out << char('A'+bp.x) << 1+bp.y;
}

/** Official Abalone notation for coordinates.
  a1 is bottom left,
  first axis is letters clockwise,
  second axis is numbers anti-clockwise.

    i - . . . . .
   h - . . . . . .
  g - . . . . . . .
 f - . . . . . . . .
e - . . . . . . . . .
 d - . . . . . . . . \
  c - . . . . . . . \ 9
   b - . . . . . . \ 8
    a - . . . . . \ 7
         \ \ \ \ \ 6
          1 2 3 4 5

BoardPos has I1 in bottom left corner
*/
void Print_A1BL_C(ostream& out, const BoardPos& bp) {
  out << char('i'-bp.y) << 1+bp.x;
}

ostream& operator << (ostream& out, const BoardPos& bp) {
#ifdef A9_BOTTOM_LEFT_COUNTERCLOCKWISE_COORDINATES
  Print_A9BL_CC(out,bp);
#else
  Print_A1BL_C(out,bp);
#endif
  return out;
}
// TODO: Conflict with StreamInterface on class Move
#ifdef FF_DIR_NOTATION
/// FF-DIR notation
ostream& operator << (ostream& out, const Move& m) {
  out << m.FromFirst();
  // @bug This is only unique if the move is expanded. It is valid to
  // have FromLast == FromFirst even if moving more than 1 of own pieces.
  if (not (m.FromLast() == m.FromFirst())) out << m.FromLast();
  out << "-" << m.moveDir;
  return out;
}
#else
/// FFTL notation
ostream& operator << (ostream& out, const Move& m) {
  out << m.FromFirst() << m.ToLast();
  // @bug This is only unique if the move is expanded. It is valid to
  // have FromLast == FromFirst even if moving more than 1 of own pieces.
  return out;
}
#endif

/** Print a Nacre Board on stdout, using the Nacre notation. */
ostream& operator << (ostream& out, const Board& board) {
/*
      0 + + + + +
     1 + + + + + +
    2 + + + + + + +
   3 + + + + + + + +
  4 + + + + + + + + +
   5 + + + + + + + + I
    6 + + + + + + + H
     7 + + + + + + G
      8 + + + + + F
         A B C D E
*/
  board.Write(out);
  return out;
}

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

int8 Board::At(BoardPos BP) const
{
  return field[BP.x][BP.y];
}

/** Given the content of a field, this function will determine if
it belongs to the player that has the move.
*/
bool Board::MyPiece(int8 F) const
{
  if (whiteToMove) return F==fPieceWhite;
  return F==fPieceBlack;
}

/** Computes a hashcode for the current position. All pieces and the
player to move is included in the hash code. It is computed by Zorbist
Hashing: Each piece-position combination has a random key associated
with it. The bord can be seen as a set of these pairs and the keys
associated to these pairs are xor'ed to generate the hash key.
*/
long Board::HashCode() const
#ifndef STORED_HASHCODE
{
  return hashingFunction.HashKey(*this);
}
#else
{ /* HashCode, simple version */
  Result=currentHashCode;
  return Result;
}
#endif

#ifdef INCREMENTAL_HASHCODE
void Board::SetField(int x, int y, int8 f);
var
  int fieldNr;
  long PosVal()
  {
    case AtField[x][y] of
      fPieceWhite: Result=fieldKeyWhite[fieldNr];
      fPieceBlack: Result=fieldKeyBlack[fieldNr];
    else
      Result=0;
    } /* end */
  } /* end */
{ /* SetField */

  case y of
    1: fieldNr=0;
    2: fieldNr=5;
    3: fieldNr=11/*5+6*/;
    4: fieldNr=18/*5+6+7*/;
    5: fieldNr=26/*5+6+7+8*/;
    6: fieldNr=35/*5+6+7+8+9*/;
    7: fieldNr=43/*5+6+7+8+9+8*/;
    8: fieldNr=50/*5+6+7+8+9+8+7*/;
    9: fieldNr=56/*5+6+7+8+9+8+7+6*/;
  } /* end */
  fieldNr=fieldNr+x-1;

  currentHashCode=currentHashCode xor PosVal;
  AtField[x][y]=f;
  currentHashCode=currentHashCode xor PosVal;
} /* end */
#endif

// Helper functions for Board::MoveSeveral

  /* Check if it is possible to move one of my pieces from the specified
    position in the dx,dy direction */
  static bool CanMove(Board& b, BoardPos A, int dx, int dy)
  {
    BoardPos B(A.x+dx,A.y+dy);
    if (!B.Valid()) return false;
    return b.MyPiece(b.At(A)) and (b.field[B.x][B.y]==fEmpty);
  }
  /* Move the specified piece in the dx,dy direction */
  static void MovePiece(Board& b, BoardPos A, int dx, int dy)
  {
    BoardPos B(A.x+dx,A.y+dy);
    b.field[B.x][B.y]=b.field[A.x][A.y];
    b.field[A.x][A.y]=fEmpty;
  }


/** Move several pieces without pushing opponent.
  Note: Only for broadside moves.
  @return
          0:   move succeded
          1:   opponent blocks
*/
int Board::MoveSeveral(BoardPos FromFirst, BoardPos FromLast, BoardPos ToFirst)
{
  int dx,dy;
  BoardPos First(FromFirst);
  BoardPos Last(FromLast);
  BoardPos Middle;

  TRACE1("Board::MoveSeveral");

  if (LineLength(FromFirst,FromLast)==3) {
    Middle.x=(FromFirst.x+FromLast.x)/2;
    Middle.y=(FromFirst.y+FromLast.y)/2;
  }

  dx=ToFirst.x-FromFirst.x;
  dy=ToFirst.y-FromFirst.y;

  /* Check that destination is empty and legal */
  if (not CanMove(*this,First, dx,dy)) return 1;
  if (not CanMove(*this ,Last, dx,dy)) return 1;
  if (LineLength(FromFirst,FromLast)==3)
    if (not CanMove(*this,Middle,dx,dy))
      return 1;

  /* Move pieces */
  MovePiece(*this,First,dx,dy);
  MovePiece(*this,Last,dx,dy);
  if (LineLength(FromFirst,FromLast)==3)
    MovePiece(*this,Middle,dx,dy);
  return 0;
}


/* Increments the number of white or black pieces off the board.
This value is stored in one of the unused fields of the field array. */
void Board::IncPushOutOfBoard(int PieceType)
{
  if (PieceType==fPieceWhite)
    field[0][0]=field[0][0]+1; /*white off*/
  else
    field[8][8]=field[8][8]+1; /*black off*/
}

/* Returns the number of white pieces that have been pushed off the board */
int Board::WhiteOff() const
{
  return field[0][0];
}

/* Returns the number of black pieces that have been pushed off the board */
int Board::BlackOff() const
{
  return field[8][8];
}

/** Return number of pieces pushed off the board for either white or black */
int Board::OutOfBoard(bool White) const
{
  if (White) return WhiteOff();
  else return BlackOff();
}

/** Set number of pieces off board. Note that this might affect the total number
of marbles on the board */
void Board::SetOutOfBoard(bool White, int count) {
  if (White) DeltaOut(fPieceWhite,count-WhiteOff());
  else DeltaOut(fPieceBlack,count-BlackOff());
}

/** Return numberof opponent pieces pushed off board */
int Board::Score(int8 player) const {
  int8 opponent = 3 - player;
  return OutOfBoard(opponent==fPieceWhite);
}

/** Set number of opponent pieces off board. Note that this might affect
the total number of marbles on the board */
void Board::SetScore(int8 player, int count) {
  int8 opponent = 3 - player; // only for two player mode
  DeltaOut(opponent,count-Score(player));
}

Board Board::AfterMove(Move M) const
{
  Board Result(*this);
  TRACE_ASSERT_MSG(Result.DoMove(M)==0,
    "Board::AfterMove - move invalid on this board");
  return Result;
}

/** Compare two boards
 <0 means this is lower,
==0 means equal
 >0 means this > aBoard */
int Board::Compare(const Board& aBoard) const
{
  int result;
  result = (int)(whiteToMove) - aBoard.whiteToMove; if (result) return result;
  result = (int)(WhiteOff()) - aBoard.WhiteOff(); if (result) return result;
  result = (int)(BlackOff()) - aBoard.BlackOff(); if (result) return result;
  for (int y=0; y<=8; y++) {
    for (int x=Max(0,0+4-y); x<=Min(8,8+4-y); x++) {
      result = (int)(field[x][y]) - aBoard.field[x][y];
      if (result) return result;
    }
  }
  return result;
}

/** Compare two boards */
bool Board::operator == (const Board& aBoard) const
{
  return Compare(aBoard) == 0;
}

/** Compare two boards */
bool Board::operator < (const Board& aBoard) const
{
  return Compare(aBoard) < 0;
}


/** Push a number of pieces. May include opponent pieces.
Returns an error code if the push was not successfull.

@return
         -1:   illegal push-direction
         -1:   tried to move enemy
          0:   push succeded
          1:   agressor too long
          2:   agressor not long enough
          3:   victim has backup
          4:   push out of board (suicide)
*/
int Board::Push(BoardPos A, BoardPos AA)
{
  int dx,dy;
  int Alen,Blen;
  int Atype,Btype;
  BoardPos B,C;

  TRACE1("Board::Push");

  // Verify that push is from one field to the neighbour
  if (Dist(A,AA)!=1) {
    return -1;
  }
  // Verify that you only move your own pieces
  if (not MyPiece(At(A))) {
    return -1;
  }
  // Follow string of your pieces to its end. B will point to next field
  dx=AA.x-A.x; dy=AA.y-A.y;
  Alen=0;
  Atype=field[A.x][A.y];
  B=A;
  do {
    Alen=Alen+1;
    B.x=B.x+dx; B.y=B.y+dy;
    if (not ValidBoardPos(B)) return 4;
  } while (field[B.x][B.y]==Atype);
  // Verify max length of your string
  if (Alen>3) return 1;
  // Do you push opponents or only your own pieces?
  if (field[B.x][B.y]==fEmpty) {
    // Perform a move of your own pieces
    field[B.x][B.y]=field[A.x][A.y];
    field[A.x][A.y]=fEmpty;
    return 0;
  }
  // Push opponent pieces
  // C will point to next non-opponent piece
  Blen=0;
  Btype=field[B.x][B.y];
  C=B;
  do {
    Blen=Blen+1;
    C.x=C.x+dx; C.y=C.y+dy;
    if (not ValidBoardPos(C)) {
      // Trying to push outside board
      // Verify that attacker is longer than defender
      if (Alen<=Blen) return 2;
      // Perform a push-out move
      IncPushOutOfBoard(Btype);
      field[B.x][B.y]=field[A.x][A.y];
      field[A.x][A.y]=0;
      return 0;
    }
  } while (!(field[C.x][C.y]!=Btype));
  // Verify that none of your pieces is behind opponent
  // (same as that the field is empty)
  if (field[C.x][C.y]==Atype) return 3;
  // Verify that attacker is longer than defender
  if (Alen<=Blen) return 2;
  // Peform push of opponent pieces
  field[C.x][C.y]=field[B.x][B.y];
  field[B.x][B.y]=field[A.x][A.y];
  field[A.x][A.y]=0;
  return 0;
}

/** Return the first valid move. If this is NoMove, then there is no valid
moves */
void Board::FirstMove(Move& M) const
{
  M.head.x=4; M.head.y=0;
  M.moveDir=0; M.tailDir=0; M.tailCount=1;
  if (not ValidMove(M)) NextMove(M);
}

/** Return the first valid move. If this is NoMove, then there is no valid
moves */
bool Board::FirstMove(Move& M, Board& B) const
{
  M.head.x=4; M.head.y=0;
  M.moveDir=0; M.tailDir=0; M.tailCount=1;

  B=*this;
  if (B.DoMove(M)==0) return true;
  return NextMove(M,B);
}

/** Suggest the next move.
  It does verify that the pieces moved are your own. It does not verify
  that the move is possible.

  @note IMPORTANT: May generate invalid moves

  Helper function for NextMove()

  Algoritm:

  next moveDir (0..5)
  next tailCount (1..3)
  next tailDir (0..2)
  next head
*/
void Board::SuggestNextMove(Move& M) const
{
  /* next moveDir (0..5) */
  while ((M.moveDir<5)) {
    M.moveDir=M.moveDir+1;
    // 1 piece can move any direction - this will also cause push-moves
    // to be generated (in which case we ought to extend the tail)
    // TODO: Extend tail for push-moves (not trivial to fit into algorithm)
    if ((M.tailCount==1)) return;
    // Now we have more than 1 piece selected, thus it must be a broadside
    // move. These are never parallel with the tail
    if (not Parallel(M.moveDir,M.tailDir)) return;
  }
  // Below we increase tailCount. This means we now try broadside moves.
  // None of these must be parallel with the move direction
  M.moveDir=0;

  /* next tailCount (1..3) */
  while (M.tailCount<3) {
    M.tailCount=M.tailCount+1;
    if (not ValidBoardPos(M.FromLast())) break;
    if (MyPiece(At(M.FromLast()))) {
      if (Parallel(M.moveDir,M.tailDir)) M.moveDir++;
      return;
    }
    else M.tailCount=3; // If middle piece is missing, skip last-piece test
  }
  // we are examining broadside moves until all tail directions have been tried.
  M.tailCount=2;

  /* next tailDir (0..2) */
  while ((M.tailCount>1) and (M.tailDir<2)) {
    M.tailDir=M.tailDir+1;
    if (not ValidBoardPos(M.FromLast())) continue;
    if (MyPiece(At(M.FromLast()))) return;
  }
  // Back to 1-piece moves (which is also push-moves)
  M.tailDir=0;
  M.tailCount=1;
  M.moveDir=0;

  /* tailDir=0, next head (NextValidBoardPos with home content) */
  do {
    NextValidBoardPos(M.head);
    if (not ValidBoardPos(M.head)) return;
  } while (!(MyPiece(At(M.head))));
}

/** Generates the next move, given the previous move. To generate the first
legal move you should give the procedure a move from 0,0 to 0,0. If no more
legal moves are available, an illegal move is generated.
@returns true if the move is valid
@param M the generated move
@param B the board as it would be after the move was done
*/
bool Board::NextMove(Move& M, Board& B) const
{
  if (not ValidBoardPos(M.head)) {
    return false;
  }
  while (true) {
    SuggestNextMove(M);
    if (not ValidBoardPos(M.head)) return false;
    B=*this;
    if (B.DoMove(M)==0) return true;
  };
}

/** Generates the next move, given the previous move. To generate the first
legal move you should give the procedure a move from 0,0 to 0,0. If no more
legal moves are available, an illegal move is generated.
*/
void Board::NextMove(Move& M) const
{
  if (not ValidBoardPos(M.head)) {
    return;
  }
  do {
    SuggestNextMove(M);
    if (ValidMove(M)) return;
  } while (ValidBoardPos(M.head));
}

/// HACK
Board BoardAfterTestMove;

/** Determines if the move is valid for the player to move */
bool Board::ValidMove(Move M) const
{
  if (not MyPiece(At(M.head))) {
    return false;
  }

  // This global variable is used for the temporary board, since
  // some part of the code wants the board in case the move was
  // valid.
  BoardAfterTestMove=*this;
  return BoardAfterTestMove.DoMove(M)==0;
}

/** Extend the tail of the move. For generated push-moves, the tail
is always length 1. Some move notations (most?) require that the tail holds
all of your pieces that move */
void Board::ExtendTail(Move& M) const
{
  // Do nothing for broadside moves
  if (M.tailCount>1 and not Parallel(M.moveDir,M.tailDir)) return;
  // Extend the tail
  int my=At(M.head);
  // Set tail direction for single-piece moves
  if (M.tailCount==1) M.tailDir = M.moveDir;
  // Determine if the tail or the head should be extended.
  bool extendTail = M.tailDir == M.moveDir;
  if (extendTail) {
    // Extend tail while the next piece is also yours
    while (M.tailCount<3) {
      M.tailCount++;
      if (At(M.FromLast()) != my) {
        M.tailCount--;
        return;
      }
    }
  }
  else { // (M.tailDir-M.moveDir+3) % 6 == 0 .. so directions are opposite
    // Extend the head while the previous piece is also yours
    while (M.tailCount<3) {
      M.tailCount++;
      M.head.Step(M.moveDir);
      if (At(M.FromFirst()) != my) {
        M.tailCount--;
        M.head.Step(M.tailDir);
        return;
      }
    }
  }
}


////////////////////////////////////////////////////////////////
//
// GameTreeNode

class GameTreeNode;

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

#if 0
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
  bool GameTreeNode_Read_implemented = false;
  TRACE_ASSERT(GameTreeNode_Read_implemented);
  #if 0
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
  TRACE_ASSERT(currentPosition);
  return currentPosition->comment;
}

void Game::SetComment(const string& comment)
{
  TRACE_ASSERT(currentPosition);
  currentPosition->comment = comment;
}

#if 1
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
  char* seven_tag_roster[] = {
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

/*---- ReverseMove ---------------------------------------------*/

/* The ReverseMove generator can generate all moves that would
have lead to this situation. This is usefull when you traverse
a tree of Boards in the opposite direction that what the game
normally flows

Example of usage:
  for (ReverseMove m(nextBoard); m.Valid(); m.Next()) {
    Examine(m.BoardBefore(),m.Move());
  }
*/

ReverseMove::ReverseMove(const Board& _board)
  : board(_board)
{
  move.head.x=4; move.head.y=0;
  move.moveDir=0; move.tailDir=0; move.tailCount=1;
  opponentCount=0;
  if (Do()!=0) Next();
}

/** Returns an error code. 0 means that the present move was done and the
result can be found in boardBefore
error 11 means that too may opponent pieces is marked pushed.
error 12 : cannot push opponent with broadside move
error 13 : trying to pull opponent
error 14 : push-off not allowed with zero off board
*/
int ReverseMove::Do()
{
  // Check that there are no opponent pieces in association with broad-side
  // moves
  if (move.tailCount > 1
  and (not Parallel(move.moveDir,move.tailDir))
  and opponentCount > 0)
  {
    TRACE1("ReverseMove::Do() rejected broadside move with opponent pieces");
    return 12;
  }

  // Find opponent at boardBefore.
  // NOTE: assumes two player mode
  // The player to move at boardAfter == board is the opponent at boardBefore
  int opponent = board.whiteToMove ? fPieceWhite : fPieceBlack;

  // Verify that the reverse move is not "pulling" opponent pieces
  // (which is a push done backwards)
  if (move.tailCount == 1 or move.tailDir == move.moveDir) {
    // Extend tail along moveDir to include every of my pieces
    // verify that there are no opponents behind the tail
    // Since generated push-moves will have At(ToLast) == opponent
    // we can simply check if this position is empty

    // Assume generated push-move is 1 long - this is required for the code
    // below to work.
    if (move.tailCount!=1) {
      TRACE1("Assertion failed: move.tailCount==1 @ " << __FILE__ << __LINE__);
      TRACE_ASSERT(move.tailCount==1);
    }

    Move extended = move;
    board.ExtendTail(extended);
    BoardPos pullPos = move.head;
    for (int i = 0; i < extended.tailCount; i++) {
      pullPos.Step(move.moveDir);
    }
    if (pullPos.Valid() and board.At(pullPos) != fEmpty) {
      TRACE1("ExtendedTail("<<move<<")="<<extended);
      TRACE1("ReverseMove::Do(), rejected pulling opponent ("
           <<pullPos<<" = "<<board.At(pullPos)<<")");
      return 13;
    }
  }

  // Do the reverse move with help from DoMove and find boardBefore.
  // Note that we must set the side to move both before and after DoMove.
  // We set it before, because the side to move must be correct.
  // We set it after, because DoMove updates which side it is to move.
  boardBefore = board;
  boardBefore.whiteToMove = (opponent==fPieceBlack);
  int result = boardBefore.DoMove(move);
  boardBefore.whiteToMove = (opponent==fPieceBlack);

  // If DoMove failed, we fail
  if (result!=0) {
    TRACE1("ReverseMove::Do() rejected, DoMove failed with code "<<result);
    return result;
  }

  // Undo the opponent push
  BoardPos bp = move.head;
  for (int i=1; i<=opponentCount; i++) {
    boardBefore.SetBoardPos(bp,opponent); // Also updates push-off
    bp.Step(Opposite(move.moveDir));
    if (bp.Valid()) {
      boardBefore.SetBoardPos(bp,fEmpty); // Also updates push-off
    }
    else {
      // Allow only 1 opponent pieces to be pushed off
      if (i!=opponentCount) {
        TRACE1("ReverseMove::Do() rejected. No more than one opponent out at a time");
        return 11;
      }
    }
  }
  if (boardBefore.OutOfBoard(opponent==fPieceWhite) < 0) {
    TRACE1("ReverseMove::Do() rejected. Cannot undo a push-off move with no pieces off");
    return 14;
  }
  //TRACE1("ReverseMove::Do() success");
  return 0;
}

const Board& ReverseMove::BoardBefore() {
  return boardBefore;
}


/** Suggest a reverse-move.
 NOTE: This function corrupts boardBefore. You must call Do() right after
 this function to get a valid boardBefore */
void ReverseMove::SuggestNext()
{
  /* Strategy:
  Since any Abalone move has a corresponding move that undoes it we can simply
  use Board::NextMove to generate moves. This does not apply to moves where
  opponent pieces are pushed. To also generate the latter, we try to push any
  number of opponent pieces for each move generated.
  */
  BoardPos nextOppPos = move.head;
  for (int i=0; i<opponentCount; i++) nextOppPos.Step(Opposite(move.moveDir));
  if (nextOppPos.Valid()) {
    opponentCount++;
    nextOppPos.Step(Opposite(move.moveDir));
    // Generated pushmoves have tailCount==1, so they must be extended first
    Move extended = move;
    board.ExtendTail(extended);
    if (opponentCount < extended.tailCount
    and Parallel(extended.moveDir,extended.tailDir)
    ) {
      // check that there are opponents to push
      /* Since board is after the move we will undo, the player that did the move
      is the previous player. MyPiece thus indicates if the opponent has a piece */
      // This is opponent pieces in front (instead of behind) the pieces moved
      if (not nextOppPos.Valid()
          or board.MyPiece(board.At(nextOppPos))) { // ! this means opponent
        TRACE1("SuggestNext came up with "<<move<<"+"<<opponentCount);
        return;
      }
      else {
        TRACE1("SuggestNext no opponentCount="<<opponentCount
              <<"At("<<nextOppPos<<")="<<board.At(nextOppPos));
      }
    }
    else {
      TRACE1("SuggestNext: fail opponentCount = "<<opponentCount
            <<" < "
            <<extended.tailCount<<" = extended.tailCount");
    }
  }
  else {
    TRACE1("SuggestNext max push-out is 1");
  }
  opponentCount = 0;

  // Use NextMove to suggest a move.
  // Note that it must be told which side it was to move.
  boardBefore = board;
  boardBefore.whiteToMove = not board.whiteToMove;
  boardBefore.NextMove(move);

  TRACE1("SuggestNext came up with "<<move<<"+"<<opponentCount);
}

bool ReverseMove::Next() {
  // If the move head is an invalid position, we have generated all moves
  if (not ValidBoardPos(move.head)) return false;
  do {
    SuggestNext();
    if (not ValidBoardPos(move.head)) return false;
  } while (Do()!=0);
  TRACE1("Next came up with "<<move<<"+"<<opponentCount);
  return true;
}

bool ReverseMove::Valid() {
  // This assumes that Next() does not generate invalid moves with a valid head
  return ValidBoardPos(move.head);
}

ReverseMove::operator Move() const {
  Move extended = move;
  board.ExtendTail(extended);
//  TRACE1("move="<<move<<", extended="<<extended);
  return Move(extended.ToLast(),extended.ToFirst(),extended.FromLast());
}

} // namespace Haliotis
