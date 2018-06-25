#include "./output.h"

#include <iostream>

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

void print_partitioned_compressed_mm(std::ostream &stream, const matrix &m,
		std::unordered_map<int, int> &idm, std::vector<status> &row,
		std::vector<status> &col) {
	stream << "%%MatrixMarket matrix coordinate integer general" << std::endl;
	stream << m.R << ' ' << m.C << ' ' << m.NZ << std::endl;
	for (int r = 0; r < m.R; ++r) {
		const auto &rw = m[r];
		for (const auto &e : rw) {
			int c = e.rc - m.R;
			stream << r+1 << ' ' << c+1 << ' ';
			if (row[idm[r]] == status::red ||
				col[idm[c+m.R]-(int)row.size()] == status::red)
				stream << "1\n";
			else if (row[idm[r]] == status::blue ||
				col[idm[c+m.R]-(int)row.size()] == status::blue)
				stream << "2\n";
			else
				stream << "3\n";
		}
	}
	stream << std::flush;
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

void print_ppmatrix(std::ostream &stream, const partial_partition &pp) {
	stream << "  ";
	for (int c = 0; c < pp.m.C; ++c) {
		status stat = pp.get_status(pp.m.R+c);
		if (stat == status::cut)
			stream << IO_YELLOW_TEXT << 'C';
		else if (stat == status::implicitly_cut)
			stream << IO_YELLOW_TEXT << 'c';
		else if (stat == status::red)
			stream << IO_RED_TEXT << 'R';
		else if (stat == status::partial_red)
			stream << IO_NONE_TEXT << 'r';
		else if (stat == status::blue)
			stream << IO_BLUE_TEXT << 'B';
		else if (stat == status::partial_blue)
			stream << IO_NONE_TEXT << 'b';
		else
			stream << IO_NONE_TEXT << 'U';
	}
	stream << IO_NONE_TEXT << '\n';
	for (int r = 0; r < pp.m.R; ++r) {
		status stat = pp.get_status(r);
		if (stat == status::cut)
			stream << IO_YELLOW_TEXT << 'C';
		else if (stat == status::implicitly_cut)
			stream << IO_YELLOW_TEXT << 'c';
		else if (stat == status::red)
			stream << IO_RED_TEXT << 'R';
		else if (stat == status::partial_red)
			stream << IO_NONE_TEXT << 'r';
		else if (stat == status::blue)
			stream << IO_BLUE_TEXT << 'B';
		else if (stat == status::partial_blue)
			stream << IO_NONE_TEXT << 'b';
		else
			stream << IO_NONE_TEXT << 'U';
		stream << ' ';
		size_t e = 0;
		const auto &rw = pp.m[r];
		for (int c = 0; c < pp.m.C; ++c) {
			if (e < rw.size() && pp.m.R + c == rw[e].rc) {
				++e;
				if (pp.get_status(r) == status::red ||
					pp.get_status(pp.m.R+c) == status::red)
					stream << IO_RED_TEXT;
				else if (pp.get_status(r) == status::blue ||
					pp.get_status(pp.m.R+c) == status::blue)
					stream << IO_BLUE_TEXT;
				else if (pp.get_status(r) == status::cut ||
					pp.get_status(pp.m.R+c) == status::cut)
					stream << IO_YELLOW_TEXT;
				else stream << IO_NONE_TEXT;
				stream << NONZERO;
			} else
				stream << IO_NONE_TEXT << ZERO;
		}
		stream << IO_NONE_TEXT << '\n';
	}
	std::cerr << "cut: " << pp.cut;
	std::cerr << ", impcut: " << pp.implicitly_cut;
	std::cerr << ", vcg: " << pp.vcg.get_minimum_vertex_cut();
	std::cerr << ", lbc: " << pp.lower_bound_cache;
	std::cerr << std::endl;

	using vvi = std::vector<std::vector<int>>;
	vvi rtoc(pp.m.R, std::vector<int>(pp.m.C, 0));
	vvi ctor(pp.m.R, std::vector<int>(pp.m.C, 0));
	for (int i = 0; i < pp.vcg.graph.size(); ++i) {
		auto &es = pp.vcg.graph[i];
		for (int j = 0; j < es.size(); ++j) {
			if (i / 2 == es[j].v / 2) continue;
			if (es[j].cap < 1) continue;
			int fr = i/2, to = es[j].v/2;
			int flo = es[j].flow;
			if (fr < pp.m.R) { rtoc[fr][to-pp.m.R] = flo; }
			else ctor[to][fr-pp.m.R] = flo;
		}
	}

	for (int r = 0; r < pp.m.R; ++r) {
		std::cerr << std::endl;
		for (int c = 0; c < pp.m.C; ++c) {
			std::cerr << ' ' << rtoc[r][c] << '#';
		}
		std::cerr << std::endl;
		for (int c = 0; c < pp.m.C; ++c) {
			std::cerr << ' ' << '#' << ctor[r][c];
		}
		std::cerr << std::endl;
	}
}

}

