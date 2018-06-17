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

constexpr float eps_default = 0.03f;
constexpr long long timelimit_default = 60LL;

void parse_arguments(int argc, char** argv, float &eps, long long &timelimit) {
  if (argc > 1) {
    eps = atof(argv[1]);
    std::cerr << "Read eps = " << eps << " from argv" << std::endl;
  }
  if (argc > 2) {
    timelimit = atoll(argv[2]);
    std::cerr << "Read timelimit of " << timelimit << " seconds from argv"
              << std::endl;
  }
}

int main(int argc, char** argv) {
	std::ios::sync_with_stdio(false);
	std::cin.tie(nullptr);

        // Parse the input arguments.
        float eps = eps_default;
        long long timelimit = timelimit_default;
        parse_arguments(argc, argv, eps, timelimit);

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
		mp::print_partitioned_compressed_matrix(std::cerr, mat, idm, rowstat,
			colstat);
	}

	mp::print_partitioned_compressed_mm(std::cout, mat, idm, rowstat, colstat);

	return 0;
}
