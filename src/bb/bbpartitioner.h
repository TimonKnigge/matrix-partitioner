#ifndef BBPARTITIONER_H
#define BBPARTITIONER_H

#include "bbparameters.h"
#include "../matrix/matrix.h"
#include "../partitioner/partitioner.h"
#include "../partitioner/partition-util.h"

namespace mp {

// Branch and bound partitioner.
class bbpartitioner : public partitioner {
  private:
	bbparameters param;

  public:
	bbpartitioner(bbparameters _param) : param(_param) { }

	virtual bool partition(const matrix &m, std::vector<status> &row,
		std::vector<status> &col, float epsilon, std::clock_t clocks);
};

}

#endif
