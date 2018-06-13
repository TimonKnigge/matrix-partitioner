#include "./bb-parameters.h"

namespace mp {

std::pair<bool, std::string> bbparameters::valid() const {
	if (mb) return {false, "Matching bound not implemented."};
	if (mb && fb) return {false, "Can only use one flow bound."};
	if (epb && !fb) return {false, "Extended packing bound must be "
		"used in combination with the flow bound."};
	if (Uf <= 1.0f) return {false, "Scaling factor must be larger than 1."};
	if (U0 <= 0) return {false, "Initial upperbound must be positive."};
	return {true, ""};
}

}
