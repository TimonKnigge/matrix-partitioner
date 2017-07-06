#include "./input.h"

#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <utility>
#include <vector>

#include "../matrix/matrix.h"

namespace mp {

matrix error(const std::string &reason) {
	std::cerr << "Error reading matrix: " << reason << '\n';
	return matrix(0, 0, std::vector<std::pair<int, int>>());
}

// Split a string on whitespace.
std::vector<std::string> tokenize(const std::string &line) {
	std::istringstream iss(line);
	return std::vector<std::string>{
		std::istream_iterator<std::string>{iss},
		std::istream_iterator<std::string>{}};
}

matrix read_matrix(std::istream &stream) {
	std::string line;
	
	// First line contains a typecode of the form:
	// %%MatrixMarket matrix [coordinate|array] <...>
	int elements = 1;		// [0, 2] (elements per index).
	bool symmetric = false;	// Whether (i, j) implies (j, i).
	{
		if (!getline(stream, line))
			return error("no typecode found.");

		const auto &tokens = tokenize(line);

		if (tokens.size() != 5)
			return error("first line is not a valid typecode.");
		if (tokens[0] != "%%MatrixMarket")
			return error("first line did not start with a typecode.");
		if (tokens[1] != "matrix")
			return error("file does not describe a matrix.");
		if (tokens[2] != "coordinate")
			return error("matrix is dense.");
		
		if (tokens[3] == "complex")
			elements = 2;
		else if (tokens[3] == "pattern")
			elements = 0;

		if (tokens[4] == "symmetric" || tokens[4] == "hermitian" ||
			tokens[4] == "skew-symmetric")
			symmetric = true;
	}

	// Consume comments (other lines starting with a '%'.
	do { getline(stream, line); } while (line.length() > 0 && line[0] == '%');
	
	// Retrieve matrix information. Should be `r c nz`.
	int r = 0, c = 0, nz = 0;
	{
		const auto &tokens = tokenize(line);
		if (tokens.size() != 3)
			return error("Could not find matrix dimensions/nonzeros.");
		r = stoi(tokens[0]);
		c = stoi(tokens[1]);
		nz = stoi(tokens[2]);
	}

	// Read the nonzeros of the matrix.
	std::vector<std::pair<int, int>> nonzeros;
	{
		nonzeros.reserve(nz);
		for (int i = 0; i < nz; ++i) {
			int j, k;
			if (!(stream >> j >> k))
				return error("Could not read nonzeros.");

			j -= 1;
			k -= 1;

			nonzeros.push_back({j, k});
			if (symmetric) nonzeros.push_back({k, j});

			for (int l = 0; l < elements; ++l) {
				double val;
				if (!(stream >> val))
					return error("Could not read nonzeros.");
			}
		}
	}

	return matrix(r, c, std::move(nonzeros));
}

}
