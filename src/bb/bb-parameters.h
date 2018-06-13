#ifndef BBPARAMETERS_H
#define BBPARAMETERS_H

#include <string>
#include <utility>

namespace mp {

struct bbparameters {
	const bool
		// Whether or not to use the packing bound.
		pb,
		// Whether or not to use the extended packing bound.
		epb,
		// Whether or not to use the matching bound.
		mb,
		// Whether or not to use the flow bound.
		fb;

	// The initial upperbound to try, and the incremental scaling factor.
	const int U0;
	const float Uf;

	bbparameters(bool _pb, bool _epb, bool _mb, bool _fb, int _U0, float _Uf)
		: pb(_pb), epb(_epb), mb(_mb), fb(_fb), U0(_U0), Uf(_Uf) { }

	// Returns whether or not the configuration is valid, and if not, a string
	// describing the problem.
	std::pair<bool, std::string> valid() const;
};

}

#endif
