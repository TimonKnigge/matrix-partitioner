#ifndef OUTPUT_H
#define OUTPUT_H

#include <iostream>
#include <string>

#include "../bb/partial-partition.h"
#include "../datastructures/matrix.h"
#include "../partitioner/partition-util.h"

namespace mp {

const std::string
	IO_RED_TEXT = "\033[31m",
	IO_BLUE_TEXT = "\033[34m",
	IO_YELLOW_TEXT = "\033[93m",
	IO_NONE_TEXT = "\033[39m";
const char ZERO = '.', NONZERO = '#';

// Print a matrix as a grid.
void print_matrix(std::ostream &stream, const matrix &m);

// Print a partitioned and compressed matrix in MM format.
void print_partitioned_compressed_mm(std::ostream &stream, const matrix &m,
	std::unordered_map<int, int> &idm, std::vector<status> &row,
	std::vector<status> &col);

// Print a partitioned and compressed matrix. Input the *uncompressed* matrix.
void print_partitioned_compressed_matrix(std::ostream &stream, const matrix &m,
	std::unordered_map<int, int> &idm, std::vector<status> &row,
	std::vector<status> &col);

// Print a ppmatrix for debugging.
void print_ppmatrix(std::ostream &stream, const partial_partition &pp);

}

#endif
