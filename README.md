# chopchop
Solver for my local variation of the Chopsticks finger game

## Usage
`./game` : solves every position, giving an evaluation in [-100.0f, +100.0f] and the best move.

`./game a b c d` : gives information about position `a b c d` (children, parents and all evaluations and best moves). Returns nothing if `a b c d` is not a legal (reachable) position.

## Notation
A position is labeled `abcd`. `a` and `b` are the values for both hands of the player to move next, whereas `c` and `d` are the values for the player who just moved. A dead hand has a value of 0. Thus, if player 1 attacks `1x1` in the position `1111`, it becomes `2111`, with the player `2 1` to move. Note that `a` and `b` are always in decreasing order, just as `c` and `d` are.

## Compilation
`make release` : Compiles with moderate optimization

`make debug` : Compiles with no optimization and debug flags
