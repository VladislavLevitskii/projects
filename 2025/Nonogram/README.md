# Nonogram - project

## Problem Description - Nonogram

Nonogram is a type of logic puzzle consisting of a rectangular grid of cells that must be filled or left blank according to numerical clues provided for each row and column.

Each number in the clues represents a contiguous block of filled cells. Multiple numbers mean that there are multiple such blocks separated by at least one empty cell.
The goal is to fill the grid so that all the clues are satisfied, revealing a hidden picture.

Example of the solved nonogram:

![nonogram](nonogram-example.png)

## Encoding

The matrix contains $n^2$ variables which are placed in the grid of the nonogram.

For every row and column, there is at least one valid combination of blocks (not necessarily consistent with the full solution yet).

The intermediate result is a list of all possible combinations for each line.

The next step is to define new variables for each possible combination and there are these implications are to be included in CNF:

variant (combination): $v$ $\rightarrow cell$

which is: $\neg v \vee cell$ 

Then one big clause ensures that at least one variant in line is valid:

$[v_1, v_2, \ldots]$

Finally, clauses for every pair of these variants are added:

$[\neg v_i, \neg v_j]$

This enforces that exactly one variant is valid for each row or column.

And this is done for every column and row.

## Usage

Basic usage 
```
./solver.py [-v] [-h] [-t TEST]
```
example:
```
./solver.py -v --test tests/cross.txt
```

## Command-line options

- **-h, --help** : Show a help message and exit.  
- **-t TEST, --test TEST** : Run the solver in test mode using the specified test file.
- **-v, --verbose** : Adds additional information.

- **without flags** : interactive mode 

## Tests

Tests are located in folder `tests`.

Feel free to create your own tests!

There is a typical structure (from the picture above):
```
5 // size
1 1    \ 
1      |
1 1 1  | 5 columns (top to bottom)
4      |
2      /
1 2    \
1      |
1 2    | 5 rows (left to right)
2      |
2 1    /
```

### Additional information

That is not recommended to use nonograms that are bigger that 20 * 20, because even that relatively small nonogram has around 600.000 clauses and solution time is around units of seconds 

Obviously it depends on how much gaps there are and how big they are.
