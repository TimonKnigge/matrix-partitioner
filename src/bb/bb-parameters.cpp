#include "./bb-parameters.h"

namespace mp {

std::pair<bool, std::string> bbparameters::valid() const {
	if (mb) return {false, "Matching bound not implemented."};
	if (mb && fb) return {false, "Can only use one flow bound."};
	if (epb && !fb) return {false, "Extended packing bound must be "
		"used in combination with the flow bound."};
	return {true, ""};
}

}
