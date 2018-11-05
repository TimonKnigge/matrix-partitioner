#include <algorithm>
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
constexpr long long timelimit_default = 0LL;
constexpr char help_text[] = "\
 MP - Matrix Partitioner\n\n\
 Usage:\
\t./mp [-e eps] [-t tl] <input >output 2>debug\n\n\
 The program reads a matrix in MatrixMarket format\n\
 from stdin and writes the solution to stdout. Debug\n\
 is written to stderr.\n\n\
 Flags:\n\
\t-e eps\tMaximum tolerated load imbalance.\n\
\t\tDefaults to 0.03.\n\
\t-t tl\tTimelimit, in seconds. Defaults to\n\
\t\t0 for no limit.";

// Very simple argument parser. Deals with errors
// by ignoring them.
struct cmd_args {
	std::vector<std::string> args;
	cmd_args(int argc, char** argv) {
		for (int i = 0; i < argc; ++i)
			args.push_back(argv[i]);
	}
	bool flag(const std::string &f) {
		return std::find(args.begin(), args.end(), f) != args.end();
	}
	float get_float(const std::string &f, float def) {
		auto it = std::find(args.begin(), args.end(), f);
		if (it == args.end() || (++it) == args.end())
			return def;
		else
			return stof(*it);
	}
	long long get_ll(const std::string &f, long long def) {
		auto it = std::find(args.begin(), args.end(), f);
		if (it == args.end() || (++it) == args.end())
			return def;
		else
			return stoll(*it);
	}
};

int main(int argc, char** argv) {
	// Detach from C I\O.
	std::ios::sync_with_stdio(false);
	std::cin.tie(nullptr);

	// Parse the input arguments.
	cmd_args args(argc, argv);
	if (args.flag("-h")) {
		std::cerr << help_text << std::endl;
		return 0;
	}
	float eps = args.get_float("-e", eps_default);
	long long timelimit = args.get_ll("-t", timelimit_default);
	std::cerr << "Running with eps=" << eps << ", and TL="
		<< timelimit << std::endl;

	// Read matrix.
	mp::matrix mat = mp::read_matrix(std::cin);

	// Compress.
	std::unordered_map<int, int> idm;
	mp::matrix cmat = mp::compress(mat, idm);

	std::cerr << "Read " << cmat.R << 'x' << cmat.C << " matrix with "
		<< cmat.NZ << " nonzeros (after compression)" << std::endl;
	std::cerr << "Attempting partitioning with eps=" << eps << " in ";
	std::cerr << timelimit << " seconds." << std::endl;

	mp::bbpartitioner bb(mp::bbparameters{
		true,		// packing bound
		true,		// extended packing bound
		false,		// matching bound
		true,		// flow bound
		1,			// initial upperbound
		1.25f		// scaling factor
	});
	std::vector<mp::status> rowstat, colstat;
	if (bb.partition(cmat, rowstat, colstat, eps, timelimit)) {
		std::cerr << "Partitioning succesful, printing to stdout now." << std::endl;
		mp::print_partitioned_compressed_mm(std::cout, mat, idm, rowstat, colstat);
	} else {
		std::cerr << "Partitioning unsuccesful within timelimit." << std::endl;
		if (!rowstat.empty() && rowstat[0] != mp::status::unassigned) {
			std::cerr << "Got partitioning anyway, printing." << std::endl;
			mp::print_partitioned_compressed_mm(std::cout, mat, idm, rowstat, colstat);
		}
	}

	return 0;
}
