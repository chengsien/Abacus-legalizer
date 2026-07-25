# Abacus Standard-Cell Legalizer

A C++ implementation of the **Abacus** legalization algorithm for VLSI placement.
Given a global placement result, the tool removes cell overlaps and snaps every
standard cell onto a legal, site-aligned position while minimizing total cell
displacement and preserving the relative ordering from global placement.

Course project for *Nano IC Physical Design*, National Cheng Kung University.

## Results

Evaluated on the ICCAD 2013 contest benchmark suite:

| Benchmark   | Cells   | Nets    | Legality | HPWL Degradation | Runtime |
|-------------|---------|---------|----------|------------------|---------|
| superblue1  | 847,441 | 822,744 | Pass     | 5%               | 80.9 s  |
| superblue5  | 772,457 | 786,999 | Pass     | 7%               | 108.7 s |
| superblue19 | 522,775 | 511,685 | Pass     | 8%               | 251.1 s |

All three results pass the official `iccad2013_check_legality` checker with
zero overlaps and full site-grid alignment. Runtimes measured on a single
CPU core, covering parsing, legalization, and output writing.

## Algorithm

The implementation follows the Abacus formulation, with two additions for
handling real benchmark constraints:

**1. Row splitting for fixed obstacles**
Each placement row is partitioned into usable sub-row segments around fixed
macros, so cells are never placed into blocked regions.

**2. Row-based quadratic placement**
Cells are sorted by x-coordinate and processed left to right. Within each row,
positions are determined by minimizing total quadratic displacement from the
global-placement positions.

**3. Recursive cluster merging**
Abutting cells are merged into clusters and collapsed recursively when overlaps
occur, which preserves the relative cell ordering produced by global placement.

**4. Lowest-cost row search**
For every cell, candidate rows are searched dynamically and the row with the
lowest displacement cost is selected.

## Input Format

Follows the **Bookshelf** format:

| File | Description |
|------|-------------|
| `.aux` | Index file pointing to the files below |
| `.nodes` | Cell dimensions and types |
| `.nets` | Netlist connectivity |
| `.gp.pl` | Global placement result (input positions) |
| `.scl` | Row structure definition |

## Output

`legal/<benchmark>.legal.pl` — the legalized placement in Bookshelf `.pl` format.

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

Legality and wirelength are verified with the official ICCAD 2013 contest tools:

```bash
./iccad2013_check_legality superblue1.aux superblue1.legal.pl
```

```bash
./iccad2013_get_hpwl superblue1.aux superblue1.legal.pl
```

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
