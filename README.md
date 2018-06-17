# matrix-partitioner
Implementation of the matrix partitioning algorithm described in my thesis.

### Usage

The tool reads the input matrix (in MatrixMarket format) from standard in and
writes the resulting partitioning to standard out. During the partitioning,
debug info is written to standard error. The tool also accepts one or two
parameters in its `argv`s - either just `eps` (the imbalance) or `eps tl` (the
imbalance and a timelimit in seconds). When not given, these parameters are set
to `0.03` and `0` (which means no timelimit).

Example usage:

```Bash
./mp 0.03 60 < mtx/mymatrix.mtx > mtx/mymatrix_partitioned.mtx 2> mymatrix_debug.txt
```
