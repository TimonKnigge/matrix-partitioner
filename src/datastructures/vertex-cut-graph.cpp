#include "./vertex-cut-graph.h"

#include <algorithm>
#include <queue>

#include "../matrix/matrix.h"
#include "./rvector.h"

namespace mp {

vertex_cut_graph::vertex_cut_graph(const matrix &m) : V(2 * (m.R + m.C)),
		par(V, -1), pari(V, -1) {
	state.assign(m.R + m.C, vertex_state::active);
	graph.resize(V);
	for (int u = 0; u < m.R + m.C; ++u) {
		add_edge(inv(u), outv(u), 1);
	}
	for (int u = 0; u < m.R + m.C; ++u) {
		for (const entry &e : m[u]) {
			add_edge(outv(u), inv(e.rc), 1);
		}
	}
}

//bool vertex_cut_graph::set_activity(int u, vertex_state s) { }
vertex_state vertex_cut_graph::get_activity(int u) const {
	return state[u];
}

int vertex_cut_graph::get_maximum_vertex_cut() const {
	return flow;
}

int vertex_cut_graph::push(int s, std::unordered_map<int, int> &T, int c) {
	std::queue<int> q;
	par.reset_all();
	pari.reset_all();

	// Enqueue s, marking it's parent as -2 to signify it as a source.
	q.push(s);
	par.set(s, -2);

	// Do a BFS to find a feasible sink.
	int t = -1;
	while (!q.empty()) {
		int u = q.front();
		q.pop();

		auto it = T.find(u);
		if (it != T.end() && it->second + c >= 0) {
			t = u;
			break;
		}

		for (int i = 0; i < (int)graph[u].size(); ++i) {
			const internal_edge &e = graph[u][i];
			if (e.flow >= e.cap) continue;
			if (state[e.v / 2] == vertex_state::inactive) continue;
			if (par.get(e.v) != -1) continue;

			par.set(e.v, u);
			pari.set(e.v, i);
			q.push(e.v);
		}
	}

	if (t < 0) return 0;
	
	T[t] += c;
	while (t != s) {
		int p = par.get(t), pi = pari.get(t);
		graph[p][pi].flow++;
		graph[t][graph[p][pi].rev].flow--;
		t = p;
	}

	return c;
}

int vertex_cut_graph::pull(int t, std::unordered_map<int, int> &S, int c) {
	std::queue<int> q;
	par.reset_all();
	pari.reset_all();

	// Enqueue s, marking it's parent as -2 to signify it as a source.
	q.push(t);
	par.set(t, -2);

	// Do a BFS to find a feasible sink.
	int s = -1;
	while (!q.empty()) {
		int u = q.front();
		q.pop();

		auto it = S.find(u);
		if (it != S.end() && it->second + c >= 0) {
			s = u;
			break;
		}

		for (int i = 0; i < (int)graph[u].size(); ++i) {
			// Note distinction between e and re!!!
			const internal_edge &e = graph[u][i];
			const internal_edge &re = graph[e.v][e.rev];
			if (re.flow >= re.cap) continue;
			if (state[e.v / 2] == vertex_state::inactive) continue;
			if (par.get(e.v) != -1) continue;

			par.set(e.v, u);
			pari.set(e.v, i);
			q.push(e.v);
		}
	}

	if (s < 0) return 0;
	
	S[t] += c;
	while (s != t) {
		int p = par.get(s), pi = pari.get(s);
		graph[p][pi].flow--;
		graph[t][graph[p][pi].rev].flow++;
		s = p;
	}

	return c;
}

void vertex_cut_graph::add_edge(int u, int v, int cap) {
	graph[u].emplace_back(v, (int)graph[v].size(), cap);
	graph[v].emplace_back(u, (int)graph[u].size()-1, cap);
}

}
