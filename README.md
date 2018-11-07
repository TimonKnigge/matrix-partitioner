# matrix-partitioner
Implementation of the matrix partitioning algorithm described
[our paper](https://arxiv.org/abs/1811.02043). A database of optimally
partitioned matrices may be found at
[this location](http://www.staff.science.uu.nl/~bisse101/Mondriaan/Opt/).

### Usage

The tool reads an input matrix from standard in, in 
[MatrixMarket format](https://math.nist.gov/MatrixMarket/formats.html)
(coordinate, specifically, as this tool is intended for sparse matrices),
and writes the resulting partitioning to standard out, also in MatrixMarket
format.
During the partitioning, debug information is written to standard error.

```
timon@timon-laptop ~/mp $ ./mp -h
 MP - Matrix Partitioner

 Usage:	./mp [-e eps] [-t tl] <input >output 2>debug

 The program reads a matrix in MatrixMarket format
 from stdin and writes the solution to stdout. Debug
 is written to stderr.

 Flags:
	-e eps	Maximum tolerated load imbalance.
		Defaults to 0.03.
	-t tl	Timelimit, in seconds. Defaults to
		0 for no limit.
```

Example usage:

```Bash
./mp -e 0.03 -t 60 < mtx/mymatrix.mtx > mtx/mymatrix_partitioned.mtx 2> mymatrix_debug.txt
```

The output matrix is in integer coordinate format, with a `1` (resp. `2`)
indicating the nonzero goes to the first (resp. second) side of the
bipartitioning, and a `3` indicating the nonzero can be assigned arbitrarily
without violating the load balancing constraint.
