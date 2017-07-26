#include "./bb-parameters.h"

namespace mp {

std::pair<bool, std::string> bbparameters::valid() const {
	if (pb && epb) return {false, "Can only use one packing bound."};
	if (mb && fb) return {false, "Can only use one flow bound."};
	return {true, ""};
}

}
