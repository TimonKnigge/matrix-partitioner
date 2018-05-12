#ifndef VERTEX_CUT_GRAPH_H
#define VERTEX_CUT_GRAPH_H

#include <set>
#include <unordered_set>
#include <vector>

#include "../matrix/matrix.h"

namespace mp {

// In the vertex cut graph, vertices exist in four states:
enum class vertex_state { inactive, active, source, sink };

// A dynamically maintaned graph for computing minimal vertex cuts between two
// sets of vertices.
class vertex_cut_graph {
public:
	const int V;
	int flow = 0;

	vertex_cut_graph(const matrix &m);

	// Toggle vertex activity.
	bool set_activity(int u, vertex_state s);
	vertex_state get_activity(int u) const;

	// Retrieve maximal vertex cut.
	int get_maximum_vertex_cut() const;

private:
	// Current state of each vertex, in external terms.
	std::vector<vertex_state> state;
	
	// Current sources/sinks (including out/in flow)
	std::set<std::pair<int, int>> sources, sinks;

	// These two vectors maintain the collection of paths.
	// If a vertex u is an internal path vertex, prv[u] and nxt[u] correspond
	// to the indices of the outgoing edges (in the matrix). If it is unused
	// the value is -1, for sources -2 and for sinks -3.
	std::vector<int> prv, nxt;

	// Flow manipulation.
	void push(int s, std::unordered_set<std::pair<int, int>> &T, int mx = -1);
	void pull(int t, std::unordered_set<std::pair<int, int>> &S, int mx = -1);
};

}

#endif
