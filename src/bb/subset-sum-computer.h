#ifndef SUBSET_SUM_COMPUTER_H
#define SUBSET_SUM_COMPUTER_H

#include <vector>

namespace mp {

constexpr long long OPERATIONS_BOUND = 1000000;

// Solve the partition problem using a meet-in-the-middle strategy.
bool partition_mim(const std::vector<int> &a, int s1max, int s2max,
		std::vector<bool> &part);

// Solve the partition problem using a pseudo-polynomial dp strategy.
bool partition_dp(const std::vector<int> &a, int s1max, int s2max,
		std::vector<bool> &part);

// Partition a into sets of sizes at most s1max and s2max if possible.
bool partition(const std::vector<int> &a, int s1max, int s2max,
		std::vector<bool> &part);

}

#endif
