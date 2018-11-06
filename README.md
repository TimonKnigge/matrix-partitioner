# matrix-partitioner
Implementation of the matrix partitioning algorithm described in my thesis.

### Usage

The tool reads an input matrix from standard in, in 
[MatrixMarket format](https://math.nist.gov/MatrixMarket/formats.html)
(coordinate, specifically, as this tool is intended for sparse matrices),
and writes the resulting partitioning to standard out, also in MatrixMarket
format.
During the partitioning, debug information is written to standard error.

```Bash
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
