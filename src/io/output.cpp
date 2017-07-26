#include "./output.h"

#include <iostream>

#include "../matrix/matrix.h"

namespace mp {

void print_matrix(std::ostream &stream, const matrix &m) {
	for (int r = 0; r < m.R; ++r) {
		const auto &row = m[r];
		size_t j = 0;
		for (int i = 0; i < m.C; ++i) {
			if (j < row.size() && row[j].rc == m.R+i) {
				stream << '#';
				++j;
			} else	stream << '.';
		}
		stream << '\n';
	}
}

void print_partitioned_compressed_matrix(std::ostream &stream, const matrix &m,
		std::unordered_map<int, int> &idm, std::vector<status> &row,
		std::vector<status> &col) {
	stream << "  ";
	for (int c = 0; c < m.C; ++c) {
		auto it = idm.find(m.R+c);
		if (it == idm.end())
			stream << IO_NONE_TEXT << '-';
		else if (col[it->second-(int)row.size()] == status::cut)
			stream << IO_YELLOW_TEXT << 'C';
		else if (col[it->second-(int)row.size()] == status::red)
			stream << IO_RED_TEXT << 'R';
		else
			stream << IO_BLUE_TEXT << 'B';
	}
	stream << IO_NONE_TEXT << '\n';
	for (int r = 0; r < m.R; ++r) {
		auto it = idm.find(r);
		if (it == idm.end()) {
			stream << IO_NONE_TEXT << "- "
				<< std::string(m.C, ZERO) << '\n';
			continue;
		}
		else if (row[it->second] == status::cut)
			stream << IO_YELLOW_TEXT << "C ";
		else if (row[it->second] == status::red)
			stream << IO_RED_TEXT << "R ";
		else
			stream << IO_BLUE_TEXT << "B ";
		size_t e = 0;
		const auto &rw = m[r];
		for (int c = 0; c < m.C; ++c) {
			if (e < rw.size() && m.R + c == rw[e].rc) {
				++e;
				if (row[it->second] == status::red ||
					col[idm[c+m.R]-(int)row.size()] == status::red)
					stream << IO_RED_TEXT;
				else if (row[it->second] == status::blue ||
					col[idm[c+m.R]-(int)row.size()] == status::blue)
					stream << IO_BLUE_TEXT;
				else
					stream << IO_YELLOW_TEXT;
				stream << NONZERO;
			} else
				stream << IO_NONE_TEXT << ZERO;
		}
		stream << IO_NONE_TEXT << '\n';
	}
}

}
