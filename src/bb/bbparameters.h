#ifndef BBPARAMETERS_H
#define BBPARAMETERS_H

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

	explicit bbparameters(bool _pb, bool _epb, bool _mb, bool _fb)
		: pb(_pb), epb(_epb), mb(_mb), fb(_fb) { }

	// Returns whether or not the configuration is valid, and if not, a string
	// describing the problem.
	std::pair<bool, string> valid() const;
};

}

#endif
