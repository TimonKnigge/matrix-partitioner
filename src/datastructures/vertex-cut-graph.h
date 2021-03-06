#ifndef VERTEX_CUT_GRAPH_H
#define VERTEX_CUT_GRAPH_H

#include <unordered_map>
#include <vector>

#include "./matrix.h"
#include "./rvector.h"

namespace mp {

// In the vertex cut graph, vertices exist in four states:
enum class vertex_state { inactive, active, source, sink };

struct internal_edge {
	const int v;		// Index of the endpoint.
	const int rev;		// Residual edge index.
	int flow = 0, cap;	// Edge flow and capacity.
	const int crossid;	// Id of the crossing edge, if it exists.
	internal_edge(int v, int rev, int cap, int crossid = -1) :
		v(v), rev(rev), cap(cap), crossid(crossid) { }
};

inline int inv(int i) { return 2*i; }
inline int outv(int i) { return 2*i+1; }
inline int lift(int u) { return u/2; }

// A dynamically maintaned graph for computing minimal vertex cuts between two
// sets of vertices.
class vertex_cut_graph {
public:
	const int V;
	int flow = 0, magnitude = 0;

	vertex_cut_graph(const matrix &m);

	// Toggle vertex activity.
	void set_activity(int u, vertex_state s);
	vertex_state get_activity(int u) const;

	// Retrieve maximal vertex cut.
	int get_minimum_vertex_cut() const;

	// Check if vertex u is 'free', that is, if it is active and
	// not part of a flow path.
	bool is_free(int u) const;

private:
	// Current state of each vertex, in external terms.
	std::vector<vertex_state> state;

	// The graph.
	std::vector<std::vector<internal_edge>> graph;
	
	// Current sources/sinks (including out/in flow)
	std::unordered_map<int, int> sources, sinks;

	// Internally used rvectors (for finding augmenting paths).
	// Give parent vertex and parent edge (index).
	mp::rvector<int> par, pari;

	// Flow manipulation. The coefficient c (1/-1) signals whether we are
	// adding or removing flow. Pushes/pulls a single augmenting path.
	int push(int s, std::unordered_map<int, int> &T, int c);
	int pull(int t, std::unordered_map<int, int> &S, int c);

	// Add an edge - internal indexation! If this is an external edge then
	// cid should be the index of the crossing/reverse edge.
	void add_edge(int u, int v, int cap, int cid = -1);

	// Adjust the flow through an edge. Uses internal ids.
	void adjust_flow(int u, int ei, int c);
};

}

#endif
