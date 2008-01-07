#ifndef _CONFIG_H_
#define _CONFIG_H_

/** Defined if we have CppUnit available at compile time */
#define HAVE_CPPUNIT

/** Remember positions already examined. This will give one extra ply in
speedup, because the search tree is pruned quite a lot. */
#define TRANSPOSITION_TABLE

/** If defined, a patricia-trie Transposition Table will be compiled
*/
//#define USE_PATRICIA_TRIE

/** This will prune the tree without evaluating all nodes. It is not a safe
way to prune the search tree, but will considerably speed up things. */
//#define FORWARD_PRUNING 1 /* use last full eval */
#define FORWARD_PRUNING 2 /* use alpha-beta window */

/** If the KO_RULE is enabled, then repeated positions are not allowed
TODO this does not work at present. Analysis need some refactoring
Code cannot compile if this is not defined */
#define KO_RULE

/** This optimization will make the WideDeep search (which is used for the
last 2 ply) avoid going to Analysis() but instead call itself directly.
This avoids many checks, but also minimises the effect of FORWARD_PRUNING
for ply < 9 (and perhaps more) */
//#define FAST_2PLY

/** If defined, the first ply is done with a full search, instead of the
wide-narrow search */
#define FULL_FIRST_PLY

/** This will make the first ply try any move that have been best at one
time, before any other move. Note that this require that you have
FULL_FIRST_PLY */
#define ORDER_FIRST_PLY

/** This search extension will search for push-out moves at the leaf of search.
Search is continued until all push-out moves have been examined. */
//#define PUSH_OUT_EXTENSION

/** This search optimisation stores the best sequence of moves and uses
it as first guess. The guess have lower priority that one from the
transposition table, if available */
//#define PRINCIPAL_VARIATION

/** If not defined, then no logs will be made at all */
#define USE_LOG

/** A minimal set of trace output for every module. Define local to enable
trace from just that module. */
//#define DEB1

// End of config.h
#endif

