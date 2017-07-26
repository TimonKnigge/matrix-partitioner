#ifndef OUTPUT_H
#define OUTPUT_H

#include <iostream>
#include <string>

#include "../matrix/matrix.h"
#include "../partitioner/partition-util.h"

namespace mp {

const std::string
	IO_RED_TEXT = "\033[31m",
	IO_BLUE_TEXT = "\033[34m",
	IO_YELLOW_TEXT = "\033[33m",
	IO_NONE_TEXT = "\033[39m",
	IO_RED_BACK = "\033[101m",
	IO_BLUE_BACK = "\033[104m",
	IO_YELLOW_BACK = "\033[103m",
	IO_NONE_BACK = "\033[49m";
const char ZERO = '.', NONZERO = '#';

void print_matrix(std::ostream &stream, const matrix &m);

// Print a partitioned and compressed matrix. Input the *uncompressed* matrix.
void print_partitioned_compressed_matrix(std::ostream &stream, const matrix &m,
	std::unordered_map<int, int> &idm, std::vector<status> &row,
	std::vector<status> &col);

}

#endif
