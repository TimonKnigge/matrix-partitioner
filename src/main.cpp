#include <iostream>
#include <unordered_map>
#include <vector>

#include "bb/bb-parameters.h"
#include "bb/bb-partitioner.h"
#include "io/input.h"
#include "io/output.h"
#include "datastructures/matrix.h"
#include "datastructures/matrix-util.h"
#include "partitioner/partition-util.h"

constexpr float eps = 0.03f;

int main() {
	std::ios::sync_with_stdio(false);
	std::cin.tie(nullptr);

	// Read matrix.
	mp::matrix mat = mp::read_matrix(std::cin);

	// Compress.
	std::unordered_map<int, int> idm;
	mp::matrix cmat = mp::compress(mat, idm);

	std::cerr << "Read " << cmat.R << 'x' << cmat.C << " matrix with "
		<< cmat.NZ << " nonzeros (after compression)" << std::endl;
	std::cerr << "Attempting partitioning with eps=" << eps << std::endl;

	mp::bbpartitioner bb(mp::bbparameters{
		true,		// packing bound
		true,		// extended packing bound
		false,		// matching bound
		true,		// flow bound
		1,			// initial upperbound
		1.25f		// scaling factor
	});
	std::vector<mp::status> rowstat, colstat;
	if (!bb.partition(cmat, rowstat, colstat, eps)) {
		std::cerr << "Partitioning failed." << std::endl;
		return 0;
	}

	// Print if small enough.
	if (cmat.R <= 60 && cmat.C <= 60) {
		print_partitioned_compressed_matrix(std::cerr, mat, idm, rowstat,
			colstat);
	}

	print_partitioned_compressed_mm(std::cout, mat, idm, rowstat, colstat);

	return 0;
}
