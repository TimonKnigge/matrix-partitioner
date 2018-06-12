#include "./vertex-cut-graph.h"

#include <algorithm>
#include <queue>

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

bool vertex_cut_graph::is_free(int u) const {
	bool is_active = (get_activity(u) == vertex_state::active);
	int ui = inv(u);
	bool has_flow = (graph[ui][0].flow > 0);
	return is_active && !has_flow;
}

void vertex_cut_graph::set_activity(int u, vertex_state s) {
	if (get_activity(u) == s) return;

	int ui = inv(u), uo = outv(u);

	// In principle we only have switches between 'source/inactive/sink' and
	// 'active'.

	// active => inactive
	if (get_activity(u) == vertex_state::active
			&& s == vertex_state::inactive) {
		// Mark as inactive and remove passthrough edge.
		bool has_flow = graph[ui][0].flow > 0;
		graph[ui][0].flow = graph[ui][0].cap = 0;
		graph[uo][0].flow = graph[uo][0].cap = 0;

		// If there was flow we have to reroute or cancel it.
		if (has_flow) {
			// First try rerouting ...
			std::unordered_map<int, int> T{{uo, 0}};
			if (push(ui, T, 1) == 0) {
				// Rerouting failed, now we push back.
				push(ui, sources, -1);
				pull(uo, sinks, -1);
				flow -= 1;
			}
		}

		// Finally deactivate.
		state[u] = s;
	}
	// inactive => active
	else if (get_activity(u) == vertex_state::inactive
			&& s == vertex_state::active) {
		// Reenable u.
		state[u] = s;
		graph[ui][0].cap = 1;
		
		// Try pulling to ui.
		if (pull(ui, sources, 1)) {
			// Try pushing from uo.
			if (push(uo, sinks, 1)) {
				// Success, now adjust flow.
				graph[ui][0].flow += 1;
				graph[uo][0].flow -= 1;
				flow += 1;
			} else {
				// Failure, reroute ui flow.
				push(ui, sources, -1);
			}
		}
	}
	// active => source
	else if (get_activity(u) == vertex_state::active
			&& s == vertex_state::source) {
		state[u] = s;

		// Just route flow from s while possible.
		int inc = 0;
		while (push(uo, sinks, 1) > 0) ++inc;

		flow += inc;
		sources[uo] = inc;
	}
	// source => active
	else if (get_activity(u) == vertex_state::source
			&& s == vertex_state::active) {
		state[u] = s;

		// First try rerouting as much flow as possible.
		int debit = sources[uo];
		sources.erase(sources.find(uo));
		while (debit > 0 && pull(uo, sources, 1) > 0) --debit;

		// Whatever is left we need to pull back to the sinks.
		while (debit > 0 && pull(uo, sinks, -1) < 0) --debit, --flow;
	}
	// active => sink
	else if (get_activity(u) == vertex_state::active
			&& s == vertex_state::sink) {
		state[u] = s;

		// Just route flow to s while possible.
		int inc = 0;
		while (pull(ui, sources, 1) > 0) ++inc;

		flow += inc;
		sinks[ui] = inc;
	}
	// sink => active
	else if (get_activity(u) == vertex_state::sink
			&& s == vertex_state::active) {
		state[u] = s;

		// First try rerouting as much flow as possible.
		int debit = sinks[ui];
		sinks.erase(sinks.find(ui));
		while (debit > 0 && push(ui, sinks, 1) > 0) --debit;

		// Whatever is left we need to push back to the sources.
		while (debit > 0 && push(ui, sources, -1) < 0) --debit, --flow;
	}
	// else nothing interesting changes in the flow graph.
}

//bool vertex_cut_graph::set_activity(int u, vertex_state s) { }
vertex_state vertex_cut_graph::get_activity(int u) const {
	return state[u];
}

int vertex_cut_graph::get_minimum_vertex_cut() const {
	return flow;
}

int vertex_cut_graph::push(int s, std::unordered_map<int, int> &T, int c) {
	if (T.empty()) return 0;
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
	if (S.empty()) return 0;
	std::queue<int> q;
	par.reset_all();
	pari.reset_all();

	// Enqueue s, marking it's parent as -2 to signify it as a sink.
	q.push(t);
	par.set(t, -2);

	// Do a BFS to find a feasible source.
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

	S[s] += c;
	while (s != t) {
		int p = par.get(s), pi = pari.get(s);
		graph[p][pi].flow--;
		graph[s][graph[p][pi].rev].flow++;
		s = p;
	}

	return c;
}

void vertex_cut_graph::add_edge(int u, int v, int cap) {
	graph[u].emplace_back(v, (int)graph[v].size(), cap);
	graph[v].emplace_back(u, (int)graph[u].size()-1, 0);
}

}
