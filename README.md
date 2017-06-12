AbMove README
=============
The AbMove C++ library has the goal of easing implementation of Abalone playing programs, by providing several common functions:
- Basic Abalone classes: a Board, a Move, a Game
- A framework for implementing the Abalone Engine Protocol
- Simple log file facilities

Modules
-------

The library has the following modules, each module has its own header files
- Trace
- Haliotis
- AEWrap

### Haliotis Classes ###
These classes are the basics you need to implement move generation.

`#include <Board2D.hpp>`

- Board2D - A simple board that can generate moves.
- Board2D::Move - movement of marbles, e.g. a1a2 for an inline move
- Board2D::Pos - a position on a board, e.g. a1
- Game - A starting position and a tree of moves. Can be saved to and loaded from a file

### Trace macros ###
The trace module is fairly simple. 

`#include <Trace.hpp>`

- TRACE(x) x is a streaming expression as if you would write cout << x;
- TRACE_ASSERT(a) a is an assertion expression. If false, it will log the expression and its value

### AEWrap ###
Abalone Engine Wrapper code. This is primarily a single baseclass that you should extend to implement various virtual methods needed. 
```
#include <AEWrap.hpp>
using namespace AbaloneEngineProtocol;
```
- class InputHandler - Interface for processing input during search. 
- class Engine - Interface that must be implemented by engine. The AEPWrapper will use
  it to issue commands.
- class Option - Parse command line options
- int Play(Engine& player) - Drive engine through the abalone engine protocol (AEP)
