# Abacus Standard-Cell Legalizer

A C++ implementation of the **Abacus** legalization algorithm [1] for VLSI
placement. Given a global placement result, the tool removes cell overlaps and
snaps every standard cell onto a legal, site-aligned position while minimizing
total cell displacement and preserving the relative ordering from global
placement.

Course project for *Physical Design for Nanometer IC*, National Cheng Kung
University.

## Results

Evaluated on three ICCAD 2013 contest benchmarks:

| Benchmark   | Cells   | Nets    | Legality | Baseline HPWL | HPWL Degradation | Runtime |
|-------------|---------|---------|----------|---------------|------------------|---------|
| superblue1  | 847,441 | 822,744 | Pass     | 251,275,483   | 5%               | 80.9 s  |
| superblue5  | 772,457 | 786,999 | Pass     | 321,001,759   | 7%               | 108.7 s |
| superblue19 | 522,775 | 511,685 | Pass     | 135,375,236   | 8%               | 251.1 s |

Baseline HPWL is measured on the global placement input. Runtimes are measured
on a single CPU core and cover parsing, legalization, and output writing.

Both hard constraints of the assignment are satisfied on every benchmark:

| Constraint | Requirement | Worst observed |
|------------|-------------|----------------|
| HPWL degradation | ≤ 20% | 8% |
| Runtime | ≤ 5 minutes | 4 min 11 s |

## Algorithm

The implementation follows the Abacus formulation, with additional handling for
the fixed-obstacle and non-rectangular macro features present in these
benchmarks:

**1. Row splitting for fixed obstacles**
Each placement row is partitioned into usable sub-row segments around fixed
nodes, so movable cells are never placed into blocked regions. Blocked
intervals are collected per row, merged, and the remaining gaps become
site-aligned sub-rows.

Non-rectangular fixed nodes are blocked using their individual component shapes
from the `.shapes` file rather than their enclosing rectangle, which recovers
placeable area that a bounding-box approximation would discard.

**2. Row-based quadratic placement**
Cells are sorted by x-coordinate and processed left to right. Within each row,
positions are determined by minimizing total quadratic displacement from the
global-placement positions.

**3. Recursive cluster merging**
Abutting cells are merged into clusters and collapsed recursively when overlaps
occur, which preserves the relative cell ordering produced by global placement.

**4. Lowest-cost row search**
For every cell, candidate sub-rows within a window around its original row are
trial-placed, and the one with the lowest total displacement cost — counting
both the cell itself and the cells it displaces — is selected. The search
window is narrowed on larger designs to bound runtime.

## Limitations

`terminal_NI` nodes are currently treated as blocking obstacles, the same as
regular `terminal` nodes. The benchmark specification allows standard cells to
be placed beneath `terminal_NI` nodes without an overlap violation, so
excluding them from the blocked-interval set would recover additional placeable
area and reduce row fragmentation. Results remain legal either way; this is a
conservative choice, not a correctness issue.

## Input Format

Follows the **Bookshelf** format:

| File | Description |
|------|-------------|
| `.aux` | Index file listing the files below |
| `.nodes` | Node dimensions and move types (`movable` / `terminal` / `terminal_NI`) |
| `.nets` | Netlist connectivity and pin offsets |
| `.gp.pl` | Global placement result — the input positions to legalize |
| `.scl` | Circuit row structure (coordinate, height, site spacing, subrow origin) |
| `.shapes` | Component shapes of non-rectangular fixed nodes |

Note that the legalizer reads `.gp.pl`, not `.pl`. The `.pl` file contains
undefined coordinates for movable nodes.

## Output

`legal/<benchmark>.legal.pl` — the legalized placement, in the same Bookshelf
`.pl` format as the input.

## Build

```bash
make
```

## Run

```bash
./HW4_M16144122 <path-to-aux-file>
```

Example:

```bash
./HW4_M16144122 ../benchmarks/superblue1/superblue1.aux
```

Execution time is reported on completion.

## Validation

Legality and wirelength are verified with the official ICCAD 2013 contest
scripts:

```bash
./iccad2013_check_legality superblue1.aux superblue1.legal.pl
```

```bash
./iccad2013_get_hpwl superblue1.aux superblue1.legal.pl
```

The legality checker verifies five conditions: that no fixed node has moved,
that every movable node lies inside the placement area, that movable nodes are
row-aligned, that they sit on a multiple of the site spacing, and that no
overlaps exist among nodes.

## Benchmarks

The ICCAD 2013 contest benchmarks are not included in this repository due to
file size (`superblue1.nets` alone is 195 MB, above GitHub's 100 MB per-file
limit).

Place the benchmark directories as follows before running:

```
benchmarks/
├── superblue1/
├── superblue5/
└── superblue19/
```

## Project Structure

```
Abacus-legalizer/
├── README.md
├── .gitignore
└── HW4_M16144122/
    ├── Makefile
    └── src/
        ├── main.cpp              Entry point, file loading, timing
        ├── Parser.cpp / parser.h Bookshelf format parser
        ├── datatype.cpp / .h     Data structures for modules, rows, nets
        └── algo.cpp / algo.h     Abacus legalization core
```

## Reference

[1] P. Spindler, U. Schlichtmann, and F. M. Johannes, "Abacus: Fast
Legalization of Standard Cell Circuits with Minimal Movement," *ISPD*,
pp. 47–53, 2008.

[2] M.-C. Kim, N. Viswanathan, Z. Li, and C. J. Alpert, "ICCAD-2013 CAD Contest in Placement Finishing and Benchmark Suite," ICCAD, pp. 268–270, 2013.
https://ieeexplore.ieee.org/document/6691130
