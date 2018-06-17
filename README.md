# matrix-partitioner
Implementation of the matrix partitioning algorithm described in my thesis.

### Usage

The tool reads the input matrix (in MatrixMarket format) from standard in and
writes the resulting partitioning to standard out. During the partitioning,
debug info is written to standard error. Reading from and writing to files can
easily be achieved by piping:

```Bash
./mp < mtx/mymatrix.mtx > mtx/mymatrix_partitioned.mtx 2> mymatrix_debug.txt
```
